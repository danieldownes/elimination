/*******************************************************************************/
// sound.h - Function prototypes and data types used in sound.cpp
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 - 
/*******************************************************************************/

#ifndef SOUND_HEADER_INCLUDED
#define SOUND_HEADER_INCLUDED


// Globals ///////////////////////////////////////////////////////////////
extern LPDIRECTSOUND lpds;

// Defenitions ///////////////////////////////////////////////////////////
#define LPDMP  IDirectMusicPerformance*
#define LPDML  IDirectMusicLoader*
#define LPDMS  IDirectMusicSegment*
#define LPDMSS IDirectMusicSegmentState*

#define MULTI_TO_WIDE(x, y)  MultiByteToWideChar( CP_ACP, \
        MB_PRECOMPOSED, y, -1, x, _MAX_PATH );

#define WM_CHECKMIDI (WM_USER + 21)


/*-----------------------------------------------------------------------------*/
// Function Prototypes
/*-----------------------------------------------------------------------------*/

// Setup /////////////////////////////////////////////////////////////////
void InitDirectSound(void);      // init direct sound

// Main sound functions //////////////////////////////////////////////////
void DS_CreateBuffer(LPDSO*, ULONG);
void DS_Play(LPDSO, long);

// Generic Music functions ///////////////////////////////////////////////
void Music_Resume(void);
void Music_Pause(void);
void Music_PlayNext(void);
void Music_MakeList(void);
void Music_Init(void);
int  Music_Avalable(void);
void Music_Close(void);

// Standard MIDI functions ///////////////////////////////////////////////
int MID_Play(LPSTR);

// DirectMusic functions /////////////////////////////////////////////////
void  DM_Resume(void);
void  DM_Pause(void);
int   DM_Play(LPSTR);
void  DM_Check(void);
DWORD WINAPI DM_Thread(void*);
LPDMS DM_LoadSegment(LPDML, LPSTR);
int   DM_Init(void);
void  DM_Close(void);


#endif // ifndef SOUND_HEADER_INCLUDED