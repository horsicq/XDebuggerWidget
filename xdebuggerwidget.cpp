/* Copyright (c) 2022-2024 hors<horsicq@gmail.com>
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

XDebuggerWidget::XDebuggerWidget(QWidget *pParent) : XShortcutsWidget(pParent), ui(new Ui::XDebuggerWidget)
{
    ui->setupUi(this);
#ifdef Q_OS_WIN
    g_pThread = nullptr;
#endif
    g_pDebugger = nullptr;
    g_pInfoDB = nullptr;
    g_pMutex = new QMutex;

    g_currentBreakPointInfo = {};

    memset(shortCuts, 0, sizeof shortCuts);

    g_mrDisasm = {};
    g_mrStack = {};
    g_mrHex = {};
    g_pTimer = nullptr;

    g_state = {};

    _adjustState();

    //    qRegisterMetaType<XBinary::PROCESS_STATUS>("XBinary::PROCESS_STATUS");
    qRegisterMetaType<X_ID>("X_ID");
    qRegisterMetaType<X_HANDLE>("X_HANDLE");
    qRegisterMetaType<X_HANDLE_IO>("X_HANDLE_IO");
    qRegisterMetaType<X_HANDLE_MQ>("X_HANDLE_MQ");
    qRegisterMetaType<XADDR>("XADDR");

    connect(this, SIGNAL(resetWidgetsSignal()), this, SLOT(resetWidgets()));
    connect(this, SIGNAL(infoMessage(QString)), this, SLOT(infoMessageSlot(QString)));
    connect(this, SIGNAL(errorMessage(QString)), this, SLOT(errorMessageSlot(QString)));
    connect(this, SIGNAL(warningMessage(QString)), this, SLOT(warningMessageSlot(QString)));

    connect(this, SIGNAL(addSymbols(QString, XADDR)), this, SLOT(addSymbolsSlot(QString, XADDR)));
    connect(this, SIGNAL(removeSymbols(QString)), this, SLOT(removeSymbolsSlot(QString)));
    //    ui->widgetDisasm->installEventFilter(this);
    //    XDebuggerWidget::registerShortcuts(true);
    //    connect(ui->widgetDisasm,SIGNAL(debugAction(XInfoDB::DEBUG_ACTION)),this,SLOT(addDebugAction(XInfoDB::DEBUG_ACTION)));

    connect(ui->widgetDisasm, SIGNAL(followInHex(XADDR)), this, SLOT(followInHex(XADDR)));
    connect(ui->widgetHex1, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetHex2, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetHex3, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetHex4, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetHex5, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetStack, SIGNAL(followInHex(XADDR)), this, SLOT(followInHex(XADDR)));
    connect(ui->widgetStack, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));
    connect(ui->widgetRegs, SIGNAL(followInHex(XADDR)), this, SLOT(followInHex(XADDR)));
    connect(ui->widgetRegs, SIGNAL(followInDisasm(XADDR)), this, SLOT(followInDisasm(XADDR)));

    ui->tabWidgetMain->setCurrentIndex(MT_CPU);
}

XDebuggerWidget::~XDebuggerWidget()
{
    //    qDebug("XDebuggerWidget::~XDebuggerWidget()");
    cleanUp();

    delete g_pMutex;
    delete ui;
}

bool XDebuggerWidget::loadFile(const QString &sFileName, bool bShowLoadDialog)
{
    bool bResult = false;  // TODO check

    // TODO Check function

    if (!bShowLoadDialog) {
        bResult = true;
    }

    XAbstractDebugger::OPTIONS options = XAbstractDebugger::getDefaultOptions(sFileName);

    if (bShowLoadDialog) {
        XDebuggerLoadDialog debuggerLoadDialog(this, &options);

        if (debuggerLoadDialog.exec() == QDialog::Accepted) {
            bResult = true;
        }
    }

    if (bResult) {
        resetWidgets();
        cleanUp();

        g_pInfoDB = new XInfoDB;
        g_pInfoDB->setDebuggerState(true);

#ifdef Q_OS_WIN
        g_pThread = new QThread;
        g_pDebugger = new XWindowsDebugger(0, g_pInfoDB);
#endif
#ifdef Q_OS_LINUX
        g_pDebugger = new XLinuxDebugger(0, g_pInfoDB);
#endif
#ifdef Q_OS_MACOS
        g_pDebugger = new XOSXDebugger(0, g_pInfoDB);
#endif
        ui->widgetDisasm->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetHex1->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetHex2->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetHex3->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetHex4->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetHex5->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetRegs->setXInfoDB(g_pDebugger->getXInfoDB());
        ui->widgetStack->setXInfoDB(g_pDebugger->getXInfoDB());

        ui->widgetProcessModules->setXInfoDB(g_pDebugger->getXInfoDB(), false);
        ui->widgetProcessMemoryMap->setXInfoDB(g_pDebugger->getXInfoDB(), false);
        ui->widgetBreakpoints->setXInfoDB(g_pDebugger->getXInfoDB(), false);
        ui->widgetSymbols->setData(g_pDebugger->getXInfoDB(), XSymbolsWidget::MODE_ALL, QVariant(), false);
        ui->widgetThreads->setXInfoDB(g_pDebugger->getXInfoDB(), false);

        g_osInfo = XProcess::getOsInfo();

        g_pDebugger->setOptions(options);

#ifdef Q_OS_WIN
        connect(g_pThread, SIGNAL(started()), g_pDebugger, SLOT(process()));
#endif
        //    connect(pDebugger,SIGNAL(finished()),pDebugger,SLOT(deleteLater()));

        connect(g_pDebugger, SIGNAL(infoMessage(QString)), this, SLOT(infoMessageSlot(QString)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(errorMessage(QString)), this, SLOT(errorMessageSlot(QString)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(warningMessage(QString)), this, SLOT(warningMessageSlot(QString)), Qt::DirectConnection);

        connect(g_pDebugger, SIGNAL(eventCreateProcess(XInfoDB::PROCESS_INFO *)), this, SLOT(onCreateProcess(XInfoDB::PROCESS_INFO *)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventBreakPoint(XInfoDB::BREAKPOINT_INFO *)), this, SLOT(onBreakPoint(XInfoDB::BREAKPOINT_INFO *)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventExitProcess(XInfoDB::EXITPROCESS_INFO *)), this, SLOT(onExitProcess(XInfoDB::EXITPROCESS_INFO *)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventCreateThread(XInfoDB::THREAD_INFO *)), this, SLOT(eventCreateThread(XInfoDB::THREAD_INFO *)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventExitThread(XInfoDB::EXITTHREAD_INFO *)), this, SLOT(eventExitThread(XInfoDB::EXITTHREAD_INFO *)), Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO *)), this, SLOT(eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO *)),
                Qt::DirectConnection);
        connect(g_pDebugger, SIGNAL(eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO *)), this, SLOT(eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO *)),
                Qt::DirectConnection);

        //    connect(this,SIGNAL(testSignal(X_ID)),g_pDebugger,SLOT(testSlot(X_ID)),Qt::BlockingQueuedConnection);
        //    connect(this,SIGNAL(testSignal(X_ID)),g_pDebugger,SLOT(testSlot(X_ID)),Qt::QueuedConnection);

        connect(g_pInfoDB, SIGNAL(reloadSignal(bool)), this, SLOT(onReloadSignal(bool)));  // TODO remove

        // connect(g_pInfoDB, SIGNAL(memoryRegionsListChanged()), this, SLOT(memoryRegionsListChangedSlot()));
        connect(g_pInfoDB, SIGNAL(modulesListChanged()), this, SLOT(modulesListChangedSlot()));
        connect(g_pInfoDB, SIGNAL(threadsListChanged()), this, SLOT(threadsListChangedSlot()));
        connect(g_pInfoDB, SIGNAL(registersListChanged()), this, SLOT(registersListChangedSlot()));

#ifdef Q_OS_WIN
        g_pDebugger->moveToThread(g_pThread);
        g_pThread->start();
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
        g_pDebugger->process();
#endif

        if (bResult) {
            command(CM_READY);
        }
    }

    return bResult;
}

void XDebuggerWidget::setGlobal(XShortcuts *pShortcuts, XOptions *pXOptions)
{
    setActive(true);

    ui->widgetDisasm->setGlobal(pShortcuts, pXOptions);
    ui->widgetHex1->setGlobal(pShortcuts, pXOptions);
    ui->widgetHex2->setGlobal(pShortcuts, pXOptions);
    ui->widgetHex3->setGlobal(pShortcuts, pXOptions);
    ui->widgetHex4->setGlobal(pShortcuts, pXOptions);
    ui->widgetHex5->setGlobal(pShortcuts, pXOptions);
    ui->widgetRegs->setGlobal(pShortcuts, pXOptions);
    ui->widgetStack->setGlobal(pShortcuts, pXOptions);
    // TODO more

    XShortcutsWidget::setGlobal(pShortcuts, pXOptions);
}

void XDebuggerWidget::adjustView()
{
    ui->widgetDisasm->adjustView();
    ui->widgetHex1->adjustView();
    ui->widgetHex2->adjustView();
    ui->widgetHex3->adjustView();
    ui->widgetHex4->adjustView();
    ui->widgetHex5->adjustView();
    ui->widgetRegs->adjustView();
    ui->widgetStack->adjustView();
    ui->widgetProcessModules->adjustView();
    ui->widgetProcessMemoryMap->adjustView();
    ui->widgetBreakpoints->adjustView();
    ui->widgetSymbols->adjustView();
    ui->widgetThreads->adjustView();

    XShortcutsWidget::adjustView();
}

void XDebuggerWidget::reloadShortcuts()
{
    ui->widgetDisasm->reloadShortcuts();
    ui->widgetHex1->reloadShortcuts();
    ui->widgetHex2->reloadShortcuts();
    ui->widgetHex3->reloadShortcuts();
    ui->widgetHex4->reloadShortcuts();
    ui->widgetHex5->reloadShortcuts();
    ui->widgetRegs->reloadShortcuts();
    ui->widgetStack->reloadShortcuts();
    ui->widgetProcessModules->reloadShortcuts();
    ui->widgetProcessMemoryMap->reloadShortcuts();
    ui->widgetBreakpoints->reloadShortcuts();
    ui->widgetSymbols->reloadShortcuts();
    ui->widgetThreads->adjustView();

    XShortcutsWidget::reloadShortcuts();
}

XDebuggerWidget::STATE XDebuggerWidget::getState()
{
    return g_state;
}

void XDebuggerWidget::onCreateProcess(XInfoDB::PROCESS_INFO *pProcessInfo)
{
    // TODO more info
    QString sString = QString("%1 PID: %2").arg(tr("Process created"), QString::number(pProcessInfo->nProcessID));

    emit infoMessage(sString);
    emit addSymbols(pProcessInfo->sFileName, pProcessInfo->nImageBase);
}

void XDebuggerWidget::onBreakPoint(XInfoDB::BREAKPOINT_INFO *pBreakPointInfo)
{
    g_currentBreakPointInfo = *pBreakPointInfo;
#ifdef QT_DEBUG
    qDebug("ExceptionAddress %llx", pBreakPointInfo->nExceptionAddress);
#endif
    // mb TODO regs

    //    qDebug("Current Address2: %llX",XAbstractDebugger::getCurrentAddress(handleThread));
    g_pInfoDB->updateMemoryRegionsList();
    g_pInfoDB->updateModulesList();
    g_pInfoDB->updateThreadsList();

#if defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    g_pInfoDB->updateRegsById(pBreakPointInfo->nThreadID, ui->widgetRegs->getOptions());
    qDebug("TODO remove");
    g_pInfoDB->getHardwareBP_Id(pBreakPointInfo->nThreadID);
#endif
#ifdef Q_OS_WIN
    //    g_pInfoDB->suspendThread(pBreakPointInfo->hThread);
    g_pInfoDB->updateRegsByHandle(pBreakPointInfo->hThread, ui->widgetRegs->getOptions());

#ifdef QT_DEBUG
    qDebug("TODO remove");
    g_pInfoDB->getHardwareBP_Handle(pBreakPointInfo->hThread);
#endif
#endif
    // TODO reloads signals

    g_pInfoDB->clearRecordInfoCache();

    g_pInfoDB->reload(true);  // TODO Check
}

void XDebuggerWidget::onExitProcess(XInfoDB::EXITPROCESS_INFO *pExitProcessInfo)
{
    // TODO more info
    QString sString = QString("%1 PID: %2").arg(tr("Process exited"), QString::number(pExitProcessInfo->nProcessID));

    emit infoMessage(sString);

    removeSymbols(pExitProcessInfo->sFileName);

    emit resetWidgetsSignal();
}

void XDebuggerWidget::eventCreateThread(XInfoDB::THREAD_INFO *pThreadInfo)
{
    // TODO more info
    QString sString = QString("%1 ThreadID: %2").arg(tr("Thread created"), QString::number(pThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventExitThread(XInfoDB::EXITTHREAD_INFO *pExitThreadInfo)
{
    // TODO more info
    QString sString = QString("%1 ThreadID: %2").arg(tr("Thread exited"), QString::number(pExitThreadInfo->nThreadID));

    emit infoMessage(sString);
}

void XDebuggerWidget::eventLoadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString = QString("%1: %2").arg(tr("Shared object loaded"), pSharedObjectInfo->sFileName);

    emit infoMessage(sString);
    //    emit addSymbols(pSharedObjectInfo->sFileName, pSharedObjectInfo->nImageBase);
}

void XDebuggerWidget::eventUnloadSharedObject(XInfoDB::SHAREDOBJECT_INFO *pSharedObjectInfo)
{
    // TODO more info
    QString sString = QString("%1: %2").arg(tr("Shared object unloaded"), pSharedObjectInfo->sFileName);  // TODO rewrite

    emit infoMessage(sString);
    //    emit removeSymbols(pSharedObjectInfo->sFileName);
}

void XDebuggerWidget::onReloadSignal(bool bDataReload)
{
    qDebug("void XDebuggerWidget::onReloadSignal(bool bDataReload)");
    // TODO rework
    if (bDataReload) {
        quint64 nInstructionPointer = g_pInfoDB->getCurrentInstructionPointerCache();
        quint64 nStackPointer = g_pInfoDB->getCurrentStackPointerCache();

        // TODO Check tab
        followInDisasm(nInstructionPointer);
        followInStack(nStackPointer);
        followInHex(g_pInfoDB->getProcessInfo()->nImageBase); // TODO
    }

    reload();  // TODO Check remove
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

bool XDebuggerWidget::debugRun()
{
    return command(CM_DEBUG_RUN);
}

bool XDebuggerWidget::debugClose()
{
    return command(CM_DEBUG_CLOSE);
}

bool XDebuggerWidget::debugStepInto()
{
    return command(CM_DEBUG_STEPINTO);
}

bool XDebuggerWidget::debugStepOver()
{
    return command(CM_DEBUG_STEPOVER);
}

bool XDebuggerWidget::animateStepInto()
{
    return command(CM_ANIMATE_STEPINTO);
}

bool XDebuggerWidget::animateStepOver()
{
    return command(CM_ANIMATE_STEPOVER);
}

bool XDebuggerWidget::animateStop()
{
    return command(CM_ANIMATE_STOP);
}

bool XDebuggerWidget::traceStepInto()
{
    return command(CM_TRACE_STEPINTO);
}

bool XDebuggerWidget::traceStepOver()
{
    return command(CM_TRACE_STEPOVER);
}

bool XDebuggerWidget::traceStop()
{
    return command(CM_TRACE_STOP);
}

bool XDebuggerWidget::command(CM commandMode)
{
    bool bResult = false;

    if (g_currentBreakPointInfo.nProcessID) {
        if (g_pDebugger) {
            if (commandMode == CM_READY) {
                followInHex(g_pInfoDB->getProcessInfo()->nThreadLocalBase);
            } else if (commandMode == CM_DEBUG_RUN) {
                //                bResult=g_pInfoDB->resumeAllThreads();
                bResult = g_pDebugger->run();
            } else if (commandMode == CM_DEBUG_STEPINTO) {
#ifdef Q_OS_WIN
                bResult = g_pDebugger->stepIntoByHandle(g_currentBreakPointInfo.hThread, XInfoDB::BPI_STEPINTO);
#endif
#ifdef Q_OS_LINUX
                bResult = g_pDebugger->stepIntoById(g_currentBreakPointInfo.nProcessID, XInfoDB::BPI_STEPINTO);
#endif
            } else if (commandMode == CM_DEBUG_STEPOVER) {
#ifdef Q_OS_WIN
                bResult = g_pDebugger->stepOverByHandle(g_currentBreakPointInfo.hThread, XInfoDB::BPI_STEPOVER);
#endif
#ifdef Q_OS_LINUX
                bResult = g_pDebugger->stepOverById(g_currentBreakPointInfo.nProcessID, XInfoDB::BPI_STEPOVER);
#endif
            } else if (commandMode == CM_TRACE_STEPINTO) {
#ifdef Q_OS_WIN
                bResult = g_pDebugger->stepIntoByHandle(g_currentBreakPointInfo.hThread, XInfoDB::BPI_TRACEINTO);
#endif
#ifdef Q_OS_LINUX
                bResult = g_pDebugger->stepIntoById(g_currentBreakPointInfo.nProcessID, XInfoDB::BPI_TRACEINTO);
#endif
            } else if (commandMode == CM_TRACE_STEPOVER) {
#ifdef Q_OS_WIN
                bResult = g_pDebugger->stepOverByHandle(g_currentBreakPointInfo.hThread, XInfoDB::BPI_TRACEOVER);
#endif
#ifdef Q_OS_LINUX
                bResult = g_pDebugger->stepOverById(g_currentBreakPointInfo.nProcessID, XInfoDB::BPI_TRACEOVER);
#endif
            } else if (commandMode == CM_DEBUG_CLOSE) {
                bResult = g_pDebugger->stop();
            }
        }
    }

    if ((commandMode == CM_ANIMATE_STEPINTO) || (commandMode == CM_ANIMATE_STEPOVER) || (commandMode == CM_ANIMATE_STOP)) {
        if (g_pTimer) {
            g_pTimer->stop();
            delete g_pTimer;
            g_pTimer = nullptr;

            bResult = true;
        }
    }

    if ((commandMode == CM_ANIMATE_STEPINTO) || (commandMode == CM_ANIMATE_STEPOVER)) {
        g_pTimer = new QTimer(this);

        if (commandMode == CM_ANIMATE_STEPINTO) {
            connect(g_pTimer, SIGNAL(timeout()), this, SLOT(debugStepInto()));
        }
        if (commandMode == CM_ANIMATE_STEPOVER) {
            connect(g_pTimer, SIGNAL(timeout()), this, SLOT(debugStepOver()));
        }

        g_pTimer->start(1000);  // TODO const

        bResult = true;
    }

    if ((commandMode == CM_ANIMATE_STEPINTO) || (commandMode == CM_ANIMATE_STEPOVER)) {
        g_state.bAnimateStepInto = false;
        g_state.bAnimateStepOver = false;
        g_state.bAnimateStop = true;
        g_state.bTraceStepInto = false;
        g_state.bTraceStepOver = false;
        g_state.bTraceStop = false;

        _stateChanged();
    } else if ((commandMode == CM_READY) || (commandMode == CM_ANIMATE_STOP)) {
        g_state.bAnimateStepInto = true;
        g_state.bAnimateStepOver = true;
        g_state.bAnimateStop = false;
        g_state.bTraceStepInto = true;
        g_state.bTraceStepOver = true;
        g_state.bTraceStop = false;

        _stateChanged();
    }

    return bResult;
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

void XDebuggerWidget::_stateChanged()
{
    _adjustState();

    emit stateChanged();
}

void XDebuggerWidget::_adjustState()
{
    // TODO set tab CPU if debug or animate, or trace -> command
    ui->toolButtonAnimateStepInto->setEnabled(g_state.bAnimateStepInto);
    ui->toolButtonAnimateStepOver->setEnabled(g_state.bAnimateStepOver);
    ui->toolButtonAnimateStop->setEnabled(g_state.bAnimateStop);
    ui->toolButtonTraceStepInto->setEnabled(g_state.bTraceStepInto);
    ui->toolButtonTraceStepOver->setEnabled(g_state.bTraceStepOver);
    ui->toolButtonTraceStop->setEnabled(g_state.bTraceStop);

    if (g_state.bAnimateStop || g_state.bTraceStop) {
        ui->widgetDisasm->setContextMenuEnable(false);
        ui->widgetHex1->setContextMenuEnable(false);
        ui->widgetHex2->setContextMenuEnable(false);
        ui->widgetHex3->setContextMenuEnable(false);
        ui->widgetHex4->setContextMenuEnable(false);
        ui->widgetHex5->setContextMenuEnable(false);
        ui->widgetStack->setContextMenuEnable(false);
        // TODO registers

        ui->tabWidgetMain->setTabEnabled(MT_CPU, true);
        ui->tabWidgetMain->setTabEnabled(MT_LOG, false);
        ui->tabWidgetMain->setTabEnabled(MT_BREAKPOINTS, false);
        ui->tabWidgetMain->setTabEnabled(MT_MEMORYMAP, false);
        ui->tabWidgetMain->setTabEnabled(MT_CALLSTACK, false);
        ui->tabWidgetMain->setTabEnabled(MT_THREADS, false);
        ui->tabWidgetMain->setTabEnabled(MT_HANDLES, false);
        ui->tabWidgetMain->setTabEnabled(MT_MODULES, false);
        ui->tabWidgetMain->setTabEnabled(MT_SYMBOLS, false);
    } else {
        ui->widgetDisasm->setContextMenuEnable(true);
        ui->widgetHex1->setContextMenuEnable(true);
        ui->widgetHex2->setContextMenuEnable(true);
        ui->widgetHex3->setContextMenuEnable(true);
        ui->widgetHex4->setContextMenuEnable(true);
        ui->widgetHex5->setContextMenuEnable(true);
        ui->widgetStack->setContextMenuEnable(true);
        // TODO registers

        ui->tabWidgetMain->setTabEnabled(MT_CPU, true);
        ui->tabWidgetMain->setTabEnabled(MT_LOG, true);
        ui->tabWidgetMain->setTabEnabled(MT_BREAKPOINTS, true);
        ui->tabWidgetMain->setTabEnabled(MT_MEMORYMAP, true);
        ui->tabWidgetMain->setTabEnabled(MT_CALLSTACK, true);
        ui->tabWidgetMain->setTabEnabled(MT_THREADS, true);
        ui->tabWidgetMain->setTabEnabled(MT_HANDLES, true);
        ui->tabWidgetMain->setTabEnabled(MT_MODULES, true);
        ui->tabWidgetMain->setTabEnabled(MT_SYMBOLS, true);
    }
}

void XDebuggerWidget::_clearProcessMemory()
{
    qint32 nCount = g_listRegions.count();

    for (qint32 i = 0; i < nCount; i++) {
        g_listRegions.at(i)->close();
        delete g_listRegions.at(i);
    }

    g_listRegions.clear();
}

XProcess *XDebuggerWidget::_getProcessMemory(XADDR nAddress, qint64 nSize)
{
    XProcess *pResult = nullptr;

    bool bCreateNew = true;

    qint32 nCount = g_listRegions.count();

    for (qint32 i = 0; i < nCount; i++) {
        if ((g_listRegions.at(i)->getInitLocation() == nAddress) && (g_listRegions.at(i)->size() == nSize)) {
            pResult = g_listRegions.at(i);
            bCreateNew = false;
            break;
        }
    }

    if (bCreateNew) {
#ifdef Q_OS_WIN
        pResult = new XProcess(nAddress, nSize, g_currentBreakPointInfo.hProcess, this);
#endif
#ifdef Q_OS_LINUX
        pResult = new XProcess(nAddress, nSize, g_currentBreakPointInfo.pHProcessMemoryIO, this);
#endif
#ifdef Q_OS_MACOS
        pResult = new XProcess(nAddress, nSize, g_currentBreakPointInfo.hProcess, this);
#endif
        if (pResult->open(QIODevice::ReadWrite)) {
            connect(pResult, SIGNAL(readDataSignal(quint64, char *, qint64)), g_pInfoDB, SLOT(readDataSlot(quint64, char *, qint64)), Qt::DirectConnection);
            connect(pResult, SIGNAL(writeDataSignal(quint64, char *, qint64)), g_pInfoDB, SLOT(writeDataSlot(quint64, char *, qint64)), Qt::DirectConnection);

            g_listRegions.append(pResult);
        } else {
            delete pResult;
            pResult = nullptr;
        }
    }

    return pResult;
}

void XDebuggerWidget::_breakpointToggle()
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

void XDebuggerWidget::_breakpointRemove()
{
    ui->widgetDisasm->_breakpointRemove();
}

void XDebuggerWidget::_breakpointConditional()
{
    ui->widgetDisasm->_breakpointConditional();
}

void XDebuggerWidget::cleanUp()
{
#ifdef QT_DEBUG
    qDebug("void XDebuggerWidget::cleanUp()");
#endif

    if (g_pDebugger) {
        g_pDebugger->cleanUp();

        delete g_pDebugger;
        g_pDebugger = nullptr;
    }
#ifdef Q_OS_WIN
    if (g_pThread) {
        g_pThread->quit();
        g_pThread->wait();

        delete g_pThread;
        g_pThread = nullptr;
    }
#endif
    // TODO reset all widgets
    if (g_pInfoDB) {
        delete g_pInfoDB;
        g_pInfoDB = nullptr;
    }

    _clearProcessMemory();

    g_mrDisasm = {};
    g_mrStack = {};
    g_mrHex = {};

    if (g_pTimer) {
        g_pTimer->stop();
        delete g_pTimer;
        g_pTimer = nullptr;
    }

    g_state = {};

    _stateChanged();
}

void XDebuggerWidget::resetWidgets()
{
    ui->tabWidgetMain->setCurrentIndex(MT_CPU);

    ui->widgetDisasm->clear();
    ui->widgetHex1->clear();
    ui->widgetHex2->clear();
    ui->widgetHex3->clear();
    ui->widgetHex4->clear();
    ui->widgetHex5->clear();
    ui->widgetRegs->clear();
    ui->widgetStack->clear();
}

void XDebuggerWidget::errorMessageSlot(const QString &sErrorMessage)
{
    QMessageBox::critical(this, tr("Error"), sErrorMessage);
    // TODO
    writeToLog(sErrorMessage);
}

void XDebuggerWidget::infoMessageSlot(const QString &sInfoMessage)
{
    // TODO
    writeToLog(sInfoMessage);
}

void XDebuggerWidget::warningMessageSlot(const QString &sInfoMessage)
{
    // TODO
    writeToLog(sInfoMessage);
}

void XDebuggerWidget::writeToLog(const QString &sText)
{
    // TODO Show in title count of new messages
    ui->plainTextEditLog->appendPlainText(sText);
}

void XDebuggerWidget::registerShortcuts(bool bState)
{
    if (bState) {
        if (!shortCuts[SC_DISASM_BREAKPOINTTOGGLE])
            shortCuts[SC_DISASM_BREAKPOINTTOGGLE] = new QShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_TOGGLE), this, SLOT(_breakpointToggle()));
        if (!shortCuts[SC_DISASM_BREAKPOINTREMOVE])
            shortCuts[SC_DISASM_BREAKPOINTREMOVE] = new QShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_REMOVE), this, SLOT(_breakpointRemove()));
        if (!shortCuts[SC_DISASM_BREAKPOINTCONDITIONAL])
            shortCuts[SC_DISASM_BREAKPOINTCONDITIONAL] =
                new QShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_CONDITIONAL), this, SLOT(_breakpointConditional()));
    } else {
        for (qint32 i = 0; i < __SC_SIZE; i++) {
            if (shortCuts[i]) {
                delete shortCuts[i];
                shortCuts[i] = nullptr;
            }
        }
    }
    // TODO more
    // TODO Check if main window contains the shortcuts

    if (bState) {
#ifdef QT_DEBUG
        qDebug("registerShortcuts");
#endif
        //        if(!g_scRun)                    g_scRun                     =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_RUN), this,SLOT(_run()));
        //        if(!g_scStepInto)               g_scStepInto                =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPINTO),
        //        this,SLOT(_stepInto())); if(!g_scStepOver)               g_scStepOver                =new
        //        QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPOVER),               this,SLOT(_stepOver())); if(!g_scBreakpointToggle)
        //        g_scBreakpointToggle         =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_DISASM_BREAKPOINTTOGGLE),
        //        this,SLOT(_toggleBreakpoint()));
    } else {
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
    qint32 nIndex = ui->tabWidgetMain->currentIndex();

    if (nIndex == MT_CPU) {
        ui->widgetDisasm->reload(true);
        ui->widgetHex1->reload(true);
        ui->widgetHex2->reload(true);
        ui->widgetHex3->reload(true);
        ui->widgetHex4->reload(true);
        ui->widgetHex5->reload(true);
        ui->widgetStack->reload(true);
        ui->widgetRegs->reload();
    } else if (nIndex == MT_BREAKPOINTS) {
        ui->widgetBreakpoints->reload();
    } else if (nIndex == MT_MEMORYMAP) {
        ui->widgetProcessMemoryMap->reload();
    } else if (nIndex == MT_CALLSTACK) {
        // TODO
    } else if (nIndex == MT_THREADS) {
        ui->widgetThreads->reload();
    } else if (nIndex == MT_HANDLES) {
        // TODO
    } else if (nIndex == MT_MODULES) {
        ui->widgetProcessModules->reload();
    } else if (nIndex == MT_SYMBOLS) {
        ui->widgetSymbols->reload(true);
    }
}

void XDebuggerWidget::followInDisasm(XADDR nAddress)
{
    // TODO memory manager
    // TODO aprox Address

    if (!XProcess::isAddressInMemoryRegion(&g_mrDisasm, nAddress)) {
        g_mrDisasm = XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(), nAddress);

        XProcess *pProcessMemory = _getProcessMemory(g_mrDisasm.nAddress, g_mrDisasm.nSize);

        if (pProcessMemory) {
            XBinary binary(pProcessMemory, true, g_mrDisasm.nAddress);
            binary.setArch(g_osInfo.sArch);
            binary.setMode(g_osInfo.mode);

            XDisasmView::OPTIONS disasmOptions = {};
            disasmOptions.nInitAddress = nAddress;
            disasmOptions.memoryMapRegion = binary.getMemoryMap();
            disasmOptions.bAprox = true;
            ui->widgetDisasm->setData(pProcessMemory, disasmOptions, false);
            ui->widgetDisasm->setReadonly(false);
        }
    } else {
        ui->widgetDisasm->goToAddress(nAddress, true, true);
    }

    ui->widgetDisasm->setSelectionAddress(nAddress, 1);
}

void XDebuggerWidget::followInHex(XADDR nAddress)
{
    if (!XProcess::isAddressInMemoryRegion(&g_mrHex, nAddress)) {
        g_mrHex = XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(), nAddress);

        XProcess *pProcessMemory = _getProcessMemory(g_mrHex.nAddress, g_mrHex.nSize);

        if (pProcessMemory) {
            XHexView::OPTIONS hexOptions = {};
            hexOptions.nStartAddress = g_mrHex.nAddress;
            hexOptions.nStartSelectionOffset = nAddress - hexOptions.nStartAddress;
            hexOptions.addressMode = XHexView::LOCMODE_ADDRESS;
            ui->widgetHex1->setData(pProcessMemory, hexOptions, false); // TODO Current
            ui->widgetHex1->setReadonly(false); // TODO Current
        }
    } else {
        ui->widgetHex1->goToAddress(nAddress); // TODO Current
    }

    ui->widgetHex1->setSelectionAddress(nAddress, 1); // TODO Current
}

void XDebuggerWidget::followInStack(XADDR nAddress)
{
#ifdef Q_OS_WIN
    XProcess::getCallStack(ui->widgetStack->getXInfoDB()->getProcessInfo()->hProcess, ui->widgetStack->getXInfoDB()->getProcessInfo()->hMainThread);
#endif

    if (!XProcess::isAddressInMemoryRegion(&g_mrStack, nAddress)) {
        g_mrStack = XProcess::getMemoryRegionByAddress(g_pInfoDB->getCurrentMemoryRegionsList(), nAddress);

        XProcess *pProcessMemory = _getProcessMemory(g_mrStack.nAddress, g_mrStack.nSize);

        if (pProcessMemory) {
            XStackView::OPTIONS stackOptions = {};
            stackOptions.nStartAddress = g_mrStack.nAddress;
            stackOptions.nCurrentAddress = nAddress;
            stackOptions.nCurrentStackPointer = nAddress;
            ui->widgetStack->setData(pProcessMemory, stackOptions, false);  // TODO XInfoDB !!!
            ui->widgetStack->setReadonly(false);
        }
    } else {
        ui->widgetStack->setCurrentStackPointer(nAddress);
        ui->widgetStack->goToAddress(nAddress);
    }

    ui->widgetStack->setSelectionAddress(nAddress);
}

void XDebuggerWidget::on_toolButtonAnimateStepInto_clicked()
{
    animateStepInto();
}

void XDebuggerWidget::on_toolButtonAnimateStepOver_clicked()
{
    animateStepOver();
}

void XDebuggerWidget::on_toolButtonAnimateStop_clicked()
{
    animateStop();
}

void XDebuggerWidget::on_toolButtonTraceStepInto_clicked()
{
    traceStepInto();
}

void XDebuggerWidget::on_toolButtonTraceStepOver_clicked()
{
    traceStepOver();
}

void XDebuggerWidget::on_toolButtonTraceStop_clicked()
{
    traceStop();
}

// void XDebuggerWidget::memoryRegionsListChangedSlot()
//{
//     qDebug("void XDebuggerWidget::memoryRegionsListChangedSlot()");

//    QList<XProcess::MEMORY_REGION> *pListMR = g_pInfoDB->getCurrentMemoryRegionsList();

//    ui->comboBoxHexRegion->blockSignals(true);

//    ui->comboBoxHexRegion->clear();

//    qint32 nNumberOfRecirds = pListMR->count();

//    for (qint32 i = 0; i < nNumberOfRecirds; i++) {
//        QString sString = XProcess::memoryRegionToString(pListMR->at(i));

//        ui->comboBoxHexRegion->addItem(sString);

//        // TODO
//    }

//    ui->comboBoxHexRegion->blockSignals(false);
//}

void XDebuggerWidget::modulesListChangedSlot()
{
    // TODO change Threads
    qDebug("void XDebuggerWidget::modulesListChangedSlot()");
}

void XDebuggerWidget::threadsListChangedSlot()
{
    qDebug("void XDebuggerWidget::threadsListChangedSlot()");
}

void XDebuggerWidget::registersListChangedSlot()
{
    qDebug("void XDebuggerWidget::registersListChangedSlot()");
    updateWidget(MT_CPU);  // TODO update if active or there are changes
}

void XDebuggerWidget::updateWidget(MT mt)
{
    // TODO Hashes
    if (mt == MT_CPU) {
        quint64 nInstructionPointer = g_pInfoDB->getCurrentInstructionPointerCache();
        quint64 nStackPointer = g_pInfoDB->getCurrentStackPointerCache();

        // TODO Check tab
        followInDisasm(nInstructionPointer);
        followInStack(nStackPointer);

        ui->widgetRegs->reload();
    }
}

void XDebuggerWidget::on_pushButtonCommandRun_clicked()
{
    QString sCommand = ui->lineEditCommand->text();

    XDebuggerConsole::COMMAND_RESULT commandResult = {};

    XDebuggerConsole::commandControl(&commandResult, sCommand, g_pDebugger);

    qint32 nNumberOfTexts = commandResult.listTexts.count();
    qint32 nNumberOfErrors = commandResult.listErrors.count();

    for (qint32 i = 0; i < nNumberOfTexts; i++) {
        infoMessageSlot(commandResult.listTexts.at(i));
    }

    for (qint32 i = 0; i < nNumberOfErrors; i++) {
        errorMessageSlot(commandResult.listErrors.at(i));
    }

    reload();
}

void XDebuggerWidget::addSymbolsSlot(QString sFileName, XADDR nImageBase)
{
#ifdef QT_DEBUG
    qDebug("addSymbolsSlot %s >>>>>>>>>", sFileName.toUtf8().data());
#endif
    g_pMutex->lock();

    DialogXInfoDBTransferProcess dialogTransfer(this);

    XInfoDBTransfer::OPTIONS options = {};
    options.sFileName = sFileName;
    options.bIsImage = false;
    options.nModuleAddress = nImageBase;
    options.fileType = XBinary::FT_PE;

    dialogTransfer.setData(g_pInfoDB, XInfoDBTransfer::COMMAND_SYMBOLS, options);

    dialogTransfer.showDialogDelay();

    g_pMutex->unlock();

#ifdef QT_DEBUG
    qDebug("addSymbolsSlot %s <<<<<<<<<", sFileName.toUtf8().data());
#endif
}

void XDebuggerWidget::removeSymbolsSlot(QString sFileName)
{
#ifdef QT_DEBUG
    qDebug("removeSymbolsSlot %s", sFileName.toUtf8().data());
#endif
    g_pMutex->lock();
    g_pMutex->unlock();
}
