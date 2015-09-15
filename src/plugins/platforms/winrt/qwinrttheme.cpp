/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwinrttheme.h"
#include "qwinrtmessagedialoghelper.h"
#include "qwinrtfiledialoghelper.h"

#include <QtCore/qfunctions_winrt.h>
#include <QtGui/QPalette>

#include <wrl.h>
#include <windows.ui.h>
#include <windows.ui.viewmanagement.h>
using namespace Microsoft::WRL;
using namespace ABI::Windows::UI;
using namespace ABI::Windows::UI::ViewManagement;

QT_BEGIN_NAMESPACE

static IUISettings *uiSettings()
{
    static ComPtr<IUISettings> settings;
    if (!settings) {
        HRESULT hr;
        hr = RoActivateInstance(Wrappers::HString::MakeReference(RuntimeClass_Windows_UI_ViewManagement_UISettings).Get(),
                                &settings);
        Q_ASSERT_SUCCEEDED(hr);
    }
    return settings.Get();
}

class QWinRTThemePrivate
{
public:
    QPalette palette;
};

static inline QColor fromColor(const Color &color)
{
    return QColor(color.R, color.G, color.B, color.A);
}

QWinRTTheme::QWinRTTheme()
    : d_ptr(new QWinRTThemePrivate)
{
    Q_D(QWinRTTheme);

    HRESULT hr;
    Color color;

#ifdef Q_OS_WINPHONE
    hr = uiSettings()->UIElementColor(UIElementType_PopupBackground, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ToolTipBase, fromColor(color));
    d->palette.setColor(QPalette::AlternateBase, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_NonTextMedium, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Button, fromColor(color));
    hr = uiSettings()->UIElementColor(UIElementType_NonTextMediumHigh, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Midlight, fromColor(color));
    hr = uiSettings()->UIElementColor(UIElementType_NonTextHigh, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Light, fromColor(color));
    hr = uiSettings()->UIElementColor(UIElementType_NonTextMediumLow, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Mid, fromColor(color));
    hr = uiSettings()->UIElementColor(UIElementType_NonTextLow, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Dark, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_TextHigh, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ButtonText, fromColor(color));
    d->palette.setColor(QPalette::Text, fromColor(color));
    d->palette.setColor(QPalette::WindowText, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_TextMedium, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ToolTipText, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_AccentColor, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Highlight, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_PageBackground, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Window, fromColor(color));
    d->palette.setColor(QPalette::Base, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_TextContrastWithHigh, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::BrightText, fromColor(color));
#else
    hr = uiSettings()->UIElementColor(UIElementType_ActiveCaption, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ToolTipBase, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_Background, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::AlternateBase, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_ButtonFace, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Button, fromColor(color));
    d->palette.setColor(QPalette::Midlight, fromColor(color).lighter(110));
    d->palette.setColor(QPalette::Light, fromColor(color).lighter(150));
    d->palette.setColor(QPalette::Mid, fromColor(color).dark(130));
    d->palette.setColor(QPalette::Dark, fromColor(color).dark(150));

    hr = uiSettings()->UIElementColor(UIElementType_ButtonText, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ButtonText, fromColor(color));
    d->palette.setColor(QPalette::Text, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_CaptionText, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::ToolTipText, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_Highlight, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Highlight, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_HighlightText, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::HighlightedText, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_Window, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::Window, fromColor(color));
    d->palette.setColor(QPalette::Base, fromColor(color));

    hr = uiSettings()->UIElementColor(UIElementType_Hotlight, &color);
    Q_ASSERT_SUCCEEDED(hr);
    d->palette.setColor(QPalette::BrightText, fromColor(color));
#endif
}

bool QWinRTTheme::usePlatformNativeDialog(DialogType type) const
{
    static bool useNativeDialogs = qEnvironmentVariableIsSet("QT_USE_WINRT_NATIVE_DIALOGS")
            ? qEnvironmentVariableIntValue("QT_USE_WINRT_NATIVE_DIALOGS") : true;

    if (type == FileDialog || type == MessageDialog)
        return useNativeDialogs;
    return false;
}

QPlatformDialogHelper *QWinRTTheme::createPlatformDialogHelper(DialogType type) const
{
    switch (type) {
    case FileDialog:
        return new QWinRTFileDialogHelper;
    case MessageDialog:
        return new QWinRTMessageDialogHelper(this);
    default:
        break;
    }
    return QPlatformTheme::createPlatformDialogHelper(type);
}

QVariant QWinRTTheme::styleHint(QPlatformIntegration::StyleHint hint)
{
    HRESULT hr;
    switch (hint) {
    case QPlatformIntegration::CursorFlashTime: {
        quint32 blinkRate;
        hr = uiSettings()->get_CaretBlinkRate(&blinkRate);
        RETURN_IF_FAILED("Failed to get caret blink rate", return defaultThemeHint(CursorFlashTime));
        return blinkRate;
    }
    case QPlatformIntegration::KeyboardInputInterval:
        return defaultThemeHint(KeyboardInputInterval);
    case QPlatformIntegration::MouseDoubleClickInterval: {
        quint32 doubleClickTime;
        hr = uiSettings()->get_DoubleClickTime(&doubleClickTime);
        RETURN_IF_FAILED("Failed to get double click time", return defaultThemeHint(MouseDoubleClickInterval));
        return doubleClickTime;
    }
    case QPlatformIntegration::StartDragDistance:
        return defaultThemeHint(StartDragDistance);
    case QPlatformIntegration::StartDragTime:
        return defaultThemeHint(StartDragTime);
    case QPlatformIntegration::KeyboardAutoRepeatRate:
        return defaultThemeHint(KeyboardAutoRepeatRate);
    case QPlatformIntegration::ShowIsFullScreen:
        return false;
    case QPlatformIntegration::PasswordMaskDelay:
        return defaultThemeHint(PasswordMaskDelay);
    case QPlatformIntegration::FontSmoothingGamma:
        return qreal(1.7);
    case QPlatformIntegration::StartDragVelocity:
        return defaultThemeHint(StartDragVelocity);
    case QPlatformIntegration::UseRtlExtensions:
        return false;
    case QPlatformIntegration::PasswordMaskCharacter:
        return defaultThemeHint(PasswordMaskCharacter);
    case QPlatformIntegration::SetFocusOnTouchRelease:
        return false;
    case QPlatformIntegration::ShowIsMaximized:
        return true;
    case QPlatformIntegration::MousePressAndHoldInterval:
        return defaultThemeHint(MousePressAndHoldInterval);
    default:
        break;
    }
    return QVariant();
}

const QPalette *QWinRTTheme::palette(Palette type) const
{
    Q_D(const QWinRTTheme);
    if (type == SystemPalette)
        return &d->palette;
    return QPlatformTheme::palette(type);
}

QT_END_NAMESPACE
