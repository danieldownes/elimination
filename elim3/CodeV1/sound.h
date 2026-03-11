/************************************************************************/
// sound.cpp - Utilities & wrappers for DirectSound & DirectMusic
//
// Author: Justin Hoffman
// Date:   11/2/99 - 5/1/00
/************************************************************************/

#ifndef SOUND_HEADER_INCLUDED
#define SOUND_HEADER_INCLUDED


/*——————————————————————————————————————————————————————————————————————*/
// Definitions
/*——————————————————————————————————————————————————————————————————————*/

// DirectSound & DirectMusic types ///////////////////////////////////////
#define LPDSB           IDirectSoundBuffer*
#define LPDMP           IDirectMusicPerformance*
#define LPDML           IDirectMusicLoader*
#define LPDMS           IDirectMusicSegment*
#define LPDMSS          IDirectMusicSegmentState*

// The Hz of the sound ///////////////////////////////////////////////////
#define D_SOUNDSPEED    22050//11025

// Music Definitions /////////////////////////////////////////////////////
#define SONG_ACTIVE     0x01
#define SONG_AVALABLE   0x02
#define MUSIC_DM        0
#define MUSIC_MIDI      1
#define MUSIC_CD        2
#define MUSIC_OFF       3
#define M_NSONGS        9
#define M_NAMELEN       32
#define M_FILENAME      "music%d.mid"

// Globals ///////////////////////////////////////////////////////////////
extern LPDIRECTSOUND lpds;
extern UCHAR *mSongFlags ;
extern UCHAR *mSongList  ;
extern UCHAR  mNextSong  ;
extern UCHAR  mNumSongs  ;
extern UCHAR  mMusicType ;
extern UCHAR  mListLength;


/*——————————————————————————————————————————————————————————————————————*/
// Function prototypes
/*——————————————————————————————————————————————————————————————————————*/

// DirectSound Code //////////////////////////////////////////////////////
void DSInit(void);
void DsbCreate(LPDSB*, UCHAR);
bool DsbLoad(LPDSB, UCHAR);
bool DsbRestore(LPDSB, UCHAR);
void DsbPlay(LPDSB);
void DsbPlayPan(LPDSB, long);

// DirectMusic Code //////////////////////////////////////////////////////
bool  MPlayDM(char*);
bool  DMInit(void);
void  DMClose(void);
DWORD WINAPI DMThread(void*);

// General Music Code ////////////////////////////////////////////////////
void MInit(void);
void MClose(void);
void MPause(void);
void MResume(void);
bool MGetStatus(void);
void MReList(void);
void MCheckNext(WPARAM);
void MPlayNext(void);
bool MPlayMid(char*);
void MPlayCD(UCHAR);


/*——————————————————————————————————————————————————————————————————————*/
// DirectSound return value handelers
/*——————————————————————————————————————————————————————————————————————*/

// Process DirectDraw return values [debug version] //////////////////////
#define DSRVAL_DEBUG(f, s, a)         \
HRESULT hr;                           \
while(1) {                            \
   hr = f;                            \
   if(hr == DS_OK) break;             \
   else if(hr == DSERR_BUFFERLOST)  { \
      if(!ProgRestoreSound()) {       \
         DebugWrite("FAILED: DSRVAL; ProgRestoreSound"); \
         break;                       \
      }                               \
   } else {                           \
      DebugWrite("DSRVAL_DEBUG failed: %s, %d, %d", s, hr, a); \
      break;                          \
   }                                  \
}

// Process DirectDraw return values //////////////////////////////////////
#define DSRVAL(f) if(f == DSERR_BUFFERLOST) ProgRestoreSound();


#endif // #ifndef SOUND_HEADER_INCLUDED