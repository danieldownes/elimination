/*******************************************************************************/
// sound.cpp - Functions for playing sound and music
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "main.h"
#include "graph.h"
#include "sound.h"


// Globals ///////////////////////////////////////////////////////////////
LPDIRECTSOUND lpds        = NULL; // main DirectSound object
HANDLE        gHEvent     = NULL; // handle for the thread's event
LPDMP         gDMPerf     = NULL; // main DirectMusic Preformance
LPDML         gDMLoader   = NULL; // DirectMusic Loader
LPDMS         gDMSegment  = NULL; // music segment
LPDMSS        gDMSegState = NULL; // used for info about the music
long          gDMStart    = 0;    // music start time
long          gDMOffset   = 0;    // music offset time for pause

int gSongList[D_NUMSONGS];        // random list of songs
int gNextSong;                    // next song to be played
int gDMActive;                    // tells weather DirectMusic is being used



/*-----------------------------------------------------------------------------*/
// DirectSound Setup
/*-----------------------------------------------------------------------------*/

// Initilize DirectSound /////////////////////////////////////////////////
void InitDirectSound(void)
{
   // create the directSound object & set the cooperative level
   if(DirectSoundCreate(NULL, &lpds, NULL) != DS_OK ||
      lpds->SetCooperativeLevel(g_hWnd, DSSCL_PRIORITY) != DS_OK)
   {
      g_settings &= ~SET_SOUND_ON;
      lpds = NULL;
      Menu_Disable(6);
      return;
   }
}


/*-----------------------------------------------------------------------------*/
// Main Sound Functions
/*-----------------------------------------------------------------------------*/

// Play a DirectSound Buffer /////////////////////////////////////////////
void DS_Play(LPDSO dso, long pan)
{
   if(dso == NULL || !(g_settings & SET_SOUND_ON)) return;

   DWORD dwStatus;
   HRESULT dsrval;

   LONG dsPan = -2500 + (pan * (5000 / SCREEN_W));
   dso->SetPan(dsPan);

   // Get the status of the wave
   if(dso->GetStatus(&dwStatus) != DS_OK) return;

   if(dwStatus == DSBSTATUS_PLAYING) {
      dso->SetCurrentPosition(0); // Don't bother playing, just restart
   } else {
      while(1) { // play the wave
         dsrval = dso->Play(0, 0, 0);
         if(dsrval == DS_OK) break;
         else if(dsrval == DSERR_BUFFERLOST) dso->Restore();
         else return;
      }
   }
}


// Create a DirectSound buffer ///////////////////////////////////////////
void DS_CreateBuffer(LPDSO *dso, ULONG resId)
{
   // make sure DirectSound is present
   if(lpds == NULL) {
      *dso = NULL;
      return;
   }

   RES           res    ; // resource data
   DWORD         bufflen; // total buffer length
   DSBUFFERDESC  dsbd   ; // DirectSound buffer description
   WAVEFORMATEX  pcmwf  ; // wave format structure

   UCHAR *pBufferA = NULL; // data ptr to first write buffer 
	UCHAR *pBufferB = NULL; // data ptr to second write buffer
   DWORD  buffLenA = 0   ; // length of first write buffer
	DWORD  buffLenB = 0   ; // length of second write buffer

   // open & read the resource file
   ResOpen(&res, (UCHAR)resId);
   ResRead(&res, &bufflen, sizeof(bufflen));

   // set up the format data structure
   memset(&pcmwf, 0, sizeof(pcmwf));
   pcmwf.cbSize		    = 0;
   pcmwf.nChannels		 = 1;
   pcmwf.nBlockAlign	    = 1;
   pcmwf.wBitsPerSample  = 8;
   pcmwf.nSamplesPerSec  = D_SOUNDSPEED;
   pcmwf.nAvgBytesPerSec = D_SOUNDSPEED;
   pcmwf.wFormatTag	    = WAVE_FORMAT_PCM;

   // prepare to create the sound buffer
   memset(&dsbd, 0, sizeof(dsbd));
   dsbd.dwSize          = sizeof(dsbd);
   dsbd.dwFlags		   = DSBCAPS_CTRLPAN | DSBCAPS_STATIC;
   dsbd.dwBufferBytes	= bufflen;
   dsbd.lpwfxFormat	   = &pcmwf;

   // create the sound buffer
   if(lpds->CreateSoundBuffer(&dsbd, dso, NULL) != DS_OK) {
      ResClose(&res);
      return;
   }

   // copy data into sound buffer
   if((*dso)->Lock(0, bufflen, (void**)&pBufferA, &buffLenA,
      (void**)&pBufferB, &buffLenB, DSBLOCK_FROMWRITECURSOR) != DS_OK) {
      ResClose(&res);
      return;
   }

   // read the file into the DirectSound buffer
   ResRead(&res, pBufferA, buffLenA);
   ResRead(&res, pBufferB, buffLenB);

   // unlock the buffer
   (*dso)->Unlock(pBufferA, buffLenA, pBufferB, buffLenB);

   // close the file
   ResClose(&res);
}



/*-----------------------------------------------------------------------------*/
// Generic Music Functions
/*-----------------------------------------------------------------------------*/

// Resume the current song ///////////////////////////////////////////////
void Music_Resume(void)
{
   if(gDMActive) DM_Resume();
   else mciSendString("play MUSIC notify", 0, 0, g_hWnd);
}


// Pause the current song ////////////////////////////////////////////////
void Music_Pause(void)
{
   if(gDMActive) DM_Pause();
   else mciSendString("stop MUSIC", 0, 0, 0);
}


// Play the next song in the list ////////////////////////////////////////
void Music_PlayNext(void)
{
   char szFile[32];

   for(int i = 0; i < D_NUMSONGS; i++) {
      // get the file name of the song to play
      wsprintf(szFile, TXT_MUSIC_NAME, gSongList[gNextSong]);
      
      // move the number onto the next song
      if(++gNextSong >= D_NUMSONGS) {
         Music_MakeList();
         gNextSong = 0;
      }

      // play the song from the file name just created
      if(gDMActive) {
         if(DM_Play(szFile)) break;
      } else {
         if(MID_Play(szFile)) break;
      }
   }
}


// Create the random file list ///////////////////////////////////////////
void Music_MakeList(void)
{
   int i, a;

   // clear the list
   for(i = 0; i < D_NUMSONGS; i++) gSongList[i] = 0;

   // create the new one, the do-while loop looks for an empty item
   for(i = 1; i <= D_NUMSONGS; i++) {
      do { a = RAND(D_NUMSONGS); } while(gSongList[a] != 0);
      gSongList[a] = i;
   }
}


// Initilize the music system ////////////////////////////////////////////
void Music_Init(void)
{
   // make sure there are even any midi files to play
   if(!Music_Avalable()) {
      g_settings &= ~SET_MUSIC_ON; // remove the music_on flag
      Menu_Disable(7);             // disable the music button
   } else {
      gDMActive = DM_Init(); // attempt to initilze direct music
      Music_MakeList();      // make the random list of songs
      gNextSong = 0;         // set the next song to zero
   }
}


// Test to see if there are any MIDI files to play ///////////////////////
int Music_Avalable(void)
{
   int  file;
   char szFileName[32];
   
   for(int i = 0; i < D_NUMSONGS; i++) {
      wsprintf(szFileName, TXT_MUSIC_NAME, i);
      if((file = _open(szFileName, _O_BINARY | _O_RDONLY)) != -1) {
         _close(file);
         return 1;
      }
   }
   return 0;
}


// Shutdown the music system /////////////////////////////////////////////
void Music_Close()
{
   if(gDMActive) DM_Close();
   else mciSendString("close all", 0, 0, 0);
}



/*-----------------------------------------------------------------------------*/
// Standard MIDI functions
/*-----------------------------------------------------------------------------*/

// Play a MIDI file //////////////////////////////////////////////////////
int MID_Play(LPSTR szFile)
{
   char command[256];
 
   // create the command string
   wsprintf(command, "open %s alias MUSIC type sequencer", szFile);
   
   // set the directory so the song will be found
   SetCurrentDirectory(g_appPath);

   // close all other curently playing devices
   if(mciSendString("close all", 0, 0, 0)) return 0;

   // send that command we just made
   if(mciSendString(command, 0, 0, 0)) return 0;
   
   // start the music playin'
   if(mciSendString("play MUSIC from 0 notify", 0, 0, g_hWnd)) return 0;
   
   // return sucess
   return 1;
}



/*-----------------------------------------------------------------------------*/
// DirectMusic Functions
/*-----------------------------------------------------------------------------*/

// Resume the current song ///////////////////////////////////////////////
void DM_Resume(void)
{
   gDMPerf->PlaySegment(gDMSegment, DMUS_SEGF_BEAT, 0, &gDMSegState);
   gDMSegState->GetStartTime(&gDMStart);
}


// Pause the current song ////////////////////////////////////////////////
void DM_Pause(void)
{
   long mtNow;

   // stop the music and get the time
   gDMPerf->Stop(NULL, NULL, 0, 0);
   gDMPerf->GetTime(NULL, &mtNow);

   // re-calculate the offset
   gDMOffset += (mtNow - gDMStart);

   // set the start point for the segment for unpausing
   gDMSegment->SetStartPoint(gDMOffset);
   
   // release the segment state
   if(gDMSegState) {
      gDMSegState->Release();
      gDMSegState = NULL;
   }
}


// Get DirectMusic to play a song ////////////////////////////////////////
int DM_Play(LPSTR szFile)
{
   // clean up old segment
   if(gDMSegment) {
       gDMSegment->Release();
       gDMSegment = NULL;
   }

   // load in new MIDI file
   if(gDMLoader) {
      if((gDMSegment = DM_LoadSegment(gDMLoader, szFile)) == NULL) return 0;

      // ensure it plays as a standard midi
      gDMSegment->SetParam(GUID_StandardMIDIFile, -1, 0, 0, (void*)gDMPerf);

      // download the DLS
      gDMSegment->SetParam(GUID_Download, -1, 0, 0, (void*)gDMPerf);
   }

   // Play the segment
   if(gDMSegment) gDMPerf->PlaySegment(gDMSegment, 0, 0, &gDMSegState);

   // get the start time incase we want to pause
   gDMSegState->GetStartTime(&gDMStart);
   
   // return sucess
   return 1;
}


// A WM_CHECKMIDI message was sent ///////////////////////////////////////
void DM_Check(void)
{
   DMUS_NOTIFICATION_PMSG *pperfmsg;
   if(gDMPerf && gDMSegment) { // make sure preformance and segment actualy exist
      if(gDMPerf->GetNotificationPMsg(&pperfmsg) == S_OK) {
         if(IsEqualGUID(pperfmsg->guidNotificationType, GUID_NOTIFICATION_SEGMENT)) {
            if(pperfmsg->dwNotificationOption == DMUS_NOTIFICATION_SEGEND) {
               // the segment has stoped, go to the next song
               Music_PlayNext();
            }
         }
         gDMPerf->FreePMsg((DMUS_PMSG*)pperfmsg);
      }
   }
}


// Thread incharge of sending notify messages ////////////////////////////
DWORD WINAPI DM_Thread(void* pv)
{
   while(1) {
      // wait untill a message needs to be sent
      if(WaitForSingleObject(gHEvent, INFINITE) != WAIT_OBJECT_0) return 0;
      PostMessage(g_hWnd, WM_CHECKMIDI, 0, 0);
   }
   return 1;
}


// Load a segment ////////////////////////////////////////////////////////
LPDMS DM_LoadSegment(LPDML pLoader, LPSTR szFile)
{
   DMUS_OBJECTDESC des; 
   LPDMS           segment = NULL;
   WCHAR           wszFile[_MAX_PATH];

   // Convert from multibyte format to Unicode
   MULTI_TO_WIDE(wszFile, szFile);

   // describe the object to be loaded
   des.guidClass   = CLSID_DirectMusicSegment;
   des.dwSize      = sizeof(DMUS_OBJECTDESC);
   des.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
   wcscpy(des.wszFileName, wszFile);

   // get midi ready for play back
   pLoader->GetObject(&des, IID_IDirectMusicSegment2, (void**)&segment);

   // return the object
   return segment;
}


// Initilize DirectMusic /////////////////////////////////////////////////
int DM_Init(void)
{
   DWORD  dwTrdID;
   HANDLE hTrd;
   WCHAR  wszDir[_MAX_PATH];
   char   szDir[_MAX_PATH];

   // Initilize COM
   if(FAILED(CoInitialize(NULL))) return 0;

   // create the preformance
   if(FAILED(CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
      CLSCTX_INPROC, IID_IDirectMusicPerformance2, (void**)&gDMPerf))) return 0;

   // initilize the preformance
   if(FAILED(gDMPerf->Init(NULL, lpds, NULL))) return 0;

   // create the event for the thread
   if((gHEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL) return 0;

   // create the thread
   if((hTrd = CreateThread(NULL, 0, DM_Thread, NULL, 0, &dwTrdID)) == NULL) return 0;
   CloseHandle(hTrd);

   // set the notification handle of the preformance
   if(FAILED(gDMPerf->SetNotificationHandle(gHEvent, 0))) return 0;

   // add a notificatoin type
   if(FAILED(gDMPerf->AddNotificationType(GUID_NOTIFICATION_SEGMENT))) return 0;

   // add the port
   if(FAILED(gDMPerf->AddPort(NULL))) return 0;

   // create the loader
   if(FAILED(CoCreateInstance(CLSID_DirectMusicLoader, NULL,
      CLSCTX_INPROC, IID_IDirectMusicLoader, (void**)&gDMLoader))) return 0;

   // set the loader search directory
   GetCurrentDirectory(_MAX_PATH, szDir);
   MULTI_TO_WIDE(wszDir, szDir); // Convert from multibyte to Unicode
   if(FAILED(gDMLoader->SetSearchDirectory(GUID_DirectMusicAllTypes, wszDir, FALSE)))
      return 0;
 
   // return sucess
   return 1;
}


// Shutdown DirectMusic /////////////////////////////////////////////////.
void DM_Close(void)
{
   if(gDMPerf) {

      // stop the music
      gDMPerf->Stop(NULL, NULL, 0, 0);

      // make sure that gDMSegment is not null
      if(gDMSegment) {
         gDMSegment->SetParam(GUID_Unload, -1, 0, 0, (void*)gDMPerf);
         gDMSegment->Release();
         gDMSegment = NULL;
      }

      // release the loader
      if(gDMLoader) {
         gDMLoader->Release();
         gDMLoader = NULL;
      }

      // close the main preformance object
      gDMPerf->CloseDown();
      gDMPerf->Release();
      gDMPerf = NULL;

      // Release COM
      CoUninitialize();
   }
}
