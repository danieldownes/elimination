/************************************************************************/
// sound.h 
//
// Author: Justin Hoffman
// Date:   6/4/00 - 7/18/2000
/************************************************************************/

#ifndef SOUND_HEADER_INCLUDED
#define SOUND_HEADER_INCLUDED

//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// misc. definitions /////////////////////////////////////////////////////
#define MUSIC_USEDLS                           // set to use DLS music samples
#define LPDSB              IDirectSoundBuffer* // keyboard saver
#define SOUND_SPEED        22050               // Hz of sound files 
#define SOUND_CENTER       320                 // center channel for sounds
#define MUSIC_NAME         "music%d.mid"       // file name of music files
#define MUSIC_DLSNAME      "main.dls"          // file name of DLS sound collection
#define MUSIC_MAXCD        20                  // maximum number of CD tracks

// music format types ////////////////////////////////////////////////////
#define MUSIC_MCI          0
#define MUSIC_DM           1
#define MUSIC_CD           2
#define MUSIC_OFF          3

// song flags ////////////////////////////////////////////////////////////
#define MUSIC_SONGACTIVE   0x01
#define MUSIC_SONGAVALABLE 0x02


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// misc. music and sound interfaces //////////////////////////////////////
extern LPDIRECTSOUND  lpds;
extern UCHAR         *mSongFlags;
extern UCHAR         *mSongList;
extern UCHAR          mListLength;
extern UCHAR          mNumSongs;
extern UCHAR          mThisSong;
extern UCHAR          mNextSong;
extern UCHAR          mMusicType;
extern UCHAR          MUSIC_NSONGS;


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// application interface /////////////////////////////////////////////////
void MusicInit(void);
void MusicClose(void);
void MusicPause(void);
void MusicResume(void);
void MusicChangeType(UCHAR);
void MusicCheckNext(WPARAM);

// playback handeling ////////////////////////////////////////////////////
long MusicPlaySong(UCHAR);
long MusicCheckCaps(void);
void MusicReList(void);
void MusicPlayNext(void);
long MusicPlayMci(long);

// cdaudio code //////////////////////////////////////////////////////////
long MusicPlayCd(long);
void MusicCheckCdAval(void);
void MusicCheckCdChange(WPARAM, LPARAM);

// DirectSound Code //////////////////////////////////////////////////////
void DsbPlay(LPDSB, long);
void DsInit(void);
void DsbCreate(LPDSB*, UCHAR);
long DsbLoad(LPDSB, long);
long DsbRestore(LPDSB, UCHAR);

// DirectMusic Code //////////////////////////////////////////////////////
long MusicPlayDm(UCHAR);
long DmInit(void);
void DmClose(void);
DWORD WINAPI DmThread(void*);


//————————————————————————————————————————————————————————————————————————
// DirectSound return value handelers
//————————————————————————————————————————————————————————————————————————

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