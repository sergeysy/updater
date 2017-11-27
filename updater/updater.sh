ssh-keyscan -t rsa 192.168.50.55 | ssh-keygen -lv --f
./updater &>> log.txt