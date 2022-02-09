/* Copyright (c) 2022 hors<horsicq@gmail.com>
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

//    g_scRun=nullptr;
//    g_scStepInto=nullptr;
//    g_scStepOver=nullptr;
    g_scBreakpointToggle=nullptr;

    g_mrCode={};
    g_mrStack={};
    g_mrHex={};
    g_pPDCode=nullptr;
    g_pPDStack=nullptr;
    g_pPDHex=nullptr;

    g_regOptions={};
    g_regOptions.bGeneral=true;
    g_regOptions.bIP=true;
    g_regOptions.bFlags=true;
    g_regOptions.bSegments=true;
    g_regOptions.bDebug=true;
    g_regOptions.bFloat=true;
    g_regOptions.bXMM=true;
    // TODO ARM
#ifdef Q_OS_WIN
#ifndef Q_OS_WIN64
    ui->widgetRegs->setOptions(XBinary::DM_X86_32,g_regOptions);
#else
    ui->widgetRegs->setOptions(XBinary::DM_X86_64,g_regOptions);
#endif
#endif

    connect(this,SIGNAL(showStatus()),this,SLOT(onShowStatus()));
    connect(this,SIGNAL(cleanUpSignal()),this,SLOT(cleanUp()));
    connect(this,SIGNAL(infoMessage(QString)),this,SLOT(infoMessageSlot(QString)));
    connect(this,SIGNAL(errorMessage(QString)),this,SLOT(errorMessageSlot(QString)));
//    ui->widgetDisasm->installEventFilter(this);
//    XDebuggerWidget::registerShortcuts(true);
    connect(ui->widgetDisasm,SIGNAL(debugAction(XAbstractDebugger::DEBUG_ACTION)),this,SLOT(addDebugAction(XAbstractDebugger::DEBUG_ACTION)));

    ui->tabWidgetMain->setCurrentIndex(MT_CPU);
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
    g_pDebugger=new XWindowsDebugger;
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

    g_osInfo=XProcess::getOsInfo();

    XAbstractDebugger::OPTIONS options={};

    options.bShowConsole=true;
    options.sFileName=sFileName;
    options.bBreakpointOnEntryPoint=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_ENTRYPOINT).toBool();
    options.bBreakPointOnDLLMain=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_DLLMAIN).toBool();
    options.bBreakPointOnTLSFunction=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_TLSFUNCTIONS).toBool();

    g_pDebugger->setOptions(options);

    connect(g_pThread,SIGNAL(started()),g_pDebugger,SLOT(process()));
//    connect(pDebugger,SIGNAL(finished()),pDebugger,SLOT(deleteLater()));

    connect(g_pDebugger,SIGNAL(eventCreateProcess(XAbstractDebugger::PROCESS_INFO*)),this,SLOT(onCreateProcess(XAbstractDebugger::PROCESS_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventEntryPoint(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onEntryPoint(XAbstractDebugger::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventStep(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onStep(XAbstractDebugger::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventStepInto(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onStepInto(XAbstractDebugger::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventStepOver(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onStepOver(XAbstractDebugger::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventBreakPoint(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitProcess(XAbstractDebugger::EXITPROCESS_INFO*)),this,SLOT(onExitProcess(XAbstractDebugger::EXITPROCESS_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventCreateThread(XAbstractDebugger::THREAD_INFO*)),this,SLOT(eventCreateThread(XAbstractDebugger::THREAD_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitThread(XAbstractDebugger::EXITTHREAD_INFO*)),this,SLOT(eventExitThread(XAbstractDebugger::EXITTHREAD_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO*)),this,SLOT(eventLoadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO*)),this,SLOT(eventUnloadSharedObject(XAbstractDebugger::SHAREDOBJECT_INFO*)),Qt::DirectConnection);

    g_pDebugger->moveToThread(g_pThread);
    g_pThread->start();

    return false;
}

void XDebuggerWidget::setGlobal(XShortcuts *pShortcuts,XOptions *pXOptions)
{
    setActive(true);

    ui->widgetDisasm->setGlobal(pShortcuts,pXOptions);
    ui->widgetHex->setGlobal(pShortcuts,pXOptions);
    ui->widgetRegs->setGlobal(pShortcuts,pXOptions);
    ui->widgetStack->setGlobal(pShortcuts,pXOptions);

    XShortcutsWidget::setGlobal(pShortcuts,pXOptions);
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
#ifdef QT_DEBUG
    qDebug("ExceptionAddress %llx",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs

    XAbstractDebugger::suspendThread(pBreakPointInfo->handleIDThread);

    emit showStatus();
}

void XDebuggerWidget::onEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("EntryPoint %x",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->handleIDThread);

    emit showStatus();
}

void XDebuggerWidget::onStep(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("Step %x",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->handleIDThread);

    emit showStatus();
}

void XDebuggerWidget::onStepInto(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("StepInto %llx",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->handleIDThread);

    emit showStatus();
}

void XDebuggerWidget::onStepOver(XAbstractDebugger::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("StepOver %llx",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs
    XAbstractDebugger::suspendThread(pBreakPointInfo->handleIDThread);

    emit showStatus();
}

void XDebuggerWidget::onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *pExitProcessInfo)
{
    // TODO more info
    QString sString=QString("%1 PID: %2").arg(tr("Process exited"),QString::number(pExitProcessInfo->nProcessID));

    emit infoMessage(sString);

    emit cleanUpSignal();

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
    QString sString=QString("%1: %2").arg(tr("Shared object unloaded"),pSharedObjectInfo->sFileName); // TODO rewrite

    emit infoMessage(sString);
}

void XDebuggerWidget::onShowStatus()
{
    // TODO getMemoryRegions

    if(!XBinary::isAddressInMemoryRegion(&g_mrCode,g_currentBreakPointInfo.nAddress))
    {
        g_mrCode=XProcess::getMemoryRegion(g_currentBreakPointInfo.handleIDProcess,g_currentBreakPointInfo.nAddress);

        if(g_pPDCode)
        {
            delete g_pPDCode;
            g_pPDCode=nullptr;
        }

        g_pPDCode=new XProcessDevice(this);

        if(g_pPDCode->openHandle(g_currentBreakPointInfo.handleIDProcess.hHandle,g_mrCode.nAddress,g_mrCode.nSize,QIODevice::ReadOnly)) // Windows TODO Linux
        {
            XBinary binary(g_pPDCode,true,g_mrCode.nAddress);
            binary.setArch(g_osInfo.sArch);
            binary.setMode(g_osInfo.mode);

            XDisasmView::OPTIONS disasmOptions={};
            disasmOptions.nInitAddress=g_currentBreakPointInfo.nAddress;
            disasmOptions.nCurrentIPAddress=g_currentBreakPointInfo.nAddress;
            disasmOptions.memoryMap=binary.getMemoryMap();
            ui->widgetDisasm->setData(g_pPDCode,disasmOptions);
        }
    }
    else
    {
        ui->widgetDisasm->setCurrentIPAddress(g_currentBreakPointInfo.nAddress);
        ui->widgetDisasm->goToAddress(g_currentBreakPointInfo.nAddress,true);
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

    QMap<QString, XBinary::XVARIANT> mapRegisters=g_pDebugger->getRegisters(g_currentBreakPointInfo.handleIDThread,g_regOptions); // TODO move to RegisterWidget
    ui->widgetRegs->setData(&mapRegisters);

    qint64 nStackPointer=g_pDebugger->getStackPointer(g_currentBreakPointInfo.handleIDThread);

    if(!XBinary::isAddressInMemoryRegion(&g_mrStack,nStackPointer))
    {
        g_mrStack=XProcess::getMemoryRegion(g_currentBreakPointInfo.handleIDProcess,nStackPointer);

        if(g_pPDStack)
        {
            delete g_pPDStack;
            g_pPDStack=nullptr;
        }

        g_pPDStack=new XProcessDevice(this);

        if(g_pPDStack->openHandle(g_currentBreakPointInfo.handleIDProcess.hHandle,g_mrStack.nAddress,g_mrStack.nSize,QIODevice::ReadOnly))
        {
            XStackView::OPTIONS stackOptions={};
            stackOptions.nStartAddress=g_mrStack.nAddress;
            stackOptions.nCurrentAddress=nStackPointer;
            ui->widgetStack->setData(g_pPDStack,stackOptions);

            XHexView::OPTIONS hexOptions={};
            hexOptions.nStartAddress=g_mrStack.nAddress;
            ui->widgetHex->setData(g_pPDStack,hexOptions);
        }
    }
    else
    {
        ui->widgetStack->goToAddress(nStackPointer);
    }

    ui->widgetStack->setSelectionAddress(nStackPointer);
    ui->widgetStack->showStatus(); // TODO do not reload data mb parameter
}

void XDebuggerWidget::on_toolButtonRun_clicked()
{
    debugRun();
}

void XDebuggerWidget::on_toolButtonStepInto_clicked()
{
    debugStepInto();
}

void XDebuggerWidget::on_toolButtonStepOver_clicked()
{
    debugStepOver();
}

void XDebuggerWidget::debugRun()
{
    if(g_currentBreakPointInfo.handleIDThread.nID)
    {
        XAbstractDebugger::resumeThread(g_currentBreakPointInfo.handleIDThread);
    }
}

void XDebuggerWidget::debugStepInto()
{
    if(g_currentBreakPointInfo.handleIDThread.nID)
    {
        g_pDebugger->stepInto(g_currentBreakPointInfo.handleIDThread);
        XAbstractDebugger::resumeThread(g_currentBreakPointInfo.handleIDThread);
    }
}

void XDebuggerWidget::debugStepOver()
{
    if(g_currentBreakPointInfo.handleIDThread.nID)
    {
        g_pDebugger->stepOver(g_currentBreakPointInfo.handleIDThread);
        XAbstractDebugger::resumeThread(g_currentBreakPointInfo.handleIDThread);
    }
}

void XDebuggerWidget::viewCPU()
{
    ui->tabWidgetMain->setCurrentIndex(MT_CPU);
}

void XDebuggerWidget::viewActions()
{
    ui->tabWidgetMain->setCurrentIndex(MT_ACTIONS);
}

void XDebuggerWidget::viewLog()
{
    ui->tabWidgetMain->setCurrentIndex(MT_LOG);
}

void XDebuggerWidget::viewBreakpoints()
{
    ui->tabWidgetMain->setCurrentIndex(MT_BREAKPOINTS);
}

void XDebuggerWidget::viewMemoryMap()
{
    ui->tabWidgetMain->setCurrentIndex(MT_MEMORYMAP);
}

void XDebuggerWidget::viewCallstack()
{
    ui->tabWidgetMain->setCurrentIndex(MT_CALLSTACK);
}

void XDebuggerWidget::viewThreads()
{
    ui->tabWidgetMain->setCurrentIndex(MT_THREADS);
}

void XDebuggerWidget::viewHandles()
{
    ui->tabWidgetMain->setCurrentIndex(MT_HANDLES);
}

void XDebuggerWidget::_toggleBreakpoint()
{
    ui->widgetDisasm->_breakpointToggle();
}

void XDebuggerWidget::cleanUp()
{
#ifdef QT_DEBUG
    qDebug("void XDebuggerWidget::cleanUp()");
#endif

    ui->tabWidgetMain->setCurrentIndex(MT_CPU);

    ui->widgetDisasm->clear();
    ui->widgetHex->clear();
    ui->widgetRegs->clear();
    ui->widgetStack->clear();

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
    #ifdef QT_DEBUG
        qDebug("registerShortcuts");
    #endif
//        if(!g_scRun)                    g_scRun                     =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_RUN),                    this,SLOT(_run()));
//        if(!g_scStepInto)               g_scStepInto                =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPINTO),               this,SLOT(_stepInto()));
//        if(!g_scStepOver)               g_scStepOver                =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPOVER),               this,SLOT(_stepOver()));
        if(!g_scBreakpointToggle)        g_scBreakpointToggle         =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_DISASM_BREAKPOINTTOGGLE),      this,SLOT(_toggleBreakpoint()));
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("unregisterShortcuts");
    #endif
//        if(g_scRun)                     {delete g_scRun;                    g_scRun=nullptr;}
//        if(g_scStepInto)                {delete g_scStepInto;               g_scStepInto=nullptr;}
//        if(g_scStepOver)                {delete g_scStepOver;               g_scStepOver=nullptr;}
        if(g_scBreakpointToggle)     {delete g_scBreakpointToggle;    g_scBreakpointToggle=nullptr;}
    }
}

void XDebuggerWidget::on_pushButtonActionsRecord_clicked()
{
    // TODO record/Stop actions
}

void XDebuggerWidget::addDebugAction(XAbstractDebugger::DEBUG_ACTION action)
{
#ifdef QT_DEBUG
    qDebug("addDebugAction");
#endif
    // TODO encode/decode functions for DEBUG_ACTION
    QString sResult=XAbstractDebugger::debugActionToString(action);

    ui->plainTextEditActions->appendPlainText(sResult);
}

void XDebuggerWidget::on_tabWidgetMain_currentChanged(int nIndex)
{
    if(nIndex==MT_MEMORYMAP)
    {
        if(g_pDebugger)
        {
            ui->widgetProcessMemoryMap->setData(g_pDebugger->getProcessInfo()->nProcessID); // TODO optimize
        }
    }
    else if(nIndex==MT_MODULES)
    {
        if(g_pDebugger)
        {
            ui->widgetProcessModules->setData(g_pDebugger->getProcessInfo()->nProcessID); // TODO optimize
        }
    }
}
