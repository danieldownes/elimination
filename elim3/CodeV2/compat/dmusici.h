// Compatibility header for deprecated DirectMusic API
// Provides minimal type/interface definitions for compilation
#pragma once

#include <windows.h>
#include <objbase.h>
#include <mmsystem.h>

#ifndef DMUSICI_H_COMPAT
#define DMUSICI_H_COMPAT

// Max filename length for DMUS_OBJECTDESC
#define DMUS_MAX_FILENAME       MAX_PATH

// Segment playback flags
#define DMUS_SEGF_BEAT          0x00000004

// Notification option flags
#define DMUS_NOTIFICATION_SEGEND    2

// Object descriptor valid data flags
#define DMUS_OBJ_CLASS          0x00000002
#define DMUS_OBJ_FILENAME       0x00000100

// Forward declare interfaces
struct IDirectMusicPerformance;
struct IDirectMusicLoader;
struct IDirectMusicSegment;
struct IDirectMusicSegmentState;
struct IDirectMusicCollection;

// Time type (same as REFERENCE_TIME / MUSIC_TIME)
typedef long MUSIC_TIME;
typedef LONGLONG REFERENCE_TIME;

// Version structure
typedef struct _DMUS_VERSION {
    DWORD dwVersionMS;
    DWORD dwVersionLS;
} DMUS_VERSION;

// Object descriptor
typedef struct _DMUS_OBJECTDESC {
    DWORD dwSize;
    DWORD dwValidData;
    GUID  guidClass;
    GUID  guidObject;
    FILETIME ftDate;
    DMUS_VERSION vVersion;
    WCHAR wszFileName[DMUS_MAX_FILENAME];
    WCHAR wszName[64];
    WCHAR wszCategory[64];
    CHAR  szFileName[MAX_PATH];
} DMUS_OBJECTDESC, *LPDMUS_OBJECTDESC;

// Notification PMSG
typedef struct _DMUS_NOTIFICATION_PMSG {
    // DMUS_PMSG base fields
    DWORD dwSize;
    REFERENCE_TIME rtTime;
    MUSIC_TIME mtTime;
    DWORD dwFlags;
    DWORD dwPChannel;
    DWORD dwVirtualTrackID;
    void *pTool;
    void *pGraph;
    DWORD dwType;
    DWORD dwVoiceID;
    DWORD dwGroupID;
    void *punkUser;
    // Notification-specific
    GUID  guidNotificationType;
    DWORD dwNotificationOption;
    DWORD dwField1;
    DWORD dwField2;
} DMUS_NOTIFICATION_PMSG;

// Base PMSG
typedef struct _DMUS_PMSG {
    DWORD dwSize;
    REFERENCE_TIME rtTime;
    MUSIC_TIME mtTime;
    DWORD dwFlags;
    DWORD dwPChannel;
    DWORD dwVirtualTrackID;
    void *pTool;
    void *pGraph;
    DWORD dwType;
    DWORD dwVoiceID;
    DWORD dwGroupID;
    void *punkUser;
} DMUS_PMSG;

// GUIDs (defined in dmusic_guid.cpp)
extern "C" {
    extern const GUID CLSID_DirectMusicPerformance;
    extern const GUID CLSID_DirectMusicLoader;
    extern const GUID CLSID_DirectMusicSegment;
    extern const GUID CLSID_DirectMusicCollection;

    extern const GUID IID_IDirectMusicPerformance2;
    extern const GUID IID_IDirectMusicLoader;
    extern const GUID IID_IDirectMusicSegment2;
    extern const GUID IID_IDirectMusicCollection;

    extern const GUID GUID_NOTIFICATION_SEGMENT;
    extern const GUID GUID_StandardMIDIFile;
    extern const GUID GUID_Download;
    extern const GUID GUID_Unload;
    extern const GUID GUID_DirectMusicAllTypes;
    extern const GUID GUID_ConnectToDLSCollection;
}

// IDirectMusicSegmentState interface
#undef INTERFACE
#define INTERFACE IDirectMusicSegmentState
DECLARE_INTERFACE_(IDirectMusicSegmentState, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    STDMETHOD(GetRepeats)(DWORD*) PURE;
    STDMETHOD(GetSegment)(IDirectMusicSegment**) PURE;
    STDMETHOD(GetStartTime)(MUSIC_TIME*) PURE;
    STDMETHOD(GetSeek)(MUSIC_TIME*) PURE;
    STDMETHOD(GetStartPoint)(MUSIC_TIME*) PURE;
};

// IDirectMusicSegment interface
#undef INTERFACE
#define INTERFACE IDirectMusicSegment
DECLARE_INTERFACE_(IDirectMusicSegment, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    STDMETHOD(GetLength)(MUSIC_TIME*) PURE;
    STDMETHOD(SetLength)(MUSIC_TIME) PURE;
    STDMETHOD(GetRepeats)(DWORD*) PURE;
    STDMETHOD(SetRepeats)(DWORD) PURE;
    STDMETHOD(GetDefaultResolution)(DWORD*) PURE;
    STDMETHOD(SetDefaultResolution)(DWORD) PURE;
    STDMETHOD(GetTrack)(REFGUID, DWORD, DWORD, IUnknown**) PURE;
    STDMETHOD(GetTrackGroup)(IUnknown*, DWORD*) PURE;
    STDMETHOD(InsertTrack)(IUnknown*, DWORD) PURE;
    STDMETHOD(RemoveTrack)(IUnknown*) PURE;
    STDMETHOD(InitPlay)(IDirectMusicSegmentState**, IDirectMusicPerformance*, DWORD) PURE;
    STDMETHOD(GetGraph)(IUnknown**) PURE;
    STDMETHOD(SetGraph)(IUnknown*) PURE;
    STDMETHOD(AddNotificationType)(REFGUID) PURE;
    STDMETHOD(RemoveNotificationType)(REFGUID) PURE;
    STDMETHOD(GetParam)(REFGUID, DWORD, DWORD, MUSIC_TIME, MUSIC_TIME*, void*) PURE;
    STDMETHOD(SetParam)(REFGUID, DWORD, DWORD, MUSIC_TIME, void*) PURE;
    STDMETHOD(Clone)(MUSIC_TIME, MUSIC_TIME, IDirectMusicSegment**) PURE;
    STDMETHOD(SetStartPoint)(MUSIC_TIME) PURE;
    STDMETHOD(GetStartPoint)(MUSIC_TIME*) PURE;
    STDMETHOD(SetLoopPoints)(MUSIC_TIME, MUSIC_TIME) PURE;
    STDMETHOD(GetLoopPoints)(MUSIC_TIME*, MUSIC_TIME*) PURE;
};

// IDirectMusicCollection interface
#undef INTERFACE
#define INTERFACE IDirectMusicCollection
DECLARE_INTERFACE_(IDirectMusicCollection, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    STDMETHOD(GetInstrument)(DWORD, IUnknown**) PURE;
    STDMETHOD(EnumInstrument)(DWORD, DWORD*, LPWSTR, DWORD) PURE;
};

// IDirectMusicPerformance interface
#undef INTERFACE
#define INTERFACE IDirectMusicPerformance
DECLARE_INTERFACE_(IDirectMusicPerformance, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    STDMETHOD(Init)(IUnknown**, IUnknown*, HWND) PURE;
    STDMETHOD(PlaySegment)(IDirectMusicSegment*, DWORD, LONGLONG, IDirectMusicSegmentState**) PURE;
    STDMETHOD(Stop)(IDirectMusicSegment*, IDirectMusicSegmentState*, MUSIC_TIME, DWORD) PURE;
    STDMETHOD(GetTime)(REFERENCE_TIME*, MUSIC_TIME*) PURE;
    STDMETHOD(SetNotificationHandle)(HANDLE, REFERENCE_TIME) PURE;
    STDMETHOD(AddNotificationType)(REFGUID) PURE;
    STDMETHOD(RemoveNotificationType)(REFGUID) PURE;
    STDMETHOD(AddPort)(IUnknown*) PURE;
    STDMETHOD(GetNotificationPMsg)(DMUS_NOTIFICATION_PMSG**) PURE;
    STDMETHOD(FreePMsg)(DMUS_PMSG*) PURE;
    STDMETHOD(CloseDown)(void) PURE;
};

// IDirectMusicLoader interface
#undef INTERFACE
#define INTERFACE IDirectMusicLoader
DECLARE_INTERFACE_(IDirectMusicLoader, IUnknown)
{
    STDMETHOD(QueryInterface)(REFIID, LPVOID*) PURE;
    STDMETHOD_(ULONG, AddRef)(void) PURE;
    STDMETHOD_(ULONG, Release)(void) PURE;

    STDMETHOD(GetObject)(LPDMUS_OBJECTDESC, REFIID, void**) PURE;
    STDMETHOD(SetObject)(LPDMUS_OBJECTDESC) PURE;
    STDMETHOD(SetSearchDirectory)(REFGUID, WCHAR*, BOOL) PURE;
    STDMETHOD(ScanDirectory)(REFGUID, WCHAR*, WCHAR*) PURE;
    STDMETHOD(CacheObject)(IUnknown*) PURE;
    STDMETHOD(ReleaseObject)(IUnknown*) PURE;
    STDMETHOD(ClearCache)(REFGUID) PURE;
    STDMETHOD(EnableCache)(REFGUID, BOOL) PURE;
    STDMETHOD(EnumObject)(REFGUID, DWORD, LPDMUS_OBJECTDESC) PURE;
};

#endif // DMUSICI_H_COMPAT
