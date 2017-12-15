TEMPLATE = app
VERSION = 1.0.0.0

TARGET = updater
CONFIG += c++14
# QMAKE_CXXFLAGS += -std=c++14

unix:!macx: LIBS += -L$$ROOT_DIR/3rd_party/ZipLib/Bin/ -l:libzip.a

INCLUDEPATH += $$ROOT_DIR/3rd_party/ZipLib/Source/ZipLib
DEPENDPATH += $$ROOT_DIR/3rd_party/ZipLib/Source


INCLUDEPATH += \
	./pinger \
        "$$LIB_UTILS_DIR/include" \
        "$$LIB_TRANSPORT_DIR/include" \
        "$$LIB_FACADE_STORAGE_TRANSACTIONS_DIR" \

win32 {
    INCLUDEPATH += $(BOOST_ROOT)
}
message($$INCLUDEPATH)
message($$LIB_FACADE_STORAGE_TRANSACTIONS_DIR)

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
    detectorvalidator.hpp \
    gui/changeiddialog.h \
    commands/commands.hpp

SOURCES += ./main.cpp \
	./updater.cpp \
	./pinger/pinger.cpp \ 
    model/validatorlistmodel.cpp \
    detectorvalidator.cpp \
    gui/changeiddialog.cpp \
    commands/commands.cpp
#	./settingsApplication/settings.ini

FORMS += ./updater.ui \
    gui/changeiddialog.ui

# RESOURCES += updater.qrc

LIBS += -L"$$ROOT_DIR/build/$$CONFIGURATION/$$LIBUTILS"
LIBS += -l"$$LIBUTILS"
LIBS += -L"$$ROOT_DIR/build/$$CONFIGURATION/$$LIBTRANSPORT"
LIBS += -l"$$LIBTRANSPORT"
LIBS += -L"$$ROOT_DIR/build/$$CONFIGURATION/$$LIBFACADE_STORAGE_TRANSACTIONS"
LIBS += -l"$$LIBFACADE_STORAGE_TRANSACTIONS"

message($$LIBS)
message($$LIBUTILS)
win32 {
LIBS += -L"$(BOOST_ROOT)/lib/x32/lib"
#LIBS += -llibboost_system-vc140-mt-gd-1_64

}

unix {
LIBS += -l:libboost_system.a
LIBS += -l:libboost_filesystem.a
LIBS += -l:libboost_date_time.a
LIBS += -l:libboost_thread.a
LIBS += -l:libboost_regex.a
LIBS += -l:libboost_program_options.a
LIBS += -lcurl
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
QMAKE_POST_LINK += "cp $$_PRO_FILE_PWD_/settingsApplication/settings.ini $$DESTDIR/$$TARGET.ini;"
QMAKE_POST_LINK += "cp -r $$_PRO_FILE_PWD_/scripts $$DESTDIR/;"
QMAKE_POST_LINK += "cp -r $$_PRO_FILE_PWD_/*.qm $$DESTDIR/;"
}
