#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright &copy; 2016, Alfa Robotics.
# \brief  Асинхронно запускает файл на выполнение через заданные промежутки.
# \author Величко Р.Н.
# \date   04.03.2016

# Установить флаг исполняемого файла: chmod +x ./async_run.py
# Пример запуска: ./async_run.py -t 10 -f <имя файла> -v

import sys
import getopt
import xml.dom.minidom
import os
import re
import shutil
import threading
import time

from xml.dom.minidom import parse
from optparse import OptionParser
from subprocess import Popen


# Класс наследник от потокового класса
class AsyncRun(threading.Thread):
    # Конструктор
    def __init__(self, seconds, binary):
        threading.Thread.__init__(self)
        self.daemon = True
        self.seconds = seconds
        self.binary = binary

    # Потоковый метод
    def run(self):
        while True:
            time.sleep(float(self.seconds))
            proc = Popen(os.path.abspath(self.binary), shell=True, stdin=None, stdout=None, stderr=None, close_fds=True)


# Функция приложения
if __name__ == "__main__":
    # Чтение параметров приложения
    argv = sys.argv[1:]
    op = OptionParser(usage=u"Показать список параметров.")
    op.add_option("-t", "--timer", metavar="CODEC",
                help=u"CODEC: Интервал времени асинхронного запуска в секундах.")
    op.add_option("-b", "--binary", metavar="FILE",
                help=u"FILE: Запускаемый исполняемый файл.")
    (options, args) = op.parse_args()

    if options.timer is None and options.binary is None:
        print(u"Укажите исходный файл и задержку запуска")
    else:
        ar = AsyncRun(options.timer, options.binary)
        ar.start(); # Запуск потока
        ar.join(); # Ожидание завершения выполнения

