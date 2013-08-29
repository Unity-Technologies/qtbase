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

#include "qwinrtpageflipper.h"

#include <d3d11_1.h>
#include <wrl.h>
#include <Windows.ApplicationModel.core.h>
#include <windows.ui.core.h>
#include <windows.ui.input.h>
#include <windows.devices.input.h>

#include <QtCore/qvector.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::System;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Devices::Input;

// Copy rectangles - TODO: use SIMD if available
inline void rectCopy(void *dst, int dstStride, const void *src, int srcStride, const QRect &rect, const QPoint &offset)
{
    int w = rect.width() * 4;
    int h = rect.height();
    char *d = reinterpret_cast<char*>(dst)
              + (offset.x() + rect.x()) * 4
              + (offset.y() + rect.y()) * dstStride;
    const char *s = reinterpret_cast<const char*>(src)
              + rect.x() * 4
              + rect.y() * srcStride;
    for (int i = 0; i < h; ++i) {
        memcpy(d, s, w);
        d += dstStride; s += srcStride;
    }
}

inline void throwIfFailed(HRESULT hr)
{
    if (FAILED(hr)) {
        // Set a breakpoint on this line to catch Win32 API errors.
        qFatal("WinRT PageFlipper Direct3D error: %d", hr);
    }
}

QT_BEGIN_NAMESPACE

QWinRTPageFlipper::QWinRTPageFlipper(ICoreWindow *window)
    : m_window(window)
{
    createDeviceResources();
    createWindowSizeDependentResources();
}

QWinRTPageFlipper::~QWinRTPageFlipper()
{
}

void QWinRTPageFlipper::createDeviceResources()
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    // If the project is in a debug build, enable debugging via SDK Layers with this flag.
    creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    // Select feature levels
    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };

    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;

    // Create Device
    throwIfFailed(D3D11CreateDevice(
                    nullptr,
                    D3D_DRIVER_TYPE_HARDWARE,
                    nullptr,
                    creationFlags,
                    featureLevels,
                    ARRAYSIZE(featureLevels),
                    D3D11_SDK_VERSION,
                    &device,
                    &featureLevel,
                    &context));

    // Ensure D3D 11.1
    throwIfFailed(device.As(&m_device));
    throwIfFailed(context.As(&m_context));
}

const QSize &QWinRTPageFlipper::size() const
{
    return m_windowBounds.size();
}

void QWinRTPageFlipper::handleDeviceLost()
{
    // Reset these member variables to ensure that UpdateForWindowSizeChange recreates all resources.
    m_windowBounds = QRect();
    m_swapChain = nullptr;
    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;
    m_device = nullptr;
    m_context = nullptr;

    // recreate everything
    createDeviceResources();
    createWindowSizeDependentResources();
    updateForWindowSizeChange();
}

void QWinRTPageFlipper::releaseResourcesForSuspending()
{
    // Phone applications operate in a memory-constrained environment, so when entering
    // the background it is a good idea to free memory-intensive objects that will be
    // easy to restore upon reactivation. The swapchain and backbuffer are good candidates
    // here, as they consume a large amount of memory and can be reinitialized quickly.
    m_swapChain = nullptr;
    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;
}

void QWinRTPageFlipper::present()
{
    // The first argument instructs DXGI to block until VSync, putting the application
    // to sleep until the next VSync. This ensures we don't waste any cycles rendering
    // frames that will never be displayed to the screen.
    HRESULT hr;
#ifdef Q_OS_WINPHONE
    hr = m_swapChain->Present(1, 0); // No dirty region support on phone
#else
    if (m_dirtyRegion == m_windowBounds) {
        hr = m_swapChain->Present(1, 0);
    } else {
        QVector<RECT> rects;
        rects.reserve(m_dirtyRegion.rects().count());
        DXGI_PRESENT_PARAMETERS presentParameters = { rects.count(), rects.data(), NULL, NULL };
        foreach (const QRect &rect, m_dirtyRegion.rects())
            rects.push_back(CD3D11_RECT(rect.left(), rect.top(), rect.right(), rect.bottom()));

        hr = m_swapChain->Present1(1, 0, &presentParameters);
    }

    // Clear the dirty tracker
    m_dirtyRegion = QRegion();
#endif // Q_OS_WINPHONE

    // If the device was removed either by a disconnect or a driver upgrade, we
    // must recreate all device resources.
    if (hr == DXGI_ERROR_DEVICE_REMOVED)
        handleDeviceLost();
    else if (FAILED(hr))
        qWarning(Q_FUNC_INFO ": failed with %08x", hr);
}

void QWinRTPageFlipper::createWindowSizeDependentResources()
{
    Rect windowBounds;
    m_window->get_Bounds(&windowBounds);

    // Calculate the necessary swap chain and render target size in pixels.
    m_windowBounds = QRect(0, 0, windowBounds.Width, windowBounds.Height);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Width = static_cast<UINT>(m_windowBounds.width()); // Match the size of the window.
    swapChainDesc.Height = static_cast<UINT>(m_windowBounds.height());
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // This is the most common swap chain format.
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1; // Don't use multi-sampling.
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // On phone, only stretch and aspect-ratio stretch scaling are allowed.
#ifdef Q_OS_WINPHONE
    swapChainDesc.BufferCount = 1; // On phone, only single buffering is supported.
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // On phone, no swap effects are supported.
#else
    swapChainDesc.BufferCount = 2; // has to be between 2 and DXGI_MAX_SWAP_CHAIN_BUFFERS
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // On non-phone, it's sequential flip only
#endif
    swapChainDesc.Flags = 0;

    ComPtr<IDXGIDevice1> dxgiDevice;
    throwIfFailed(m_device.As(&dxgiDevice));

    ComPtr<IDXGIAdapter> dxgiAdapter;
    throwIfFailed(dxgiDevice->GetAdapter(&dxgiAdapter));

    ComPtr<IDXGIFactory2> dxgiFactory;
    throwIfFailed(dxgiAdapter->GetParent(
            __uuidof(IDXGIFactory2), &dxgiFactory));

    throwIfFailed(dxgiFactory->CreateSwapChainForCoreWindow(
            m_device.Get(), m_window.Get(),
            &swapChainDesc, nullptr,
            &m_swapChain));

    // Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
    // ensures that the application will only render after each VSync, minimizing power consumption.
    throwIfFailed(dxgiDevice->SetMaximumFrameLatency(1));

    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;

    throwIfFailed(m_swapChain->GetBuffer(
            0, __uuidof(ID3D11Texture2D), &m_frontBuffer));

    D3D11_TEXTURE2D_DESC desc = {
        m_windowBounds.width(), m_windowBounds.height(),
        1, 1,
        DXGI_FORMAT_B8G8R8A8_UNORM,
        {1, 0},
        D3D11_USAGE_DYNAMIC,
        D3D11_BIND_SHADER_RESOURCE,
        D3D11_CPU_ACCESS_WRITE,
        0
    };

    throwIfFailed(m_device->CreateTexture2D(&desc, nullptr, &m_backBuffer));

    m_dirtyRegion = m_windowBounds;
    m_bufferData.resize(m_windowBounds.width() * m_windowBounds.height() * 4);
    displayBuffer(0);
}

void QWinRTPageFlipper::updateForWindowSizeChange()
{
    m_swapChain = nullptr;
    m_frontBuffer = nullptr;
    m_backBuffer = nullptr;
    m_context->Flush();
    createWindowSizeDependentResources();
}

void QWinRTPageFlipper::update(const QRegion &region, const QPoint &offset, const void *handle, int stride)
{
    if (!m_context || !m_backBuffer)
        return;

    m_dirtyRegion += region.translated(offset);

    // Copy to internal backing store
    foreach (const QRect &rect, region.rects())
        rectCopy(m_bufferData.data(), m_windowBounds.width() * 4, handle, stride, rect, offset);

    displayBuffer(0);
}

bool QWinRTPageFlipper::displayBuffer(QPlatformScreenBuffer *buffer)
{
    Q_UNUSED(buffer)    // Using internal buffer - consider if this could be encapsulated into separate class
    if (m_dirtyRegion.isEmpty())
        return false;
    if (!m_context || !m_swapChain)
        return false;

    // TODO: determine if this can be delayed/queued
    D3D11_MAPPED_SUBRESOURCE target;
    if (FAILED(m_context->Map(
                   m_backBuffer.Get(), 0,
                   D3D11_MAP_WRITE_DISCARD, 0,
                   &target))) {
        qWarning(Q_FUNC_INFO ": failed to map backbuffer");
        return false;
    }
    rectCopy(target.pData, target.RowPitch, m_bufferData.data(),
             m_windowBounds.width() * 4, m_windowBounds, QPoint());
    m_context->Unmap(m_backBuffer.Get(), 0);

    m_context->CopyResource(m_frontBuffer.Get(), m_backBuffer.Get());
    emit bufferReleased(buffer);
    present();
    emit bufferDisplayed(buffer);
    return true;
}

void QWinRTPageFlipper::resize(const QSize &size)
{
    if (size == m_windowBounds.size())
        return;

    updateForWindowSizeChange();
}

QT_END_NAMESPACE
