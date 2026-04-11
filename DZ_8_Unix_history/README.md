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
## Установите недостающие в WSL2 пакеты:
```sh
sudo apt update && sudo apt install libpsl-dev -y
```
либо добавьте в конфигурацию опцию **--without-libpsl**  

## Команды  

```sh
cd curl-8.18.0/
```

```sh
./configure --prefix=~/Work/otus-c-2025-09-bn/DZ_8_Unix_history --with-ssl --enable-http \
--enable-telnet --enable-httpsrr --disable-ftp --disable-ipfs --disable-ldap --disable-ldaps \
--disable-rtsp --disable-proxy --disable-dict --disable-tftp --disable-pop3 --disable-imap --disable-smb \
--disable-smtp --disable-gopher --disable-mqtt --disable-aws --disable-ntlm --disable-tls-srp \
--disable-unix-sockets --disable-file --disable-websockets **--without-libpsl**

......

configure: Configured to build curl/libcurl:

  Host setup:       x86_64-pc-linux-gnu
  Install prefix:   /home/bn/Work/otus-c-2025-09-bn/DZ_8_Unix_history
  Compiler:         gcc
   CFLAGS:          -Werror-implicit-function-declaration -O2 -Wno-system-headers
   CFLAGS extras:
   CPPFLAGS:        -D_GNU_SOURCE
   LDFLAGS:
     curl-config:
   LIBS:            -lpsl -lssl -lcrypto -lssl -lcrypto -lzstd -lbrotlidec -lz

  curl version:     8.18.0
  SSL:              enabled (OpenSSL)
  SSH:              no       (--with-{libssh,libssh2})
  zlib:             enabled
  brotli:           enabled (libbrotlidec)
  zstd:             enabled (libzstd)
  GSS-API:          no       (--with-gssapi)
  GSASL:            no       (--with-gsasl)
  TLS-SRP:          no       (--enable-tls-srp)
  resolver:         POSIX threaded
  IPv6:             enabled
  Unix sockets:     no       (--enable-unix-sockets)
  IDN:              no       (--with-{libidn2,winidn})
  Build docs:       enabled  (--disable-docs)
  Build libcurl:    Shared=yes, Static=yes
  Built-in manual:  enabled
  --libcurl option: enabled  (--disable-libcurl-option)
  Type checking:    enabled  (--disable-typecheck)
  Verbose errors:   enabled  (--disable-verbose)
  Code coverage:    disabled
  SSPI:             no       (--enable-sspi)
  ca native:        no
  ca cert bundle:   /etc/ssl/certs/ca-certificates.crt
  ca cert path:     /etc/ssl/certs
  ca cert embed:    no
  ca fallback:      no
  LDAP:             no       (--enable-ldap / --with-ldap-lib / --with-lber-lib)
  LDAPS:            no       (--enable-ldaps)
  IPFS/IPNS:        no       (--enable-ipfs)
  RTSP:             no       (--enable-rtsp)
  RTMP:             no       (--with-librtmp)
  PSL:              enabled
  Alt-svc:          enabled  (--disable-alt-svc)
  Headers API:      enabled  (--disable-headers-api)
  HSTS:             enabled  (--disable-hsts)
  HTTP1:            enabled  (internal)
  HTTP2:            no       (--with-nghttp2)
  HTTP3:            no       (--with-ngtcp2 --with-nghttp3, --with-quiche, --with-openssl-quic)
  ECH:              no      (--enable-ech)
  HTTPS RR:         enabled (--disable-httpsrr)
  SSLS-EXPORT:      no      (--enable-ssls-export)
  Protocols:        http https telnet
  Features:         alt-svc AsynchDNS brotli HSTS HTTPSRR IPv6 Largefile libz PSL SSL threadsafe zstd

configure: WARNING: HTTPSRR is enabled but marked EXPERIMENTAL. Use with caution!

```

```sh
make -j$(nproc)
make install
```

```sh
cd ../bin/
./curl --version

curl 8.18.0 (x86_64-pc-linux-gnu) libcurl/8.18.0 OpenSSL/3.0.13 zlib/1.3 brotli/1.1.0 zstd/1.5.5
Release-Date: 2026-01-07
Protocols: **http https telnet**
Features: alt-svc AsynchDNS brotli HSTS HTTPSRR IPv6 Largefile libz SSL threadsafe zstd
```
