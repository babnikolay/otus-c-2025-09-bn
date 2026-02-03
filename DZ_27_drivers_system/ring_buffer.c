// ring_buffer.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <linux/wait.h>

#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Кольцевой буфер для чтения и записи через sysfs");

static char *buffer;
static size_t head = 0;
static size_t tail = 0;
static size_t current_size = 0;

// Ожидания
static DECLARE_WAIT_QUEUE_HEAD(read_queue);
static DECLARE_WAIT_QUEUE_HEAD(write_queue);

static size_t max_size = BUFFER_SIZE;

// Функции для записи и чтения

ssize_t ring_buffer_read(struct file *file, char __user *user_buffer, size_t count, loff_t *pos) {
    size_t bytes_to_read;

    while (current_size == 0)
    {
        if (wait_event_interruptible(read_queue, current_size != 0))
            return -ERESTARTSYS;
    }

    bytes_to_read = current_size < count ? current_size : count;
    if (copy_to_user(user_buffer, buffer + head, bytes_to_read))
    {
        return -EFAULT;
    }

    head = (head + bytes_to_read) % max_size;
    current_size -= bytes_to_read;

    wake_up_interruptible(&write_queue);
    return bytes_to_read;
}

ssize_t ring_buffer_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *pos) {
    size_t bytes_to_write;

    while (current_size + count > max_size)
    {
        if (wait_event_interruptible(write_queue, current_size + count <= max_size))
            return -ERESTARTSYS;
    }

    bytes_to_write = (max_size - current_size < count) ? (max_size - current_size) : count;
    if (copy_from_user(buffer + tail, user_buffer, bytes_to_write))
    {
        return -EFAULT;
    }

    tail = (tail + bytes_to_write) % max_size;
    current_size += bytes_to_write;

    wake_up_interruptible(&read_queue);
    return bytes_to_write;
}

// Функция, вызываемая при открытии устройства
int ring_buffer_open(struct inode *inode, struct file *file) {
    return 0;
}

// Функция, вызываемая при закрытии устройства
int ring_buffer_release(struct inode *inode, struct file *file) {
    return 0;
}

static struct file_operations ring_buffer_fops = {
    .owner = THIS_MODULE,
    .read = ring_buffer_read,
    .write = ring_buffer_write,
    .open = ring_buffer_open,
    .release = ring_buffer_release,
};

static struct class *ring_buffer_class;
static struct device *ring_buffer_device;

// Настройки через sysfs

static ssize_t size_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    return sprintf(buf, "%zu\n", max_size);
}

static ssize_t size_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {
    sscanf(buf, "%zu", &max_size);
    return count;
}

static struct kobj_attribute size_attribute = __ATTR(size, 0664, size_show, size_store);

static int __init ring_buffer_init(void) {
    buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!buffer)
    {
        return -ENOMEM;
    }

    ring_buffer_class = class_create(THIS_MODULE, "ring_buffer_class");
    ring_buffer_device = device_create(ring_buffer_class, NULL, 0, NULL, "ring_buffer_device");

    // Создание sysfs атрибутов
    sysfs_create_file(&ring_buffer_device->kobj, &size_attribute.attr);

    register_chrdev(0, "ring_buffer_device", &ring_buffer_fops);
    return 0;
}

static void __exit ring_buffer_exit(void) {
    kfree(buffer);
    device_destroy(ring_buffer_class, 0);
    class_destroy(ring_buffer_class);
    unregister_chrdev(0, "ring_buffer_device");
}

module_init(ring_buffer_init);
module_exit(ring_buffer_exit);
