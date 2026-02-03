Не все linux-headers подходят, даже те, что имеют тот же номер, что и ядро Linux
Для ядра 6.14 хедеры linux-headers-6.14.0-37-generic не прокатывают
Ссылку в /lib/modules/$(shell uname -r)/build надо менять на ссылку на ядро
```
#define IOCTL_SET_MSG _IOW(IOCTL_MAGIC, 0, char*) 
// OR define a type
typedef char* char_ptr;
#define IOCTL_SET_MSG _IOW(IOCTL_MAGIC, 0, char_ptr)
```
```
#include <linux/ioctl.h> // Inside Kernel Module
// or
#include <sys/ioctl.h>   // Inside User Application
```
Если
```
$ dmesg | tail

[17102.368731] module lesson_example: .gnu.linkonce.this_module section size must match the kernel's built struct module size at run time
[18883.788289] lesson_example: loading out-of-tree module taints kernel.
```
и модуль не загрузился в /dev, то необходимо убрать
```
lesson_example.c:138:20: warning: ‘chardev_exit’ defined but not used [-Wunused-function]
  138 | static void __exit chardev_exit(void)
      |                    ^~~~~~~~~~~~
lesson_example.c:98:19: warning: ‘chardev_init’ defined but not used [-Wunused-function]
   98 | static int __init chardev_init(void)
      |                   ^~~~~~~~~~~~
```
а именно запустить обе эти функции:
```
module_init(chardev_init);
module_exit(chardev_exit);
```