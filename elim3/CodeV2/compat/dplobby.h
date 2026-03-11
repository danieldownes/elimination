// Compatibility header for deprecated DirectPlay Lobby API
#pragma once

#include "dplay.h"

#ifndef DPLOBBY_H_COMPAT
#define DPLOBBY_H_COMPAT

// Callback for EnumAddress
typedef BOOL (FAR PASCAL *LPDPENUMADDRESSCALLBACK)(REFGUID, DWORD, LPCVOID, LPVOID);

// GUIDs (defined in dplayx_guid.cpp)
extern "C" {
    extern const GUID CLSID_DirectPlayLobby;
    extern const GUID IID_IDirectPlayLobby3A;
}

// IDirectPlayLobby3 interface (A version)
#undef INTERFACE
#define INTERFACE IDirectPlayLobby3A
DECLARE_INTERFACE_(IDirectPlayLobby3A, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    // IDirectPlayLobby3A
    STDMETHOD(CreateCompoundAddress)(LPDPCOMPOUNDADDRESSELEMENT, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(EnumAddress)(LPDPENUMADDRESSCALLBACK, LPCVOID, DWORD, LPVOID) PURE;
};

typedef IDirectPlayLobby3A *LPDIRECTPLAYLOBBY3;

#endif // DPLOBBY_H_COMPAT
