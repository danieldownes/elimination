/************************************************************************/
// sound.cpp - Utilities & wrappers for DirectSound & DirectMusic
//
// Author: Justin Hoffman
// Date:   11/2/99 - 1/30/00
/************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "user.h"
/*#include "file.h"*/
#include "game.h"


// Globals ///////////////////////////////////////////////////////////////
LPDIRECTSOUND lpds = 0; // main DirectSound object
UCHAR *mSongFlags  = 0; // array of bools, (is the song active?)
UCHAR *mSongList   = 0; // randomized list of songs
UCHAR  mNextSong   = 0; // next song to be played
UCHAR  mNumSongs   = 0; // number of songs
UCHAR  mListLength = 0; // length of randomized list
UCHAR  mMusicType  = MUSIC_DM; // default with direct music

static HANDLE gHEvent    = 0; // handle for the thread's event
static LPDMP  dmPerf     = 0; // main DirectMusic Preformance
static LPDML  dmLoader   = 0; // DirectMusic Loader
static LPDMS  dmSegment  = 0; // music segment
static LPDMSS dmSegState = 0; // used for info about the music
static long   dmStart    = 0; // music start time
static long   dmOffset   = 0; // music offset time for pause


/*——————————————————————————————————————————————————————————————————————*/
// DirectSound Code
/*——————————————————————————————————————————————————————————————————————*/

// Create the main DirectSound object & set the cooperative level ////////
void DSInit(void)
{
   if(DirectSoundCreate(0, &lpds, 0)                   != DS_OK ||
      lpds->SetCooperativeLevel(gHWnd, DSSCL_PRIORITY) != DS_OK) {

      // sound could not be initilized or is not avalable
      lpds = 0;
      BSET(game_flags, GAME_SUNAVALABLE);
      BREM(game_flags, GAME_SOUNDON);
   }
}

// Create a DirectSound buffer from a resource ///////////////////////////
void DsbCreate(LPDSB *dsb, UCHAR id)
{
   // make sure DirectSound is present
   if(!lpds) return;

   RES           res ; // resource data
   DWORD         len ; // total buffer length
   DSBUFFERDESC  dsbd; // DirectSound buffer description
   WAVEFORMATEX  wfx ; // wave format structure

   // read buffer length
   ResOpen(&res, id);
   ResRead(&res, &len, sizeof(len));
   ResClose(&res);

   // set up the format data structure
   memset(&wfx, 0, sizeof(wfx));
   wfx.cbSize          = 0;
   wfx.nChannels       = 1;
   wfx.nBlockAlign     = 1;
   wfx.wBitsPerSample  = 8;
   wfx.nSamplesPerSec  = D_SOUNDSPEED;
   wfx.nAvgBytesPerSec = D_SOUNDSPEED;
   wfx.wFormatTag      = WAVE_FORMAT_PCM;

   // prepare to create sound buffer
   STRUCT_INIT(dsbd);
   dsbd.dwFlags	    = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME;
   dsbd.dwBufferBytes = len;
   dsbd.lpwfxFormat   = &wfx;

   // create sound buffer
   if(lpds->CreateSoundBuffer(&dsbd, dsb, 0) != DS_OK) {
      DebugSimple("FAILED: DsbCreate; CreateSoundBuffer");
      return;
   }

   // load data into sound buffer
   DsbLoad(*dsb, id);
}

// Load a sound file into a DirectSound buffer ///////////////////////////
bool DsbLoad(LPDSB dsb, UCHAR id)
{
   if(dsb == 0) return 0;

   RES    res         ; // resource data
   DWORD  bufflen  = 0; // total buffer length
   DWORD  buffLenA = 0; // length of first write buffer
	DWORD  buffLenB = 0; // length of second write buffer
   UCHAR *pBufferA = 0; // data ptr to first write buffer 
	UCHAR *pBufferB = 0; // data ptr to second write buffer

   // open resource file
   ResOpen(&res, id);
   ResRead(&res, &bufflen, sizeof(bufflen));

   // copy data into sound buffer
   if(dsb->Lock(0, bufflen, (void**)&pBufferA, &buffLenA,
      (void**)&pBufferB, &buffLenB, DSBLOCK_FROMWRITECURSOR) != DS_OK) {
      DebugSimple("FAILED: DsbLoad; Lock");
      ResClose(&res);
      return 0;
   }

   // read the file into the DirectSound buffer
   ResRead(&res, pBufferA, buffLenA);
   ResRead(&res, pBufferB, buffLenB);

   // unlock the buffer & close the file
   dsb->Unlock(pBufferA, buffLenA, pBufferB, buffLenB);
   ResClose(&res);
   return 1;
}

// Restore a direct sound buffer /////////////////////////////////////////
bool DsbRestore(LPDSB dsb, UCHAR id)
{
   if(dsb->Restore() != DS_OK) return 0;
   if(!DsbLoad(dsb, id))       return 0;
   return 1;
}

// Play a sound without formatting ///////////////////////////////////////
void DsbPlay(LPDSB dsb)
{
   DWORD status;

   // make sure the buffer is not null & the sound is on
   if(dsb == 0 || (game_flags & GAME_SOUNDON) == 0) return;

   // see if sound should just be re-started
   dsb->GetStatus(&status);

   if(status == DSBSTATUS_PLAYING) {
      dsb->SetCurrentPosition(0);

   // play sound
   } else {
#ifdef GAME_DEBUG
      DSRVAL_DEBUG(dsb->Play(0, 0, 0), "DsbPlay; Play", 0);
#else
      DSRVAL(dsb->Play(0, 0, 0));
#endif
   }
}

// Play a sound with pan & volume control ////////////////////////////////
void DsbPlayPan(LPDSB dsb, long x)
{
   // make sure the buffer is not null & the sound is on
   if(dsb == 0 || (game_flags & GAME_SOUNDON) == 0) return;

   long  volume;
   DWORD status;

   // The pan & volume of a sound should be determined
   // by the 'screen chord' of the sound, if the sound
   // is 'within' the screen, the volume is left at max
   // and the pan is set to the screen chord, if the
   // sound is past the edge of the screen, the volume
   // is then diminished according to how far from the
   // edge of the screen the sound is.
   if(x < 0) {
      dsb->SetPan(DSBPAN_LEFT);
      volume = 10*x;
   } else if(x > 640) {
      dsb->SetPan(DSBPAN_RIGHT);
      volume = -10*(x - 800);
   } else {
      dsb->SetPan(-10000 + 25 * x);
      volume = DSBVOLUME_MAX;
   }

   // set volume of sound
   dsb->SetVolume(volume < DSBVOLUME_MIN? DSBVOLUME_MIN : volume);

   // see if sound should just be re-started
   dsb->GetStatus(&status);
   if(status == DSBSTATUS_PLAYING) {
      dsb->SetCurrentPosition(0);

   // otherwiser start playing
   } else {
#ifdef GAME_DEBUG
      DSRVAL_DEBUG(dsb->Play(0, 0, 0), "DsbPlayPan; Play", 0);
#else
      DSRVAL(dsb->Play(0, 0, 0));
#endif
   }
}


/*——————————————————————————————————————————————————————————————————————*/
// DirectMusic Code
/*——————————————————————————————————————————————————————————————————————*/

// Play a song through DirectMusic ///////////////////////////////////////
bool MPlayDM(char *sFile)
{
   DMUS_OBJECTDESC dmdesc; 
   WCHAR           wsFile[_MAX_PATH];

   // clean up old segment
   DXOBJ_DELETE(dmSegment);

   // load in new MIDI file
   if(dmLoader) {

      // Convert from multibyte format to Unicode
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sFile, -1, wsFile, _MAX_PATH);

      // describe the object to be loaded
      dmdesc.guidClass   = CLSID_DirectMusicSegment;
      dmdesc.dwSize      = sizeof(DMUS_OBJECTDESC);
      dmdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
      wcscpy(dmdesc.wszFileName, wsFile);

      // create directmusic segment
      dmLoader->GetObject(&dmdesc, IID_IDirectMusicSegment2, (void**)&dmSegment);
      if(!dmSegment) return 0;

      // ensure it plays as a standard midi & download DLS
      dmSegment->SetParam(GUID_StandardMIDIFile, -1, 0, 0, (void*)dmPerf);
      dmSegment->SetParam(GUID_Download, -1, 0, 0, (void*)dmPerf);

      // play the segment, create a segment state
      dmPerf->PlaySegment(dmSegment, 0, 0, &dmSegState);
      if(!dmSegState) return 0;

      // get the starting time
      dmSegState->GetStartTime(&dmStart);
   }

   // return sucess
   return 1;
}

// Initilize DirectMusic /////////////////////////////////////////////////
static bool DMInit(void)
{
   // If this function fails, set the main DM object to NULL,
   // and use "if(dmPerf)" to see if direct music is active
   HANDLE hTrd;
   DWORD  dwTrdID;
   WCHAR  wszDir[_MAX_PATH];
   char   szDir[_MAX_PATH];

   // Initilize COM
   if(FAILED(CoInitialize(0))) return 0;

   // create the preformance
   if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, 0,
      CLSCTX_INPROC, IID_IDirectMusicPerformance2, (void**)&dmPerf))) return 0;

   // initilize the preformance
   if(FAILED(dmPerf->Init(0, lpds, 0))) return 0;

   // create the event for the thread
   if(!(gHEvent = CreateEvent(0, 0, 0, 0))) return 0;

   // create the thread
   if(!(hTrd = CreateThread(0, 0, DMThread, 0, 0, &dwTrdID))) return 0;
   CloseHandle(hTrd);

   // set the notification handle of the preformance
   if(FAILED(dmPerf->SetNotificationHandle(gHEvent, 0))) return 0;

   // add a notificatoin type
   if(FAILED(dmPerf->AddNotificationType(GUID_NOTIFICATION_SEGMENT))) return 0;

   // add the port
   if(FAILED(dmPerf->AddPort(0))) return 0;

   // create the loader
   if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader, 0,
      CLSCTX_INPROC, IID_IDirectMusicLoader, (void**)&dmLoader))) return 0;

   // set the loader search directory
   GetCurrentDirectory(_MAX_PATH, szDir);
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szDir, -1, wszDir, _MAX_PATH);
   if(FAILED(dmLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE))) return 0;

   return 1;
}

// Close DirectMusic /////////////////////////////////////////////////////
static void DMClose(void)
{
   // stop the music
   if(dmPerf) dmPerf->Stop(0, 0, 0, 0);

   // release the segment
   if(dmSegment) {
      dmSegment->SetParam(GUID_Unload, -1, 0, 0, (void*)dmPerf);
      dmSegment->Release();
      dmSegment = 0;
   }

   // release the segment state & loader
   DXOBJ_DELETE(dmSegState);
   DXOBJ_DELETE(dmLoader);

   // close the main preformance object
   if(dmPerf) {
      dmPerf->CloseDown();
      dmPerf->Release();
      dmPerf = 0;
   }

   // release handle
   CloseHandle(gHEvent);

   // Release COM
   CoUninitialize();
}

// Wait for the song to end, and send a message to the main window ///////
static DWORD WINAPI DMThread(void *pv)
{
   while(1) {
      if(WaitForSingleObject(gHEvent, INFINITE) != WAIT_OBJECT_0) return 0;
      PostMessage(gHWnd, PROG_CHECKMUSIC, 0, 0);
   }
   return 1;
}


/*——————————————————————————————————————————————————————————————————————*/
// General Music Code
/*——————————————————————————————————————————————————————————————————————*/

// Initilize anything pertaining to music ////////////////////////////////
void MInit(void)
{
   // make sure there are even any midi files to play
   if(MGetStatus()) {
      switch(mMusicType) {

      // initilize direct music
      case MUSIC_DM:
         if(!DMInit()) dmPerf = 0;
         break;

      // initilize CD music
      case MUSIC_CD:
         mciSendString("open cdaudio", 0, 0, 0);
         break;
      }

      // set music on flags & make the random list of songs
      MReList();

   // music is not avalable
   } else {
      mMusicType = MUSIC_OFF;
      BSET(game_flags, GAME_MUNAVALABLE);
      BREM(game_flags, GAME_MUSICON);
   }
}

// Close down anything pertaining to the music ///////////////////////////
void MClose(void)
{
   switch(mMusicType) {
   case MUSIC_DM:
      DMClose();
      break;
   case MUSIC_MIDI:
      mciSendString("close all", 0, 0, 0);
      break;
   case MUSIC_CD:
      mciSendString("Stop cdaudio" , 0, 0, 0);
      mciSendString("Close cdaudio", 0, 0, 0);
      break;
   }

   // release allocated data
   if(mSongFlags) delete [] mSongFlags;
   if(mSongList ) delete [] mSongList;
   mSongFlags = 0;
   mSongList  = 0;
}

// Pause the current song ////////////////////////////////////////////////
void MPause(void)
{
   long mtnow;

   switch(mMusicType) {
   case MUSIC_DM:
      if(dmPerf) {

         // stop the music and get the time
         dmPerf->Stop(0, 0, 0, 0);
         dmPerf->GetTime(0, &mtnow);

         // re-calculate the offset
         dmOffset += (mtnow - dmStart);

         // set the start point for the segment for unpausing
         dmSegment->SetStartPoint(dmOffset);

         // release the segment state
         DXOBJ_DELETE(dmSegState);
      }
      break;
   case MUSIC_MIDI: mciSendString("stop MUSIC"  , 0, 0, 0); break;
   case MUSIC_CD  : mciSendString("stop cdaudio", 0, 0, 0); break;
   }
}

// Resume the current song ///////////////////////////////////////////////
void MResume(void)
{
   switch(mMusicType) {
   case MUSIC_DM:
      if(dmPerf) {
         dmPerf->PlaySegment(dmSegment, DMUS_SEGF_BEAT, 0, &dmSegState);
         dmSegState->GetStartTime(&dmStart);
      }
      break;
   case MUSIC_MIDI: mciSendString("play MUSIC notify"  , 0, 0, gHWnd); break;
   case MUSIC_CD  : mciSendString("play cdaudio notify", 0, 0, gHWnd); break;
   }
}

// Setup song status list ////////////////////////////////////////////////
static bool MGetStatus(void)
{
   char  sztext[64] = "0";
   long  nsongs     = 0;
   bool  rval       = 0;
   int   file;
   UCHAR i;

   // get cd music info
   mciSendString("open cdaudio", 0, 0, 0);
   mciSendString("status cdaudio number of tracks", sztext, 63, 0);
   sscanf(sztext, "%d", &nsongs);
   mciSendString("stop cdaudio" , 0, 0, 0);
   mciSendString("close cdaudio", 0, 0, 0);
      
   // no more than 20 cd songs allowed
   if(nsongs > 20) nsongs = 20;

   // cd music is unavalable
   if(nsongs < 2) {
      if(mMusicType == MUSIC_CD) mMusicType = MUSIC_DM;
      BSET(game_flags, GAME_CDUNAVALABLE);
   }

   // see it directmusic is unavalable, if so change to standard midi
   if(FAILED(CoInitialize(0)) ||
      FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, 0,
      CLSCTX_INPROC, IID_IDirectMusicPerformance2, (void**)&dmPerf))) {
      if(mMusicType == MUSIC_DM) mMusicType = MUSIC_MIDI;
      BSET(game_flags, GAME_DMUNAVALABLE);
   } else {
      DXOBJ_DELETE(dmPerf);
      CoUninitialize();
   }

   // figure out number of songs
   mNumSongs = (mMusicType == MUSIC_CD? nsongs : M_NSONGS);

   // allocate song status
   if(mSongFlags) delete [] mSongFlags;
   mSongFlags = new UCHAR[mNumSongs];

   // clear songs status
   memset(mSongFlags, 0, sizeof(UCHAR)*mNumSongs);

   // set all active & avalable flags to true
   for(i = 0; i < mNumSongs; i++) {
      BSET(mSongFlags[i], SONG_ACTIVE);
      BSET(mSongFlags[i], SONG_AVALABLE);
   }

   // make sure this isnt cd music
   if(mMusicType != MUSIC_CD) {

      // set current directory
      SetCurrentDirectory(gszAppPath);

      // cycle through all of the songs, and see if any exist
      for(i = 0; i < mNumSongs; i++) {
         wsprintf(sztext, M_FILENAME, i+1);

         // see if file exists
         if((file = _open(sztext, _O_BINARY | _O_RDONLY)) != -1) {
            BSET(mSongFlags[i], SONG_AVALABLE);
            _close(file);
            rval = 1;
         } else BREM(mSongFlags[i], SONG_AVALABLE);
      }

      /*
      // load active flags
      IoSettingsC(1);
      */
   } else rval = 1;
   return rval;
}

// Create the randomized list of midi files //////////////////////////////
void MReList(void)
{
   UCHAR i, j;

   // calculate number of active songs
   mListLength = 0;
   for(i = 0; i < mNumSongs; i++) {
      if(mSongFlags[i] == (SONG_ACTIVE | SONG_AVALABLE)) mListLength++;
   }

   // allocate song status
   if(mSongList) delete [] mSongList;
   mSongList = new UCHAR[mListLength];

   // clear list
   memset(mSongList, 0, sizeof(UCHAR)*mListLength);

   // create the new one, the do-while loop looks for an empty item
   for(i = 0; i < mNumSongs; i++) {
      if(mSongFlags[i] == (SONG_ACTIVE | SONG_AVALABLE)) {

         // find a random empty spot on the list
         do { j = RAND(mListLength); } while(mSongList[j] > 0);
         mSongList[j] = i + 1;
      }
   }
}

// Test to see if the message received should start a new song ///////////
void MCheckNext(WPARAM wParam)
{
   DMUS_NOTIFICATION_PMSG *dmmsg;

   // make sure it was DirectMusic signiling
   if(dmPerf && dmSegment) {
      if(dmPerf->GetNotificationPMsg(&dmmsg) == S_OK) {
         if(IsEqualGUID(dmmsg->guidNotificationType, GUID_NOTIFICATION_SEGMENT)) {
            
            // the segment has stoped, go to the next song
            if(dmmsg->dwNotificationOption == DMUS_NOTIFICATION_SEGEND) MPlayNext();
         }
         dmPerf->FreePMsg((DMUS_PMSG*)dmmsg);
      }

   // check to see if standard midi should go to next song
   } else if(wParam == MCI_NOTIFY_SUCCESSFUL) MPlayNext();
}

// Play the next song in the list ////////////////////////////////////////
void MPlayNext(void)
{
   char szFile[M_NAMELEN];

   // if we have repeated this for as many songs as there
   // are, it is pointless to continue, no songs will work
   for(UCHAR i = 0; i < mListLength; i++) {

      // get the file name of the song to play
      wsprintf(szFile, M_FILENAME, mSongList[mNextSong]);
 
      // move the number onto the next song
      if(++mNextSong >= mListLength) {
         mNextSong = 0;
         MReList();
      }

      // play the song from the file name just created
      switch(mMusicType) {
      case MUSIC_DM:
         if(MPlayDM(szFile)) return;
         break;
      case MUSIC_MIDI:
         if(MPlayMid(szFile)) return;
         break;
      case MUSIC_CD:
         MPlayCD(mSongList[mNextSong]);
         return;
      }
   }
}

// Play a MIDI file throught standard methods ////////////////////////////
bool MPlayMid(LPSTR szFile)
{
   char command[64];
 
   // set current directory so micSendString will find it
   SetCurrentDirectory(gszAppPath);

   // create and send the command string
   wsprintf(command, "open %s alias MUSIC type sequencer", szFile);
   if(mciSendString("close all", 0, 0, 0)) return 0;
   if(mciSendString(command, 0, 0, 0)) return 0;
   if(mciSendString("play MUSIC from 0 notify", 0, 0, gHWnd)) return 0;
   return 1;
}

// Play a CD song ////////////////////////////////////////////////////////
void MPlayCD(UCHAR track)
{
   char command[64];

   // Set time format to Tracks, minutes, seconds, and frames
   mciSendString("set cdaudio time format tmsf", 0, 0, 0);

   // if track is last track, seek to it and play to end of cd
   if(track < mNumSongs) {
      wsprintf(command, "play cdaudio from %d to %d notify", track, track + 1);
   } else {
      wsprintf(command, "play cdaudio from %d notify", track);
   }
  
   // send the command
   mciSendString(command, 0, 0, gHWnd);
}