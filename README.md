# elcus-1553-driver-linux
Копия драйвера с сайта elcus.ru, собирающаяся под ядра третьей версии (проверено на 3.16.7), дополнено примером.

Драйвер собирается с поддержкой потоков (приложения, использующие драйвер могут быть многопоточными)

Для сборки драйвера выполните

cd driver
chmod +x make3
./make3


В случае удачной компиляции можно загрузить драйвер используя скрипт loaddrv

./loaddrv


В этом случае будет автоматически создано устройство /dev/tmk1553b. Скрипт передает в модуль параметры 

d0=1 t0="TAI" misc=1 

, тем самым предназначен для работы с платами типа TA, для настройки других плат смотрите оригинальные readme

modify /etc/sudoers


ALL    ALL = (root) NOPASSWD: /absolute/path/to/your/install_and_load


Загрузка драйвера для TA1-PE2

/sbin/insmod /opt/tmk1553b.ko d0=1 e0=1 t0="TAI" d1=1 e1=2 t1="TAI" misc=1 && chmod o+rwx /dev/tmk1553b


Для простоты можно записать конфигурацию в файл /etc/modprobe.d/tmk1553b.conf


  alias pci:v000010b5d00009030sv*sd*bc*sc*i* tmk1553b
  
  options tmk1553b d0=1 t0="TAI" d1=1 t1="TAI" misc=1
  
  install tmk1553b /sbin/insmod /opt/tmk1553b.ko d0=1 e0=1 t0="TAI" d1=1 e1=2 t1="TAI" d2=1 e2=3 t2="TAI" d3=1 e3=4 t3="TAI" d4=2 e4=1 t4="TAI" d5=2 e5=2 t5="TAI" d6=2 e6=3 t6="TAI" d7=2 e7=4 t7="TAI"   misc=1 && chmod o+rwx /dev/tmk1553b

