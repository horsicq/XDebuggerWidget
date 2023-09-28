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
#include "xdebuggerloaddialog.h"
#include "ui_xdebuggerloaddialog.h"

XDebuggerLoadDialog::XDebuggerLoadDialog(QWidget *pParent, XAbstractDebugger::OPTIONS *pOptions) : QDialog(pParent), ui(new Ui::XDebuggerLoadDialog)
{
    ui->setupUi(this);

    g_pOptions = pOptions;

    ui->lineEditFilename->setText(pOptions->sFileName);
    ui->lineEditArguments->setText(pOptions->sArguments);
    ui->lineEditDirectory->setText(pOptions->sDirectory);

    _setupCheckBox(ui->checkBoxShowConsole, XAbstractDebugger::OPTIONS_TYPE_SHOWCONSOLE);
    _setupCheckBox(ui->checkBoxBreakpointDLLMain, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTDLLMAIN);
    _setupCheckBox(ui->checkBoxBreakpointEntryPoint, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTENTRYPOINT);
    _setupCheckBox(ui->checkBoxBreakpointSystem, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTSYSTEM);
    _setupCheckBox(ui->checkBoxBreakpointTLSFunctions, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTTLSFUNCTION);
}

XDebuggerLoadDialog::~XDebuggerLoadDialog()
{
    delete ui;
}

void XDebuggerLoadDialog::_setupCheckBox(QCheckBox *pCheckBox, XAbstractDebugger::OPTIONS_TYPE optionsStype)
{
    if (g_pOptions->records[optionsStype].bValid) {
        pCheckBox->show();
        pCheckBox->setChecked(g_pOptions->records[optionsStype].varValue.toBool());
    } else {
        pCheckBox->hide();
    }
}

void XDebuggerLoadDialog::_getCheckBoxValue(QCheckBox *pCheckBox, XAbstractDebugger::OPTIONS_TYPE optionsStype)
{
    if (g_pOptions->records[optionsStype].bValid) {
        pCheckBox->setChecked(g_pOptions->records[optionsStype].varValue.toBool());
        g_pOptions->records[optionsStype].varValue = pCheckBox->isChecked();
    }
}

void XDebuggerLoadDialog::on_pushButtonCancel_clicked()
{
    reject();
}

void XDebuggerLoadDialog::on_pushButtonOK_clicked()
{
    g_pOptions->sFileName = ui->lineEditFilename->text();
    g_pOptions->sArguments = ui->lineEditArguments->text();
    g_pOptions->sDirectory = ui->lineEditDirectory->text();
    _getCheckBoxValue(ui->checkBoxShowConsole, XAbstractDebugger::OPTIONS_TYPE_SHOWCONSOLE);
    _getCheckBoxValue(ui->checkBoxBreakpointDLLMain, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTDLLMAIN);
    _getCheckBoxValue(ui->checkBoxBreakpointEntryPoint, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTENTRYPOINT);
    _getCheckBoxValue(ui->checkBoxBreakpointSystem, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTSYSTEM);
    _getCheckBoxValue(ui->checkBoxBreakpointTLSFunctions, XAbstractDebugger::OPTIONS_TYPE_BREAKPOINTTLSFUNCTION);

    accept();
}
