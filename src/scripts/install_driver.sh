#!/bin/bash

##################################################################################################################################
echo "Скрипт осуществляет установку и настройку локальной версии рабочей среды робота."

##################################################################################################################################
echo "Установка требуемых компонентов"
sudo aptitude update

ehco "1. libboost1.55"
if [[ -z $(aptitude search libboost1.55-all-dev|grep "i "|grep " libboost1.55-all-dev ") ]]; then sudo aptitude -y install libboost1.55-all-dev || exit 1; fi

echo "2. libcurl3"
if [[ -z $(aptitude search libcurl3|grep "i "|grep " libcurl3 ") ]]; then sudo aptitude -y install libcurl3 || exit 2; fi

echo "3. libssl1.0.0"
if [[ -z $(aptitude search libssl1.0.0|grep "i "|grep " libssl1.0.0 ") ]]; then sudo aptitude -y install libssl1.0.0 || exit 3; fi

echo "4. libjsoncpp0"
if [[ -z $(aptitude search libjsoncpp0|grep "i "|grep " libjsoncpp0 ") ]]; then sudo aptitude -y install libjsoncpp0 || exit 4; fi

echo "5. Установка GUMBO парсера"
wget https://github.com/google/gumbo-parser/archive/master.zip || exit 5
unzip master.zip || exit 6
rm -f master.zip
pushd gumbo-parser-master || exit 7
if [[ -z $(aptitude search libtool|grep "i "|grep " libtool ") ]]; then sudo aptitude -y install libtool || exit 8; fi
if [[ -z $(aptitude search autoconf|grep "i "|grep " autoconf ") ]]; then sudo aptitude -y install autoconf || exit 9; fi
if [[ -z $(aptitude search g\\+\\+|grep "i "|grep " g++ ") ]]; then sudo aptitude -y install g++ || exit 10; fi
./autogen.sh || exit 11
./configure || exit 12
make || exit 13
sudo make install || exit 14
popd
rm -rf gumbo-parser-master
sudo aptitude -y remove libtool || exit 15;
sudo aptitude -y remove autoconf || exit 16;
sudo aptitude -y remove g++ || exit 17;

##################################################################################################################################
echo "6. Создание требуемых директорий"
sudo mkdir -p /alfarobotics/bin || exit 18
sudo mkdir -p /alfarobotics/media || exit 19

##################################################################################################################################
echo "7. Копирование компонент"
sudo cp ws-saver ws-saver.sh /alfarobotics/bin/ || exit 20
sudo cp ws-wifi ws-wifi.sh /alfarobotics/bin/ || exit 21
sudo cp ws-robot ws-robot.sh /alfarobotics/bin/ || exit 22
sudo cp alfa-robot-services.sh /etc/init.d/ || exit 23
sudo cp media/* /alfarobotics/media || exit 24
sudo cp -r alfa_robot /var/www/html || exit 25

##################################################################################################################################
echo "8. Генерация самоподписываемого сертификата"
./generate_certificate.sh || exit 26

##################################################################################################################################
echo "9. Настроить apache2"
./configure_apache2.sh || exit 27

##################################################################################################################################
echo "10. Запустить локальные сервисы"
sudo update-rc.d alfa-robot-services.sh defaults || exit 28
sudo /etc/init.d/alfa-robot-services.sh start || exit 29

exit 0
