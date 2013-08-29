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

#include "qwinrtintegration.h"
#include "qwinrtwindow.h"
#include "qwinrteventdispatcher.h"
#include "qwinrtbackingstore.h"
#include "qwinrtscreen.h"
#include "qwinrtinputcontext.h"
#include "qwinrtservices.h"

#include <QtPlatformSupport/private/qbasicfontdatabase_p.h>

#include <wrl.h>
#include <windows.ui.core.h>
#include <Windows.ApplicationModel.core.h>

using namespace Microsoft::WRL;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI::Core;
using namespace ABI::Windows::ApplicationModel::Core;

QWinRTIntegration::QWinRTIntegration()
{
    qputenv("QT_QPA_FONTDIR", ":/fonts/");

    // Obtain the WinRT Application, view, and window
    ComPtr<ICoreApplication> appFactory;
    if (FAILED(GetActivationFactory(Wrappers::HString::MakeReference(
                                        RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
                                        &appFactory)))
        qFatal("Could not attach to the application factory.");

    ICoreApplicationView *view;
    if (FAILED(appFactory->GetCurrentView(&view)))
        qFatal("Could not obtain the application view - have you started outside of WinRT?");

    // Get core window (will act as our screen)
    ICoreWindow *window;
    if (FAILED(view->get_CoreWindow(&window)))
        qFatal("Could not obtain the application window - have you started outside of WinRT?");
    window->Activate();
    m_screen = new QWinRTScreen(window);
    screenAdded(m_screen);

    // Get event dispatcher
    ICoreDispatcher *dispatcher;
    if (FAILED(window->get_Dispatcher(&dispatcher)))
        qFatal("Could not capture UI Dispatcher");
    m_eventDispatcher = new QWinRTEventDispatcher(dispatcher);

    m_fontDatabase = new QBasicFontDatabase();

    m_services = new QWinRTServices();
}

QWinRTIntegration::~QWinRTIntegration()
{
    Windows::Foundation::Uninitialize();
}

QAbstractEventDispatcher *QWinRTIntegration::guiThreadEventDispatcher() const
{
    return m_eventDispatcher;
}

bool QWinRTIntegration::hasCapability(QPlatformIntegration::Capability cap) const
{
    switch (cap) {
    case ThreadedPixmaps:
    case OpenGL:
    case ThreadedOpenGL:
        return true;
    default:
        return QPlatformIntegration::hasCapability(cap);
    }
}

QPlatformWindow *QWinRTIntegration::createPlatformWindow(QWindow *window) const
{
    return new QWinRTWindow(window);
}

QPlatformBackingStore *QWinRTIntegration::createPlatformBackingStore(QWindow *window) const
{
    return new QWinRTBackingStore(window);
}

QPlatformFontDatabase *QWinRTIntegration::fontDatabase() const
{
    return m_fontDatabase;
}

QPlatformInputContext *QWinRTIntegration::inputContext() const
{
    return m_screen->inputContext();
}

QPlatformServices *QWinRTIntegration::services() const
{
    return m_services;
}

QT_END_NAMESPACE
