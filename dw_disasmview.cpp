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
#include "dw_disasmview.h"

DW_DisasmView::DW_DisasmView(QWidget *pParent) : XDisasmView(pParent)
{
}

void DW_DisasmView::_breakpointToggle()
{
    if (getXInfoDB()) {
        XADDR nAddress = getSelectionInitAddress();

        if (nAddress != -1) {
            if (getXInfoDB()->breakpointToggle(nAddress)) {
                getXInfoDB()->reload(true);
            }
        }
    }
}

void DW_DisasmView::_goToAddress()
{
    // TODO another modules
    _goToAddressSlot();
}

void DW_DisasmView::contextMenu(const QPoint &pos)
{
    // TODO Search
    // TODO Signatures
    if (isContextMenuEnable()) {
        if (getXInfoDB()) {
            QMenu contextMenu(this);

            QMenu menuBreakpoint(tr("Breakpoint"), this);
            QMenu menuGoTo(tr("Go to"), this);
            QMenu menuCopy(tr("Copy"), this);
            QMenu menuEdit(tr("Edit"), this);
            QMenu menuFollowIn(tr("Follow in"), this);

            QAction actionBreakpointToggle(tr("Toggle"), this);
            actionBreakpointToggle.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_TOGGLE));
            connect(&actionBreakpointToggle, SIGNAL(triggered()), this, SLOT(_breakpointToggle()));

            QAction actionGoToAddress(tr("Address"), this);
            actionGoToAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_GOTO_ADDRESS));
            connect(&actionGoToAddress, SIGNAL(triggered()), this, SLOT(_goToAddressSlot()));

            QAction actionCopyAddress(tr("Address"), this);
            actionCopyAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_COPY_ADDRESS));
            connect(&actionCopyAddress, SIGNAL(triggered()), this, SLOT(_copyAddressSlot()));

            QAction actionEditHex(tr("Hex"), this);
            actionEditHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_EDIT_HEX));
            connect(&actionEditHex, SIGNAL(triggered()), this, SLOT(_editHex()));

            QAction actionFollowInHex(tr("Hex"), this);
            actionFollowInHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_FOLLOWIN_HEX));
            connect(&actionFollowInHex, SIGNAL(triggered()), this, SLOT(_followInHexSlot()));

            menuGoTo.addAction(&actionGoToAddress);
            contextMenu.addMenu(&menuGoTo);

            menuBreakpoint.addAction(&actionBreakpointToggle);
            contextMenu.addMenu(&menuBreakpoint);

            menuCopy.addAction(&actionCopyAddress);
            contextMenu.addMenu(&menuCopy);

            menuEdit.addAction(&actionEditHex);
            contextMenu.addMenu(&menuEdit);

            menuFollowIn.addAction(&actionFollowInHex);
            contextMenu.addMenu(&menuFollowIn);

            contextMenu.exec(pos);

            //        qint64 nAddress=getSelectionInitAddress();
            //        if(!g_pDebugger->isSoftwareBreakpointPresent(nAddress))
            //        {
            //            actionToggle.setText(tr("Set Breakpoint"));
            //        }
            //        else
            //        {
            //            actionToggle.setText(tr("Remove Breakpoint"));
            //        }
        }
    }
}

void DW_DisasmView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)

    // TODO !!!
}
