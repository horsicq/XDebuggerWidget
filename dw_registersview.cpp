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
#include "dw_registersview.h"

DW_RegistersView::DW_RegistersView(QWidget *pParent) : XRegistersView(pParent)
{
    memset(shortCuts, 0, sizeof shortCuts);
}

void DW_RegistersView::contextMenu(const QPoint &pos)
{
    QMenu contextMenu(this);

    qint32 nIndex = -1;
    XInfoDB::XREG reg = pointToReg(pos, &nIndex);

    if (nIndex != -1) {
        qDebug("Index: %d", nIndex);
    }

    QMenu menuView(tr("View"), this);

    QAction actionGeneral(QString("General"), this);
    actionGeneral.setCheckable(true);
    actionGeneral.setChecked(getOptions().bGeneral);
    connect(&actionGeneral, SIGNAL(triggered()), this, SLOT(_actionViewGeneral()));
    menuView.addAction(&actionGeneral);

    QAction actionIP(QString("IP"), this);
    actionIP.setCheckable(true);
    actionIP.setChecked(getOptions().bIP);
    connect(&actionIP, SIGNAL(triggered()), this, SLOT(_actionViewIP()));
    menuView.addAction(&actionIP);

#ifdef Q_PROCESSOR_X86
    QAction actionFlags(QString("Flags"), this);
    actionFlags.setCheckable(true);
    actionFlags.setChecked(getOptions().bFlags);
    connect(&actionFlags, SIGNAL(triggered()), this, SLOT(_actionViewFlags()));
    menuView.addAction(&actionFlags);

    QAction actionSegments(QString("Segments"), this);
    actionSegments.setCheckable(true);
    actionSegments.setChecked(getOptions().bSegments);
    connect(&actionSegments, SIGNAL(triggered()), this, SLOT(_actionViewSegments()));
    menuView.addAction(&actionSegments);

    QAction actionDebug(QString("Debug"), this);
    actionDebug.setCheckable(true);
    actionDebug.setChecked(getOptions().bDebug);
    connect(&actionDebug, SIGNAL(triggered()), this, SLOT(_actionViewDebug()));
    menuView.addAction(&actionDebug);

    QAction actionFloat(QString("Float"), this);
    actionFloat.setCheckable(true);
    actionFloat.setChecked(getOptions().bFloat);
    connect(&actionFloat, SIGNAL(triggered()), this, SLOT(_actionViewFloat()));
    menuView.addAction(&actionFloat);

    QAction actionXMM(QString("XMM"), this);
    actionXMM.setCheckable(true);
    actionXMM.setChecked(getOptions().bXMM);
    connect(&actionXMM, SIGNAL(triggered()), this, SLOT(_actionViewXMM()));
    menuView.addAction(&actionXMM);
#endif

    contextMenu.addMenu(&menuView);

    contextMenu.exec(viewport()->mapToGlobal(pos));
}

void DW_RegistersView::registerShortcuts(bool bState)
{
    if (bState) {
        if (!shortCuts[SC_REGISTERS_EDIT])
            shortCuts[SC_REGISTERS_EDIT] = new QShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_REGISTERS_EDIT), this, SLOT(_actionEdit()));
    } else {
        for (qint32 i = 0; i < __SC_SIZE; i++) {
            if (shortCuts[i]) {
                delete shortCuts[i];
                shortCuts[i] = nullptr;
            }
        }
    }
}
