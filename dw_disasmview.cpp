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
#include "dw_disasmview.h"

DW_DisasmView::DW_DisasmView(QWidget *pParent) : XDisasmView(pParent)
{
}

void DW_DisasmView::_breakpointToggle()
{
    // TODO isToggleEnable
    if (getXInfoDB()) {
        XADDR nAddress = getSelectionInitAddress();

        if (nAddress != (XADDR)-1) {
            qint64 nSelectionSize = getSelectionInitSize();

            if (getXInfoDB()->breakpointToggle(nAddress)) {
                getXInfoDB()->reloadView();
            }

            setSelectionAddress(nAddress, nSelectionSize);
        }
    }
}

void DW_DisasmView::_breakpointRemove()
{
    // TODO isRemoveEnable
    if (getXInfoDB()) {
        XADDR nAddress = getSelectionInitAddress();

        if (nAddress != (XADDR)-1) {
            qint64 nSelectionSize = getSelectionInitSize();

            if (getXInfoDB()->breakpointRemove(nAddress)) {
                getXInfoDB()->reloadView();
            }

            setSelectionAddress(nAddress, nSelectionSize);
        }
    }
}

void DW_DisasmView::_breakpointConditional()
{
    // TODO isConditionalEnable
    if (getXInfoDB()) {
        // TODO
    }
}

void DW_DisasmView::_breakpointHardware0()
{
}

void DW_DisasmView::_breakpointHardware1()
{
}

void DW_DisasmView::_breakpointHardware2()
{
}

void DW_DisasmView::_breakpointHardware3()
{
}

void DW_DisasmView::_breakpointHardwareShow()
{
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
            QMenu menuBreakpointHardware(tr("Hardware"), this);
            QMenu menuGoTo(tr("Go to"), this);
            QMenu menuCopy(tr("Copy"), this);
            QMenu menuEdit(tr("Edit"), this);
            QMenu menuFollowIn(tr("Follow in"), this);

            QAction actionBreakpointToggle(tr("Toggle"), this);
            actionBreakpointToggle.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_TOGGLE));
            connect(&actionBreakpointToggle, SIGNAL(triggered()), this, SLOT(_breakpointToggle()));

            QAction actionBreakpointRemove(tr("Remove"), this);
            actionBreakpointRemove.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_REMOVE));
            connect(&actionBreakpointRemove, SIGNAL(triggered()), this, SLOT(_breakpointRemove()));

            QAction actionBreakpointConditional(tr("Conditional"), this);
            actionBreakpointConditional.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_BREAKPOINT_CONDITIONAL));
            connect(&actionBreakpointConditional, SIGNAL(triggered()), this, SLOT(_breakpointConditional()));

            QAction actionGoToAddress(tr("Address"), this);
            actionGoToAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_GOTO_ADDRESS));
            connect(&actionGoToAddress, SIGNAL(triggered()), this, SLOT(_goToAddressSlot()));  // TODO go to custom slot

            QAction actionCopyAddress(tr("Address"), this);
            actionCopyAddress.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_COPY_ADDRESS));
            connect(&actionCopyAddress, SIGNAL(triggered()), this, SLOT(_copyAddressSlot()));

            QAction actionEditHex(tr("Hex"), this);
            actionEditHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_EDIT_HEX));
            connect(&actionEditHex, SIGNAL(triggered()), this, SLOT(_editHex()));

            QAction actionFollowInHex(tr("Hex"), this);
            actionFollowInHex.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_DISASM_FOLLOWIN_HEX));
            connect(&actionFollowInHex, SIGNAL(triggered()), this, SLOT(_followInHexSlot()));

            QAction actionBreakpointHardware0("", this);
            actionBreakpointHardware0.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_BREAKPOINT_HARDWARE_0));
            connect(&actionBreakpointHardware0, SIGNAL(triggered()), this, SLOT(_breakpointHardware0()));
            QAction actionBreakpointHardware1("", this);
            actionBreakpointHardware1.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_BREAKPOINT_HARDWARE_1));
            connect(&actionBreakpointHardware1, SIGNAL(triggered()), this, SLOT(_breakpointHardware1()));
            QAction actionBreakpointHardware2("", this);
            actionBreakpointHardware2.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_BREAKPOINT_HARDWARE_2));
            connect(&actionBreakpointHardware2, SIGNAL(triggered()), this, SLOT(_breakpointHardware2()));
            QAction actionBreakpointHardware3("", this);
            actionBreakpointHardware3.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_BREAKPOINT_HARDWARE_3));
            connect(&actionBreakpointHardware3, SIGNAL(triggered()), this, SLOT(_breakpointHardware3()));

            QAction actionBreakpointHardwareShow(tr("Show"), this);
            actionBreakpointHardwareShow.setShortcut(getShortcuts()->getShortcut(X_ID_DEBUGGER_BREAKPOINT_HARDWARE_SHOW));
            connect(&actionBreakpointHardwareShow, SIGNAL(triggered()), this, SLOT(_breakpointHardwareShow()));

            menuGoTo.addAction(&actionGoToAddress);
            contextMenu.addMenu(&menuGoTo);

            menuBreakpoint.addAction(&actionBreakpointToggle);
            menuBreakpointHardware.addAction(&actionBreakpointHardware0);
            menuBreakpointHardware.addAction(&actionBreakpointHardware1);
            menuBreakpointHardware.addAction(&actionBreakpointHardware2);
            menuBreakpointHardware.addAction(&actionBreakpointHardware3);
            menuBreakpointHardware.addSeparator();
            menuBreakpointHardware.addAction(&actionBreakpointHardwareShow);

            menuBreakpoint.addMenu(&menuBreakpointHardware);
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
