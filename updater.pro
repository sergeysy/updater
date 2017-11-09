TEMPLATE = subdirs
SUBDIRS += \
    submodules/utils/utils.pro \
    submodules/transport/transport.pro \
    submodules/facadeStorageTransactions/facadeStorageTransactions.pro \
    updater/updater.pro \
    screen

CONFIG += ordered

utils.subdir     = submodules/utils
transport.subdir = submodules/transport
facadeStorageTransactions.subdir = submodules/facadeStorageTransactions
updater.subdir   = updater

updater.depends = utils transport facadeStorageTransactions

