TEMPLATE = app
VERSION = 1.0.0.0

TARGET = updater
CONFIG += c++14

INCLUDEPATH += \
	./pinger \
	"$$LIB_UTILS_DIR/include" \

win32 {
    INCLUDEPATH += $(BOOST_ROOT)
}

CONFIG(debug, debug|release) {
    CONFIGURATION=debug
    DEFINES += DEBUG_ON
} else {
    CONFIGURATION=release
}
message($$CONFIGURATION)
DESTDIR = "$$ROOT_DIR/build/$$CONFIGURATION/$$TARGET"
message($$DESTDIR);
QT += core widgets gui network
#DEFINES += WIN64 QT_DLL QT_WIDGETS_LIB

DEPENDPATH += .
OBJECTS_DIR = "$$DESTDIR/.obj"
MOC_DIR = "$$DESTDIR/.moc"
RCC_DIR = "$$DESTDIR/.qrc"
UI_DIR = "$$DESTDIR/.u"

#MOC_DIR += ./GeneratedFiles/debug
#OBJECTS_DIR += debug
#UI_DIR += ./GeneratedFiles
#RCC_DIR += ./GeneratedFiles

win32:RC_FILE = updater.rc
HEADERS += ./updater.h \
	./pinger/ipv4_header.hpp \
	./pinger/icmp_header.hpp \
	./pinger/pinger.hpp \
    model/validatorlistmodel.hpp \
    detectorvalidator.hpp

SOURCES += ./main.cpp \
	./updater.cpp \
	./pinger/pinger.cpp \ 
    model/validatorlistmodel.cpp \
    detectorvalidator.cpp
#	./settingsApplication/settings.ini

FORMS += ./updater.ui

# RESOURCES += updater.qrc

LIBS += -L"$$ROOT_DIR/build/$$CONFIGURATION/$$LIBUTILS"
LIBS += -l"$$LIBUTILS"
message($$LIBS)
message($$LIBUTILS)
win32 {
LIBS += -L"$(BOOST_ROOT)/lib/x32/lib"
#LIBS += -llibboost_system-vc140-mt-gd-1_64

}

unix {
LIBS += -lboost_system
LIBS += -lboost_filesystem
LIBS += -lboost_date_time
LIBS += -lboost_thread
LIBS += -lboost_regex
LIBS += -lboost_program_options
}


DEFINES += QT_NO_CAST_FROM_ASCII

QMAKE_CXXFLAGS += -finput-charset="UTF-8"
unix {
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -g
# QMAKE_CXXFLAGS += -Wconversion
}

MYFILE = $$TARGET
message($$TARGET);
message($$MYFILE);
message($$DESTDIR);
win32 {
QMAKE_POST_LINK += "copy \"$$_PRO_FILE_PWD_\settingsApplication\settings.ini\" \"$$DESTDIR/$$MYFILE.ini\""
}

unix {
QMAKE_POST_LINK += "cp $$_PRO_FILE_PWD_/settingsApplication/settings.ini $$DESTDIR/$$TARGET.ini"
}
