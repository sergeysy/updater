#!/bin/sh

PATH_SCRIPT=`dirname "$0"`
source ${PATH_SCRIPT}/config.sh

LOGIN=$1
IP_HOST=$2
DEST_FOLDER=%3

ssh-keyscan -t ecdsa ${IP_HOST} >> ~/.ssh/known_hosts && \
scp -oStrictHostKeyChecking=no -r ${LOGIN}@${IP_HOST}:${PATH_TRANSACTIONS} ${DEST_FOLDER} && \
ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_HOST} rm -f ${PATH_TRANSACTIONS}/*

exit $?