#!/bin/sh

### BEGIN INIT INFO
# Provides:          Nasladdin
# Required-Start:    $remote_fs $syslog
# Required-Stop:     $remote_fs $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: Nasladdin driver service
# Description:       This file starts and stops Nasladdin driver service
### END INIT INFO

# Для прописывания сервисов в автоматический старт после загрузки системы.
# Скопировать в /etc/init.d скрипт driver-services.sh
# update-rc.d driver-services.sh defaults

find_proc() {
    if pgrep $1; then
        return 0
    else
        return 1
    fi
}

get_pid() {
    #SYM=$(echo $1|cut -c 1)
    #LINE=$(echo $1|cut -c 2-)
    #echo $(ps ax|grep [$SYM]$LINE|sed 's/^[ ]*//'|cut -d " " -f1)
    echo $(pgrep $1)
}

super_kill() {
    if find_proc $1; then
        PID=$(get_pid $1)
        echo "Принудительная остановка сервиса $1: $PID"
        kill -9 $PID
    fi
}

kill_all() {
    super_kill "driver"
}

stop_service() {
    if find_proc $1; then
        PID=$(get_pid $1)
        echo "Остановка сервиса $1: $PID"
        kill $PID
        
        # Ожидаем остановку сервиса
        COUNTER=10
        RES=1
        while : ; do
            sleep 1
            COUNTER=$((COUNTER-1))
            if find_proc $1; then
                RES=1
            else
                RES=0
                break
            fi
            if [ $COUNTER -eq 0 ]; then
                RES=1
                break
            fi
        done
        
        if [ $RES -eq 1 ]; then
            echo "Не получилось остановить сервис $1, останавливаем его принудительно"
            super_kill $1
            if [ find_proc $1 ]; then
                echo "Не получилось остановить сервис $1 даже принудительно!"
            else
                echo "OK"
            fi
        fi
    else
        echo "Cервер $1 не запущен."
    fi
}

start_service() {
    echo "Запуск сервиса $1.. "
    /nasladdin/bin/$1 &
}

start() {
    start_service "driver.sh"
}

stop() {
    stop_service "driver"
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    *)
        echo "Usage: service driver-services.sh {start|stop|restart}" >&2
        exit 3
        ;;
esac
