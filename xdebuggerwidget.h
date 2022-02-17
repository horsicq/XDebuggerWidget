/* Copyright (c) 2021-2022 hors<horsicq@gmail.com>
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
#ifdef Q_OS_OSX
#include "xosxdebugger.h"
#endif
#include "xshortcutswidget.h"

// TODO WinAPI page/ Widget

namespace Ui {
class XDebuggerWidget;
}

class XDebuggerWidget : public XShortcutsWidget
{
    Q_OBJECT

    enum MT
    {
        MT_CPU=0,
        MT_ACTIONS,
        MT_LOG,
        MT_BREAKPOINTS,
        MT_MEMORYMAP,
        MT_CALLSTACK,
        MT_THREADS,
        MT_HANDLES,
        MT_MODULES
    };

public:
    explicit XDebuggerWidget(QWidget *pParent=nullptr);
    ~XDebuggerWidget();

    bool loadFile(QString sFileName);
    void setGlobal(XShortcuts *pShortcuts,XOptions *pXOptions);
    virtual void adjustView();

signals:
    void showStatus();
    void errorMessage(QString sErrorMessage);
    void infoMessage(QString sInfoMessage);
    void cleanUpSignal();

public slots:
    void debugRun();
    void debugStepInto();
    void debugStepOver();
    void viewCPU();
    void viewActions();
    void viewLog();
    void viewBreakpoints();
    void viewMemoryMap();
    void viewCallstack();
    void viewThreads();
    void viewHandles();

private slots:
    void onCreateProcess(XAbstractDebugger::PROCESS_INFO *pProcessInfo);
    void onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onProcessEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onProgramEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onStep(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onStepInto(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onStepOver(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *pExitProcessInfo);
    void eventCreateThread(XAbstractDebugger::THREAD_INFO *pThreadInfo);
    void eventExitThread(XAbstractDebugger::EXITTHREAD_INFO *pExitThreadInfo);
    void eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *pSharedObjectInfo);
    void onShowStatus();
    void on_toolButtonRun_clicked();
    void on_toolButtonStepInto_clicked();
    void on_toolButtonStepOver_clicked();

    void _toggleBreakpoint();

    void cleanUp();

    void errorMessageSlot(QString sErrorMessage);
    void infoMessageSlot(QString sInfoMessage);
    void writeToLog(QString sText);
    void on_pushButtonActionsRecord_clicked();

    void addDebugAction(XAbstractDebugger::DEBUG_ACTION action);
    void on_tabWidgetMain_currentChanged(int nIndex);

protected:
    virtual void registerShortcuts(bool bState);

private:
    Ui::XDebuggerWidget *ui;
    QThread *g_pThread;
#ifdef Q_OS_WIN
    XWindowsDebugger *g_pDebugger;
#endif
#ifdef Q_OS_LINUX
    XLinuxDebugger *g_pDebugger;
#endif
#ifdef Q_OS_OSX
    XOSXDebugger *g_pDebugger;
#endif
    XAbstractDebugger::BREAKPOINT_INFO g_currentBreakPointInfo;

//    QShortcut *g_scRun;
//    QShortcut *g_scStepInto;
//    QShortcut *g_scStepOver;
    QShortcut *g_scBreakpointToggle;

    XBinary::OSINFO g_osInfo;

    XBinary::MEMORY_REGION g_mrCode;
    XBinary::MEMORY_REGION g_mrStack;
    XBinary::MEMORY_REGION g_mrHex;

    XProcessDevice *g_pPDCode;
    XProcessDevice *g_pPDStack;
    XProcessDevice *g_pPDHex;
    XAbstractDebugger::REG_OPTIONS g_regOptions;
};

#endif // XDEBUGGERWIDGET_H
