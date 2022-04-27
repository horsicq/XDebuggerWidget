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
#include "xdebuggeroptionswidget.h"
#include "ui_xdebuggeroptionswidget.h"

XDebuggerOptionsWidget::XDebuggerOptionsWidget(QWidget *pParent) :
    QWidget(pParent),
    ui(new Ui::XDebuggerOptionsWidget)
{
    ui->setupUi(this);
}

XDebuggerOptionsWidget::~XDebuggerOptionsWidget()
{
    delete ui;
}

void XDebuggerOptionsWidget::setOptions(XOptions *pOptions)
{
    g_pOptions=pOptions;

    reload();
}

void XDebuggerOptionsWidget::save()
{
    g_pOptions->getCheckBox(ui->checkBoxDebuggerBreakPointEntryPoint,XOptions::ID_DEBUGGER_BREAKPOINT_ENTRYPOINT);
    g_pOptions->getCheckBox(ui->checkBoxDebuggerBreakPointDLLMain,XOptions::ID_DEBUGGER_BREAKPOINT_DLLMAIN);
    g_pOptions->getCheckBox(ui->checkBoxDebuggerBreakPointTLSFunctions,XOptions::ID_DEBUGGER_BREAKPOINT_TLSFUNCTIONS);
}

void XDebuggerOptionsWidget::setDefaultValues(XOptions *pOptions)
{
    pOptions->addID(XOptions::ID_DEBUGGER_BREAKPOINT_ENTRYPOINT,true);
    pOptions->addID(XOptions::ID_DEBUGGER_BREAKPOINT_DLLMAIN,true);
    pOptions->addID(XOptions::ID_DEBUGGER_BREAKPOINT_TLSFUNCTIONS,true);
}

void XDebuggerOptionsWidget::reload()
{
    g_pOptions->setCheckBox(ui->checkBoxDebuggerBreakPointEntryPoint,XOptions::ID_DEBUGGER_BREAKPOINT_ENTRYPOINT);
    g_pOptions->setCheckBox(ui->checkBoxDebuggerBreakPointDLLMain,XOptions::ID_DEBUGGER_BREAKPOINT_DLLMAIN);
    g_pOptions->setCheckBox(ui->checkBoxDebuggerBreakPointTLSFunctions,XOptions::ID_DEBUGGER_BREAKPOINT_TLSFUNCTIONS);
}
