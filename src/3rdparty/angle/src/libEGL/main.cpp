//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "libEGL/main.h"

#include "common/debug.h"

#ifndef QT_OPENGL_ES_2_ANGLE_STATIC

#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
__declspec(thread) egl::Current *thread_local_current = nullptr;
#else
static DWORD currentTLS = TLS_OUT_OF_INDEXES;
#endif

static inline egl::Current *getCurrent()
{
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
    return thread_local_current;
#else
    return (egl::Current*)TlsGetValue(currentTLS);
#endif
}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        {
#if !defined(ANGLE_DISABLE_TRACE)
            FILE *debug = fopen(TRACE_OUTPUT_FILE, "rt");

            if (debug)
            {
                fclose(debug);
                debug = fopen(TRACE_OUTPUT_FILE, "wt");   // Erase
                
                if (debug)
                {
                    fclose(debug);
                }
            }
#endif

#ifndef QT_OPENGL_ES_2_ANGLE_WINRT
            currentTLS = TlsAlloc();

            if (currentTLS == TLS_OUT_OF_INDEXES)
            {
                return FALSE;
            }
#endif
        }
        // Fall throught to initialize index
      case DLL_THREAD_ATTACH:
        {
            egl::Current *current = getCurrent();

            if (!current) {
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
                current = thread_local_current = (egl::Current*)HeapAlloc(GetProcessHeap(),
                                                                      HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY,
                                                                      sizeof(egl::Current));
#else
                current = (egl::Current*)LocalAlloc(LPTR, sizeof(egl::Current));
#endif
            }

            if (current)
            {
#ifndef QT_OPENGL_ES_2_ANGLE_WINRT
                TlsSetValue(currentTLS, current);
#endif

                current->error = EGL_SUCCESS;
                current->API = EGL_OPENGL_ES_API;
                current->display = EGL_NO_DISPLAY;
                current->drawSurface = EGL_NO_SURFACE;
                current->readSurface = EGL_NO_SURFACE;
            }
        }
        break;
      case DLL_THREAD_DETACH:
        {
            egl::Current *current = getCurrent();

            if (current)
            {
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
                HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, current);
                thread_local_current = nullptr;
#else
                LocalFree((HLOCAL)current);
#endif
            }
        }
        break;
      case DLL_PROCESS_DETACH:
        {
            egl::Current *current = getCurrent();

            if (current)
            {
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
                HeapFree(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, current);
                thread_local_current = nullptr;
#else
                LocalFree((HLOCAL)current);
#endif
            }

#ifndef QT_OPENGL_ES_2_ANGLE_WINRT
            TlsFree(currentTLS);
#endif
        }
        break;
      default:
        break;
    }

    return TRUE;
}

#else // !QT_OPENGL_ES_2_ANGLE_STATIC

static egl::Current *getCurrent()
{
    // No precautions for thread safety taken as ANGLE is used single-threaded in Qt.
    static egl::Current curr = { EGL_SUCCESS, EGL_OPENGL_ES_API, EGL_NO_DISPLAY, EGL_NO_SURFACE, EGL_NO_SURFACE };
    return &curr;
}

#endif // QT_OPENGL_ES_2_ANGLE_STATIC

namespace egl
{
void setCurrentError(EGLint error)
{
    getCurrent()->error = error;
}

EGLint getCurrentError()
{
    return getCurrent()->error;
}

void setCurrentAPI(EGLenum API)
{
    getCurrent()->API = API;
}

EGLenum getCurrentAPI()
{
    return getCurrent()->API;
}

void setCurrentDisplay(EGLDisplay dpy)
{
    getCurrent()->display = dpy;
}

EGLDisplay getCurrentDisplay()
{
    return getCurrent()->display;
}

void setCurrentDrawSurface(EGLSurface surface)
{
    getCurrent()->drawSurface = surface;
}

EGLSurface getCurrentDrawSurface()
{
    return getCurrent()->drawSurface;
}

void setCurrentReadSurface(EGLSurface surface)
{
    getCurrent()->readSurface = surface;
}

EGLSurface getCurrentReadSurface()
{
    return getCurrent()->readSurface;
}

void error(EGLint errorCode)
{
    egl::setCurrentError(errorCode);
}

}
