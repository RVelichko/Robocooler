#!/bin/bash

# Скопировать сертификаты в общие папки и настроить права доступа
echo "- Копирование результата ..."
sudo cp rootCA.pem /etc/ssl/certs/ || exit 1
sudo cp server.pem /etc/ssl/certs/ || exit 2
sudo cp server.key /etc/ssl/private/ || exit 3
sudo chmod 0600 /etc/ssl/private/server.key || exit 4

# Включить SSL на сервере apache2
echo "- Включить поддержку SSH ..."
sudo a2enmod ssl || exit 5

# Указать расположение сертификатов apache - серверу
echo "- Настроить конфигурацию для поддержки собственных сертификатов ..."
sudo bash -c "mv /etc/apache2/sites-available/000-default.conf /etc/apache2/sites-available/000-default.conf.old" || exit 6
sudo bash -c "mv /etc/apache2/sites-available/default-ssl.conf /etc/apache2/sites-available/default-ssl.conf.old" || exit 7
sudo bash -c "mv /etc/apache2/sites-enabled/000-default.conf /etc/apache2/sites-enabled/000-default.conf.old" || exit 8
sudo bash -c "cp 000-default.conf /etc/apache2/sites-available/000-default.conf" || exit 9
sudo bash -c "ln -s /etc/apache2/sites-available/000-default.conf /etc/apache2/sites-enabled/000-default.conf" || exit 10

echo "- Перезапустить apache сервер ..."
sudo service apache2 restart || exit 11

