#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>    // Библиотека кольцевого буфера
#include <linux/slab.h>    // Для kmalloc/kfree
#include <linux/mutex.h>    // Заголовок для мьютексов
#include <linux/wait.h>     // Для очередей ожидания
#include <linux/device.h>
#include <linux/cdev.h>     // Добавлен для cdev_init/cdev_add
#include <linux/version.h>  // Обязательно для макроса KERNEL_VERSION

#define DEVICE_NAME "bn_device"
#define CLASS_NAME "bn_class"
#define FIFO_SIZE 1024      // Размер должен быть степенью двойки (2, 4, 8...)

static dev_t dev_num;          // Хранит major и minor
static struct cdev my_cdev;    // Объект символьного устройства
static struct class *cls;
static struct kfifo my_fifo;
static struct device *dev_ptr;

static int buffer_size = 1024;
static char *ring_buffer = NULL;
static int head = 0;                            // Указатель записи
static int tail = 0;                            // Указатель чтения
static DEFINE_MUTEX(fifo_lock);                 // Статическая инициализация мьютекса
static DECLARE_WAIT_QUEUE_HEAD(read_queue);     // Инициализация очереди ожидания
static DECLARE_WAIT_QUEUE_HEAD(write_queue);    // Новая очередь для писателей

// Функция-сеттер, которая вызывается при записи в sysfs
static int set_buffer_size(const char *val, const struct kernel_param *kp) {
    int new_size;
    int ret;
    char *new_ptr;

    // Преобразуем строку из sysfs в число
    ret = kstrtoint(val, 10, &new_size);
    if (ret < 0 || new_size <= 0) return -EINVAL;

    // Захватываем мьютекс, чтобы никто не читал/писал в это время
    if (mutex_lock_interruptible(&fifo_lock)) return -ERESTARTSYS;

    printk(KERN_INFO "bn_device: Пересоздание буфера: %d -> %d байт\n", buffer_size, new_size);

    // Выделяем новую память
    new_ptr = kmalloc(new_size, GFP_KERNEL);
    if (!new_ptr) {
        mutex_unlock(&fifo_lock);
        return -ENOMEM;
    }

    // Освобождаем старый буфер и обновляем указатели
    kfree(ring_buffer);
    ring_buffer = new_ptr;
    buffer_size = new_size;
    head = 0;               // СБРОС: начинаем с чистого листа
    tail = 0;
    
    mutex_unlock(&fifo_lock);
    return 0;
}

// Стандартный геттер (просто читает значение)
static const struct kernel_param_ops buffer_size_ops = {
    .set = set_buffer_size,
    .get = param_get_int,
};

// Регистрация параметра с использованием callback
module_param_cb(buffer_size, &buffer_size_ops, &buffer_size, 0644);

// Функция записи (store) для сброса FIFO
static ssize_t clear_fifo_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    int val;

    // Преобразуем ввод в число
    if (kstrtoint(buf, 10, &val) < 0)
        return -EINVAL;

    if (val != 0) {
        // Защищаем операцию мьютексом
        mutex_lock(&fifo_lock);
        kfifo_reset(&my_fifo);
        mutex_unlock(&fifo_lock);

        // Будим писателей, так как место в буфере гарантированно появилось
        wake_up_interruptible(&write_queue);

        pr_info("FIFO buffer cleared via sysfs\n");
    }

    return count;
}

static ssize_t current_len_show(struct device *dev, struct device_attribute *attr, char *buf) {
    return sysfs_emit(buf, "%u\n", kfifo_len(&my_fifo));
}

// Создаем атрибут (только запись root — WO)
// Вместо DEVICE_ATTR_WO(clear_fifo);
// Используем __ATTR(имя, права, show, store)
// Права 0664 позволяют писать всем (User/Group/Others)
static struct device_attribute dev_attr_clear_fifo = 
    __ATTR(clear_fifo, 0664, NULL, clear_fifo_store);
// Создаем атрибут с правами 0644 (чтение для root)
static DEVICE_ATTR_RO(current_len);

static struct attribute *bn_attrs[] = {
    &dev_attr_clear_fifo.attr,
    &dev_attr_current_len.attr,
    NULL,
};

static const struct attribute_group bn_group = {
    .attrs = bn_attrs,
};

// Вызывается при чтении: cat /dev/my_device
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
     int ret;
    unsigned int copied;

    // Проверяем, пусто ли в FIFO
    if (kfifo_is_empty(&my_fifo)) {
        // Если флаг O_NONBLOCK установлен, выходим с ошибкой "Ресурс временно недоступен"
        if (filep->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }

        // Ждем, пока в FIFO появятся данные (kfifo_is_empty == false)
        // wait_event_interruptible вернет 0, когда условие станет истинным
        if (wait_event_interruptible(read_queue, !kfifo_is_empty(&my_fifo))) {
            return -ERESTARTSYS;    // Прервано сигналом
        }
    }

    // Захватываем мьютекс перед доступом к FIFO
    if (mutex_lock_interruptible(&fifo_lock)) {
        return -ERESTARTSYS;    // Прерывание, если процесс получил сигнал во время ожидания
    }

    // Извлекаем данные из FIFO и копируем в user-space
    ret = kfifo_to_user(&my_fifo, buffer, len, &copied);
    mutex_unlock(&fifo_lock);   // Освобождаем мьютекс

    // Раз что-то прочитали, значит появилось место для записи — будим писателей
    if (copied > 0) {
        wake_up_interruptible(&write_queue);
    }

    return ret ? ret : copied;  // Возвращаем количество реально прочитанных байт
}

// Вызывается при записи: echo "hello" > /dev/my_device
static ssize_t dev_write(struct file *filep, const char __user *buffer, size_t len, loff_t *offset) {
    int ret;
    unsigned int copied;

    // Если буфер полон
    if (kfifo_is_full(&my_fifo)) {
        // И установлен флаг O_NONBLOCK — выходим не дожидаясь
        if (filep->f_flags & O_NONBLOCK) {
            return -EAGAIN;
        }

        if (wait_event_interruptible(write_queue, !kfifo_is_full(&my_fifo)))
            return -ERESTARTSYS;
    }

    // Захватываем мьютекс перед записью
    if (mutex_lock_interruptible(&fifo_lock)) {
        return -ERESTARTSYS;
    }

    // Копируем из user-space и кладем в FIFO
    ret = kfifo_from_user(&my_fifo, buffer, len, &copied);
    mutex_unlock(&fifo_lock);   // Освобождаем мьютекс

    // Будим все процессы, ожидающие в очереди read_queue
    if (copied > 0) {
        wake_up_interruptible(&read_queue);
    }

    return ret ? ret : copied;  // Если буфер полон, вернет меньше, чем len
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
    .write = dev_write,
};

static int __init my_module_init(void) {
    // Инициализация FIFO
    int ret;

    // Динамическая инициализация мьютекса (если не использовали DEFINE_MUTEX)
    // mutex_init(&fifo_lock);
    
    // 1. Выделяем диапазон номеров устройств (динамически)
    ret = kfifo_alloc(&my_fifo, FIFO_SIZE, GFP_KERNEL);
    if (ret) return ret;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) goto err_fifo;

    // 2. Инициализируем cdev и связываем с fops
    cdev_init(&my_cdev, &fops);
    if (cdev_add(&my_cdev, dev_num, 1) < 0) goto err_region;

    // Создаем файл в sysfs
    // Регистрация класса (в новых ядрах 6.4+ макрос class_create изменился, используем совместимый вариант)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
    cls = class_create(CLASS_NAME);
#else
    cls = class_create(THIS_MODULE, CLASS_NAME);
#endif

    if (IS_ERR(cls)) {
        ret = PTR_ERR(cls);
        goto err_cdev;
    }

    // DEVICE
    dev_ptr = device_create(cls, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_ptr)) {
        ret = PTR_ERR(dev_ptr);
        pr_err("bn_device: Failed to create device!\n");
        goto err_class;
    }

    // FILES (Явное создание)
    ret = device_create_file(dev_ptr, &dev_attr_clear_fifo);
    if (ret) pr_err("bn_device: Failed to create sysfs file\n");

    printk(KERN_INFO "FIFO модуль загружен. Major: %d, Minor: %d\n", MAJOR(dev_num), MINOR(dev_num));
    return 0;

err_class: class_destroy(cls);
err_cdev: cdev_del(&my_cdev);
err_region: unregister_chrdev_region(dev_num, 1);
err_fifo: kfifo_free(&my_fifo);
    return ret;
}

static void __exit my_module_exit(void) {
    device_destroy(cls, dev_num);           // Удаляем устройство
    class_destroy(cls);                     // Удаляем класс
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    kfifo_free(&my_fifo);                   // Освобождаем память
    // mutex_destroy(&fifo_lock);              // Опционально для обычных мьютексов
    printk(KERN_INFO "FIFO модуль выгружен\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Простой модуль ядра Hello World");
