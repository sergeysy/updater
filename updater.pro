TEMPLATE = subdirs
SUBDIRS += \
    updater/updater.pro \
    submodules/utils/utils.pro

CONFIG += ordered

utils.subdir     = submodules/utils
updater.subdir  = updater

updater.depends = utils
