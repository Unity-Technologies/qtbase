/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Windows main function of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
  This file contains the code in the qtmain library for WinRT.
  qtmain contains the WinRT startup code and is required for
  linking to the Qt DLL.

  When a Windows application starts, the WinMain function is
  invoked. WinMain creates the WinRT application which in turn
  calls the main entry point.
*/

extern "C" int main(int, char **);

#include <qbytearray.h>
#include <qstring.h>
#include <qlist.h>
#include <qvector.h>

#include <wrl.h>
#include <Windows.ApplicationModel.core.h>

typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::ApplicationModel::Core::CoreApplicationView *, ABI::Windows::ApplicationModel::Activation::IActivatedEventArgs *> ActivatedHandler;

class AppxContainer : public Microsoft::WRL::RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkView>
{
public:
    AppxContainer(int argc, wchar_t **argv) : m_argc(argc)
    {
        m_argv.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            QByteArray arg = QString::fromWCharArray(argv[i]).toLocal8Bit();
            m_argv.append(new char[arg.length() + 1]);
            qstrcpy(m_argv.last(), arg.constData());
        }
    }

    // IFrameworkView Methods
    HRESULT __stdcall Initialize(ABI::Windows::ApplicationModel::Core::ICoreApplicationView *view)
    {
        view->add_Activated(Microsoft::WRL::Callback<ActivatedHandler>(this, &AppxContainer::onActivated).Get(),
                            &m_activationToken);
        return S_OK;
    }
    HRESULT __stdcall SetWindow(ABI::Windows::UI::Core::ICoreWindow *) { return S_OK; }
    HRESULT __stdcall Load(HSTRING) { return S_OK; }
    HRESULT __stdcall Run()
    {
        return main(m_argv.count(), m_argv.data());
    }
    HRESULT __stdcall Uninitialize() { return S_OK; }

private:
    // Activation handler
    HRESULT onActivated(ABI::Windows::ApplicationModel::Core::ICoreApplicationView *,
                      ABI::Windows::ApplicationModel::Activation::IActivatedEventArgs *args)
    {
        ABI::Windows::ApplicationModel::Activation::ILaunchActivatedEventArgs *launchArgs;
        if (SUCCEEDED(args->QueryInterface(&launchArgs))) {
            qDeleteAll(m_argv.begin() + m_argc, m_argv.end());
            m_argv.resize(m_argc);
            HSTRING arguments;
            launchArgs->get_Arguments(&arguments);
            QList<QByteArray> args = QString::fromWCharArray(WindowsGetStringRawBuffer(arguments, nullptr)).toLocal8Bit().split(' ');
            foreach (const QByteArray &arg, args) {
                m_argv.append(new char[arg.length() + 1]);
                qstrcpy(m_argv.last(), arg.constData());
            }
        }
        return S_OK;
    }

    int m_argc;
    QVector<char*> m_argv;
    EventRegistrationToken m_activationToken;
};

class AppxViewSource : public Microsoft::WRL::RuntimeClass<ABI::Windows::ApplicationModel::Core::IFrameworkViewSource>
{
public:
    AppxViewSource(int argc, wchar_t *argv[]) : argc(argc), argv(argv) { }
    HRESULT __stdcall CreateView(ABI::Windows::ApplicationModel::Core::IFrameworkView **frameworkView)
    {
        return (*frameworkView = Microsoft::WRL::Make<AppxContainer>(argc, argv).Detach()) ? S_OK : E_OUTOFMEMORY;
    }
private:
    int argc;
    wchar_t **argv;
};

// Main entry point for Appx containers
int wmain(int argc, wchar_t *argv[])
{
    if (FAILED(Windows::Foundation::Initialize(RO_INIT_MULTITHREADED)))
        return 1;

    Microsoft::WRL::ComPtr<ABI::Windows::ApplicationModel::Core::ICoreApplication> appFactory;
    if (FAILED(Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HString::MakeReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(), &appFactory)))
        return 2;

    return appFactory->Run(Microsoft::WRL::Make<AppxViewSource>(argc, argv).Get());
}
