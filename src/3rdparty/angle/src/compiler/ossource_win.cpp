//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/osinclude.h"
//
// This file contains contains the window's specific functions
//

#if !defined(ANGLE_OS_WIN)
#error Trying to build a windows specific file in a non windows build.
#endif


//
// Thread Local Storage Operations
//

#ifdef QT_OPENGL_ES_2_ANGLE_WINRT

#include <vector>

__declspec(thread) std::vector<void*> *tls = nullptr;

OS_TLSIndex OS_AllocTLSIndex()
{
    if (!tls)
        tls = new std::vector<void*>(1, nullptr);

    tls->push_back(nullptr);
    return tls->size() - 1;
}

void *OS_GetTLSValue(OS_TLSIndex nIndex)
{
    ASSERT(nIndex != OS_INVALID_TLS_INDEX);
    ASSERT(tls);

    return tls->at(nIndex);
}

bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
    if (!tls || nIndex >= tls->size() || nIndex == OS_INVALID_TLS_INDEX) {
        assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
        return false;
    }

    tls->at(nIndex) = lpvValue;
    return true;
}


bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
    if (!tls || nIndex >= tls->size() || nIndex == OS_INVALID_TLS_INDEX) {
        assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
        return false;
    }

    // TODO: push freed indices into a reuse stack...
    return true;
}


#else


OS_TLSIndex OS_AllocTLSIndex()
{
	DWORD dwIndex = TlsAlloc();
	if (dwIndex == TLS_OUT_OF_INDEXES) {
		assert(0 && "OS_AllocTLSIndex(): Unable to allocate Thread Local Storage");
		return OS_INVALID_TLS_INDEX;
	}

	return dwIndex;
}


void *OS_GetTLSValue(OS_TLSIndex nIndex)
{
    ASSERT(nIndex != OS_INVALID_TLS_INDEX);

    return TlsGetValue(nIndex);
}


bool OS_SetTLSValue(OS_TLSIndex nIndex, void *lpvValue)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	if (TlsSetValue(nIndex, lpvValue))
		return true;
	else
		return false;
}


bool OS_FreeTLSIndex(OS_TLSIndex nIndex)
{
	if (nIndex == OS_INVALID_TLS_INDEX) {
		assert(0 && "OS_SetTLSValue(): Invalid TLS Index");
		return false;
	}

	if (TlsFree(nIndex))
		return true;
	else
		return false;
}

#endif
