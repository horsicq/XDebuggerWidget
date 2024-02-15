INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

DEFINES += USE_XPROCESS

HEADERS += \
    $$PWD/dw_breakpointswidget.h \
    $$PWD/dw_disasmview.h \
    $$PWD/dw_hexview.h \
    $$PWD/dw_processmemorymapwidget.h \
    $$PWD/dw_processmoduleswidget.h \
    $$PWD/dw_registersview.h \
    $$PWD/dw_stackview.h \
    $$PWD/dw_symbolswidget.h \
    $$PWD/dw_threadswidget.h \
    $$PWD/xdebuggerloaddialog.h \
    $$PWD/xdebuggeroptionswidget.h \
    $$PWD/xdebuggerwidget.h

SOURCES += \
    $$PWD/dw_breakpointswidget.cpp \
    $$PWD/dw_disasmview.cpp \
    $$PWD/dw_hexview.cpp \
    $$PWD/dw_processmemorymapwidget.cpp \
    $$PWD/dw_processmoduleswidget.cpp \
    $$PWD/dw_registersview.cpp \
    $$PWD/dw_stackview.cpp \
    $$PWD/dw_symbolswidget.cpp \
    $$PWD/dw_threadswidget.cpp \
    $$PWD/xdebuggerloaddialog.cpp \
    $$PWD/xdebuggeroptionswidget.cpp \
    $$PWD/xdebuggerwidget.cpp

FORMS += \
    $$PWD/xdebuggerloaddialog.ui \
    $$PWD/xdebuggeroptionswidget.ui \
    $$PWD/xdebuggerwidget.ui

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

#!contains(XCONFIG, xdebugscript) {
#    XCONFIG += xdebugscript
#    include($$PWD/../XDebugScript/xdebugscript.pri)
#}

!contains(XCONFIG, xprocessmemorymapwidget) {
    XCONFIG += xprocessmemorymapwidget
    include($$PWD/../XProcessMemoryMapWidget/xprocessmemorymapwidget.pri)
}

!contains(XCONFIG, xprocessmoduleswidget) {
    XCONFIG += xprocessmoduleswidget
    include($$PWD/../XProcessModulesWidget/xprocessmoduleswidget.pri)
}

!contains(XCONFIG, xhandleswidget) {
    XCONFIG += xhandleswidget
    include($$PWD/../XHandlesWidget/xhandleswidget.pri)
}

!contains(XCONFIG, xcallstackwidget) {
    XCONFIG += xcallstackwidget
    include($$PWD/../XCallStackWidget/xcallstackwidget.pri)
}

!contains(XCONFIG, xsymbolswidget) {
    XCONFIG += xsymbolswidget
    include($$PWD/../XSymbolsWidget/xsymbolswidget.pri)
}

!contains(XCONFIG, xthreadswidget) {
    XCONFIG += xthreadswidget
    include($$PWD/../XThreadsWidget/xthreadswidget.pri)
}

!contains(XCONFIG, xbreakpointswidget) {
    XCONFIG += xbreakpointswidget
    include($$PWD/../XBreakPointsWidget/xbreakpointswidget.pri)
}

!contains(XCONFIG, xprocesswidgetadvanced) {
    XCONFIG += xprocesswidgetadvanced
    include($$PWD/../XProcessWidget/xprocesswidgetadvanced.pri)
}

!contains(XCONFIG, xinfodb) {
    XCONFIG += xinfodb
    include($$PWD/../XInfoDB/xinfodb.pri)
}

!contains(XCONFIG, allformatwidgets) {
    XCONFIG += allformatwidgets
    include($$PWD/../FormatWidgets/allformatwidgets.pri)
}

#TODO callstack

DISTFILES += \
    $$PWD/LICENSE \
    $$PWD/README.md
