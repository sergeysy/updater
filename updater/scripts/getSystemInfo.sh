#!/bin/sh

LOGIN=$1
IP_ADDRESS=$2
DEST_FOLDER="$3"

ssh-keyscan -t ecdsa ${IP_ADDRESS} >> ~/.ssh/known_hosts
ssh -oStrictHostKeyChecking=no -q ${LOGIN}@${IP_ADDRESS} echo
ssh -oStrictHostKeyChecking=no ${LOGIN}@${IP_ADDRESS} exec > /validator/settings/systeminfo;uname -or &&  grep -E 'MemTotal|MemFree' /proc/meminfo | grep -oE '[0-9]+.*' && ls /mnt/dom/transaction -1 | wc -l
scp -oStrictHostKeyChecking=no -r ${LOGIN}@${IP_ADDRESS}:/validator/settings/ ${DEST_FOLDER}/${IP_ADDRESS}