include_directories(${CMAKE_CURRENT_LIST_DIR})


if (NOT DEFINED XPECDEBUGGER_SOURCES)
    include(${CMAKE_CURRENT_LIST_DIR}/../XSpecDebugger/xspecdebugger.cmake)
    set(XDEBUGGERWIDGET_SOURCES ${XDEBUGGERWIDGET_SOURCES} ${XPECDEBUGGER_SOURCES})
endif()


include(${CMAKE_CURRENT_LIST_DIR}/../XDisasmView/xdisasmview.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XHexView/xhexview.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XRegistersView/xregistersview.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XStackView/xstackview.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XProcessMemoryMapWidget/xprocessmemorymapwidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XProcessModulesWidget/xprocessmoduleswidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XHandlesWidget/xhandleswidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XCallStackWidget/xcallstackwidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XSymbolsWidget/xsymbolswidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XThreadsWidget/xthreadswidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XBreakPointsWidget/xbreakpointswidget.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XProcessWidget/xprocesswidgetadvanced.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../XInfoDB/xinfodb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../FormatWidgets/allformatwidgets.cmake)

#    ${XHANDLESWIDGET_SOURCES}
#    ${XCALLSTACKWIDGET_SOURCES}

set(XDEBUGGERWIDGET_SOURCES
    ${XDEBUGGERWIDGET_SOURCES}
    ${XPECDEBUGGER_SOURCES}
    ${XDISASMVIEW_SOURCES}
    ${XHEXVIEW_SOURCES}
    ${XREGISTERSVIEW_SOURCES}
    ${XSTACKVIEW_SOURCES}
    ${XPROCESSMEMORYMAPWIDGET_SOURCES}
    ${XPROCESSMODULESWIDGET_SOURCES}
    ${XSYMBOLSWIDGET_SOURCES}
    ${XTHREADSWIDGET_SOURCES}
    ${XBREAKPOINTSWIDGET_SOURCES}
    ${XPROCESSWIDGETADVANCED_SOURCES}
    ${XINFODB_SOURCES}
    ${ALLFORMATWIDGETS_SOURCES}
    ${CMAKE_CURRENT_LIST_DIR}/dw_breakpointswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_breakpointswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_disasmview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_disasmview.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_hexview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_hexview.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_processmemorymapwidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_processmemorymapwidget.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_processmoduleswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_processmoduleswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_registersview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_registersview.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_stackview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_stackview.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_symbolswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_symbolswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/dw_threadswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dw_threadswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerloaddialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerloaddialog.h
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerloaddialog.ui
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggeroptionswidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggeroptionswidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggeroptionswidget.ui
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerwidget.cpp
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerwidget.h
    ${CMAKE_CURRENT_LIST_DIR}/xdebuggerwidget.ui
)
