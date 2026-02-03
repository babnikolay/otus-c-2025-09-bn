// example.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/sysfs.h>
#include <linux/kthread.h>
#include <linux/wait.h>

#define MODULE

MODULE_LICENSE("GPL");
MODULE_AUTHOR("bn");
MODULE_DESCRIPTION("Кольцевой буфер для чтения и записи через sysfs");

int init_module(void) {
    printk("<1> Привет, драйвер\n");
    return 0;
}

void cleanup_module(void) { 
    printk("<1> Прощай, драйвер\n"); 
}

module_init(init_module);
module_exit(cleanup_module);
