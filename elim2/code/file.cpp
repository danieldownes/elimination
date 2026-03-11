/*******************************************************************************/
// file.cpp - File I/O for the game
//
// Written By : Justin Hoffman
// Date       : July 15, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "elimres.h"
#include "main.h"
#include "board.h"
#include "file.h"


/*-----------------------------------------------------------------------------*/
// Settings I/O
/*-----------------------------------------------------------------------------*/

/*
Data format for the settings is currently:

DWORD fileID            - used to see if the file is really what it says
bool  musicOn           - the music is on
bool  soundOn           - the sound is on
bool  showMoves         - showMoves is on
char  g_name[4][32]     - the players names
DWORD g_pFlags[4]       - player flags
int   g_nPlayers        - number of players in this game
int   g_nSinkholes      - number of sinkholes
TEAM  g_teams           - the description of the current teams
*/

// save/load the game settings ///////////////////////////////////////////
void IO_Settings(int read)
{
   int    i;
   bool   musicOn;
   bool   soundOn;
   bool   showMoves;
   DWORD  tempFlags[4];
   HANDLE hFile;

   // save/load the registration flag
   SET_FLAG(g_settings, SET_REGISTERED, 1);
   /*IO_Registered(read);
   */

   //////////////////////////////////////////////////////////////////
   // Open the file & translate data
   if(read)
   {
      // if the file does not exist, make a new file
      if(!IO_OpenFileRead(FILE_SETTINGS, &hFile, OPEN_EXISTING, FILE_IS_SETTINGS))
      {
         CloseHandle(hFile);
         IO_Settings(WRITE);
         return;
      }
   }
   else
   {
      // ajust the player flags, removing the out flag
      for(i = 0; i < 4; i++)
      {
         tempFlags[i] = g_pFlags[i];
         tempFlags[i] &= ~PLAYER_OUT;
      }

      // set the settings
      musicOn    = FLAG_SET(g_settings, SET_MUSIC_ON);
      soundOn    = FLAG_SET(g_settings, SET_SOUND_ON);
      showMoves  = FLAG_SET(g_settings, SET_SHOW_MOVES);

      // open the file for writing
      if(!IO_OpenFileWrite(FILE_SETTINGS, &hFile, OPEN_ALWAYS, FILE_IS_SETTINGS)) return;
   }


   //////////////////////////////////////////////////////////////////
   // Read/Write the setttings
   IO_ReadWrite(&hFile, &musicOn,      sizeof(musicOn),      read); // music is on
   IO_ReadWrite(&hFile, &soundOn,      sizeof(soundOn),      read); // sound is on
   IO_ReadWrite(&hFile, &showMoves,    sizeof(showMoves),    read); // showMoves is on
   IO_ReadWrite(&hFile, &g_name,       sizeof(g_name),       read); // player names
   IO_ReadWrite(&hFile, &tempFlags,    sizeof(tempFlags),    read); // player flags
   IO_ReadWrite(&hFile, &g_nPlayers,   sizeof(g_nPlayers),   read); // number of players
   IO_ReadWrite(&hFile, &g_nSinkholes, sizeof(g_nSinkholes), read); // number of sinkholes
   IO_ReadWrite(&hFile, &g_teams,      sizeof(g_teams),      read); // team data


   //////////////////////////////////////////////////////////////////
   // Translate data
   if(read)
   {
      // set the g_settings flags
      SET_FLAG(g_settings, SET_MUSIC_ON, musicOn);
      SET_FLAG(g_settings, SET_SOUND_ON, soundOn);
      SET_FLAG(g_settings, SET_SHOW_MOVES, showMoves);
   
      // the player flags
      for(i = 0; i < 4; i++) g_pFlags[i] = tempFlags[i];
   }

   // close the file handle
   CloseHandle(hFile);
}


/*-----------------------------------------------------------------------------*/
// Registered I/O
/*-----------------------------------------------------------------------------*/
/*
void IO_Registered(int read)
{
   HANDLE hFile;
   bool   registered;
   char   szWinPath[2048];
   DWORD  dwId = FILE_IS_REGISTERED;

   // set the current directory to the windows directory
   GetWindowsDirectory(szWinPath, 2048);
   SetCurrentDirectory(szWinPath);

   // Open the file
   if(read)
   {
      // if the file does not exist, make a new file
      if((hFile = CreateFile(FILE_REGISTERED, GENERIC_READ, 0, NULL, OPEN_EXISTING,
         FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE)
      {
         IO_Registered(WRITE);
         return;
      }

      // make sure the file is of the correct format
      IO_ReadWrite(&hFile, &dwId, sizeof(dwId), READ);
      if(dwId != FILE_IS_REGISTERED)
      {
         CloseHandle(hFile);
         IO_Registered(WRITE);
         return;
      }
   }
   else
   {
      // set the registered boolean
      registered = (g_settings & SET_REGISTERED);

      // open the file for writing
      if((hFile = CreateFile(FILE_REGISTERED, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
         FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) return;
   }

   // write the data (if the game is registered or not)
   IO_ReadWrite(&hFile, &registered, sizeof(registered), read);

   // set the registration flag accordingly
   if(read) SET_FLAG(g_settings, SET_REGISTERED, registered);
}
*/

/*-----------------------------------------------------------------------------*/
// Saved games I/O
/*-----------------------------------------------------------------------------*/

/*
Data Format for saved games is:

char     name[32]           - the saved name of the game
BOOL     active             - the game is active
TEAMS    teams;             - teams data
char     pNames[4][32]      - the player names
int      pFlags[4]          - the player status flags
int      pMarbles[4]        - number of remaining marbles a player has
int      pScore[4]          - the players scores
int      dice[2]            - the value of the dice
int      nPlayers           - the number of players
int      cPlayer            - the current player
DWORD    used_time          - amout of unregestered time used up
SPRITE   bullet             - the status of the bullet
int      jumpStatus         - the current status of the jump
long     jumpFrames         - the current number of frames passed in the jump
MARBLEID jumpSrc            - source of jumping marble
MARBLEID jumpDst            - destination of jumping marble
int      board[16][16]      - the board data

NOTE: the first item before the saved games is a DWORD with the file id
*/


// Create a new empty saved games file ///////////////////////////////////
void IO_NewGameFile(void)
{
   HANDLE hFile;
   char   szFileName[32] = "Empty";
   BOOL   fileInactive   = FALSE;

   // open the file, check for failure
   if(!IO_OpenFileWrite(FILE_SAVED, &hFile, OPEN_ALWAYS, FILE_IS_SAVED)) return;

   for(int i = 0; i < 11; i++)
   {
      // set the file pointer
      SetFilePointer(hFile, SAVED_SIZE * i + sizeof(DWORD), NULL, FILE_BEGIN);

      IO_ReadWrite(&hFile, &szFileName,   sizeof(szFileName),   WRITE); // file name
      IO_ReadWrite(&hFile, &fileInactive, sizeof(fileInactive), WRITE); // set the game to inactive
   }

   // close the file handle
   CloseHandle(hFile);
}


// Load/save the game names //////////////////////////////////////////////
BOOL IO_GameNames(char names[4][32], int read)
{
   HANDLE hFile;

   // open the file
   if(read)
   {
      if(!IO_OpenFileRead(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED))
      {
         // create a new saved game file
         CloseHandle(hFile);
         IO_NewGameFile();

         // re-open the file
         if(!IO_OpenFileRead(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED))
            return FALSE;
      }
   }
   else
   {
      // check for failure
      if(!IO_OpenFileWrite(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED)) return FALSE;
   }

   // write/read the names
   for(int i = 0; i < 10; i++)
   {
      // Set the file pointer
      SetFilePointer(hFile, SAVED_SIZE * i + sizeof(DWORD), NULL, FILE_BEGIN);

      // write/read the name
      IO_ReadWrite(&hFile, &names[i], (sizeof(char) * 32), read);
   }

   // close the file handle
   CloseHandle(hFile);

   // return sucess
   return TRUE;
}


// Load the game information /////////////////////////////////////////////
BOOL IO_LoadGameStats(EDAT_LOADGAME *data)
{
   HANDLE hFile;
   DWORD playerFlags[10][4];
   int i, j;

   // open the file for reading
   if(!IO_OpenFileRead(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED))
   {
      // make a new settings file
      IO_NewGameFile();

      // re-load the settings file, check for errors
      if(!IO_OpenFileRead(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED)) return FALSE;
   }

   // read the data
   for(i = 0; i < 10; i++)
   {
      // set the file pointer
      SetFilePointer(hFile, SAVED_SIZE * i + sizeof(DWORD), NULL, FILE_BEGIN);

      // read the data
      IO_ReadWrite(&hFile, &data->gameName[i], sizeof(data->gameName[0]), READ); // file name
      IO_ReadWrite(&hFile, &data->active[i],   sizeof(data->active[0]),   READ); // game is active
      
      // if it is not active, we have all the data we need
      if(!data->active[i]) continue;

      IO_ReadWrite(&hFile, &data->teams[i], sizeof(data->teams[0]), READ); // teams data
      IO_ReadWrite(&hFile, &data->names[i], sizeof(data->names[0]), READ); // player names
      IO_ReadWrite(&hFile, &playerFlags[i], sizeof(playerFlags[0]), READ); // player flags
   
      // modify the data
      for(j = 0; j < 4; j++)
      {
         // see if the player is active, if not clear the name
         if(!(playerFlags[i][j] & PLAYER_ACTIVE)) wsprintf(data->names[i][j], "----");
      }
   }

   CloseHandle(hFile);
   return TRUE;
}


// Load/save the game ////////////////////////////////////////////////////
BOOL IO_Game(DWORD slot, int read)
{
   int      i, j;
   SPRITE   *temp;
   int      jumpStatus;
   long     jumpFrame;
   MARBLEID jumpDst;
   MARBLEID jumpSrc;
   HANDLE   hFile;
   DWORD    usedTime      = 0;
   BOOL     active        = TRUE;
   DWORD    thisTick      = GetTickCount();
   int      diceValue[2]  = {g_diceB[0].value, g_diceB[1].value};
   BOOL     (*pfn)(LPSTR, HANDLE*, DWORD, DWORD);
   
   // see if we should open for reading or for writing
   pfn = read? IO_OpenFileRead : IO_OpenFileWrite;

   // open the file for reading/writing
   if(!pfn(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED))
   {
      // create a new file
      CloseHandle(hFile);
      IO_NewGameFile();

      // try to re-open it, report any errors
      if(!pfn(FILE_SAVED, &hFile, OPEN_EXISTING, FILE_IS_SAVED)) return FALSE;
   }

   // set the file pointer's position
   SetFilePointer(hFile, SAVED_SIZE * slot + sizeof(DWORD) + sizeof(char) * 32,
      NULL, FILE_BEGIN);

   if(!read)
   {
      // clear the marble id's and status
      jumpDst.a = jumpDst.b = D_INVALID;
      jumpSrc.a = jumpSrc.b = D_INVALID;
      jumpStatus = 0;

      if(g_sprBullet.status == MARBLE_JUMPING)
      {
         // figure out what sprite was going to move
         temp = Marble_Search(g_sprBullet.a, g_sprBullet.b);

         // record where the marble wanted to go
         jumpDst.a = temp->da;
         jumpDst.b = temp->db;
      }
      else
      {

         // see if any marbles are jumping/ready to jump
         temp = g_sprMarbles.next;

         while(temp != &g_sprMarbles)
         {
            // see what the marble is currently doing
            if(temp->status == MARBLE_JUMPING || temp->status == MARBLE_INIT_JUMP)
            {
               // set the jumpStatus
               jumpStatus = temp->status;

               // set the jump chordinates
               jumpSrc.a = temp->a;
               jumpSrc.b = temp->b;
               jumpDst.a = temp->da;
               jumpDst.b = temp->db;

               // set the jump's number of frames
               jumpFrame = temp->frame;
               break;
            }
            
            // go on to the next link
            temp = temp->next;
         }
      }

      // calculate usedTime
      usedTime = (thisTick - g_startTime); 
   }

   // read/write the data
   IO_ReadWrite(&hFile, &active,      sizeof(active),      read); // game is active
   IO_ReadWrite(&hFile, &g_teams,     sizeof(g_teams),     read); // team data
   IO_ReadWrite(&hFile, &g_name,      sizeof(g_name),      read); // player names
   IO_ReadWrite(&hFile, &g_pFlags,    sizeof(g_pFlags),    read); // player flags
   IO_ReadWrite(&hFile, &g_marbles,   sizeof(g_marbles),   read); // remaining marbles   
   IO_ReadWrite(&hFile, &g_score,     sizeof(g_score),     read); // player score
   IO_ReadWrite(&hFile, &diceValue,   sizeof(diceValue),   read); // the value of the dice
   IO_ReadWrite(&hFile, &g_nPlayers,  sizeof(g_nPlayers),  read); // number of players
   IO_ReadWrite(&hFile, &g_cPlayer,   sizeof(g_cPlayer),   read); // current player
   IO_ReadWrite(&hFile, &usedTime,    sizeof(usedTime),    read); // amout of reg time used
   IO_ReadWrite(&hFile, &g_sprBullet, sizeof(g_sprBullet), read); // bullet sprite
   IO_ReadWrite(&hFile, &jumpStatus,  sizeof(jumpStatus),  read); // jumping marbles status
   IO_ReadWrite(&hFile, &jumpFrame,   sizeof(jumpFrame),   read); // the jumps current frame
   IO_ReadWrite(&hFile, &jumpSrc,     sizeof(jumpSrc),     read); // jumping marbles source
   IO_ReadWrite(&hFile, &jumpDst,     sizeof(jumpDst),     read); // jumping marbles destination
   IO_ReadWrite(&hFile, &g_board,     sizeof(g_board),     read); // board data

   // close the file handle
   CloseHandle(hFile);
   
   // interpret the read data
   if(read)
   {
      // figure out what to set startTime to so the clock is correct
      g_startTime = (thisTick - usedTime);

      Marble_DeleteAll();                      // delete the current marbles
      g_selected.a = g_selected.b = D_INVALID; // clear the selected marble
      g_settings &= ~SET_MARBLE_JUMPING;       // remove the marble jumping flag

      // setup the bullet
      g_sprBullet.lastTick = thisTick;

      // create the marbles
      for(i = 0; i < 16; i++)
         for(j = 0; j < 16; j++)
            if(g_board[i][j] < 4) Marble_New(FALSE, g_board[i][j], i, j);

      // create a marble in the air, if there was one
      if(!INVALID_MARBLE(jumpSrc.a, jumpSrc.b))
      {
         // find the sprite
         temp = Marble_Search(jumpSrc.a, jumpSrc.b);

         // edit the sprite
         temp->status   = jumpStatus;
         temp->frame    = jumpFrame;
         temp->lastTick = thisTick;
         temp->da = jumpDst.a;
         temp->db = jumpDst.b;

         // set the marble jumping flag
         g_settings |= SET_MARBLE_JUMPING; 
      }

      // edit the sprite if the bullet was in the air
      if(g_sprBullet.status == MARBLE_JUMPING)
      {
         // find the sprite to edit
         temp = Marble_Search(g_sprBullet.a, g_sprBullet.b);

         // edit the sprite
         temp->da = jumpDst.a;
         temp->db = jumpDst.b;

         // set the marble jumping flag
         g_settings |= SET_MARBLE_JUMPING; 
      }

      // set the dice values
      g_diceB[0].value = diceValue[0];
      g_diceB[1].value = diceValue[1];
   }
   
   return TRUE;
}


/*-----------------------------------------------------------------------------*/
// Main file I/O functions
/*-----------------------------------------------------------------------------*/

// Open a file for writing ///////////////////////////////////////////////
static BOOL IO_OpenFileWrite(LPSTR szFile, HANDLE* phFile, DWORD dwFlag, DWORD dwFileId)
{
   DWORD written;

   // set the current path to the application path so the files can be found
   SetCurrentDirectory(g_appPath);

   // attempt to open the file
   if((*phFile = CreateFile(szFile, GENERIC_WRITE, 0, NULL, dwFlag,
      FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) return FALSE;

   // write the id to the file
   if(!WriteFile(*phFile, &dwFileId, sizeof(dwFileId), &written, NULL)) return FALSE;

   // return success
   return TRUE;
}


// Open a file for reading ///////////////////////////////////////////////
static BOOL IO_OpenFileRead(LPSTR szFile, HANDLE* phFile, DWORD dwFlag, DWORD dwFileId)
{
   DWORD dwMatchId;
   DWORD written;

   // set the current path to the application path so the files can be found
   SetCurrentDirectory(g_appPath);

   // attempt to open the file
   if((*phFile = CreateFile(szFile, GENERIC_READ, 0, NULL, dwFlag,
      FILE_ATTRIBUTE_NORMAL, NULL)) == INVALID_HANDLE_VALUE) return FALSE;

   // read the file id
   ReadFile(*phFile, &dwMatchId, sizeof(dwMatchId), &written, NULL);

   // return the file is the correct file
   return dwFileId == dwMatchId;
}


// Read/Write to a file //////////////////////////////////////////////////
void IO_ReadWrite(HANDLE* phFile, LPVOID buffer, DWORD size, int read)
{
   DWORD dwReal;

   if(read) ReadFile(*phFile, buffer, size, &dwReal, NULL);
   else WriteFile(*phFile, buffer, size, &dwReal, NULL);
}


#ifdef ELIM_DEBUG
/*-----------------------------------------------------------------------------*/
// debug I/O
/*-----------------------------------------------------------------------------*/

static HANDLE dbgFile;
extern error_detect;

/*
File Formatt: time (since game start),event,object pointer,player,error
*/

// Open the debug file ///////////////////////////////////////////////////
void IO_DebugOpen(void)
{
   DWORD dwWritten;
   char szText[] = "Time,Event,Pointer,Player,Computer,Error\r";

   DeleteFile("debug.txt");

   // open the file for writing
   dbgFile = CreateFile("debug.txt", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
      NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

   // write the headings
   WriteFile(dbgFile, szText, strlen(szText), &dwWritten, NULL);
}


// Write to the debug file ///////////////////////////////////////////////
void IO_DebugWrite(char* event, DWORD ptr)
{
   DWORD dwWritten;
   char szText[256];

   wsprintf(szText, "%d,%s,%d,%d,%s,%s\r", (GetTickCount() - g_startTime), event,
      ptr, g_cPlayer, (g_pFlags[g_cPlayer] & PLAYER_IS_PC)? "c":"h",
      error_detect? "ERROR":" ");
   
   WriteFile(dbgFile, szText, strlen(szText), &dwWritten, NULL);
}


// Close the debug file //////////////////////////////////////////////////
void IO_DebugClose(void) { CloseHandle(dbgFile); }

#endif