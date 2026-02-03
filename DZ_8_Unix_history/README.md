```
./configure --prefix=/home/bn/Work/OTUS/otus-c-2025-09-bn/DZ_8_Unix_history --with-ssl --enable-http \
--enable-telnet --enable-httpsrr --disable-ftp --disable-ipfs --disable-ldap --disable-ldaps \
--disable-rtsp --disable-proxy --disable-dict --disable-tftp --disable-pop3 --disable-imap --disable-smb \
--disable-smtp --disable-gopher --disable-mqtt --disable-aws --disable-ntlm --disable-tls-srp \
--disable-unix-sockets --disable-file --disable-websockets
```
```
make
make install
```
```
cd ../bin/
./curl --version
```
