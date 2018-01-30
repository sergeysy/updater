#!/bin/sh

# "./images/Application"
SOURCE=$1

#/home/savin/UpdateValidator/
DESTINITION_FOLDER=$2


rsync -avv --delete-during -compress-level=9 -e "ssh -p 22"  kartel@192.168.50.105:${SOURCE} ${DESTINITION_FOLDER}

exit $?