#!/bin/sh

PATH_SCRIPT=`dirname "$0"`
source ${PATH_SCRIPT}/config.sh

#export $(cat config.sh | grep -v ^# | xargs)

LOGIN=$1
IP_ADDRESS=$2
DEST_FOLDER="$3"

ssh-keyscan -t ecdsa ${IP_ADDRESS} >> ~/.ssh/known_hosts && \
ssh -oStrictHostKeyChecking=no -q ${LOGIN}@${IP_ADDRESS} echo && \
ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "exec > ${PATH_SETTINGS}/systeminfo;uname -or &&  grep -E 'MemTotal|MemFree' /proc/meminfo | grep -oE '[0-9]+.*' && ls ${PATH_TRANSACTIONS} -1 | wc -l" && \
rm -rf ${DEST_FOLDER}/${IP_ADDRESS} && \
scp -oStrictHostKeyChecking=no -r ${LOGIN}@${IP_ADDRESS}:${PATH_SETTINGS}/ ${DEST_FOLDER}/${IP_ADDRESS}
exit $?