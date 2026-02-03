// simple_ring_buffer.c
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/mutex.h>

#define DEVICE_NAME "ringbuf"
#define BUFFER_SIZE 1024

static char ring_buffer[BUFFER_SIZE];
static size_t head = 0; // write position
static size_t tail = 0; // read position
static struct mutex ringbuf_lock;

static struct cdev ringbuf_cdev;
static dev_t dev_number;
static struct class *ringbuf_class;

static struct kobject *ringbuf_kobj;

// Параметр, доступный через sysfs
static int sysfs_param = 0;

// Вспомогательная функция для подсчёта количества байт в буфере
static size_t ringbuf_data_size(void)
{
    if (head >= tail)
        return head - tail;
    else
        return BUFFER_SIZE - tail + head;
}

// Вспомогательная функция для подсчёта свободного места
static size_t ringbuf_space_left(void)
{
    return BUFFER_SIZE - ringbuf_data_size() - 1;
}

// Операция чтения из устройства
static ssize_t ringbuf_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    size_t data_size;
    size_t to_read;
    size_t first_chunk;

    if (mutex_lock_interruptible(&ringbuf_lock))
        return -ERESTARTSYS;

    data_size = ringbuf_data_size();
    if (data_size == 0)
    {
        mutex_unlock(&ringbuf_lock);
        return 0; // EOF
    }

    to_read = min(count, data_size);

    // Читаем по частям, если кольцо "перекручено"
    if (tail + to_read <= BUFFER_SIZE)
    {
        if (copy_to_user(buf, ring_buffer + tail, to_read))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        tail = (tail + to_read) % BUFFER_SIZE;
    }
    else
    {
        first_chunk = BUFFER_SIZE - tail;
        if (copy_to_user(buf, ring_buffer + tail, first_chunk))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        if (copy_to_user(buf + first_chunk, ring_buffer, to_read - first_chunk))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        tail = to_read - first_chunk;
    }

    mutex_unlock(&ringbuf_lock);
    return to_read;
}

// Операция записи в устройство
static ssize_t ringbuf_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    size_t space_left;
    size_t to_write;
    size_t first_chunk;

    if (mutex_lock_interruptible(&ringbuf_lock))
        return -ERESTARTSYS;

    space_left = ringbuf_space_left();
    if (space_left == 0)
    {
        mutex_unlock(&ringbuf_lock);
        return -ENOSPC; // Нет места
    }

    to_write = min(count, space_left);

    if (head + to_write <= BUFFER_SIZE)
    {
        if (copy_from_user(ring_buffer + head, buf, to_write))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        head = (head + to_write) % BUFFER_SIZE;
    }
    else
    {
        first_chunk = BUFFER_SIZE - head;
        if (copy_from_user(ring_buffer + head, buf, first_chunk))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        if (copy_from_user(ring_buffer, buf + first_chunk, to_write - first_chunk))
        {
            mutex_unlock(&ringbuf_lock);
            return -EFAULT;
        }
        head = to_write - first_chunk;
    }

    mutex_unlock(&ringbuf_lock);
    return to_write;
}

static int ringbuf_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int ringbuf_release(struct inode *inode, struct file *file)
{
    return 0;
}

static const struct file_operations ringbuf_fops = {
    .owner = THIS_MODULE,
    .read = ringbuf_read,
    .write = ringbuf_write,
    .open = ringbuf_open,
    .release = ringbuf_release,
};

// Sysfs show/store для sysfs_param
static ssize_t sysfs_param_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", sysfs_param);
}

static ssize_t sysfs_param_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    int ret, val;

    ret = kstrtoint(buf, 10, &val);
    if (ret < 0)
        return ret;

    sysfs_param = val;
    return count;
}

static struct kobj_attribute sysfs_param_attr = __ATTR(sysfs_param, 0664, sysfs_param_show, sysfs_param_store);

static int __init ringbuf_init(void)
{
    int ret;

    mutex_init(&ringbuf_lock);

    // Резервируем устройство
    ret = alloc_chrdev_region(&dev_number, 0, 1, DEVICE_NAME);
    if (ret < 0)
    {
        pr_err("Failed to allocate char device region\n");
        return ret;
    }

    cdev_init(&ringbuf_cdev, &ringbuf_fops);
    ringbuf_cdev.owner = THIS_MODULE;

    ret = cdev_add(&ringbuf_cdev, dev_number, 1);
    if (ret)
    {
        pr_err("Failed to add cdev\n");
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    ringbuf_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(ringbuf_class))
    {
        pr_err("Failed to create class\n");
        cdev_del(&ringbuf_cdev);
        unregister_chrdev_region(dev_number, 1);
        return PTR_ERR(ringbuf_class);
    }

    if (!device_create(ringbuf_class, NULL, dev_number, NULL, DEVICE_NAME))
    {
        pr_err("Failed to create device\n");
        class_destroy(ringbuf_class);
        cdev_del(&ringbuf_cdev);
        unregister_chrdev_region(dev_number, 1);
        return -ENOMEM;
    }

    // Создаём kobject для sysfs
    ringbuf_kobj = kobject_create_and_add("ringbuf_sysfs", kernel_kobj);
    if (!ringbuf_kobj)
    {
        pr_err("Failed to create kobject\n");
        device_destroy(ringbuf_class, dev_number);
        class_destroy(ringbuf_class);
        cdev_del(&ringbuf_cdev);
        unregister_chrdev_region(dev_number, 1);
        return -ENOMEM;
    }

    ret = sysfs_create_file(ringbuf_kobj, &sysfs_param_attr.attr);
    if (ret)
    {
        pr_err("Failed to create sysfs file\n");
        kobject_put(ringbuf_kobj);
        device_destroy(ringbuf_class, dev_number);
        class_destroy(ringbuf_class);
        cdev_del(&ringbuf_cdev);
        unregister_chrdev_region(dev_number, 1);
        return ret;
    }

    pr_info("Ring buffer module loaded\n");
    return 0;
}

static void __exit ringbuf_exit(void)
{
    sysfs_remove_file(ringbuf_kobj, &sysfs_param_attr.attr);
    kobject_put(ringbuf_kobj);
    device_destroy(ringbuf_class, dev_number);
    class_destroy(ringbuf_class);
    cdev_del(&ringbuf_cdev);
    unregister_chrdev_region(dev_number, 1);
    pr_info("Ring buffer module unloaded\n");
}

module_init(ringbuf_init);
module_exit(ringbuf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Simple Linux kernel module with ring buffer and sysfs");
MODULE_VERSION("1.0");