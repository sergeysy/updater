#!/bin/sh

PATH_SCRIPT=`dirname "$0"`
source ${PATH_SCRIPT}/config.sh

# may be is safety more. prevent execute config.sh, variable export only
#export $(cat config.sh | grep -v ^# | xargs)

LOGIN=$1
IP_ADDRESS=$2
VALUE_TO_FILE=$3

ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} "mkdir -p ${PATH_SETTINGS} && echo ${VALUE_TO_FILE} > ${PATH_SETTINGS}/client_id"

exit $?