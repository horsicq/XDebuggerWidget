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
#include "xdebuggerwidget.h"
#include "ui_xdebuggerwidget.h"

XDebuggerWidget::XDebuggerWidget(QWidget *pParent) :
    XShortcutsWidget(pParent),
    ui(new Ui::XDebuggerWidget)
{
    ui->setupUi(this);

    g_pThread=nullptr;
    g_pDebugger=nullptr;

    g_currentBreakPointInfo={};

    g_scRun=nullptr;
    g_scStepInto=nullptr;
    g_scSetRemoveBreakpoint=nullptr;

    g_mrCode={};
    g_mrStack={};
    g_mrHex={};
    g_pPDCode=nullptr;
    g_pPDStack=nullptr;
    g_pPDHex=nullptr;

    ui->widgetRegs->setMode(XRegistersView::MODE_X86_32);

    connect(this,SIGNAL(showStatus()),this,SLOT(onShowStatus()));
    connect(this,SIGNAL(infoMessage(QString)),this,SLOT(infoMessageSlot(QString)));
    connect(this,SIGNAL(errorMessage(QString)),this,SLOT(errorMessageSlot(QString)));
//    ui->widgetDisasm->installEventFilter(this);
//    XDebuggerWidget::registerShortcuts(true);
    connect(ui->widgetDisasm,SIGNAL(debugAction(XAbstractDebugger::DEBUG_ACTION)),this,SLOT(addDebugAction(XAbstractDebugger::DEBUG_ACTION)));
}

XDebuggerWidget::~XDebuggerWidget()
{
//    qDebug("XDebuggerWidget::~XDebuggerWidget()");

    cleanUp();

    delete ui;
}

bool XDebuggerWidget::loadFile(QString sFileName)
{
    cleanUp();

    g_pThread=new QThread;
#ifdef Q_OS_WIN
    g_pDebugger=new XWindowsDebugger; // TODO CleanUP !!!
#endif
#ifdef Q_OS_LINUX
    g_pDebugger=new XLinuxDebugger;
#endif
#ifdef Q_OS_OSX
    g_pDebugger=new XOSXDebugger;
#endif
    ui->widgetDisasm->setDebugger(g_pDebugger);
    ui->widgetHex->setDebugger(g_pDebugger);
    ui->widgetRegs->setDebugger(g_pDebugger);
    ui->widgetStack->setDebugger(g_pDebugger);

    XAbstractDebugger::OPTIONS options={};

    options.bShowConsole=true;
    options.sFileName=sFileName;
    options.bBreakpointOnTargetEntryPoint=true;

    g_pDebugger->setOptions(options);

    connect(g_pThread, SIGNAL(started()), g_pDebugger, SLOT(process()));
//    connect(pDebugger, SIGNAL(finished()), pDebugger, SLOT(deleteLater()));

    connect(g_pDebugger,SIGNAL(eventCreateProcess(XAbstractDebugger::PROCESS_INFO *)),this,SLOT(onCreateProcess(XAbstractDebugger::PROCESS_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *)),this,SLOT(onEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventStep(XAbstractDebugger::BREAKPOINT_INFO *)),this,SLOT(onStep(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *)),this,SLOT(onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitProcess(XAbstractDebugger::EXITPROCESS_INFO *)),this,SLOT(onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventCreateThread(XAbstractDebugger::THREAD_INFO *)),this,SLOT(eventCreateThread(XAbstractDebugger::THREAD_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitThread(XAbstractDebugger::EXITTHREAD_INFO *)),this,SLOT(eventExitThread(XAbstractDebugger::EXITTHREAD_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *)),this,SLOT(eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *)),this,SLOT(eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *)),Qt::DirectConnection);

    g_pDebugger->moveToThread(g_pThread);
    g_pThread->start();

    return false;
}

void XDebuggerWidget::setShortcuts(XShortcuts *pShortcuts)
{
    setActive(true);

    ui->widgetDisasm->setShortcuts(pShortcuts);
    ui->widgetHex->setShortcuts(pShortcuts);
    ui->widgetRegs->setShortcuts(pShortcuts);
    ui->widgetStack->setShortcuts(pShortcuts);

    XShortcutsWidget::setShortcuts(pShortcuts);
}

void XDebuggerWidget::onCreateProcess(XAbstractDebugger::PROCESS_INFO *pProcessInfo)
{
    // TODO more info
    QString sString=QString("%1 PID: %2").arg(tr("Process created"),QString::number(pProcessInfo->nProcessID));

    emit infoMessage(sString);
}

void XDebuggerWidget::onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;

    qDebug("ExceptionAddress %x",pBreakPointInfo->nAddress);

    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->hThread);

    emit showStatus();
}

void XDebuggerWidget::onEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;

    qDebug("EntryPoint %x",pBreakPointInfo->nAddress);

    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->hThread);

    emit showStatus();
}

void XDebuggerWidget::onStep(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;

    qDebug("Step %x",pBreakPointInfo->nAddress);

    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->hThread);

    emit showStatus();
}

void XDebuggerWidget::onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *pExitProcessInfo)
{
    // TODO more info
    QString sString=QString("%1 PID: %2").arg(tr("Process exited"),QString::number(pExitProcessInfo->nProcessID));

    emit infoMessage(sString);

    // TODO Clear screen
}

void XDebuggerWidget::eventCreateThread(XAbstractDebugger::THREAD_INFO *pThreadInfo)
{
    // TODO more info
    QString sString=QString("%1 ThreadID: %2").arg(tr("Thread created"),QString::number(pThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventExitThread(XAbstractDebugger::EXITTHREAD_INFO *pExitThreadInfo)
{
    // TODO more info
    QString sString=QString("%1 ThreadID: %2").arg(tr("Thread exited"),QString::number(pExitThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString=QString("%1: %2").arg(tr("Shared object loaded"),pSharedObjectInfo->sFileName);

    emit infoMessage(sString);
}

void XDebuggerWidget::eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString=QString("%1: %2").arg(tr("Shared object unloaded"),pSharedObjectInfo->sFileName);

    emit infoMessage(sString);
}

void XDebuggerWidget::onShowStatus()
{
    if(!XProcess::isAddressInMemoryRegion(&g_mrCode,g_currentBreakPointInfo.nAddress))
    {
        g_mrCode=XProcess::getMemoryRegion(g_currentBreakPointInfo.hProcess,g_currentBreakPointInfo.nAddress);

        if(g_pPDCode)
        {
            delete g_pPDCode;
            g_pPDCode=nullptr;
        }

        g_pPDCode=new XProcessDevice(this);

        if(g_pPDCode->openHandle(g_currentBreakPointInfo.hProcess,g_mrCode.nAddress,g_mrCode.nSize,QIODevice::ReadOnly))
        {
            XBinary binary(g_pPDCode,true,g_mrCode.nAddress);
            binary.setArch(g_pDebugger->getArch());
            binary.setMode(g_pDebugger->getMode());

            XDisasmView::OPTIONS disasmOptions={};
            disasmOptions.nInitAddress=g_currentBreakPointInfo.nAddress;
            disasmOptions.nCurrentIPAddress=g_currentBreakPointInfo.nAddress;
            disasmOptions.memoryMap=binary.getMemoryMap();
            disasmOptions.bHideOffset=true;
            ui->widgetDisasm->setData(g_pPDCode,disasmOptions);
        }
    }
    else
    {
        ui->widgetDisasm->setCurrentIPAddress(g_currentBreakPointInfo.nAddress);
        ui->widgetDisasm->goToAddress(g_currentBreakPointInfo.nAddress);
    }

    ui->widgetDisasm->setSelectionAddress(g_currentBreakPointInfo.nAddress,1);
    ui->widgetDisasm->showStatus(); // TODO do not reload data mb parameter

//    // TODO address cache
//    XProcessDevice *pPDCode=new XProcessDevice(this);
//    if(pPDCode->openHandle(currentBreakPointInfo.hProcess,g_mrCode.nAddress,g_mrCode.nSize,QIODevice::ReadOnly))
//    {


////        XHexView::OPTIONS hexOptions={};
////        hexOptions.nStartAddress=mrCode.nAddress;

////        ui->widgetHex->setData(pPDCode,hexOptions);
////        ui->widgetHex->goToAddress(currentBreakPointInfo.nAddress);
//    }

    QMap<QString, QVariant> mapRegisters=g_pDebugger->getRegisters(g_currentBreakPointInfo.hThread);
    ui->widgetRegs->setData(&mapRegisters);

    qint64 nESP=mapRegisters.value("ESP").toLongLong();

    if(!XProcess::isAddressInMemoryRegion(&g_mrStack,nESP))
    {
        g_mrStack=XProcess::getMemoryRegion(g_currentBreakPointInfo.hProcess,nESP);

        if(g_pPDStack)
        {
            delete g_pPDStack;
            g_pPDStack=nullptr;
        }

        g_pPDStack=new XProcessDevice(this);

        if(g_pPDStack->openHandle(g_currentBreakPointInfo.hProcess,g_mrStack.nAddress,g_mrStack.nSize,QIODevice::ReadOnly))
        {
            XStackView::OPTIONS stackOptions={};
            stackOptions.nStartAddress=g_mrStack.nAddress;
            stackOptions.nCurrentAddress=nESP;
            ui->widgetStack->setData(g_pPDStack,stackOptions);

            XHexView::OPTIONS hexOptions={};
            hexOptions.nStartAddress=g_mrStack.nAddress;
            ui->widgetHex->setData(g_pPDStack,hexOptions);
        }
    }
    else
    {
        ui->widgetStack->goToAddress(nESP);
    }

    ui->widgetStack->setSelectionAddress(nESP);
    ui->widgetStack->showStatus(); // TODO do not reload data mb parameter
}

void XDebuggerWidget::on_pushButtonRun_clicked()
{
    _run();
}

void XDebuggerWidget::on_pushButtonStep_clicked()
{
    _stepInto();
}

void XDebuggerWidget::_run()
{
    if(g_currentBreakPointInfo.hThread)
    {
        XAbstractDebugger::resumeThread(g_currentBreakPointInfo.hThread);
    }
}

void XDebuggerWidget::_stepInto()
{
    if(g_currentBreakPointInfo.hThread)
    {
        g_pDebugger->setSingleStep(g_currentBreakPointInfo.hThread);
        XAbstractDebugger::resumeThread(g_currentBreakPointInfo.hThread);
    }
}

void XDebuggerWidget::_setRemoveBreakpoint()
{
    ui->widgetDisasm->_setRemoveBreakpoint();
}

void XDebuggerWidget::cleanUp()
{
    if(g_pDebugger&&g_pThread)
    {
        g_pDebugger->stop();

        g_pThread->quit();
        g_pThread->wait();

        delete g_pThread;
        delete g_pDebugger;
        g_pThread=nullptr;
        g_pDebugger=nullptr;
    }

    g_mrCode={};
    g_mrStack={};
    g_mrHex={};

    if(g_pPDCode)
    {
        delete g_pPDCode;
        g_pPDCode=nullptr;
    }

    if(g_pPDStack)
    {
        delete g_pPDStack;
        g_pPDStack=nullptr;
    }

    if(g_pPDHex)
    {
        delete g_pPDHex;
        g_pPDHex=nullptr;
    }
}

void XDebuggerWidget::errorMessageSlot(QString sErrorMessage)
{
    // TODO
    writeToLog(sErrorMessage);
}

void XDebuggerWidget::infoMessageSlot(QString sInfoMessage)
{
    // TODO
    writeToLog(sInfoMessage);
}

void XDebuggerWidget::writeToLog(QString sText)
{
    ui->plainTextEditLog->appendPlainText(sText);
}

void XDebuggerWidget::registerShortcuts(bool bState)
{
    // TODO more
    // TODO Check if main window contains the shortcuts

    if(bState)
    {
        qDebug("registerShortcuts"); // TODO remove
        if(!g_scRun)                    g_scRun                     =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_RUN),                    this,SLOT(_run()));
        if(!g_scStepInto)               g_scStepInto                =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPINTO),               this,SLOT(_stepInto()));
        if(!g_scSetRemoveBreakpoint)    g_scSetRemoveBreakpoint     =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_SETREMOVEBREAKPOINT),    this,SLOT(_setRemoveBreakpoint()));
    }
    else
    {
        qDebug("unregisterShortcuts"); // TODO remove
        if(g_scRun)                     {delete g_scRun;                    g_scRun=nullptr;}
        if(g_scStepInto)                {delete g_scStepInto;               g_scStepInto=nullptr;}
        if(g_scSetRemoveBreakpoint)     {delete g_scSetRemoveBreakpoint;    g_scSetRemoveBreakpoint=nullptr;}
    }
}

void XDebuggerWidget::on_pushButtonActionsRecord_clicked()
{
    // TODO record/Stop actions
}

void XDebuggerWidget::addDebugAction(XAbstractDebugger::DEBUG_ACTION action)
{
    qDebug("addDebugAction");
    // TODO encode/decode functions for DEBUG_ACTION
    QString sResult=XAbstractDebugger::debugActionToString(action);

    ui->plainTextEditActions->appendPlainText(sResult);
}
