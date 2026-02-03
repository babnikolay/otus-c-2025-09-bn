Сборка модуля
Создайте Makefile рядом с исходником

Запустите:

make
sudo insmod simple_ring_buffer.ko

Тестирование
Проверка устройства:

ls -l /dev/ringbuf
lsmod | grep ring_buffer

Запись в устройство:

echo "Hello WSL2 kernel module" > /dev/ringbuf

Чтение из устройства:

cat /dev/ringbuf

Работа с sysfs:

cat /sys/kernel/ringbuf_sysfs/sysfs_param
echo 42 | sudo tee /sys/kernel/ringbuf_sysfs/sysfs_param
cat /sys/kernel/ringbuf_sysfs/sysfs_param
