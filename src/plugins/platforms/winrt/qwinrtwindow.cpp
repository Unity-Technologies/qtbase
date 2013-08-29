/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwinrtwindow.h"

#include <qpa/qwindowsysteminterface.h>
#include <qpa/qplatformscreen.h>

#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

QWinRTWindow::QWinRTWindow(QWindow *window)
    : QPlatformWindow(window)
{
    setWindowFlags(window->flags());
    setWindowState(window->windowState());
    setGeometry(window->geometry());
}

void QWinRTWindow::setGeometry(const QRect &rect)
{
    if (window()->isTopLevel()) {
        QPlatformWindow::setGeometry(screen()->geometry());
        QWindowSystemInterface::handleGeometryChange(window(), screen()->geometry());
        QWindowSystemInterface::handleExposeEvent(window(), screen()->geometry());
    } else {
        QPlatformWindow::setGeometry(rect);
        QWindowSystemInterface::handleGeometryChange(window(), rect);
        QWindowSystemInterface::handleExposeEvent(window(), QRect(QPoint(), rect.size()));
    }
}

QSurfaceFormat QWinRTWindow::format() const
{
    static QSurfaceFormat fmt;

    if (!fmt.alphaBufferSize()) {
        fmt.setAlphaBufferSize(8);
        fmt.setRedBufferSize(8);
        fmt.setGreenBufferSize(8);
        fmt.setBlueBufferSize(8);
    }
    return fmt;
}

void QWinRTWindow::setWindowState(Qt::WindowState state)
{
    // The screen is aware of 4 logical window states:
    // hidden (Minimized), snapped (NoState), filled (Maximized), FullScreen
    // This is irrelevant to the window size, which always matches the viewport size
    Qt::WindowState screenState = static_cast<QWinRTScreen *>(screen())->windowState();
    // Non-top-level and QWinRTScreen-initiated changes are OK
    if (!window()->isTopLevel() || state == screenState) {
        QWindowSystemInterface::handleWindowStateChanged(window(), state);
        return;
    }
    // These situations are initiated by the application; we can only handle one case: unsnap.
    // We can't minimize or restore programmatically, so those cases are ignored.
    if (screenState == Qt::WindowNoState && (state == Qt::WindowMaximized || Qt::WindowFullScreen))
        static_cast<QWinRTScreen *>(screen())->tryUnsnap();
}
