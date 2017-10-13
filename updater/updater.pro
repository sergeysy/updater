# 2017-05-24 13:11 MSK 
# Init project

TEMPLATE = app

VERSION = 1.0.0.0

win32 {
DEFINES += CURL_STATICLIB
DEFINES += "_WIN32_WINNT=0x0501"
}
TARGET = updater

CONFIG += c++14



INCLUDEPATH +=  "$$LIB_UTILS_DIR/include" \
                "$$ROOT_DIR/updater"
win32 {
    INCLUDEPATH += c:/local/boost_1_61_0
}

CONFIG(debug, debug|release) {
    CONFIGURATION=debug
    DEFINES += DEBUG_ON
} else {
    CONFIGURATION=release
}

DESTDIR = "$$ROOT_DIR/build/$$CONFIGURATION/$$TARGET"

win32 {
#INCLUDEPATH += 	"$$ROOT_DIR/3rd_party/curl/include"
INCLUDEPATH += "$(BOOST_ROOT)"
}

message($$INCLUDEPATH)
OBJECTS_DIR = "$$DESTDIR/.obj"
MOC_DIR = "$$DESTDIR/.moc"
RCC_DIR = "$$DESTDIR/.qrc"
UI_DIR = "$$DESTDIR/.u"

TRANSLATIONS += "$$TARGET"_ru.ts

QT += core gui widgets
#QT +=  network

HEADERS +=	 \
		context.h \
		checkpoint_env.h \
		product_version.h \
		version_info.h \
		build_defs.h

SOURCES +=	\
		main.cpp \
		context.cpp \
		signal_setter.cpp \
		build_timestamp.cpp	\
		version_info.cpp \
                resources.cpp

FORMS +=	gui/MainWindow.ui

RESOURCES += resources/icons.qrc
#			translations.qrcit 

LIBS += -L"$$ROOT_DIR/build/$$CONFIGURATION/$$LIBUTILS"
win32 {
LIBS += -L"$(BOOST_ROOT)/lib/x32/lib"
# LIBS += -L"$$ROOT_DIR/3rd_party/curl/build/Win32/VC14/LIB Debug"
}

# Resolve circular dependencies among the libraries
unix {
#LIBS += -Wl,--start-group -l"$$LIBUTILS" -Wl,--end-group
#LIBS += -l"$$LIBSETTINGS"
#LIBS += -l"$$LIBTRANSPORT"
#LIBS += -l"$$LIBSQLITE"

LIBS += -lboost_system
#LIBS += -lboost_filesystem
#LIBS += -lboost_date_time
#LIBS += -lboost_thread
#LIBS += -lboost_regex
LIBS += -lboost_program_options
#LIBS += -lcrypto
}

win32|win64:debug {
#LIBS += -llibboost_system-vc140-mt-gd-1_64
#LIBS += -llibboost_filesystem-vc140-mt-gd-1_64
#LIBS += -llibboost_date_time-vc140-mt-gd-1_64
#LIBS += -llibboost_thread-vc140-mt-gd-1_64
#LIBS += -llibboost_regex-vc140-mt-gd-1_64
#LIBS += -llibboost_program_options-vc140-mt-gd-1_64
}

win32|win64:debug {
#LIBS += -llibcurld
#LIBS += -lWldap32
}
unix {
#LIBS += -lcurl
#LIBS += -loti
#LIBS += -lpdhw

LIBS += -ldl
LIBS += -lpthread
}

DEFINES += QT_NO_CAST_FROM_ASCII

#TRANSLATIONS = checkpoint_ru.ts

#QMAKE_CXXFLAGS += -s

# source code encoding
QMAKE_CXXFLAGS += -finput-charset="UTF-8"
unix {
QMAKE_CXXFLAGS += -Werror
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -g
# QMAKE_CXXFLAGS += -Wconversion
}

message($$STRIP)


#QMAKE_POST_LINK += "$$STRIP $$ROOT_DIR/build/$$CONFIGURATION/$$TARGET/$$TARGET"
