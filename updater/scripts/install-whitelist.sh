#!/bin/sh

PATH_SCRIPT=`dirname "$0"`
source ${PATH_SCRIPT}/config.sh

#export $(cat config.sh | grep -v ^# | xargs)

LOGIN=$1
IP_ADDRESS=$2
SOURCE_DB=$3

ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "mkdir -p ${PATH_INSTALL}" && \
scp -oStrictHostKeyChecking=no -r ${SOURCE_DB} ${LOGIN}@${IP_ADDRESS}:/${PATH_DB}

exit $?