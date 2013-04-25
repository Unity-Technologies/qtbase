#include "precompiled.h"
//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// main.cpp: DLL entry point and management of thread-local data.

#include "libGLESv2/main.h"

#include "libGLESv2/Context.h"

#ifndef QT_OPENGL_ES_2_ANGLE_STATIC

#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
__declspec(thread) gl::Current *thread_local_current = nullptr;
#else
static DWORD currentTLS = TLS_OUT_OF_INDEXES;
#endif

static gl::Current *getCurrent()
{
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
    return thread_local_current;
#else
    return (gl::Current*)TlsGetValue(currentTLS);
#endif
}

extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        {
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
            gl::Current *current = getCurrent();

            if (!current) {
#ifdef QT_OPENGL_ES_2_ANGLE_WINRT
                current = thread_local_current = (gl::Current*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, sizeof(gl::Current));
#else
                current = (gl::Current*)LocalAlloc(LPTR, sizeof(gl::Current));
#endif
            }

            if (current)
            {
#ifndef QT_OPENGL_ES_2_ANGLE_WINRT
                TlsSetValue(currentTLS, current);
#endif
                current->context = NULL;
                current->display = NULL;
            }
        }
        break;
      case DLL_THREAD_DETACH:
        {
            gl::Current *current = getCurrent();

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
            gl::Current *current = getCurrent();

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

static inline gl::Current *getCurrent()
{
    // No precautions for thread safety taken as ANGLE is used single-threaded in Qt.
    static gl::Current curr = { 0, 0 };
    return &curr;
}

#endif // QT_OPENGL_ES_2_ANGLE_STATIC

namespace gl
{
void makeCurrent(Context *context, egl::Display *display, egl::Surface *surface)
{
    Current *curr = getCurrent();

    curr->context = context;
    curr->display = display;

    if (context && display && surface)
    {
        context->makeCurrent(surface);
    }
}

Context *getContext()
{
    return getCurrent()->context;
}

Context *getNonLostContext()
{
    Context *context = getContext();
    
    if (context)
    {
        if (context->isContextLost())
        {
            gl::error(GL_OUT_OF_MEMORY);
            return NULL;
        }
        else
        {
            return context;
        }
    }
    return NULL;
}

egl::Display *getDisplay()
{
    return getCurrent()->display;
}

// Records an error code
void error(GLenum errorCode)
{
    gl::Context *context = glGetCurrentContext();

    if (context)
    {
        switch (errorCode)
        {
          case GL_INVALID_ENUM:
            context->recordInvalidEnum();
            TRACE("\t! Error generated: invalid enum\n");
            break;
          case GL_INVALID_VALUE:
            context->recordInvalidValue();
            TRACE("\t! Error generated: invalid value\n");
            break;
          case GL_INVALID_OPERATION:
            context->recordInvalidOperation();
            TRACE("\t! Error generated: invalid operation\n");
            break;
          case GL_OUT_OF_MEMORY:
            context->recordOutOfMemory();
            TRACE("\t! Error generated: out of memory\n");
            break;
          case GL_INVALID_FRAMEBUFFER_OPERATION:
            context->recordInvalidFramebufferOperation();
            TRACE("\t! Error generated: invalid framebuffer operation\n");
            break;
          default: UNREACHABLE();
        }
    }
}

}

