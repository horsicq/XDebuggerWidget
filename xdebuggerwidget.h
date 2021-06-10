// copyright (c) 2021 hors<horsicq@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#ifndef XDEBUGGERWIDGET_H
#define XDEBUGGERWIDGET_H

#include <QThread>
#ifdef Q_OS_WIN
#include "xwindowsdebugger.h"
#endif
#ifdef Q_OS_LINUX
#include "xlinuxdebugger.h"
#endif
#include "xshortcutswidget.h"

namespace Ui {
class XDebuggerWidget;
}

class XDebuggerWidget : public XShortcutsWidget
{
    Q_OBJECT

public:
    explicit XDebuggerWidget(QWidget *pParent=nullptr);
    ~XDebuggerWidget();

    bool loadFile(QString sFileName);
    void setShortcuts(XShortcuts *pShortcuts);

signals:
    void showStatus();

private slots:
    void onCreateProcess(XAbstractDebugger::PROCESS_INFO *pProcessInfo);
    void onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo);
    void onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *pExitProcessInfo);
    void onShowStatus();
    void on_pushButtonRun_clicked();
    void on_pushButtonStep_clicked();

    void _run();
    void _stepInto();
    void _setBreakpoint();

    void cleanUp();

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
    XAbstractDebugger::BREAKPOINT_INFO g_currentBreakPointInfo;

    QShortcut *g_scRun;
    QShortcut *g_scStepInto;
    QShortcut *g_scSetBreakpoint;

    XProcess::MEMORY_REGION g_mrCode;
    XProcess::MEMORY_REGION g_mrStack;
    XProcess::MEMORY_REGION g_mrHex;
    XProcessDevice *g_pPDCode;
    XProcessDevice *g_pPDStack;
    XProcessDevice *g_pPDHex;
};

#endif // XDEBUGGERWIDGET_H
