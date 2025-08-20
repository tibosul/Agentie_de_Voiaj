QT += core network sql
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = Agentie_de_Voiaj_Server
TEMPLATE = app

# Define output directory
CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/debug
} else {
    DESTDIR = $$PWD/release
}

# Include paths
INCLUDEPATH += $$PWD/include
INCLUDEPATH += $$PWD/config

# Header files
HEADERS += \
    include/database/Database_Manager.h \
    include/network/Socket_Server.h \
    include/network/Client_Handler.h \
    include/network/Protocol_Handler.h \
    include/network/Network_Types.h \
    include/utils/utils.h \
    include/models/All_Data_Structures.h \
    config/config.h

# Source files
SOURCES += \
    src/core/main.cpp \
    src/database/Database_Manager.cpp \
    src/network/Socket_Server.cpp \
    src/network/Client_Handler.cpp \
    src/network/Protocol_Handler.cpp \
    src/utils/utils.cpp

# Additional dependencies for Windows compatibility
win32 {
    LIBS += -lws2_32
    DEFINES += WIN32_LEAN_AND_MEAN
}

# Compiler flags
QMAKE_CXXFLAGS += -std=c++17

# For cross-platform compatibility
unix:!macx {
    # Linux specific settings
}

macx {
    # macOS specific settings
}

# Define version and build info
DEFINES += \
    APP_VERSION=\\\"1.0.0\\\" \
    APP_NAME=\\\"Agentie_de_Voiaj_Server\\\"