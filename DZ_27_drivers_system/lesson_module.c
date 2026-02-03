// lesson_module.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/cdev.h>
// #include <linux/stat.h>
// #include <asm/uaccess.h>

#define DEVICE_NAME "mychardev"
#define CLASS_NAME "mychar"
#define IOCTL_MAGIC 'N'
#define IOCTL_SET_MSG _IOW(IOCTL_MAGIC, 0, char *)

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *my_class;
static struct device *my_device;
static char kernel_buffer[256] = {0};
static size_t buffer_size = 0;
static DEFINE_MUTEX(device_mutex);

static int my_open(struct inode *inode, struct file *file)
{
    if (!mutex_trylock(&device_mutex))
    {
        printk(KERN_WARNING "Device is busy\n");
        return -EBUSY;
    }
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    mutex_unlock(&device_mutex);
    printk(KERN_INFO "Device closed\n");
    return 0;
}

static ssize_t my_read(struct file *file, char __user *user_buf, size_t count,
                       loff_t *offset)
{
    size_t to_read;
    if (*offset >= buffer_size)
        return 0; // EOF
    to_read = min(count, buffer_size - (size_t)*offset);
    if (copy_to_user(user_buf, kernel_buffer + *offset, to_read))
        return -EFAULT;
    *offset += to_read;
    printk(KERN_INFO "Read %zu bytes\n", to_read);
    return to_read;
}

static ssize_t my_write(struct file *file, const char __user *user_buf, size_t count, loff_t *offset)
{
    if (count >= sizeof(kernel_buffer))
    {
        printk(KERN_ERR "Buffer overflow attempt\n");
        return -EINVAL;
    }
    if (copy_from_user(kernel_buffer, user_buf, count))
        return -EFAULT;
    buffer_size = count;
    kernel_buffer[count] = '\0'; // Добавляем терминатор
    printk(KERN_INFO "Wrote %zu bytes: %s", count, kernel_buffer);
    return count;
}

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    char msg[64];
    if (cmd != IOCTL_SET_MSG)
        return -EINVAL;
    if (copy_from_user(msg, (char *)arg, sizeof(msg)))
        return -EFAULT;
    printk(KERN_INFO "IOCTL received: %s", msg);
    sprintf(kernel_buffer, "[IOCTL] %s", msg);
    buffer_size = strlen(kernel_buffer);
    return 0;
}

// --- Регистрация операций ---
static const struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .read = my_read,
    .write = my_write,
    .unlocked_ioctl = my_ioctl,
};

static int __init chardev_init(void)
{
    int ret;
    // 1. Выделяем диапазон major/minor номеров
    if ((ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME)) < 0)
    {
        printk(KERN_ERR "Failed to allocate char dev region\n");
        return ret;
    }
    // 2. Инициализируем cdev
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;
    if ((ret = cdev_add(&my_cdev, dev_num, 1)) < 0)
    {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    // 3. Создаем класс устройств (/sys/class/mychar/)
    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class))
    {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_class);
    }
    // 4. Создаем устройство (автоматически появится в /dev/mychardev)
    my_device = device_create(my_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(my_device))
    {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(my_device);
    }

    printk(KERN_INFO "Char device registered!!! (major=%d)\n", dev_num);
    return 0;
}

static void __exit chardev_exit(void)
{
    device_destroy(my_class, dev_num);
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk(KERN_INFO "Char device unregistered!!!\n");
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Кольцевой буфер для чтения и записи через sysfs");