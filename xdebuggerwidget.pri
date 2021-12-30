INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
    $$PWD/xdebuggeroptionswidget.ui \
    $$PWD/xdebuggerwidget.ui

HEADERS += \
    $$PWD/dw_disasmview.h \
    $$PWD/dw_hexview.h \
    $$PWD/dw_processmemorymap.h \
    $$PWD/dw_registersview.h \
    $$PWD/dw_stackview.h \
    $$PWD/xdebuggeroptionswidget.h \
    $$PWD/xdebuggerwidget.h

SOURCES += \
    $$PWD/dw_disasmview.cpp \
    $$PWD/dw_hexview.cpp \
    $$PWD/dw_processmemorymap.cpp \
    $$PWD/dw_registersview.cpp \
    $$PWD/dw_stackview.cpp \
    $$PWD/xdebuggeroptionswidget.cpp \
    $$PWD/xdebuggerwidget.cpp

!contains(XCONFIG, xspecdebugger) {
    XCONFIG += xspecdebugger
    include($$PWD/../XSpecDebugger/xspecdebugger.pri)
}

!contains(XCONFIG, xdisasmview) {
    XCONFIG += xdisasmview
    include($$PWD/../XDisasmView/xdisasmview.pri)
}

!contains(XCONFIG, xhexview) {
    XCONFIG += xhexview
    include($$PWD/../XHexView/xhexview.pri)
}

!contains(XCONFIG, xregistersview) {
    XCONFIG += xregistersview
    include($$PWD/../XRegistersView/xregistersview.pri)
}

!contains(XCONFIG, xstackview) {
    XCONFIG += xstackview
    include($$PWD/../XStackView/xstackview.pri)
}

!contains(XCONFIG, xdebugscript) {
    XCONFIG += xdebugscript
    include($$PWD/../XDebugScript/xdebugscript.pri)
}

!contains(XCONFIG, xprocessmemorymapwidget) {
    XCONFIG += xprocessmemorymapwidget
    include($$PWD/../XProcessMemoryMapWidget/xprocessmemorymapwidget.pri)
}

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md
