QT += core

CONFIG += c++20 console

SOURCES += \
    ./Sources/Exception.cpp \
    ./Sources/Interpreter.cpp \
    ./Sources/Parser.cpp \
    ./Sources/Source.cpp \
    ./Sources/Token.cpp \
    ./Sources/VarStorage.cpp

HEADERS += \
    ./Include/Exception.hpp \
    ./Include/Interpreter.hpp \
    ./Include/Parser.hpp \
    ./Include/Token.hpp \
    ./Include/VarStorage.hpp \
    ./Include/Guard.hpp \
    ./Include/Operator.hpp

macx {
    QMAKE_CXXFLAGS += -std=c++20
    QMAKE_CXXFLAGS += -stdlib=libc++
    QMAKE_LFLAGS += -stdlib=libc++
}
