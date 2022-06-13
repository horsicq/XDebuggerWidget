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
    g_pInfoDB=nullptr;

    g_currentBreakPointInfo={};

    memset(shortCuts,0,sizeof shortCuts);

    g_mrDisasm={};
    g_mrStack={};
    g_mrHex={};
    g_pPDDisasm=nullptr;
    g_pPDStack=nullptr;
    g_pPDHex=nullptr;

//    qRegisterMetaType<XBinary::PROCESS_STATUS>("XBinary::PROCESS_STATUS");

    connect(this,SIGNAL(cleanUpSignal()),this,SLOT(cleanUp()));
    connect(this,SIGNAL(infoMessage(QString)),this,SLOT(infoMessageSlot(QString)));
    connect(this,SIGNAL(errorMessage(QString)),this,SLOT(errorMessageSlot(QString)));
//    ui->widgetDisasm->installEventFilter(this);
//    XDebuggerWidget::registerShortcuts(true);
//    connect(ui->widgetDisasm,SIGNAL(debugAction(XInfoDB::DEBUG_ACTION)),this,SLOT(addDebugAction(XInfoDB::DEBUG_ACTION)));

    connect(ui->widgetDisasm,SIGNAL(followInHex(XADDR)),this,SLOT(followInHex(XADDR)));
    connect(ui->widgetHex,SIGNAL(followInDisasm(XADDR)),this,SLOT(followInDisasm(XADDR)));
    connect(ui->widgetStack,SIGNAL(followInHex(XADDR)),this,SLOT(followInHex(XADDR)));
    connect(ui->widgetStack,SIGNAL(followInDisasm(XADDR)),this,SLOT(followInDisasm(XADDR)));

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
    bool bResult=true; // TODO check

    // TODO Check function

    cleanUp();

    g_pInfoDB=new XInfoDB;

    g_pThread=new QThread;
#ifdef Q_OS_WIN
    g_pDebugger=new XWindowsDebugger(0,g_pInfoDB);
#endif
#ifdef Q_OS_LINUX
    g_pDebugger=new XLinuxDebugger(0,g_pInfoDB);
#endif
#ifdef Q_OS_OSX
    g_pDebugger=new XOSXDebugger(0,g_pInfoDB);
#endif

    ui->widgetDisasm->setXInfoDB(g_pDebugger->getXInfoDB());
    ui->widgetHex->setXInfoDB(g_pDebugger->getXInfoDB());
    ui->widgetRegs->setXInfoDB(g_pDebugger->getXInfoDB());
    ui->widgetStack->setXInfoDB(g_pDebugger->getXInfoDB());

    ui->widgetProcessModules->setXInfoDB(g_pDebugger->getXInfoDB(),false);
    ui->widgetProcessMemoryMap->setXInfoDB(g_pDebugger->getXInfoDB(),false);
    ui->widgetBreakpoints->setXInfoDB(g_pDebugger->getXInfoDB(),false);
    ui->widgetSymbols->setXInfoDB(g_pDebugger->getXInfoDB(),false);
    ui->widgetThreads->setXInfoDB(g_pDebugger->getXInfoDB(),false);

    g_osInfo=XProcess::getOsInfo();

    XAbstractDebugger::OPTIONS options={};

    options.bShowConsole=true;
    options.sFileName=sFileName;
    options.bBreakpointOnSystem=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_SYSTEM).toBool();
    options.bBreakpointOnProgramEntryPoint=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_ENTRYPOINT).toBool();
    options.bBreakPointOnDLLMain=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_DLLMAIN).toBool();
    options.bBreakPointOnTLSFunction=getGlobalOptions()->getValue(XOptions::ID_DEBUGGER_BREAKPOINT_TLSFUNCTIONS).toBool();

    g_pDebugger->setOptions(options);

    connect(g_pThread,SIGNAL(started()),g_pDebugger,SLOT(process()));
//    connect(pDebugger,SIGNAL(finished()),pDebugger,SLOT(deleteLater()));

    connect(g_pDebugger,SIGNAL(eventCreateProcess(XInfoDB::PROCESS_INFO*)),this,SLOT(onCreateProcess(XInfoDB::PROCESS_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventBreakPoint(XInfoDB::BREAKPOINT_INFO*)),this,SLOT(onBreakPoint(XInfoDB::BREAKPOINT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitProcess(XInfoDB::EXITPROCESS_INFO*)),this,SLOT(onExitProcess(XInfoDB::EXITPROCESS_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventCreateThread(XInfoDB::THREAD_INFO*)),this,SLOT(eventCreateThread(XInfoDB::THREAD_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitThread(XInfoDB::EXITTHREAD_INFO*)),this,SLOT(eventExitThread(XInfoDB::EXITTHREAD_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO*)),this,SLOT(eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO*)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO*)),this,SLOT(eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO*)),Qt::DirectConnection);

    connect(g_pInfoDB,SIGNAL(dataChanged(bool)),this,SLOT(onDataChanged(bool)));

    g_pDebugger->moveToThread(g_pThread);
    g_pThread->start();

    return bResult;
}

void XDebuggerWidget::setGlobal(XShortcuts *pShortcuts,XOptions *pXOptions)
{
    setActive(true);

    ui->widgetDisasm->setGlobal(pShortcuts,pXOptions);
    ui->widgetHex->setGlobal(pShortcuts,pXOptions);
    ui->widgetRegs->setGlobal(pShortcuts,pXOptions);
    ui->widgetStack->setGlobal(pShortcuts,pXOptions);
    // TODO more

    XShortcutsWidget::setGlobal(pShortcuts,pXOptions);
}

void XDebuggerWidget::adjustView()
{
    ui->widgetDisasm->adjustView();
    ui->widgetHex->adjustView();
    ui->widgetRegs->adjustView();
    ui->widgetStack->adjustView();
    ui->widgetProcessModules->adjustView();
    ui->widgetProcessMemoryMap->adjustView();
    ui->widgetBreakpoints->adjustView();
    ui->widgetSymbols->adjustView();
    ui->widgetThreads->adjustView();

    XShortcutsWidget::adjustView();
}

void XDebuggerWidget::onCreateProcess(XInfoDB::PROCESS_INFO *pProcessInfo)
{
    // TODO more info
    QString sString=QString("%1 PID: %2").arg(tr("Process created"),QString::number(pProcessInfo->nProcessID));

    emit infoMessage(sString);
}

void XDebuggerWidget::onBreakPoint(XInfoDB::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo=*pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("ExceptionAddress %llx",pBreakPointInfo->nAddress);
#endif
    // mb TODO regs

//    qDebug("Current Address2: %llX",XAbstractDebugger::getCurrentAddress(handleThread));

#ifdef Q_OS_LINUX
    g_pInfoDB->updateRegsById(pBreakPointInfo->nThreadID,ui->widgetRegs->getOptions());
#endif
#ifdef Q_OS_WIN
//    g_pInfoDB->suspendThread(pBreakPointInfo->hThread);
    g_pInfoDB->updateRegsByHandle(pBreakPointInfo->hThread,ui->widgetRegs->getOptions());
#endif

    g_pInfoDB->updateMemoryRegionsList();
    g_pInfoDB->updateModulesList();
    g_pInfoDB->clearRecordInfoCache();

    g_pInfoDB->reload(true);
}

void XDebuggerWidget::onExitProcess(XInfoDB::EXITPROCESS_INFO *pExitProcessInfo)
{
    // TODO more info
    QString sString=QString("%1 PID: %2").arg(tr("Process exited"),QString::number(pExitProcessInfo->nProcessID));

    emit infoMessage(sString);

    emit cleanUpSignal();

    // TODO Clear screen
}

void XDebuggerWidget::eventCreateThread(XInfoDB::THREAD_INFO *pThreadInfo)
{
    // TODO more info
    QString sString=QString("%1 ThreadID: %2").arg(tr("Thread created"),QString::number(pThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventExitThread(XInfoDB::EXITTHREAD_INFO *pExitThreadInfo)
{
    // TODO more info
    QString sString=QString("%1 ThreadID: %2").arg(tr("Thread exited"),QString::number(pExitThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString=QString("%1: %2").arg(tr("Shared object loaded"),pSharedObjectInfo->sFileName);

    emit infoMessage(sString);
}

void XDebuggerWidget::eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString=QString("%1: %2").arg(tr("Shared object unloaded"),pSharedObjectInfo->sFileName); // TODO rewrite

    emit infoMessage(sString);
}

void XDebuggerWidget::onDataChanged(bool bDataReload)
{
    if(bDataReload)
    {
        quint64 nInstructionPointer=g_pInfoDB->getCurrentInstructionPointerCache();
        quint64 nStackPointer=g_pInfoDB->getCurrentStackPointerCache();

        followInDisasm(nInstructionPointer);
        followInHex(nInstructionPointer);
        followInStack(nStackPointer);

        // TODO Memory Regions manager
    }

    reload();
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
    if(g_currentBreakPointInfo.nThreadID)
    {
    #ifdef Q_OS_WIN
//        g_pInfoDB->resumeThread(g_currentBreakPointInfo.hThread);
        g_pInfoDB->resumeAllThreads();
    #endif
    }
}

void XDebuggerWidget::debugStop()
{
    qDebug("TODO: Stop");
}

void XDebuggerWidget::debugStepInto()
{
    qDebug("void XDebuggerWidget::debugStepInto()");

    if(g_currentBreakPointInfo.nProcessID)
    {
    #ifdef Q_OS_LINUX

        ptrace(PT_CONTINUE,g_currentBreakPointInfo.nThreadID,0,0);
        qDebug("ptrace failed: %s",strerror(errno));

        g_pInfoDB->stepIntoById(g_currentBreakPointInfo.nThreadID);

        g_pInfoDB->_unlockID(g_currentBreakPointInfo.nThreadID);
//        g_pInfoDB->resumeAllThreads();

    #endif
//        g_pInfoDB->setCurrentThread(handleThread);
//        g_pInfoDB->stepInto();
//        g_pInfoDB->resumeThread(handleThread);

    #ifdef Q_OS_WIN
        g_pInfoDB->stepIntoByHandle(g_currentBreakPointInfo.hThread);
//        g_pInfoDB->resumeThread(g_currentBreakPointInfo.hThread);
        g_pInfoDB->resumeAllThreads();
    #endif
    }
}

void XDebuggerWidget::debugStepOver()
{
    if(g_currentBreakPointInfo.nThreadID)
    {
        XProcess::HANDLEID handleThread={};
        handleThread.nID=g_currentBreakPointInfo.nThreadID;
//        handleThread.hHandle=g_currentBreakPointInfo.pHThread;

        g_pDebugger->stepOver(handleThread);
    #ifdef Q_OS_WIN
//        g_pInfoDB->resumeThread(g_currentBreakPointInfo.hThread);
        g_pInfoDB->resumeAllThreads();
    #endif
    }
}

void XDebuggerWidget::viewCPU()
{
    ui->tabWidgetMain->setCurrentIndex(MT_CPU);
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

void XDebuggerWidget::viewModules()
{
    ui->tabWidgetMain->setCurrentIndex(MT_MODULES);
}

void XDebuggerWidget::viewSymbols()
{
    ui->tabWidgetMain->setCurrentIndex(MT_SYMBOLS);
}

void XDebuggerWidget::_toggleBreakpoint()
{
    ui->widgetDisasm->_breakpointToggle();

//    QList<XBinary::MEMORY_REPLACE> listReplaces=g_pDebugger->getMemoryReplaces();

//    ui->widgetDisasm->setMemoryReplaces(listReplaces); // TODO remove
//    ui->widgetHex->setMemoryReplaces(listReplaces); // TODO remove
//    ui->widgetStack->setMemoryReplaces(listReplaces); // TODO remove

//    ui->widgetDisasm->reload(true); // TODO signal/slot
//    ui->widgetHex->reload(true); // TODO signal/slot
//    ui->widgetStack->reload(true); // TODO signal/slot
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
        delete g_pInfoDB;
        g_pThread=nullptr;
        g_pDebugger=nullptr;
        g_pInfoDB=nullptr;
    }

    g_mrDisasm={};
    g_mrStack={};
    g_mrHex={};

    if(g_pPDDisasm)
    {
        delete g_pPDDisasm;
        g_pPDDisasm=nullptr;
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
    if(bState)
    {
        if(!shortCuts[SC_DISASM_BREAKPOINTTOGGLE])          shortCuts[SC_DISASM_BREAKPOINTTOGGLE]           =new QShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_TOGGLE),     this,SLOT(_toggleBreakpoint()));
    }
    else
    {
        for(qint32 i=0;i<__SC_SIZE;i++)
        {
            if(shortCuts[i])
            {
                delete shortCuts[i];
                shortCuts[i]=nullptr;
            }
        }
    }
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
//        if(!g_scBreakpointToggle)        g_scBreakpointToggle         =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_DISASM_BREAKPOINTTOGGLE),      this,SLOT(_toggleBreakpoint()));
    }
    else
    {
    #ifdef QT_DEBUG
        qDebug("unregisterShortcuts");
    #endif
//        if(g_scRun)                     {delete g_scRun;                    g_scRun=nullptr;}
//        if(g_scStepInto)                {delete g_scStepInto;               g_scStepInto=nullptr;}
//        if(g_scStepOver)                {delete g_scStepOver;               g_scStepOver=nullptr;}
//        if(g_scBreakpointToggle)     {delete g_scBreakpointToggle;    g_scBreakpointToggle=nullptr;}
    }
}

void XDebuggerWidget::on_tabWidgetMain_currentChanged(int nIndex)
{
    Q_UNUSED(nIndex)

    reload();
}

void XDebuggerWidget::reload()
{
    qint32 nIndex=ui->tabWidgetMain->currentIndex();

    if(nIndex==MT_CPU)
    {
        ui->widgetDisasm->reload(true);
        ui->widgetHex->reload(true);
        ui->widgetStack->reload(true);
        ui->widgetRegs->reload();
    }
    else if(nIndex==MT_BREAKPOINTS)
    {
        ui->widgetBreakpoints->reload();
    }
    else if(nIndex==MT_MEMORYMAP)
    {
        ui->widgetProcessMemoryMap->reload();
    }
    else if(nIndex==MT_CALLSTACK)
    {
        // TODO
    }
    else if(nIndex==MT_THREADS)
    {
        ui->widgetThreads->reload();
    }
    else if(nIndex==MT_HANDLES)
    {
        // TODO
    }
    else if(nIndex==MT_MODULES)
    {
        ui->widgetProcessModules->reload();
    }
    else if(nIndex==MT_SYMBOLS)
    {
        ui->widgetSymbols->reload();
    }
}

void XDebuggerWidget::followInDisasm(XADDR nAddress)
{
    // TODO aprox Address

    if(!XProcess::isAddressInMemoryRegion(&g_mrDisasm,nAddress))
    {
        g_mrDisasm=XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(),nAddress);

        if(g_pPDDisasm)
        {
            g_pPDDisasm->close();
            delete g_pPDDisasm;
            g_pPDDisasm=nullptr;
        }
    #ifdef Q_OS_WIN
        g_pPDDisasm=new XProcess(g_currentBreakPointInfo.hProcess,g_mrDisasm.nAddress,g_mrDisasm.nSize,this);
    #endif
    #ifdef Q_OS_LINUX
        g_pPDDisasm=new XProcess(g_currentBreakPointInfo.pHProcessMemoryIO,g_mrDisasm.nAddress,g_mrDisasm.nSize,this);
    #endif
    #ifdef Q_OS_MACOS
        g_pPDDisasm=new XProcess(g_currentBreakPointInfo.hProcess,g_mrDisasm.nAddress,g_mrDisasm.nSize,this);
    #endif
        if(g_pPDDisasm->open(QIODevice::ReadWrite))
        {
            XBinary binary(g_pPDDisasm,true,g_mrDisasm.nAddress);
            binary.setArch(g_osInfo.sArch);
            binary.setMode(g_osInfo.mode);

            XDisasmView::OPTIONS disasmOptions={};
            disasmOptions.nInitAddress=nAddress;
            disasmOptions.nCurrentIntructionPointer=g_pInfoDB->getCurrentInstructionPointerCache();
            disasmOptions.memoryMapRegion=binary.getMemoryMap();
            disasmOptions.bAprox=true;
            ui->widgetDisasm->setData(g_pPDDisasm,disasmOptions,false);
            ui->widgetDisasm->setReadonly(false);
        }
    }
    else
    {
        ui->widgetDisasm->setCurrentIntructionPointer(nAddress);
        ui->widgetDisasm->goToAddress(nAddress,true,true);
    }

    ui->widgetDisasm->setSelectionAddress(nAddress,1);
}

void XDebuggerWidget::followInHex(XADDR nAddress)
{
    if(!XProcess::isAddressInMemoryRegion(&g_mrHex,nAddress))
    {
        g_mrHex=XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(),nAddress);

        if(g_pPDHex)
        {
            g_pPDHex->close();
            delete g_pPDHex;
            g_pPDHex=nullptr;
        }
    #ifdef Q_OS_WIN
        g_pPDHex=new XProcess(g_currentBreakPointInfo.hProcess,g_mrHex.nAddress,g_mrHex.nSize,this);
    #endif
    #ifdef Q_OS_LINUX
        g_pPDHex=new XProcess(g_currentBreakPointInfo.pHProcessMemoryIO,g_mrHex.nAddress,g_mrHex.nSize,this);
    #endif
    #ifdef Q_OS_MACOS
        g_pPDHex=new XProcess(g_currentBreakPointInfo.hProcess,g_mrHex.nAddress,g_mrHex.nSize,this);
    #endif
        if(g_pPDHex->open(QIODevice::ReadWrite))
        {
            XHexView::OPTIONS hexOptions={};
            hexOptions.nStartAddress=g_mrHex.nAddress;
            hexOptions.nStartSelectionOffset=nAddress-hexOptions.nStartAddress;
            ui->widgetHex->setData(g_pPDHex,hexOptions,false);
            ui->widgetHex->setReadonly(false);
        }
    }
    else
    {
        ui->widgetHex->goToAddress(nAddress);
    }

    ui->widgetHex->setSelectionAddress(nAddress,1);
}

void XDebuggerWidget::followInStack(XADDR nAddress)
{
    if(!XProcess::isAddressInMemoryRegion(&g_mrStack,nAddress))
    {
        g_mrStack=XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(),nAddress);

        if(g_pPDStack)
        {
            g_pPDStack->close();
            delete g_pPDStack;
            g_pPDStack=nullptr;
        }
    #ifdef Q_OS_WIN
        g_pPDStack=new XProcess(g_currentBreakPointInfo.hProcess,g_mrStack.nAddress,g_mrStack.nSize,this);
    #endif
    #ifdef Q_OS_LINUX
        g_pPDStack=new XProcess(g_currentBreakPointInfo.pHProcessMemoryIO,g_mrStack.nAddress,g_mrStack.nSize,this);
    #endif
    #ifdef Q_OS_MACOS
        g_pPDStack=new XProcess(g_currentBreakPointInfo.hProcess,g_mrStack.nAddress,g_mrStack.nSize,this);
    #endif
        if(g_pPDStack->open(QIODevice::ReadWrite))
        {
            XStackView::OPTIONS stackOptions={};
            stackOptions.nStartAddress=g_mrStack.nAddress;
            stackOptions.nCurrentAddress=nAddress;
            stackOptions.nCurrentStackPointer=nAddress;
            ui->widgetStack->setData(g_pPDStack,stackOptions,false);
            ui->widgetStack->setReadonly(false);
        }
    }
    else
    {
        ui->widgetStack->setCurrentStackPointer(nAddress);
        ui->widgetStack->goToAddress(nAddress);
    }

    ui->widgetStack->setSelectionAddress(nAddress);
}
