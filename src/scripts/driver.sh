#!/bin/bash
RETURN=1
while [ $RETURN -ne 0 ] ; do 
    DATE=$(date +"%m.%d.%y")
    TIME=$(date +"%T")
    echo "Run: $DATE - $TIME"  >> /nasladdin/driver.log  # Залогировать запуск.
    echo "Run: $DATE - $TIME"
    /nasladdin/bin/driver > /dev/null 2>&1;
    RETURN=$?
    DATE=$(date +"%m.%d.%y")
    TIME=$(date +"%T")
    echo "Stop: $DATE - $TIME return: $RETURN" >> /nasladdin/driver.log # Залогировать запуск
    echo "Stop: $DATE - $TIME return: $RETURN"
    sleep 3; # Подождать 3 секунды.
done
