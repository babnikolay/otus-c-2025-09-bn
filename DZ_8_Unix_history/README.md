# Задание  

Необходимо скачать curl последней версии и собрать его в любой UNIX-подобной ОС с поддержкой лишь
трёх протоколов: HTTP, HTTPS и TELNET. Обратите внимание, начиная с версии 8.6, поддержка HTTP
автоматически включает поддержку протоколов IPFS и IPNS.  

# Сложность  
★☆☆☆☆

# Цель задания  

Получить опыт сборки программ для UNIX-подобных ОС.  

# Критерии успеха  

1. Работа осуществляется в UNIX-подобной ОС (любой дистрибутив Linux, любая BSD-система, MacOS).  
2. Скачан и распакован исходный код curl.  
3. Сборка сконфигурирована с поддержкой лишь трёх протоколов HTTP, HTTPS и TELNET.  
4. Осуществлена сборка (установку в систему осуществлять не требуется и не рекомендуется).  
5. Собранный curl запущен с ключом --version для подтверждения корректности сборки.  

---
## Результат 

```sh
./configure --prefix=/home/bn/Work/OTUS/otus-c-2025-09-bn/DZ_8_Unix_history --with-ssl --enable-http \
--enable-telnet --enable-httpsrr --disable-ftp --disable-ipfs --disable-ldap --disable-ldaps \
--disable-rtsp --disable-proxy --disable-dict --disable-tftp --disable-pop3 --disable-imap --disable-smb \
--disable-smtp --disable-gopher --disable-mqtt --disable-aws --disable-ntlm --disable-tls-srp \
--disable-unix-sockets --disable-file --disable-websockets
```

```sh
make
make install
```

```sh
cd ../bin/
./curl --version
```
