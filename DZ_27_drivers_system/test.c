// test.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/ring_buffer.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

#define DEVICE_NAME "ring_buffer_device"
#define BUFFER_SIZE 256

// Структура для кольцевого буфера
struct ring_buffer {
    char *buffer;
    size_t head; // Индекс для записи
    size_t tail; // Индекс для чтения
    size_t size; // Размер буфера
};

static struct ring_buffer *rb;
static int major;

static struct kobject *example_kobject;
static int buffer_size;

// Функция для чтения из устройства
static ssize_t rb_read(struct file *file, char __user *buf, size_t len, loff_t *offset) {
    size_t i;
    size_t bytes_to_read = (rb->head >= rb->tail) ? (rb->head - rb->tail) : (rb->size - rb->tail + rb->head);

    if (len > bytes_to_read)
    {
        len = bytes_to_read; // Ограничиваем чтение размером доступных данных
    }

    for (i = 0; i < len; i++)
    {
        // Копируем данные в пользовательское пространство
        if (copy_to_user(&buf[i], &rb->buffer[rb->tail], 1))
        {
            return -EFAULT;
        }
        rb->tail = (rb->tail + 1) % rb->size; // Обновление указателя
    }

    return len; // Возврат количества прочитанных байтов
}

// Функция для записи в устройство
static ssize_t rb_write(struct file *file, const char __user *buf, size_t len, loff_t *offset) {
    size_t i;

    // Ограничение на максимальный размер записи
    if (len > rb->size)
    {
        len = rb->size; // Ограничиваем запись размером буфера
    }

    for (i = 0; i < len; i++)
    {
        // Копируем данные из пользовательского пространства
        if (copy_from_user(&rb->buffer[rb->head], &buf[i], 1))
        {
            return -EFAULT;
        }
        rb->head = (rb->head + 1) % rb->size; // Обновление указателя
    }

    return len; // Возврат количества записанных байтов
}

// Открытие устройства
static int rb_open(struct inode *inode, struct file *file) {
    return 0;
}

// Закрытие устройства
static int rb_release(struct inode *inode, struct file *file) {
    return 0;
}

// Операции файла
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = rb_read,
    .write = rb_write,
    .open = rb_open,
    .release = rb_release,
};

// Sysfs атрибуты
static ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%d\n", buffer_size);
}

static ssize_t size_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%d", &buffer_size);
    return count;
}

static struct kobj_attribute size_attribute = __ATTR(size, 0664, size_show, size_store);

static int __init rb_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0)
    {
        printk(KERN_ALERT "Registering char device failed with %d\n", major);
        return major;
    }

    rb = kmalloc(sizeof(struct ring_buffer), GFP_KERNEL);
    rb->buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    rb->head = 0;
    rb->tail = 0;

    // Создание kobject
    example_kobject = kobject_create_and_add("ring_buffer_module", kernel_kobj);
    if (!example_kobject)
    {
        kfree(rb->buffer);
        kfree(rb);
        return -ENOMEM;
    }
    sysfs_create_file(example_kobject, &size_attribute.attr);

    printk(KERN_INFO "Char device registered!!!\n");
    return 0;
}

static void __exit rb_exit(void) {
    kobject_put(example_kobject);
    kfree(rb->buffer);
    kfree(rb);
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "Char device unregistered\n");
}

module_init(rb_init);
module_exit(rb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Кольцевой буфер для чтения и записи через sysfs");