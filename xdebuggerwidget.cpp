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
    g_scSetBreakpoint=nullptr;

    g_mrCode={};
    g_mrStack={};
    g_mrHex={};
    g_pPDCode=nullptr;
    g_pPDStack=nullptr;
    g_pPDHex=nullptr;

    ui->widgetRegs->setMode(XRegistersView::MODE_X86_32);

    connect(this,SIGNAL(showStatus()),this,SLOT(onShowStatus()));
//    ui->widgetDisasm->installEventFilter(this);
//    XDebuggerWidget::registerShortcuts(true);
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
    connect(g_pDebugger,SIGNAL(eventEntryPoint(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onEntryPoint(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventStep(XAbstractDebugger::BREAKPOINT_INFO*)),this,SLOT(onStep(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *)),this,SLOT(onBreakPoint(XAbstractDebugger::BREAKPOINT_INFO *)),Qt::DirectConnection);
    connect(g_pDebugger,SIGNAL(eventExitProcess(XAbstractDebugger::EXITPROCESS_INFO*)),this,SLOT(onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *)),Qt::DirectConnection);

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
    qDebug("void XDebuggerWidget::onCreateProcess(XAbstractDebugger::PROCESS_INFO *pProcessInfo)");
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
    qDebug("void XDebuggerWidget::onExitProcess(XAbstractDebugger::EXITPROCESS_INFO *pExitProcessInfo)");

    // TODO Clear screen
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

void XDebuggerWidget::_setBreakpoint()
{
    ui->widgetDisasm->_setBreakpoint();
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

void XDebuggerWidget::registerShortcuts(bool bState)
{ 
    qDebug("TEST");
    if(bState)
    {
        qDebug("registerShortcuts");
        if(!g_scRun)                g_scRun             =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_RUN),            this,SLOT(_run()));
        if(!g_scStepInto)           g_scStepInto        =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_STEPINTO),       this,SLOT(_stepInto()));
        if(!g_scSetBreakpoint)      g_scSetBreakpoint   =new QShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_SETBREAKPOINT),  this,SLOT(_setBreakpoint()));
    }
    else
    {
        qDebug("unregisterShortcuts");
        if(g_scRun)                 {delete g_scRun;                g_scRun=nullptr;}
        if(g_scStepInto)            {delete g_scStepInto;           g_scStepInto=nullptr;}
        if(g_scSetBreakpoint)       {delete g_scSetBreakpoint;      g_scSetBreakpoint=nullptr;}
    }
}
