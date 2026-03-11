/************************************************************************/
// file.cpp - Code for settings file I/O
//
// Author: Justin Hoffman
// Date:   6/29/2000 - 7/18/2000
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "network.h"
#include "game.h"
#include "user.h"
#include "window.h"
#include "file.h"

//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// info about currently opened file //////////////////////////////////////
HANDLE gIoFile = 0;
bool   gIoRead = 0;

// file names stored in strings //////////////////////////////////////////
char *FILE_SETTINGS   = "data\\settings.dat";
char *FILE_LOADPATH   = "data\\*.elm";
char *FILE_SAVEDGAME  = "data\\%s.elm";
char  FILE_LOADED[32] = "";


//————————————————————————————————————————————————————————————————————————
// Main Settings I/O Functions
//————————————————————————————————————————————————————————————————————————

/*
settings file format:

UCHAR    = gnSinkHoles
UCHAR    = gnGames
UCHAR[4] = WndNewGamePlrsOn
UCHAR[4] = WndNewGamePlrsType
UCHAR[4] = WndNewGamePlrsTeam
char[16] = Player Name 1
char[16] = Player Name 2
char[16] = Player Name 3
char[16] = Player Name 4

char[32] = Session Name/IP
char[32] = Network Player Name
bool     = UseLocalAreaNetwork

UCHAR    = mMusicType
UCHAR[x] = music on/off flags
*/

// create a default settings file ////////////////////////////////////////
void IoSettingsCreateDefault(void)
{
   long           i;
   HANDLE         file;
   DWORD          dwActual;
   unsigned short bits = 0xFFFF;

   // attempt to create a new file
   if((file = CreateFile(FILE_SETTINGS, GENERIC_WRITE, 0, 0,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) return;

   // write game info
   WriteFile(file, &gnSinkHoles      , sizeof(gnSinkHoles)       , &dwActual, 0);
   WriteFile(file, &gnGames          , sizeof(gnGames)           , &dwActual, 0);
   WriteFile(file, WndNewGamePlrsOn  , sizeof(WndNewGamePlrsOn)  , &dwActual, 0);
   WriteFile(file, WndNewGamePlrsType, sizeof(WndNewGamePlrsType), &dwActual, 0);
   WriteFile(file, WndNewGamePlrsTeam, sizeof(WndNewGamePlrsTeam), &dwActual, 0);

   // write player names
   for(i = 0; i < 4; i++)
      WriteFile(file, pdat[i].sName, sizeof(pdat[i].sName), &dwActual, 0);

   // seek to the proper location, write info, & close file
   SetFilePointer(file, FILE_OFFSET_SOUNDON, 0, FILE_BEGIN);
   WriteFile(file, &bits      , sizeof(bits)      , &dwActual, 0);
   WriteFile(file, &mMusicType, sizeof(mMusicType), &dwActual, 0);
   WriteFile(file, &bits      , sizeof(bits)      , &dwActual, 0);
   CloseHandle(file);
}

// get the information for the new game window ///////////////////////////
void IoSettingsNewGameInfo(bool read)
{
   long i;

   // attempt to open the file
   if(IoOpenFile(read, FILE_SETTINGS)) {
      IoSettingsCreateDefault();
      return;
   }

   // make sure game varables match those in window
   gnSinkHoles = (UCHAR)gWndA.pnums[0].value;
   gnGames     = (UCHAR)gWndA.pnums[1].value;

   // read/write misc data
   IoReadWrite(&gnSinkHoles      , sizeof(gnSinkHoles));
   IoReadWrite(&gnGames          , sizeof(gnGames));
   IoReadWrite(WndNewGamePlrsOn  , sizeof(WndNewGamePlrsOn));
   IoReadWrite(WndNewGamePlrsType, sizeof(WndNewGamePlrsType));
   IoReadWrite(WndNewGamePlrsTeam, sizeof(WndNewGamePlrsTeam));

   // read/write names
   for(i = 0; i < 4; i++) IoReadWrite(gWndA.ptxts[i].text, sizeof(gWndA.ptxts[i].text));

   // close the file
   IoClose();
}

// get the information for the connection window /////////////////////////
void IoSettingsNetInfo(bool read)
{
   bool UseLan;

   // attempt to open the file
   if(IoOpenFile(read, FILE_SETTINGS)) {
      IoSettingsCreateDefault();
      return;
   }

   // skip game info section
   SetFilePointer(gIoFile, FILE_OFFSET_NETINFO, 0, FILE_BEGIN);

   // see if we need to calculate UseLan
   if(!read) UseLan = BTN_DOWN(gWndA.pbtns[2]);

   // read/write network data
   IoReadWrite(gWndA.ptxts[0].text, sizeof(gWndA.ptxts[0].text));
   IoReadWrite(gWndA.ptxts[2].text, sizeof(gWndA.ptxts[2].text));
   IoReadWrite(&UseLan            , sizeof(UseLan));

   // see if we need to process based on UseLan
   if(read) {
      BSETB(gWndA.pbtns[2].flags, BTN_TDOWN, UseLan);
      BtnDraw(&gWndA.pbtns[2]);
   }

   // close the file
   IoClose();
}

// sound activation data /////////////////////////////////////////////////
void IoSettingsSoundOn(bool read)
{
   // attempt to open the file
   if(IoOpenFile(read, FILE_SETTINGS)) {
      IoSettingsCreateDefault();
      return;
   }

   // seek to proper location
   SetFilePointer(gIoFile, FILE_OFFSET_SOUNDON, 0, FILE_BEGIN);

   // read/write the data
   IoReadWrite(&gbSoundOn, sizeof(gbSoundOn));
   IoReadWrite(&gbMusicOn, sizeof(gbMusicOn));
   IoReadWrite(&mMusicType, sizeof(mMusicType));
   IoClose();
}

// load the song activation flags ////////////////////////////////////////
void IoSettingsSongInfo(bool read)
{
   long   i;
   long   dataLen;
   UCHAR *data;

   // make sure we are not using CD music
   if(mMusicType == MUSIC_CD) return;

   // attempt to open the file
   if(IoOpenFile(read, FILE_SETTINGS)) {
      IoSettingsCreateDefault();
      return;
   }

   // skip game info & net info sections
   SetFilePointer(gIoFile, FILE_OFFSET_SONGINFO, 0, FILE_BEGIN);

   // figure out how much mem is needed to store info, make
   // sure to add one bit if list length not divisible by 8
   dataLen  = (long)(mNumSongs / 8);
   dataLen += (mNumSongs % 8 > 0);

   // allocate a temp list
   data = (UCHAR*)MoreMem(dataLen);

   // write into temp list
   if(!read) {
      for(i = 0; i < mNumSongs; i++) {
         if(mSongFlags[i] & MUSIC_SONGACTIVE) data[i/8] |=  (0x01 << i%8);
         else                                 data[i/8] &= ~(0x01 << i%8);
      }
   }

   // read/write temp list
   IoReadWrite(data, dataLen);
   IoClose();

   // translate list
   if(read) {
      for(i = 0; i < mNumSongs; i++) {
         if(data[i/8] & (0x01 << i%8)) mSongFlags[i] |=  MUSIC_SONGACTIVE;
         else                          mSongFlags[i] &= ~MUSIC_SONGACTIVE;
      }
   }

   // release da mem
   LessMem(data);
}


//————————————————————————————————————————————————————————————————————————
// Saved Game I/O Functions
//————————————————————————————————————————————————————————————————————————

/*
Saved Game Format:

char[16] = Player 1 name
char[16] = Player 2 name
char[16] = Player 3 name
char[16] = Player 4 name

[Player info] x4
   UCHAR = player flags
   long  = player frame
   long  = frameDir
   long  = nGamesWon
   long  = nMarbles
   UCHAR = team
   UCHAR = type

UCHAR       = game_state
UCHAR       = game_nxtst
UCHAR       = game_psest
double      = gFrame
double      = gFrameVel
DWORD       = gTimerStart [interpreted]

UCHAR       = gnGames
UCHAR       = gnSinkHoles
UCHAR[2]    = gnTeamPlrs
UCHAR[2]    = gnTeamPlrsLeft
UCHAR[2][3] = gTeams
BYTE        = gMoveData[8];
char        = gBoard[16][16];

UCHAR       = gThisPlr
UCHAR       = nplrs
UCHAR       = pcur

char        = gAnimateX
char        = gAnimateY
char        = gMoveDstVal

double      = gObjFrame
double      = gObjWScale
double      = gObjHScale
char        = gObjValue
long        = gObjX
long        = gObjY
char        = gObjSrcX
char        = gObjSrcY
char        = gObjDstX
char        = gObjDstY
*/

// get saved games info //////////////////////////////////////////////////
char *IoGameGetInfo(long *pListLen)
{
   long             i      = 1;
   long             nGames = 1;
   char            *rval;
   HANDLE           hItem;
   WIN32_FIND_DATA  ItemInfo;

   // open the first file to figure out how many files there are
   if((hItem = FindFirstFile(FILE_LOADPATH, &ItemInfo)) == INVALID_HANDLE_VALUE) {
      *pListLen = 0;
      return 0;
   }

   // go through all of the other files
   while(FindNextFile(hItem, &ItemInfo)) nGames++;
   FindClose(hItem);
   
   // allocate memory for game info
   rval = (char*)MoreMem(nGames*80);

   // get first file name
   hItem = FindFirstFile(FILE_LOADPATH, &ItemInfo);
   IoGameGetFileInfo(0, &ItemInfo, rval);

   // get the rest of the file names
   while(FindNextFile(hItem, &ItemInfo)) {
      IoGameGetFileInfo(i, &ItemInfo, rval);
      i++;
   }

   // close the search
   FindClose(hItem);

   // return data & number of games
   *pListLen = nGames;
   return rval;
}

// get a particular files information ////////////////////////////////////
void IoGameGetFileInfo(long index, WIN32_FIND_DATA *pFileData, char *pData)
{
   long    i;
   HANDLE  file;
   DWORD   dwActual;
   char   *cache;
   char    sFile[32];

   // cache value
   cache = pData + index*80;

   // copy the file name to the main data buffer thingie
   memcpy(cache, pFileData->cFileName, lstrlen(pFileData->cFileName) - 4);

   // calculate the file name
   wsprintf(sFile, FILE_SAVEDGAME, cache);

   // open the file
   file = CreateFile(sFile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);     
  
   // read the player names
   for(i = 0; i < 4; i++) ReadFile(file, cache + i*16 + 16, 16, &dwActual, 0);

   // close the file
   CloseHandle(file);
}

// load a game ///////////////////////////////////////////////////////////
void IoGame(char *sFile, bool read)
{
   long  i;
   DWORD timer;
   char  sName[16] = "----";

   gIoRead = read;

   // attempt to open the file for reading
   if(read) {

      gIoFile = CreateFile(sFile, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   } else {

      // delete the old file & create a new one
      DeleteFile(sFile);
      gIoFile = CreateFile(sFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

      // determine gTimerStart's elaps time
      timer = gThisTick - gTimerStart;
   }

   // make sure the file is valid
   if(gIoFile == INVALID_HANDLE_VALUE) return;

   // read/write player names
   for(i = 0; i < 4; i++)
      IoReadWrite((pdat[i].flags & PLR_ON)? pdat[i].sName : sName, 16);

   // read/write other player info
   for(i = 0; i < 4; i++) {
      IoReadWrite(&pdat[i].flags    , sizeof(pdat[i].flags));
      IoReadWrite(&pdat[i].frame_a  , sizeof(pdat[i].frame_a));
      IoReadWrite(&pdat[i].frame_b  , sizeof(pdat[i].frame_b));
      IoReadWrite(&pdat[i].frameDir , sizeof(pdat[i].frameDir));
      IoReadWrite(&pdat[i].nGamesWon, sizeof(pdat[i].nGamesWon));
      IoReadWrite(&pdat[i].nMarbles , sizeof(pdat[i].nMarbles));
      IoReadWrite(&pdat[i].team     , sizeof(pdat[i].team));
      IoReadWrite(&pdat[i].type     , sizeof(pdat[i].type));
   }

   // read/write game state info
   IoReadWrite(&game_state, sizeof(game_state));
   IoReadWrite(&game_nxtst, sizeof(game_nxtst));
   IoReadWrite(&game_psest, sizeof(game_psest));
   IoReadWrite(&gFrame    , sizeof(gFrame));
   IoReadWrite(&gFrameVel , sizeof(gFrameVel));
   IoReadWrite(&timer     , sizeof(timer));

   // read/write game settings info
   IoReadWrite(&gnGames      , sizeof(gnGames));
   IoReadWrite(&gnSinkHoles  , sizeof(gnSinkHoles));
   IoReadWrite(gnTeamPlrs    , sizeof(gnTeamPlrs));
   IoReadWrite(gnTeamPlrsLeft, sizeof(gnTeamPlrsLeft));
   IoReadWrite(gTeams        , sizeof(gTeams));
   IoReadWrite(gMoveData     , sizeof(gMoveData));
   IoReadWrite(gBoard        , sizeof(gBoard));
  
   // read/write player info
   IoReadWrite(&gThisPlr, sizeof(gThisPlr));
   IoReadWrite(&nplrs   , sizeof(nplrs));
   IoReadWrite(&pcur    , sizeof(pcur));

   // read/write animation info
   IoReadWrite(&gAnimateX  , sizeof(gAnimateX));
   IoReadWrite(&gAnimateY  , sizeof(gAnimateY));
   IoReadWrite(&gMoveDstVal, sizeof(gMoveDstVal));

   // read/write "flying object" data
   IoReadWrite(&gObjFrame , sizeof(gObjFrame));
   IoReadWrite(&gObjWScale, sizeof(gObjWScale));
   IoReadWrite(&gObjHScale, sizeof(gObjHScale));
   IoReadWrite(&gObjValue , sizeof(gObjValue));
   IoReadWrite(&gObjX     , sizeof(gObjX));
   IoReadWrite(&gObjY     , sizeof(gObjY));
   IoReadWrite(&gObjSrcX  , sizeof(gObjSrcX));
   IoReadWrite(&gObjSrcY  , sizeof(gObjSrcY));
   IoReadWrite(&gObjDstX  , sizeof(gObjDstX));
   IoReadWrite(&gObjDstY  , sizeof(gObjDstY));

   // initlize the bars if we are reading
   if(read) {
      NameSetup();
      gObjLTick   = GetTickCount();
      gTimerStart = gObjLTick - timer;
   }

   // read/write bar data
   for(i = 0; i < 4; i++) {

      // create temp timer data
      if(!read) timer = gThisTick - gBarNames[i].ltick; 

      // read/write to file
      IoReadWrite(&gBarNames[i].x    , sizeof(gBarNames[i].x));
      IoReadWrite(&gBarNames[i].state, sizeof(gBarNames[i].state));
      IoReadWrite(&timer             , sizeof(timer));

      // read temp timer data
      if(read) gBarNames[i].ltick = gThisTick - timer;
   }

   // read the dice data
   for(i = 0; i < 2; i++) {

      // create temp timer data
      if(!read) timer = gThisTick - gBarDice[i].ltick; 

      // read/write to file
      IoReadWrite(&gBarDice[i].y    , sizeof(gBarDice[i].y));
      IoReadWrite(&gBarDice[i].state, sizeof(gBarDice[i].state));
      IoReadWrite(&gBarDice[i].extra, sizeof(gBarDice[i].extra));
      IoReadWrite(&gBarDice[i].rsrc , sizeof(gBarDice[i].rsrc));
      IoReadWrite(&timer            , sizeof(timer));

      // read temp timer data
      if(read) gBarDice[i].ltick = gThisTick - timer;
   }

   IoClose();
}


//————————————————————————————————————————————————————————————————————————
// I/O Wrappers
//————————————————————————————————————————————————————————————————————————

// see if a file exists already //////////////////////////////////////////
bool IoFileExists(char *sFile)
{
   HANDLE hFile;

   // attempt to open the file
   if((hFile = CreateFile(sFile, GENERIC_READ, 0, 0,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) return 0;

   // return sucess
   CloseHandle(hFile);
   return 1;
}

// open the file for reading/writing /////////////////////////////////////
int IoOpenFile(bool read, char *sFile)
{
   // make sure we are still in the elim directory
   SetCurrentDirectory(gszAppPath);

   // set gIoRead
   gIoRead = read;

   // attept to open the file
   gIoFile = CreateFile(sFile, read? GENERIC_READ : GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

   // return 1 if failed
   return gIoFile == INVALID_HANDLE_VALUE;
}

// read/write from a file ////////////////////////////////////////////////
void IoReadWrite(void *buffer, UINT length)
{
   DWORD dwActual;

   if(gIoRead) ReadFile( gIoFile, buffer, length, &dwActual, 0);
   else        WriteFile(gIoFile, buffer, length, &dwActual, 0);
}

// close the current file ////////////////////////////////////////////////
void IoClose(void)
{
   CloseHandle(gIoFile);
}