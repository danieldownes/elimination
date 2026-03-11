/************************************************************************/
// sound.cpp - Music and sound code
//
// Author: Justin Hoffman
// Date:   6/4/00 - 
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "window.h"
#include "resdef.h"
#include "game.h"
#include "file.h"


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// misc. muisc & sound varables //////////////////////////////////////////
LPDIRECTSOUND  lpds         = 0;
UCHAR         *mSongFlags   = 0;
UCHAR         *mSongList    = 0;
UCHAR          mListLength  = 0;
UCHAR          mNumSongs    = 0;
UCHAR          mThisSong    = 0;
UCHAR          mNextSong    = 0;
UCHAR          mMusicType   = MUSIC_CD;
UCHAR          MUSIC_NSONGS = 1;


// DirectMusic varables //////////////////////////////////////////////////
static HANDLE                    dmEvents[2]   = {0,0};
static HANDLE                    dmhThread     = 0;
static long                      dmStart       = 0;
static long                      dmOffset      = 0;
static IDirectMusicPerformance  *dmPerf        = 0;
static IDirectMusicLoader       *dmLoader      = 0;
static IDirectMusicSegmentState *dmSegState    = 0;
static IDirectMusicCollection   *dmCollection  = 0;
static IDirectMusicSegment     **dmSegments    = 0;


//————————————————————————————————————————————————————————————————————————
// Application Music Interface
//————————————————————————————————————————————————————————————————————————

// initilize music components ////////////////////////////////////////////
void MusicInit(void)
{
   // load the number of songs
   ResOpen(BIN_TRACKS);
   ResRead(&MUSIC_NSONGS, sizeof(MUSIC_NSONGS));
   ResClose();

   // get the music capablities of users computer
   if(MusicCheckCaps()) {
 
      switch(mMusicType) {

      // initilize DirectMusic
      case MUSIC_DM:
         if(!DmInit()) {
            dmPerf      = 0;
            gbMusicOn   = 0;
            gbMusicAval = 0;
            mMusicType  = MUSIC_OFF;
         }
         break;

      // initilize CD music
      case MUSIC_CD:
         mciSendString("open cdaudio", 0, 0, 0);
         break;

      // the music is off
      case MUSIC_OFF:
         gbMusicOn = 0;
         break;
      }  

      IoSettingsSongInfo(1); // load the song active flags
      MusicReList();         // make the random list of songs

   // music is not avalable
   } else {
      gbMusicOn   = 0;
      gbMusicAval = 0;
      mMusicType  = MUSIC_OFF;
   }
}

// close music components ////////////////////////////////////////////////
void MusicClose(void)
{
   switch(mMusicType) {
   case MUSIC_DM:
      DmClose();
      break;
   case MUSIC_MCI:
      mciSendString("close all", 0, 0, 0);
      break;
   case MUSIC_CD:
      mciSendString("stop cdaudio" , 0, 0, 0);
      mciSendString("close cdaudio", 0, 0, 0);
      break;
   }

   // release allocated data
   DELETE_ARRAY(mSongFlags);
   DELETE_ARRAY(mSongList);
}

// pause the current song ////////////////////////////////////////////////
void MusicPause(void)
{
   UCHAR id;
   long  mtnow;

   switch(mMusicType) {

   // pause DirectMusic
   case MUSIC_DM:
      if(dmPerf) {

         // cache the ID of the current song
         id = mThisSong - 1;

         // stop the music and get the time
         dmPerf->Stop(0, 0, 0, 0);
         dmPerf->GetTime(0, &mtnow);

         // re-calculate the offset
         dmOffset += (mtnow - dmStart);

         // set the start point for the segment for unpausing
         if(dmSegments[id]) dmSegments[id]->SetStartPoint(dmOffset);

         // release the segment state
         DXOBJ_DELETE(dmSegState);
      }
      break;
   
   // pause MCI media
   case MUSIC_MCI: mciSendString("stop MUSIC"  , 0, 0, 0); break;
   case MUSIC_CD : mciSendString("stop cdaudio", 0, 0, 0); break;
   }
}

// resume the current song ///////////////////////////////////////////////
void MusicResume(void)
{
   switch(mMusicType) {

   // resume DirectMusic
   case MUSIC_DM:
      if(dmPerf) {
         dmPerf->PlaySegment(dmSegments[mThisSong - 1], DMUS_SEGF_BEAT, 0, &dmSegState);
         if(dmSegState) dmSegState->GetStartTime(&dmStart);
      }
      break;
   
   // resume MCI media
   case MUSIC_MCI: mciSendString("play MUSIC notify"  , 0, 0, gHWnd); break;
   case MUSIC_CD : mciSendString("play cdaudio notify", 0, 0, gHWnd); break;
   }
}

// change the music type being used //////////////////////////////////////
void MusicChangeType(UCHAR type)
{
   MusicClose();
   mMusicType = type;
   MusicInit();
   MusicPlayNext();
}

// check to see if we should advance to the next song ////////////////////
void MusicCheckNext(WPARAM wParam)
{
   DMUS_NOTIFICATION_PMSG *dmmsg;

   // make sure it was DirectMusic signiling
   if(dmPerf) {
      if(dmPerf->GetNotificationPMsg(&dmmsg) == S_OK) {
         if(dmmsg && IsEqualGUID(dmmsg->guidNotificationType, GUID_NOTIFICATION_SEGMENT)) {

            // the segment has stoped, go to the next song
            if(dmmsg->dwNotificationOption == DMUS_NOTIFICATION_SEGEND) MusicPlayNext();
         }

         // free the message
         dmPerf->FreePMsg((DMUS_PMSG*)dmmsg);
      }

   // check to see if standard midi should go to next song
   } else if(wParam == MCI_NOTIFY_SUCCESSFUL) MusicPlayNext();
}


//————————————————————————————————————————————————————————————————————————
// Playback Handeling
//————————————————————————————————————————————————————————————————————————

// play a particular song ////////////////////////////////////////////////
long MusicPlaySong(UCHAR index)
{   
   // set the current song
   mThisSong = index;

   // play the song from the file name just created
   switch(mMusicType) {
   case MUSIC_DM : return MusicPlayDm(index - 1);
   case MUSIC_MCI: return MusicPlayMci(index);
   case MUSIC_CD : return MusicPlayCd(index);
   default       : return 1;
   }
}

// check the music capabilities of the users computer ////////////////////
long MusicCheckCaps(void)
{
   UCHAR  i;
   bool   rval       = 0;
   char   sText[32]  = "0";

   // make sure we can open music files
   SetCurrentDirectory(gszAppPath);

   // check CD Audio avalablility
   MusicCheckCdAval();

   // see if DirectMusic is unavalable, if so change to standard midi
   if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, 0, CLSCTX_INPROC,
      IID_IDirectMusicPerformance2, (void**)&dmPerf))) {

      // DirectMusic was unabalable, disable it
      gbDirectMusicAval = 0;
      if(mMusicType == MUSIC_DM) mMusicType = MUSIC_MCI;

   } else {

      // release the temporary performace we created
      DXOBJ_DELETE(dmPerf);

      /*
      // make sure the DLS file is here
      if(!IoFileExists(MUSIC_DLSNAME)) {
         gbDirectMusicAval = 0;
         if(mMusicType == MUSIC_DM) mMusicType = MUSIC_MCI;
      }
      */
   }

   // figure out number of songs
   if(mMusicType != MUSIC_CD) mNumSongs = MUSIC_NSONGS;

   // allocate song status
   DELETE_ARRAY(mSongFlags);
   mSongFlags = (UCHAR*)MoreMem(mNumSongs);

   // set all active & avalable flags to true
   for(i = 0; i < mNumSongs; i++)
      mSongFlags[i] = MUSIC_SONGACTIVE | MUSIC_SONGAVALABLE;

   // if we are using CD music, we dont need to look for music files
   if(mMusicType == MUSIC_CD) return 1;

   // cycle through all of the songs, and see if any exist
   for(i = 0; i < mNumSongs; i++) {

      // calculate the music file's name
      wsprintf(sText, MUSIC_NAME, i + 1);

      // see if file exists
      if(IoFileExists(sText)) {

         rval = 1;
         mSongFlags[i] |= MUSIC_SONGAVALABLE;

      // the file does not exist
      } else mSongFlags[i] &= ~MUSIC_SONGAVALABLE;
   }

   return rval;
}

// re-list the songs randomly ////////////////////////////////////////////
void MusicReList(void)
{
   UCHAR i;
   UCHAR j;

   // calculate number of active songs
   for(i = mListLength = 0; i < mNumSongs; i++)
      if(mSongFlags[i] == (MUSIC_SONGACTIVE | MUSIC_SONGAVALABLE))
         mListLength++;

   // allocate song status
   DELETE_ARRAY(mSongList);
   mSongList = (UCHAR*)MoreMem(mListLength);

   // create the new song list
   for(i = 0; i < mNumSongs; i++) {
      if(mSongFlags[i] == (MUSIC_SONGACTIVE | MUSIC_SONGAVALABLE)) {

         // find a random empty spot on the list
         do { j = RAND(mListLength); } while(mSongList[j] > 0);
         mSongList[j] = i + 1;
      }
   }
}

// play the next song in the list ////////////////////////////////////////
void MusicPlayNext(void)
{
   UCHAR i;

   // if we have repeated this for as many songs as there
   // are, it is pointless to continue, no songs will work
   for(i = 0; i < mListLength; i++) {

      // move the number onto the next song
      if(++mNextSong >= mListLength) {
         mNextSong = 0;
         MusicReList();
      }

      // play the next song
      if(MusicPlaySong(mSongList[mNextSong])) return;
   }
}


// play a music file with mciSendString //////////////////////////////////
long MusicPlayMci(long index)
{
   char sText[64];

   // set current directory so micSendString will find it
   SetCurrentDirectory(gszAppPath);

   // create the command string
   wsprintf(sText, "open " MUSIC_NAME " alias MUSIC type sequencer", index);

   // send the command string
   if(mciSendString("close all", 0, 0, 0)) return 0;
   if(mciSendString(sText, 0, 0, 0))      return 0;
   return (mciSendString("play MUSIC from 0 notify", 0, 0, gHWnd) == 0);
}

//————————————————————————————————————————————————————————————————————————
// CD Audio code
//————————————————————————————————————————————————————————————————————————

// play a song off of a CD ///////////////////////////////////////////////
long MusicPlayCd(long track)
{
   char sText[64];

   // Set time format to tracks, minutes, seconds, and frames
   mciSendString("set cdaudio time format tmsf", 0, 0, 0);

   // if track is last track, seek to it and play to end of cd
   if(track < mNumSongs) {
      wsprintf(sText, "play cdaudio from %d to %d notify", track, track + 1);
   } else {
      wsprintf(sText, "play cdaudio from %d notify", track);
   }

   // return weather function succedes
   return mciSendString(sText, 0, 0, gHWnd) == 0;
}

// check to see if CD Audio is avalable //////////////////////////////////
void MusicCheckCdAval(void)
{
   UCHAR nsongs    = 0;
   char  sText[16] = "0";

   // get cd music info
   mciSendString("open cdaudio", 0, 0, 0);

   // this function will return > 0 if a cd is not in the drive
   if(mciSendString("status cdaudio number of tracks", sText, 15, 0) == 0) {

      // calculate the number of tracks
      if(sText[1] == TXT_NULLCHAR) nsongs = sText[0] - '0';
      else                         nsongs = 10*(sText[0] - '0') + sText[1] - '0';

      // close CDAUDIO
      mciSendString("stop cdaudio" , 0, 0, 0);
      mciSendString("close cdaudio", 0, 0, 0);

      // limit number of cd tracks
      if(nsongs > MUSIC_MAXCD) nsongs = MUSIC_MAXCD;
   }

   // set the CD Audio avalable boolean
   gbCdAudioAval = nsongs > 1;

   // do whatever is nedded if we are currently in MUSIC_CD mode
   if(mMusicType == MUSIC_CD) {
      if(gbCdAudioAval) mNumSongs  = (UCHAR)nsongs;
      else              mMusicType = MUSIC_DM;
   }
}

// called when windwos sends WM_DEVICECHANGE message /////////////////////
void MusicCheckCdChange(WPARAM wParam, LPARAM lParam) 
{
   bool                  changed = 0;
   PDEV_BROADCAST_HDR    pInfoA  = (PDEV_BROADCAST_HDR)lParam;
   PDEV_BROADCAST_VOLUME pInfoB  = (PDEV_BROADCAST_VOLUME)lParam;

   switch(wParam)  {
   
   // See if a CD-ROM was inserted into a drive.
   case DBT_DEVICEARRIVAL:
      if(pInfoA->dbch_devicetype == DBT_DEVTYP_VOLUME) {
         if(pInfoB->dbcv_flags & DBTF_MEDIA) {
            MusicCheckCdAval();
            changed = 1;
         }
      }
      break;

   // See if a CD-ROM was removed from a drive.
   case DBT_DEVICEREMOVECOMPLETE:
      if(pInfoA->dbch_devicetype == DBT_DEVTYP_VOLUME) {
         if(pInfoB->dbcv_flags & DBTF_MEDIA) {

            // set CD Audio aval to false
            gbCdAudioAval = 0;
            changed       = 1;

            // if we were using CD music, change it
            if(mMusicType == MUSIC_CD) {
               MusicClose();
               mMusicType = MUSIC_OFF;
            }
         }
      }
      break;
   } 

   // re-initilize music options window
   if(changed) {
      if(gbMsgBox && gWndA.loop == WndMusicOptions) {
         BtnEnable(&gWndA.pbtns[2], gbCdAudioAval);
      }
   }
}



//————————————————————————————————————————————————————————————————————————
// DirectSound Code
//————————————————————————————————————————————————————————————————————————

// play a direct sound buffer ////////////////////////////////////////////
void DsbPlay(LPDSB dsb, long pan)
{
   DWORD status;

   // make sure the buffer is not null & the sound is on
   if(dsb == 0 || gbSoundOn == 0) return;

   // set the pan of the sound
   dsb->SetPan(8*pan - 2560);

   // get the status of the sound
   dsb->GetStatus(&status);

   // see if sound should just be re-started
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

// initilize DirectSound /////////////////////////////////////////////////
void DsInit(void)
{
   if(DirectSoundCreate(0, &lpds, 0)                      != DS_OK
      || lpds->SetCooperativeLevel(gHWnd, DSSCL_PRIORITY) != DS_OK) {

      // disable the music
      DXOBJ_DELETE(lpds);
      gbSoundAval = 0;
      gbSoundOn   = 0;
   }
}

// create a DirectSoundBuffer ////////////////////////////////////////////
void DsbCreate(LPDSB *pdsb, UCHAR resId)
{
   DWORD         len ; // total buffer length
   DSBUFFERDESC  dsbd; // DirectSound buffer description
   WAVEFORMATEX  wfx ; // wave format structure

   // make sure DirectSound is present
   if(lpds == 0) return;

   // read buffer length
   ResOpen(resId);
   ResRead(&len, sizeof(len));

   // set up the format data structure
   memset(&wfx, 0, sizeof(wfx));
   wfx.cbSize          = 0;
   wfx.nChannels       = 1;
   wfx.nBlockAlign     = 1;
   wfx.wBitsPerSample  = 8;
   wfx.nSamplesPerSec  = SOUND_SPEED;
   wfx.nAvgBytesPerSec = SOUND_SPEED;
   wfx.wFormatTag      = WAVE_FORMAT_PCM;

   // prepare to create sound buffer
   STRUCT_INIT(dsbd);
   dsbd.dwFlags	    = DSBCAPS_STATIC;
   dsbd.dwBufferBytes = len;
   dsbd.lpwfxFormat   = &wfx;

   // create sound buffer
   if(lpds->CreateSoundBuffer(&dsbd, pdsb, 0) != DS_OK) {
      DebugSimple("FAILED: DsbCreate; CreateSoundBuffer");
      ResClose();
      return;
   }

   // load data into sound buffer & close resource file
   DsbLoad(*pdsb, len);
   ResClose();
}

// load a wave file into a DirectSoundBuffer /////////////////////////////
long DsbLoad(LPDSB dsb, long len)
{
   DWORD  DataLenA = 0; // length of first write buffer
   DWORD  DataLenB = 0; // length of second write buffer
   UCHAR *pDataA   = 0; // data ptr to first write buffer 
   UCHAR *pDataB   = 0; // data ptr to second write buffer

   if(dsb == 0) return 0;

   // copy data into sound buffer
   if(dsb->Lock(0, len, (void**)&pDataA, &DataLenA, (void**)&pDataB, &DataLenB, DSBLOCK_FROMWRITECURSOR) != DS_OK) {
      DebugSimple("FAILED: DsbLoad; Lock");
      return 0;
   }

   // read the file into the DirectSound buffer
   ResRead(pDataA, DataLenA);
   ResRead(pDataB, DataLenB);

   // unlock the buffer
   dsb->Unlock(pDataA, DataLenA, pDataB, DataLenB);
   return 1;
}

// restore a DirectSoundBuffer ///////////////////////////////////////////
long DsbRestore(LPDSB dsb, UCHAR resId)
{
   long len;

   // restore the buffer
   if(dsb->Restore() != DS_OK) return 0;

   // open the resource file
   ResOpen(resId);
   ResRead(&len, sizeof(len));

   // re-load the sound & close the resource file
   len = DsbLoad(dsb, len);
   ResClose();
   return len;
}


//————————————————————————————————————————————————————————————————————————
// DirectMusic Code
//————————————————————————————————————————————————————————————————————————

// play a MIDI file via DirectMusic //////////////////////////////////////
long MusicPlayDm(UCHAR index)
{
   // stop the old song & start the new one
   dmPerf->Stop(0, 0, 0, 0);
   dmPerf->PlaySegment(dmSegments[index], 0, 0, &dmSegState);

   // makre sure the segment state creation was sucessful
   if(!dmSegState) return 0;

   // get the starting time
   dmSegState->GetStartTime(&dmStart);

   // return sucess
   return 1;
}

// initilize DirectMusic /////////////////////////////////////////////////
long DmInit(void)
{
   // If this function fails, set the main DM object to NULL,
   // and use "if(dmPerf)" to see if direct music is active
   DMUS_OBJECTDESC dmdesc;
   DWORD           dwTrdId;
   WCHAR           wsDir[_MAX_PATH];
   char            sFile[32];
   RECT            r;
   long            i;

   // convert the application directory to a multi-byte string
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, gszAppPath, -1, wsDir, DMUS_MAX_FILENAME);
  
   // initlize DirectMusic components
   if(CoCreateInstance(CLSID_DirectMusicPerformance, 0,
      CLSCTX_INPROC, IID_IDirectMusicPerformance2, (void**)&dmPerf)   != S_OK // create the preformance
      || dmPerf->Init(0, lpds, 0)                                     != S_OK // initilize the preformance
      || (dmEvents[0] = CreateEvent(0, 0, 0, 0))                      == 0    // create the event for the thread
      || (dmEvents[1] = CreateEvent(0, 0, 0, 0))                      == 0    // create the kill thread event
      || (dmhThread   = CreateThread(0, 0, DmThread, 0, 0, &dwTrdId)) == 0    // create the thread
      || dmPerf->SetNotificationHandle(dmEvents[0], 0)                != S_OK // set the notification handle of the preformance
      || dmPerf->AddNotificationType(GUID_NOTIFICATION_SEGMENT)       != S_OK // add a notificatoin type
      || dmPerf->AddPort(0)                                           != S_OK // add the port
      || CoCreateInstance(CLSID_DirectMusicLoader, 0, CLSCTX_INPROC,
         IID_IDirectMusicLoader, (void**)&dmLoader)                 != S_OK // create the loader
      || dmLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wsDir, 0) != S_OK) return 0; // set loader search directory

   // make sure cacheing is not enabled for segments
   dmLoader->EnableCache(GUID_DirectMusicAllTypes, 1);
   dmLoader->EnableCache(CLSID_DirectMusicSegment, 0);

   /*
   // set directory to find DLS file
   SetCurrentDirectory(gszAppPath);

   // setup info for DLS file
   dmdesc.dwSize      = sizeof(dmdesc);
   dmdesc.guidClass   = CLSID_DirectMusicCollection;  
   dmdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

   // convert the filename to wide characters
   MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, MUSIC_DLSNAME, -1, dmdesc.wszFileName, DMUS_MAX_FILENAME);

   // load the DLS collection
   if((dmLoader->GetObject(&dmdesc, IID_IDirectMusicCollection, (void**)&dmCollection)) != S_OK) return 0;   
   
   // setup the guid of the song file descriptor
   dmdesc.guidClass = CLSID_DirectMusicSegment;
   */
   // allocate the dmSegment pointers
   dmSegments = (IDirectMusicSegment**)MoreMem(MUSIC_NSONGS*sizeof(IDirectMusicSegment*));
   
   /* setup data for loading DM segmentss */ 
   dmdesc.dwSize      = sizeof(dmdesc);
   dmdesc.guidClass   = CLSID_DirectMusicSegment;  
   dmdesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;

   // initilize the songs
   for(i = 0; i < MUSIC_NSONGS; i++) {

      // draw the loading text
      RSETWX(r, 0, 19*(i%4), 120, 19);
      DDBltFast(ddsFront, ddsLoading, 260, 230, &r, NOCK);
 
      // create the file name
      wsprintf(sFile, MUSIC_NAME, i+1);

      // Convert from multibyte format to Unicode
      MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, sFile, -1, dmdesc.wszFileName, DMUS_MAX_FILENAME);

      // create directmusic segment
      dmLoader->GetObject(&dmdesc, IID_IDirectMusicSegment2, (void**)&dmSegments[i]);

      // make sure the segment creation was sucessful
      if(!dmSegments[i]) {
         dmSegments[i] = 0;
         continue;
      }

      // ensure it plays as a standard midi, and get DLS
      dmSegments[i]->SetParam(GUID_StandardMIDIFile, 0xFFFFFFFF, 0, 0, (void*)dmPerf);
      /*dmSegments[i]->SetParam(GUID_ConnectToDLSCollection, 0xFFFFFFFF, 0, 0, (void*)dmCollection);*/
      dmSegments[i]->SetParam(GUID_Download, 0xFFFFFFFF, 0, 0, (void*)dmPerf);
   }
   
   return 1;
}

// close up DirectMusic //////////////////////////////////////////////////
void DmClose(void)
{
   // to avoid a crash
   if(dmPerf == 0) return;

   // close the thread & wait for it to quit
   SetEvent(dmEvents[1]);
   WaitForSingleObject(dmhThread, INFINITE);
   CloseHandle(dmhThread);

   // close the handles
   CloseHandle(dmEvents[0]);
   CloseHandle(dmEvents[1]);

   // stop the music
   dmPerf->Stop(0, 0, 0, 0);

   // release the segments
   for(long i = 0; i < MUSIC_NSONGS; i++) {
      if(dmSegments[i]) {
         dmSegments[i]->SetParam(GUID_Unload, 0xFFFFFFFF, 0, 0, (void*)dmPerf);
         dmSegments[i]->Release();
         dmSegments[i] = 0;
      }
   }

   // release the loader
   if(dmLoader) {
      dmLoader->ClearCache(GUID_DirectMusicAllTypes);
      dmLoader->Release();
      dmLoader = 0;
   }

   // release the segment state & loader
   DXOBJ_DELETE(dmCollection);
   DXOBJ_DELETE(dmSegState);

   // close the main preformance object
   dmPerf->CloseDown();
   dmPerf->Release();
   dmPerf = 0;

   // release the segment pointers
   LessMem(dmSegments);
}

// watch to see if the DirectMusic song has terminated ///////////////////
DWORD WINAPI DmThread(void*)
{
   // keep the thread going until the kill switch is used
   while(WaitForMultipleObjects(2, dmEvents, 0, INFINITE) == WAIT_OBJECT_0)
      PostMessage(gHWnd, PROG_CHECKMUSIC, 0, 0);

   // exit the thread
   ExitThread(0);
   return 1;
}