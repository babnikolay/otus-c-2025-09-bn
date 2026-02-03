// #include <linux/module.h>
// #include <linux/kernel.h>
// #include <linux/init.h>
// #include <linux/fs.h>
// #include <linux/uaccess.h>
// #include <linux/slab.h>
// #include <linux/sysfs.h>
// #include <linux/kthread.h>
// #include <linux/wait.h>
// #include <linux/cdev.h>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/mychardev"
#define IOCTL_MAGIC 'N'
// #define IOCTL_SET_MSG _IOW(IOCTL_MAGIC, 0, unsigned long)
typedef char *char_ptr;
#define IOCTL_SET_MSG _IOW(IOCTL_MAGIC, 0, char_ptr)

int main()
{
    int fd;
    char buf[256];
    const char *test_msg = "Hello from user space!";
    const char *ioctl_msg = "IOCTL command";

    // --- Проверка open() ---
    printf("[TEST] Opening device...\n");
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0)
    {
        perror("Failed to open device");
        return 1;
    }
    printf("[OK] Device opened\n\n");

    // --- Проверка write() ---
    printf("[TEST] Writing to device: \"%s\"\n", test_msg);
    if (write(fd, test_msg, strlen(test_msg)) < 0)
    {
        perror("Write failed");
        close(fd);
        return 1;
    }
    printf("[OK] Write successful\n\n");
    // --- Проверка read() ---
    printf("[TEST] Reading from device...\n");
    lseek(fd, 0, SEEK_SET); // Сбрасываем позицию
    ssize_t bytes_read = read(fd, buf, sizeof(buf) - 1);
    if (bytes_read < 0)
    {
        perror("Read failed");
        close(fd);
        return 1;
    }
    buf[bytes_read] = '\0';
    printf("[OK] Read %zd bytes: \"%s\"\n\n", bytes_read, buf);

    // --- Проверка ioctl() ---
    printf("[TEST] Sending IOCTL command: \"%s\"\n", ioctl_msg);
    if (ioctl(fd, IOCTL_SET_MSG, ioctl_msg) < 0)
    {
        perror("IOCTL failed");
        close(fd);
        return 1;
    }
    // --- Проверка закрытия ---
    printf("[TEST] Closing device...\n");
    if (close(fd) < 0)
    {
        perror("Close failed");
        return 1;
    }
    printf("[OK] Device closed\n");

    // --- Проверка попытки повторного открытия (mutex) ---
    printf("\n[TEST] Testing concurrent access...\n");
    int fd1 = open(DEVICE_PATH, O_RDWR);
    int fd2 = open(DEVICE_PATH, O_RDWR);
    if (fd1 >= 0 && fd2 >= 0)
    {
        printf("[FAIL] Concurrent access allowed!\n");
        close(fd1);
        close(fd2);
        return 1;
    }
    if (fd1 >= 0)
        close(fd1);
    if (fd2 >= 0)
        close(fd2);
    printf("[OK] Mutex works correctly (second open failed)\n");
    return 0;
}