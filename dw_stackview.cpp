/* Copyright (c) 2021-2025 hors<horsicq@gmail.com>
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
#include "dw_stackview.h"

DW_StackView::DW_StackView(QWidget *pParent) : XStackView(pParent)
{
}

void DW_StackView::contextMenu(const QPoint &pos)
{
    if (isContextMenuEnable()) {
        if (getXInfoDB()) {
            QMenu contextMenu(this);

            QMenu menuEdit(tr("Edit"), this);
            QMenu menuGoTo(tr("Go to"), this);
            QMenu menuCopy(tr("Copy"), this);
            QMenu menuFollowIn(tr("Follow in"), this);

            QAction actionGoToAddress(tr("Address"), this);
            actionGoToAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_HEX_GOTO_ADDRESS));
            connect(&actionGoToAddress, SIGNAL(triggered()), this, SLOT(_goToAddressSlot()));

            menuGoTo.addAction(&actionGoToAddress);
            contextMenu.addMenu(&menuGoTo);

            QAction actionCopyAddress(tr("Address"), this);
            actionCopyAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_HEX_COPY_ADDRESS));
            connect(&actionCopyAddress, SIGNAL(triggered()), this, SLOT(_copyAddressSlot()));

            menuCopy.addAction(&actionCopyAddress);
            contextMenu.addMenu(&menuCopy);

            QAction actionEditHex(tr("Hex"), this);
            actionEditHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_STACK_EDIT_HEX));
            connect(&actionEditHex, SIGNAL(triggered()), this, SLOT(_editHex()));

            menuEdit.addAction(&actionEditHex);
            contextMenu.addMenu(&menuEdit);

            QAction actionFollowInHex(tr("Hex"), this);
            actionFollowInHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_STACK_FOLLOWIN_HEX));
            connect(&actionFollowInHex, SIGNAL(triggered()), this, SLOT(_followInHexSlot()));

            QAction actionFollowInDisasm(tr("Disasm"), this);
            actionFollowInDisasm.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_STACK_FOLLOWIN_DISASM));
            connect(&actionFollowInDisasm, SIGNAL(triggered()), this, SLOT(_followInDisasmSlot()));

            menuFollowIn.addAction(&actionFollowInHex);
            menuFollowIn.addAction(&actionFollowInDisasm);
            contextMenu.addMenu(&menuFollowIn);

            contextMenu.exec(pos);
        }
    }
}

void DW_StackView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}
