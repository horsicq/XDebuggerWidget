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
#include "dw_disasmview.h"

DW_DisasmView::DW_DisasmView(QWidget *pParent) : XDisasmView(pParent)
{
    g_pDebugger=nullptr;
}

void DW_DisasmView::setDebugger(XAbstractDebugger *pDebugger)
{
    g_pDebugger=pDebugger;
}

void DW_DisasmView::showStatus()
{
    qDebug("void DW_DisasmView::showStatus()");

    QList<XBinary::MEMORY_REPLACE> listReplaces=g_pDebugger->getMemoryReplaces();

    setMemoryReplaces(listReplaces);

    reload(true);
}

void DW_DisasmView::_setRemoveBreakpoint()
{
    qDebug("void DW_DisasmView::_setBreakpoint()");
    // TODO XHexView

    if(g_pDebugger)
    {
        qint64 nAddress=getSelectionInitAddress();

        if(nAddress!=-1)
        {
            if(!g_pDebugger->isSoftwareBreakpointPresent(nAddress))
            {
                if(g_pDebugger->setSoftwareBreakpoint(nAddress))
                {
//                    qDebug("Breakpoit set");
                    XAbstractDebugger::DEBUG_ACTION dbgAction={};
                    dbgAction.type=XAbstractDebugger::DBT_SETSOFTWAREBREAKPOINT;
                    dbgAction.var[0]=nAddress;

                    emit debugAction(dbgAction);

                    showStatus();
                }
            }
            else
            {
                if(g_pDebugger->removeSoftwareBreakpoint(nAddress))
                {
//                    qDebug("Breakpoit unset");
                    XAbstractDebugger::DEBUG_ACTION dbgAction={};
                    dbgAction.type=XAbstractDebugger::DBT_REMOVESOFTWAREBREAKPOINT;
                    dbgAction.var[0]=nAddress;

                    emit debugAction(dbgAction);

                    showStatus();
                }
            }
        }
    }
}

void DW_DisasmView::contextMenu(const QPoint &pos)
{
    // TODO Search
    // TODO Signatures
    // TODO Copy

    if(g_pDebugger)
    {
        QMenu menuBreakpoint(tr("Breakpoint"),this);

        QAction actionSetRemoveBreakpoint(this);

        qint64 nAddress=getSelectionInitAddress();

        if(!g_pDebugger->isSoftwareBreakpointPresent(nAddress))
        {
            actionSetRemoveBreakpoint.setText(tr("Set Breakpoint"));
        }
        else
        {
            actionSetRemoveBreakpoint.setText(tr("Remove Breakpoint"));
        }

        actionSetRemoveBreakpoint.setShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_DEBUG_SETREMOVEBREAKPOINT));
        connect(&actionSetRemoveBreakpoint,SIGNAL(triggered()),this,SLOT(_setRemoveBreakpoint()));

        menuBreakpoint.addAction(&actionSetRemoveBreakpoint);

        QMenu contextMenu(this);
        contextMenu.addMenu(&menuBreakpoint);
        contextMenu.exec(pos);
    }
}

void DW_DisasmView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}
