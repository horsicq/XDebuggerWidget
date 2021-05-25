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

void DW_DisasmView::_setBreakpoint()
{
    qDebug("void DW_DisasmView::_setBreakpoint()");
    // TODO XHexView

    if(g_pDebugger)
    {
        qint64 nAddress=getSelectionInitAddress();

        if(nAddress!=-1)
        {
            if(g_pDebugger->setBP(nAddress))
            {
                QList<XBinary::MEMORY_REPLACE> listReplaces=g_pDebugger->getMemoryReplaces();

                // mb TODO XHex too
                qDebug("Breakpoit set");
                reload(true);
            }
        }
    }
}

void DW_DisasmView::contextMenu(const QPoint &pos)
{
    QMenu menuBreakpoint(tr("Breakpoint"),this);
    QAction actionSetBreakpoint(tr("Set Breakpoint"),this);
    actionSetBreakpoint.setShortcut(getShortcuts()->getShortcut(XShortcuts::ID_DEBUGGER_SETBREAKPOINT));
    connect(&actionSetBreakpoint,SIGNAL(triggered()),this,SLOT(_setBreakpoint()));

    menuBreakpoint.addAction(&actionSetBreakpoint);

    QMenu contextMenu(this);
    contextMenu.addMenu(&menuBreakpoint);
    contextMenu.exec(pos);
}

void DW_DisasmView::registerShortcuts(bool bState)
{
    Q_UNUSED(bState)
}
