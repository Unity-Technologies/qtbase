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

#include "qwinrtbackingstore.h"

#include "qwinrtscreen.h"
#ifdef Q_WINRT_GL
#  include "qwinrteglcontext.h"
#  include <QOpenGLPaintDevice>
#else
#include <QImage>
#endif
#include <QScreen>

QT_BEGIN_NAMESPACE

QWinRTBackingStore::QWinRTBackingStore(QWindow *window)
    : QPlatformBackingStore(window)
{
    m_screen = static_cast<QWinRTScreen*>(window->screen()->handle());
#ifdef Q_WINRT_GL
    m_context.reset(new QOpenGLContext);
    m_context->setFormat(window->requestedFormat());
    m_context->setScreen(window->screen());
    m_context->create();
#endif
}

QWinRTBackingStore::~QWinRTBackingStore()
{
}

QPaintDevice *QWinRTBackingStore::paintDevice()
{
    return m_paintDevice.data();
}

void QWinRTBackingStore::flush(QWindow *window, const QRegion &region, const QPoint &offset)
{
    Q_UNUSED(offset)
    // Write to screen's buffer
#ifdef Q_WINRT_GL
    Q_UNUSED(region)
    m_context->swapBuffers(window);
#else
    m_screen->update(region, window->position(), m_paintDevice->constBits(), m_paintDevice->width() * 4);
#endif
}

void QWinRTBackingStore::resize(const QSize &size, const QRegion &staticContents)
{
    Q_UNUSED(size);
    Q_UNUSED(staticContents);
}

void QWinRTBackingStore::beginPaint(const QRegion &region)
{
    Q_UNUSED(region)
#ifdef Q_WINRT_GL
    window()->setSurfaceType(QSurface::OpenGLSurface);
    m_context->makeCurrent(window());
    if (m_paintDevice.isNull())
        m_paintDevice.reset(new QOpenGLPaintDevice(window()->size()));
    if (m_paintDevice->size() != window()->size())
        m_paintDevice->setSize(window()->size());
#else
    window()->setSurfaceType(QSurface::RasterSurface);
    if (m_paintDevice.isNull() || m_paintDevice->size() != window()->size())
        m_paintDevice.reset(new QImage(window()->size(), QImage::Format_ARGB32_Premultiplied));
    m_paintDevice->fill(Qt::transparent);
#endif
}

void QWinRTBackingStore::endPaint()
{
}

QT_END_NAMESPACE
