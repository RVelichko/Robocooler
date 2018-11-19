#!/bin/bash

##################################################################################################################################
# Установка корневого сертификата Windows Chrome:
#
# —> Settings —> Manage Certificates…
# Выбрать таб Trusted Root Certificate Authorities —> Import —> rootCA.crt
# перезапустить Chrome

# Установка expect для автоматизации скриптов
if [[ -z $(aptitude search expect|grep "i "|grep " expect ") ]]; then sudo aptitude -y install expect || exit 1; fi

# Генерация корневого сертификата
echo "- Генерация rootCA.key ..."
openssl genrsa -out rootCA.key 2048 || exit 2
echo "- Генерация rootCA.pem ..."
expect -c "spawn openssl req -x509 -new -nodes -key rootCA.key -sha256 -days 3650 -out rootCA.pem
expect -re \".+Country Name.+\"
send \"RU\r\"
expect -re \"State or Province Name.+\"
send \"Moscov\r\"
expect -re \"Locality Name.+\"
send \"Moscov\r\"
expect -re \"Organization Name.+\"
send \"AlfaRobotics\r\"
expect -re \"Organizational Unit Name.+\"
send \r
expect -re \"Common Name.+\"
send \"local.robot.iface\r\"
expect -re \"Email Address.+\"
send \r
interact"

# Генерация клиентского сертификата
echo "- Генерация server.key ..."
openssl genrsa -out server.key || exit 3
echo "- Генерация server.csr ..."
expect -c "spawn openssl req -new -key server.key -out server.csr
expect -re \".+Country Name.+\"
send \"RU\r\"
expect -re \"State or Province Name.+\"
send \"Moscov\r\"
expect -re \"Locality Name.+\"
send \"Moscov\r\"
expect -re \"Organization Name.+\"
send \"AlfaRobotics\r\"
expect -re \"Organizational Unit Name.+\"
send \r
expect -re \"Common Name.+\"
send \"local.robot.iface\r\"
expect -re \"Email Address.+\"
send \r
expect -re \"A challenge password.+\"
send \r
expect -re \"An optional company name.+\"
send \r
interact"

sudo aptitude -y purge expect || exit 4

# Подписать запрос на сертификат корневым сертификатом
echo "- Генерация server.pem ..."
openssl x509 -req -in server.csr -CA rootCA.pem -CAkey rootCA.key -CAcreateserial -out server.pem -days 1600 -sha256 || exit 5
