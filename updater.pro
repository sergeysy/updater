TEMPLATE = subdirs

SUBDIRS +=  utils \
	    updater

CONFIG += ordered

# where to find the sub projects - give the folders
utils.subdir     = submodules/utils
updater.subdir  = updater



# what subproject depends on others
#settings.depends = utils

##testtransaport.depends = transaport
#hardware.depends = utils ticketing
##transport.depends = utils
#application.depends = utils ticketing hardware settings transport sqlite


updater.depends = utils

