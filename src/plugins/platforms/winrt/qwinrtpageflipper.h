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

#ifndef QWINRTPAGEFLIPPER_H
#define QWINRTPAGEFLIPPER_H

#include <qpa/qplatformscreenpageflipper.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtGui/qregion.h>

#include <wrl.h>
#include <d3d11_1.h>
#include <windows.ui.core.h>

QT_BEGIN_NAMESPACE

class QWinRTPageFlipper : public QPlatformScreenPageFlipper
{
  Q_OBJECT

public:
    QWinRTPageFlipper(ABI::Windows::UI::Core::ICoreWindow *window);
    ~QWinRTPageFlipper();

public:
    void update(const QRegion &region, const QPoint &offset, const void *handle, int stride);
    bool displayBuffer(QPlatformScreenBuffer *buffer);
    const QSize &size() const;
    void resize(const QSize &size);

public:
    void releaseResourcesForSuspending();
    void present();

private:
    void createDeviceResources();
    void createWindowSizeDependentResources();
    void updateForWindowSizeChange();
    void handleDeviceLost();

private:
    QRegion m_dirtyRegion;
    QRect m_windowBounds;
    QByteArray m_bufferData;

    Microsoft::WRL::ComPtr<ABI::Windows::UI::Core::ICoreWindow> m_window;
    Microsoft::WRL::ComPtr<ID3D11Device1> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext1> m_context;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_frontBuffer;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_backBuffer;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
};

QT_END_NAMESPACE

#endif // QWINRTPAGEFLIPPER_H
