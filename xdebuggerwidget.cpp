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

//    g_scRun=nullptr;
//    g_scStepInto=nullptr;
//    g_scStepOver=nullptr;
//    g_scBreakpointToggle=nullptr;

    g_mrCode={};
    g_mrStack={};
    g_mrHex={};
    g_pPDCode=nullptr;
    g_pPDStack=nullptr;
    g_pPDHex=nullptr;

//    qRegisterMetaType<XBinary::PROCESS_STATUS>("XBinary::PROCESS_STATUS");

    connect(this,SIGNAL(cleanUpSignal()),this,SLOT(cleanUp()));
    connect(this,SIGNAL(infoMessage(QString)),this,SLOT(infoMessageSlot(QString)));
    connect(this,SIGNAL(errorMessage(QString)),this,SLOT(errorMessageSlot(QString)));
//    ui->widgetDisasm->installEventFilter(this);
//    XDebuggerWidget::registerShortcuts(true);
//    connect(ui->widgetDisasm,SIGNAL(debugAction(XInfoDB::DEBUG_ACTION)),this,SLOT(addDebugAction(XInfoDB::DEBUG_ACTION)));

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

    XProcess::HANDLEID handleThread={};
    handleThread.hHandle=pBreakPointInfo->pHThread;
    handleThread.nID=pBreakPointInfo->nThreadID;

    XProcess::HANDLEID handleProcess={};
    handleProcess.hHandle=pBreakPointInfo->pHProcessMemoryQuery;
    handleProcess.nID=pBreakPointInfo->nProcessID;

    qDebug("Current Address2: %llX",XAbstractDebugger::getCurrentAddress(handleThread));

    g_pDebugger->suspendThread(handleThread);

    g_pInfoDB->setCurrentThread(handleThread);
    g_pInfoDB->updateRegs(ui->widgetRegs->getOptions());
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
        quint64 nCurrentAddress=g_pInfoDB->getCurrentInstructionPointer();
        quint64 nStackPointer=g_pInfoDB->getCurrentStackPointer();

        // TODO getMemoryRegions
        // TODO Memory Regions manager

        if(!XProcess::isAddressInMemoryRegion(&g_mrCode,nCurrentAddress))
        {
    //        g_mrCode=XProcess::getMemoryRegion(handleProcessMemoryQuery,g_currentBreakPointInfo.nAddress);
            g_mrCode=XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(),nCurrentAddress);

            if(g_pPDCode)
            {
                g_pPDCode->close();
                delete g_pPDCode;
                g_pPDCode=nullptr;
            }

            g_pPDCode=new XProcess(g_currentBreakPointInfo.pHProcessMemoryIO,g_mrCode.nAddress,g_mrCode.nSize,this);

            if(g_pPDCode->open(QIODevice::ReadWrite))
            {
                XBinary binary(g_pPDCode,true,g_mrCode.nAddress);
                binary.setArch(g_osInfo.sArch);
                binary.setMode(g_osInfo.mode);

                XDisasmView::OPTIONS disasmOptions={};
                disasmOptions.nInitAddress=nCurrentAddress;
                disasmOptions.nCurrentIPAddress=nCurrentAddress;
                disasmOptions.memoryMap=binary.getMemoryMap();
                ui->widgetDisasm->setData(g_pPDCode,disasmOptions,false);
                ui->widgetDisasm->setReadonly(false);

                XHexView::OPTIONS hexOptions={};
                hexOptions.nStartAddress=g_mrCode.nAddress;
                hexOptions.nStartSelectionOffset=nCurrentAddress-hexOptions.nStartAddress;
                ui->widgetHex->setData(g_pPDCode,hexOptions,false);
                ui->widgetHex->setReadonly(false);
            }
        }
        else
        {
            ui->widgetDisasm->setCurrentPointerAddress(nCurrentAddress);
            ui->widgetDisasm->goToAddress(nCurrentAddress,true);
        }

        ui->widgetDisasm->setSelectionAddress(nCurrentAddress,1);

    //    // TODO address cache
    //    XProcessDevice *pPDCode=new XProcessDevice(this);
    //    if(pPDCode->openHandle(currentBreakPointInfo.hProcess,g_mrCode.nAddress,g_mrCode.nSize,QIODevice::ReadOnly))
    //    {


    ////        XHexView::OPTIONS hexOptions={};
    ////        hexOptions.nStartAddress=mrCode.nAddress;

    ////        ui->widgetHex->setData(pPDCode,hexOptions);
    ////        ui->widgetHex->goToAddress(currentBreakPointInfo.nAddress);
    //    }

    //    qint64 nTest=g_pDebugger->getCurrentAddress(g_currentBreakPointInfo.handleThread);

        if(!XProcess::isAddressInMemoryRegion(&g_mrStack,nStackPointer))
        {
    //        g_mrStack=XProcess::getMemoryRegion(handleProcessMemoryQuery,nStackPointer);
            g_mrStack=XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(),nStackPointer);

            if(g_pPDStack)
            {
                g_pPDStack->close();
                delete g_pPDStack;
                g_pPDStack=nullptr;
            }

            g_pPDStack=new XProcess(g_currentBreakPointInfo.pHProcessMemoryIO,g_mrStack.nAddress,g_mrStack.nSize,this);

            if(g_pPDStack->open(QIODevice::ReadWrite))
            {
                XStackView::OPTIONS stackOptions={};
                stackOptions.nStartAddress=g_mrStack.nAddress;
                stackOptions.nCurrentAddress=nStackPointer;
                ui->widgetStack->setData(g_pPDStack,stackOptions,false);
                ui->widgetStack->setReadonly(false);

    //            XHexView::OPTIONS hexOptions={};
    //            hexOptions.nStartAddress=g_mrStack.nAddress;
    //            ui->widgetHex->setData(g_pPDStack,hexOptions);
            }
        }
        else
        {
            ui->widgetStack->goToAddress(nStackPointer);
        }

        ui->widgetStack->setSelectionAddress(nStackPointer);
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
        XProcess::HANDLEID handleThread={};
        handleThread.nID=g_currentBreakPointInfo.nThreadID;
        handleThread.hHandle=g_currentBreakPointInfo.pHThread;

        g_pDebugger->resumeThread(handleThread);
    }
}

void XDebuggerWidget::debugStepInto()
{
    if(g_currentBreakPointInfo.nThreadID)
    {
        XProcess::HANDLEID handleThread={};
        handleThread.nID=g_currentBreakPointInfo.nThreadID;
        handleThread.hHandle=g_currentBreakPointInfo.pHThread;

        g_pDebugger->stepInto(handleThread);
        g_pDebugger->resumeThread(handleThread);
    }
}

void XDebuggerWidget::debugStepOver()
{
    if(g_currentBreakPointInfo.nThreadID)
    {
        XProcess::HANDLEID handleThread={};
        handleThread.nID=g_currentBreakPointInfo.nThreadID;
        handleThread.hHandle=g_currentBreakPointInfo.pHThread;

        g_pDebugger->stepOver(handleThread);
        g_pDebugger->resumeThread(handleThread);
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
