// Compatibility header for deprecated DirectPlay API
// Provides minimal type/interface definitions for compilation
#pragma once

#include <windows.h>
#include <objbase.h>

#ifndef DPLAY_H_COMPAT
#define DPLAY_H_COMPAT

// Basic types
typedef DWORD DPID;
typedef DPID *LPDPID;

// Well-known player IDs
#define DPID_SYSMSG        0
#define DPID_ALLPLAYERS    0
#define DPID_SERVERPLAYER  1

// Return values
#define DP_OK              S_OK
#define DPERR_BASE         0x80040000
#define MAKE_DPHRESULT(c)  MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, (c))

#define DPERR_ALREADYINITIALIZED    MAKE_DPHRESULT(5)
#define DPERR_ACCESSDENIED          MAKE_DPHRESULT(10)
#define DPERR_ACTIVEPLAYERS         MAKE_DPHRESULT(20)
#define DPERR_BUFFERTOOSMALL        MAKE_DPHRESULT(30)
#define DPERR_CANTADDPLAYER         MAKE_DPHRESULT(40)
#define DPERR_CANTCREATEGROUP       MAKE_DPHRESULT(50)
#define DPERR_CANTCREATEPLAYER      MAKE_DPHRESULT(60)
#define DPERR_CANTCREATESESSION     MAKE_DPHRESULT(70)
#define DPERR_CAPSNOTAVAILABLEYET   MAKE_DPHRESULT(80)
#define DPERR_EXCEPTION             MAKE_DPHRESULT(90)
#define DPERR_GENERIC               MAKE_DPHRESULT(100)
#define DPERR_INVALIDFLAGS          MAKE_DPHRESULT(120)
#define DPERR_INVALIDOBJECT         MAKE_DPHRESULT(130)
#define DPERR_INVALIDPARAMS         MAKE_DPHRESULT(150)
#define DPERR_INVALIDPLAYER         MAKE_DPHRESULT(160)
#define DPERR_BUSY                  MAKE_DPHRESULT(170)
#define DPERR_LOGONDENIED           MAKE_DPHRESULT(180)
#define DPERR_NOCAPS                MAKE_DPHRESULT(190)
#define DPERR_NOMESSAGES            MAKE_DPHRESULT(200)
#define DPERR_NONAMESERVERFOUND     MAKE_DPHRESULT(210)
#define DPERR_NOPLAYERS             MAKE_DPHRESULT(220)
#define DPERR_NOSESSIONS            MAKE_DPHRESULT(230)
#define DPERR_SENDTOOBIG            MAKE_DPHRESULT(280)
#define DPERR_TIMEOUT               MAKE_DPHRESULT(290)
#define DPERR_UNAVAILABLE           MAKE_DPHRESULT(300)
#define DPERR_UNSUPPORTED           MAKE_DPHRESULT(310)
#define DPERR_CONNECTING            MAKE_DPHRESULT(400)
#define DPERR_CONNECTIONLOST        MAKE_DPHRESULT(420)
#define DPERR_NOCONNECTION          MAKE_DPHRESULT(480)
#define DPERR_INVALIDPASSWORD       MAKE_DPHRESULT(500)
#define DPERR_NOTLOGGEDIN           MAKE_DPHRESULT(520)
#define DPERR_CANNOTCREATESERVER    MAKE_DPHRESULT(530)
#define DPERR_NONEWPLAYERS          MAKE_DPHRESULT(560)
#define DPERR_CANTLOADSSPI          MAKE_DPHRESULT(570)
#define DPERR_CANTLOADSECURITYPACKAGE MAKE_DPHRESULT(580)
#define DPERR_UNINITIALIZED         MAKE_DPHRESULT(590)
#define DPERR_USERCANCEL            MAKE_DPHRESULT(600)

// Enumeration flags
#define DPENUMSESSIONS_AVAILABLE       0x00000001
#define DPENUMSESSIONS_PASSWORDREQUIRED 0x00000004
#define DPENUMSESSIONS_RETURNSTATUS    0x00000008
#define DPENUMPLAYERS_ALL              0x00000000

// Open flags
#define DPOPEN_JOIN                    0x00000001
#define DPOPEN_CREATE                  0x00000002
#define DPOPEN_RETURNSTATUS            0x00000100

// Session flags
#define DPSESSION_JOINDISABLED         0x00000020
#define DPSESSION_KEEPALIVE            0x00000040
#define DPSESSION_MIGRATEHOST          0x00000004
#define DPSESSION_OPTIMIZELATENCY      0x00000080

// Send flags
#define DPSEND_GUARANTEED              0x00000001

// Receive flags
#define DPRECEIVE_PEEK                 0x00000008

// Enum session callback flags
#define DPESC_TIMEDOUT                 0x00000001

// System message types
#define DPSYS_CREATEPLAYERORGROUP      0x0003
#define DPSYS_DESTROYPLAYERORGROUP     0x0005
#define DPSYS_HOST                     0x0101
#define DPSYS_SESSIONLOST              0x0031

// Structures
typedef struct {
    DWORD dwSize;
    DWORD dwFlags;
    union {
        LPSTR  lpszShortNameA;
        LPWSTR lpszShortName;
    };
    union {
        LPSTR  lpszLongNameA;
        LPWSTR lpszLongName;
    };
} DPNAME, *LPDPNAME;

typedef const DPNAME *LPCDPNAME;

typedef struct {
    DWORD dwSize;
    DWORD dwFlags;
    GUID  guidInstance;
    GUID  guidApplication;
    DWORD dwMaxPlayers;
    DWORD dwCurrentPlayers;
    union {
        LPSTR  lpszSessionNameA;
        LPWSTR lpszSessionName;
    };
    union {
        LPSTR  lpszPasswordA;
        LPWSTR lpszPassword;
    };
    DWORD dwReserved1;
    DWORD dwReserved2;
    DWORD dwUser1;
    DWORD dwUser2;
    DWORD dwUser3;
    DWORD dwUser4;
} DPSESSIONDESC2, *LPDPSESSIONDESC2;

typedef const DPSESSIONDESC2 *LPCDPSESSIONDESC2;

typedef struct {
    DWORD dwType;
} DPMSG_GENERIC, *LPDPMSG_GENERIC;

typedef struct {
    DWORD  dwType;
    DPID   dpId;
    DWORD  dwPlayerType;
    DPNAME dpnName;
} DPMSG_DESTROYPLAYERORGROUP, *LPDPMSG_DESTROYPLAYERORGROUP;

// Compound address element
typedef struct {
    GUID   guidDataType;
    DWORD  dwDataSize;
    LPVOID lpData;
} DPCOMPOUNDADDRESSELEMENT, *LPDPCOMPOUNDADDRESSELEMENT;

// Callback types
typedef BOOL (FAR PASCAL *LPDPENUMPLAYERSCALLBACK2)(DPID, DWORD, LPCDPNAME, DWORD, LPVOID);
typedef BOOL (FAR PASCAL *LPDPENUMSESSIONSCALLBACK2)(const DPSESSIONDESC2*, DWORD*, DWORD, LPVOID);

// GUIDs (defined in dplayx_guid.cpp)
extern "C" {
    extern const GUID CLSID_DirectPlay;
    extern const GUID IID_IDirectPlay4A;
    extern const GUID DPSPGUID_TCPIP;
    extern const GUID DPAID_ServiceProvider;
    extern const GUID DPAID_INet;
}

// IDirectPlay4 interface (A version)
#undef INTERFACE
#define INTERFACE IDirectPlay4A
DECLARE_INTERFACE_(IDirectPlay4A, IUnknown)
{
    // IUnknown
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    // IDirectPlay4A
    STDMETHOD(Close)(void) PURE;
    STDMETHOD(CreatePlayer)(LPDPID, LPDPNAME, HANDLE, LPVOID, DWORD, DWORD) PURE;
    STDMETHOD(DestroyPlayer)(DPID) PURE;
    STDMETHOD(EnumPlayers)(LPGUID, LPDPENUMPLAYERSCALLBACK2, LPVOID, DWORD) PURE;
    STDMETHOD(EnumSessions)(LPDPSESSIONDESC2, DWORD, LPDPENUMSESSIONSCALLBACK2, LPVOID, DWORD) PURE;
    STDMETHOD(GetPlayerAddress)(DPID, LPVOID, LPDWORD) PURE;
    STDMETHOD(InitializeConnection)(LPVOID, DWORD) PURE;
    STDMETHOD(Open)(LPDPSESSIONDESC2, DWORD) PURE;
    STDMETHOD(Receive)(LPDPID, LPDPID, DWORD, LPVOID, LPDWORD) PURE;
    STDMETHOD(Send)(DPID, DPID, DWORD, LPVOID, DWORD) PURE;
    STDMETHOD(SetSessionDesc)(LPDPSESSIONDESC2, DWORD) PURE;
};

typedef IDirectPlay4A *LPDIRECTPLAY4A;

#endif // DPLAY_H_COMPAT
