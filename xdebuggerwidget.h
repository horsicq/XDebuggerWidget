/* Copyright (c) 2021-2023 hors<horsicq@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef XDEBUGGERWIDGET_H
#define XDEBUGGERWIDGET_H

#include <QThread>
#ifdef Q_OS_WIN
#include "xwindowsdebugger.h"
#endif
#ifdef Q_OS_LINUX
#include "xlinuxdebugger.h"
#endif
#ifdef Q_OS_MACOS
#include "xosxdebugger.h"
#endif
#include "xdebuggerconsole.h"
#include "xshortcutswidget.h"
#include "xinfodb.h"
#include "xdebuggerloaddialog.h"
#include "dialogxinfodbtransferprocess.h"

// TODO WinAPI page/ Widget
// TODO for Linux send signal to process widget/menu
// TODO Dialog on open Command line/ symbols

namespace Ui {
class XDebuggerWidget;
}

class XDebuggerWidget : public XShortcutsWidget {
    Q_OBJECT

    enum MT {
        MT_CPU = 0,
        MT_LOG,
        MT_BREAKPOINTS,
        MT_MEMORYMAP,
        MT_CALLSTACK,
        MT_THREADS,
        MT_HANDLES,
        MT_MODULES,
        MT_SYMBOLS
    };

    enum SHORTCUT {
        SC_DISASM_BREAKPOINTTOGGLE,
        __SC_SIZE,
        // TODO more
    };

    enum CM {
        CM_UNKNOWN = 0,
        CM_READY,
        CM_DEBUG_RUN,
        CM_DEBUG_CLOSE,
        CM_DEBUG_STEPINTO,
        CM_DEBUG_STEPOVER,
        CM_ANIMATE_STEPINTO,
        CM_ANIMATE_STEPOVER,
        CM_ANIMATE_STOP,
        CM_TRACE_STEPINTO,
        CM_TRACE_STEPOVER,
        CM_TRACE_STOP
    };

public:
    // TODO
    // TODO stateChanged -> update menus
    struct STATE {
        bool bAnimateStepInto;
        bool bAnimateStepOver;
        bool bAnimateStop;
        bool bTraceStepInto;
        bool bTraceStepOver;
        bool bTraceStop;
    };

    explicit XDebuggerWidget(QWidget *pParent = nullptr);
    ~XDebuggerWidget();

    bool loadFile(const QString &sFileName, bool bShowLoadDialog);
    void setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions);
    virtual void adjustView();
    virtual void reloadShortcuts();
    STATE getState();

signals:
    void errorMessage(QString sErrorMessage);
    void infoMessage(QString sInfoMessage);
    void resetWidgetsSignal();
    void stateChanged();
    void testSignal(X_ID nThreadId);  // TODO remove
    void addSymbols(QString sFileName, XADDR nImageBase);
    void removeSymbols(QString sFileName);
    //    void debugStepOverSignal();

public slots:
    bool debugRun();
    bool debugClose();
    bool debugStepInto();
    bool debugStepOver();
    bool animateStepInto();
    bool animateStepOver();
    bool animateStop();
    bool traceStepInto();
    bool traceStepOver();
    bool traceStop();
    bool command(XDebuggerWidget::CM commandMode);
    void viewCPU();
    void viewLog();
    void viewBreakpoints();
    void viewMemoryMap();
    void viewCallstack();
    void viewThreads();
    void viewHandles();
    void viewModules();
    void viewSymbols();

private:
    void _stateChanged();
    void _adjustState();
    void _clearProcessMemory();
    XProcess *_getProcessMemory(XADDR nAddress, qint64 nSize);

private slots:
    void onCreateProcess(XInfoDB::PROCESS_INFO *pProcessInfo);
    void onBreakPoint(XInfoDB::BREAKPOINT_INFO *pBreakPointInfo);
    void onExitProcess(XInfoDB::EXITPROCESS_INFO *pExitProcessInfo);
    void eventCreateThread(XInfoDB::THREAD_INFO *pThreadInfo);
    void eventExitThread(XInfoDB::EXITTHREAD_INFO *pExitThreadInfo);
    void eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void onReloadSignal(bool bDataReload);  // TODO Check remove
    void on_toolButtonRun_clicked();
    void on_toolButtonStepInto_clicked();
    void on_toolButtonStepOver_clicked();
    void _toggleBreakpoint();
    void cleanUp();
    void resetWidgets();
    void errorMessageSlot(QString sErrorMessage);
    void infoMessageSlot(QString sInfoMessage);
    void writeToLog(QString sText);
    void on_tabWidgetMain_currentChanged(int nIndex);
    void reload();
    void followInDisasm(XADDR nAddress);
    void followInHex(XADDR nAddress);
    void followInStack(XADDR nAddress);
    void on_toolButtonAnimateStepInto_clicked();
    void on_toolButtonAnimateStepOver_clicked();
    void on_toolButtonAnimateStop_clicked();
    void on_toolButtonTraceStepInto_clicked();
    void on_toolButtonTraceStepOver_clicked();
    void on_toolButtonTraceStop_clicked();
    //    void memoryRegionsListChangedSlot();
    void modulesListChangedSlot();
    void threadsListChangedSlot();
    void registersListChangedSlot();
    void updateWidget(MT mt);
    void on_pushButtonCommandRun_clicked();
    void addSymbolsSlot(QString sFileName, XADDR nImageBase);
    void removeSymbolsSlot(QString sFileName);

protected:
    virtual void registerShortcuts(bool bState);

private:
    Ui::XDebuggerWidget *ui;
#ifdef Q_OS_WIN
    QThread *g_pThread;
    XWindowsDebugger *g_pDebugger;
#endif
#ifdef Q_OS_LINUX
    XLinuxDebugger *g_pDebugger;
#endif
#ifdef Q_OS_MACOS
    XOSXDebugger *g_pDebugger;
#endif
    XInfoDB *g_pInfoDB;
    XInfoDB::BREAKPOINT_INFO g_currentBreakPointInfo;
    QShortcut *shortCuts[__SC_SIZE];
    XBinary::OSINFO g_osInfo;
    QList<XProcess *> g_listRegions;

    XProcess::MEMORY_REGION g_mrDisasm;
    XProcess::MEMORY_REGION g_mrStack;
    XProcess::MEMORY_REGION g_mrHex;

    STATE g_state;
    QTimer *g_pTimer;
    QMutex *g_pMutex;
};

#endif  // XDEBUGGERWIDGET_H
