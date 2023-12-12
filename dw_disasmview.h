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
#ifndef DW_DISASMVIEW_H
#define DW_DISASMVIEW_H

#include "xdisasmview.h"

class DW_DisasmView : public XDisasmView {
    Q_OBJECT

public:
    explicit DW_DisasmView(QWidget *pParent = nullptr);

public slots:
    void _breakpointToggle();
    void _breakpointRemove();
    void _breakpointConditional();
    void _breakpointHardware0();
    void _breakpointHardware1();
    void _breakpointHardware2();
    void _breakpointHardware3();
    void _breakpointHardwareShow();
    void _goToAddress();

protected:
    virtual void contextMenu(const QPoint &pos);
    virtual void registerShortcuts(bool bState);
};

#endif  // DW_DISASMVIEW_H
