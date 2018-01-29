#!/bin/sh

PATH_SCRIPT=`dirname "$0"`
source ${PATH_SCRIPT}/config.sh

#export $(cat config.sh | grep -v ^# | xargs)

LOGIN=$1
IP_ADDRESS=$2
SOURCE_APPLICATION=$3

ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "mkdir -p ${PATH_INSTALL}" && \
scp -oStrictHostKeyChecking=no -r ${SOURCE_APPLICATION} ${LOGIN}@${IP_ADDRESS}:/${PATH_INSTALL} && \
ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "unzip -o ${PATH_INSTALL}/*.zip -d ${PATH_INSTALL}" && \
ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "bash ${PATH_INSTALL}/install.sh"

exit $?