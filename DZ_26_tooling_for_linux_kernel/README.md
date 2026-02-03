# Предварительные описания
Все работы и проверки производились в WSL2.
Так как неизвестно где и как установлено ядро Linux в WSL, пришлось установить новое ядро.
На просторах Микрософта есть информация про ядра для WSL2, но там последнее из доступных оказалось только 5-й версии.
Поэтому:
1. Идем на сайт https://www.kernel.org и скачиваем поледнюю версию (на момент проведения данных проверок - это версия 6.18.6)
2. Разархивируем ее в WSL2 в доступном каталоге.
3. Чтобы не изобретать велосипед, создаём автоматически конфигурацию на основе текущих загруженных модулей, т.е. из ядра WSL2
```
$ make localmodconfig
```
4. Заходим в конфигуратор ядра и устанавливает нужные параметры для трассировки программ:
```
$ make menuconfig
```
Устанавливаемые нужные параметры в разделе:
```
Kernel hacking - Tracers
```

5. Собираем и устанавливаем ядро:
```
$ make -j2
$ make modeles
$ sudo make modeles_install
$ sudo make install
```

Ядро установилось в каталог /boot
```
$ ll /boot
-rw-r--r-- 1 root root  8874971 Jan 23 11:18 System.map-6.18.6-microsoft-standard-WSL2
-rw-r--r-- 1 root root   144262 Jan 23 11:18 config-6.18.6-microsoft-standard-WSL2
-rw-r--r-- 1 root root 14671872 Jan 23 11:18 vmlinuz-6.18.6-microsoft-standard-WSL2
```


6. Копируем установленное ядро в Windows, в каталог с достаточными для запуска правами доступа, например, в c:\Temp

7. В файле конфигурации для WSL, который находится по пути C:\Users\ваш_пользователь\\.wslconfig прописываем следующую строку:
```
[wsl2]
kernel=C:\\Temp\\vmlinuz-6.18.6-microsoft-standard-WSL2
```

8. Выходим из WSL и перезапускаем систему:
```
wsl --shutdown
wsl
```

9. После загрузки системы WSL проверяем установленную версию ядра:
```
$ uname -r
6.18.6-microsoft-standard-WSL2
```


# Работа с ftrace

### Выполнить её можно двумя путями:
1. Запустить bash скрипт get_trace.sh из текущего каталога (по умолчанию он выведет справку, как правильно его запускать)
2. Выполнить трассировку вручную, как написано ниже.
### 1. Включение ftrace

Для настройки ftrace, все следующие шаги выполняются от root'a:
Перейдите в каталог tracefs:
```
# cd /sys/kernel/debug/tracing
```

Очистите любые предыдущие следы:
```
# echo 0 > trace
```

Включите трассировку системных вызовов:
```
# echo '__x64_sys_*' > set_ftrace_filter
# echo 1 > tracing_on
```

### 2. Запуск вашей программы
Теперь выполните вашу программу:
```
# gcc your_program.c -o your_program
# ./your_program
```

### 3. Отключите трассировку
Чтобы отключить трассировку, выполните:
```
# echo 0 > tracing_on
```

### 4. Сбор данных о системных вызовах
После запуска программы, можно собрать данные:
```
# cat trace
```

### 5. Очистите фильтр
```
# echo > set_ftrace_filter
```

### Отчёт о системных вызовах
Системные вызовы, использованные в программе:
Системный вызов	Описание
```
open	Открывает файл и возвращает дескриптор файла
write	Записывает данные в файл
read	Читает данные из файла
close	Закрывает открытый файл
unlink  Удаляет файл
```

## Результаты трассировки (пример)
При выполнении программы, ftrace зафиксирует вызовы системных функций, включая их время выполнения и параметры.
Это позволяет анализировать производительность и поведение системы.
```
# tracer: function
#
# entries-in-buffer/entries-written: 9216/9216   #P:2
#
#                                _-----=irqs-off/BH-disabled
#                               / _----=need-resched
#                              | / _---=hardirq/softirq
#                              || / _--=preempt-depth
#                              ||| / _-=migrate-disable
#                              |||| /     delay
#           TASK-PID     CPU#  |||||  TIMESTAMP  FUNCTION
#              | |         |   |||||     |         |
         tracing-9173    [001] ..... 20903.036796: __x64_sys_set_robust_list <-x64_sys_call
         tracing-9173    [001] ..... 20903.036953: __x64_sys_close <-x64_sys_call
         tracing-9173    [001] ..... 20903.036971: __x64_sys_rt_sigprocmask <-x64_sys_call
         tracing-9173    [001] ..... 20903.036973: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.036975: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.036977: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.036989: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.036994: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.036995: __x64_sys_rt_sigaction <-x64_sys_call
         tracing-9173    [001] ..... 20903.037011: __x64_sys_execve <-x64_sys_call
         tracing-9173    [000] ..... 20903.037796: __x64_sys_brk <-x64_sys_call
         tracing-9173    [000] ..... 20903.037972: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038008: __x64_sys_access <-x64_sys_call
         tracing-9173    [000] ..... 20903.038023: __x64_sys_openat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038040: __x64_sys_newfstat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038045: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038058: __x64_sys_close <-x64_sys_call
         tracing-9173    [000] ..... 20903.038076: __x64_sys_openat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038094: __x64_sys_read <-x64_sys_call
         tracing-9173    [000] ..... 20903.038102: __x64_sys_pread64 <-x64_sys_call
         tracing-9173    [000] ..... 20903.038106: __x64_sys_newfstat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038111: __x64_sys_pread64 <-x64_sys_call
         tracing-9173    [000] ..... 20903.038126: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038142: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038175: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038195: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038244: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038271: __x64_sys_close <-x64_sys_call
         tracing-9173    [000] ..... 20903.038285: __x64_sys_mmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038303: __x64_sys_arch_prctl <-x64_sys_call
         tracing-9173    [000] ..... 20903.038305: __x64_sys_set_tid_address <-x64_sys_call
         tracing-9173    [000] ..... 20903.038306: __x64_sys_set_robust_list <-x64_sys_call
         tracing-9173    [000] ..... 20903.038307: __x64_sys_rseq <-x64_sys_call
         tracing-9173    [000] ..... 20903.038394: __x64_sys_mprotect <-x64_sys_call
         tracing-9173    [000] ..... 20903.038415: __x64_sys_mprotect <-x64_sys_call
         tracing-9173    [000] ..... 20903.038428: __x64_sys_mprotect <-x64_sys_call
         tracing-9173    [000] ..... 20903.038459: __x64_sys_prlimit64 <-x64_sys_call
         tracing-9173    [000] ..... 20903.038473: __x64_sys_munmap <-x64_sys_call
         tracing-9173    [000] ..... 20903.038521: __x64_sys_newfstat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038529: __x64_sys_getrandom <-x64_sys_call
         tracing-9173    [000] ..... 20903.038534: __x64_sys_brk <-x64_sys_call
         tracing-9173    [000] ..... 20903.038535: __x64_sys_brk <-x64_sys_call
         tracing-9173    [000] ..... 20903.038559: __x64_sys_write <-x64_sys_call
         tracing-9173    [000] ..... 20903.038624: __x64_sys_newfstat <-x64_sys_call
         tracing-9173    [000] ..... 20903.038629: __x64_sys_read <-x64_sys_call
         tracing-9173    [000] ..... 20904.542944: __x64_sys_openat <-x64_sys_call
         tracing-9173    [000] ..... 20904.543087: __x64_sys_write <-x64_sys_call
         tracing-9173    [000] ..... 20904.543124: __x64_sys_lseek <-x64_sys_call
         tracing-9173    [000] ..... 20904.543125: __x64_sys_read <-x64_sys_call
         tracing-9173    [000] ..... 20904.543130: __x64_sys_newfstat <-x64_sys_call
         tracing-9173    [000] ..... 20904.543135: __x64_sys_close <-x64_sys_call
         tracing-9173    [000] ..... 20904.543140: __x64_sys_unlink <-x64_sys_call
         tracing-9173    [000] ..... 20904.543215: __x64_sys_write <-x64_sys_call
         tracing-9173    [000] ..... 20904.543337: __x64_sys_exit_group <-x64_sys_call
```
## Заключение
### Данная программа демонстрирует основные системные вызовы на C и процесс их отслеживания с помощью ftrace. 
### Вы можете использовать эту информацию для дальнейшего анализа и оптимизации программ в системах на базе Linux.