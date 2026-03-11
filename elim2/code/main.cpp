/*******************************************************************************/
// main.cpp - Setup and manage window and the application
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "elimres.h"
#include "main.h"
#include "file.h"
#include "board.h"
#include "window.h"
#include "computer.h"
#include "graph.h"
#include "sound.h"


// Constant Globals //////////////////////////////////////////////////////
RECT      R_SCREEN = {0, 0, SCREEN_W, SCREEN_H};
char*     TXT_MENU_DESC[] = {
   "Start a new game",
   "Load a game",
   "Save this game",
   "Exit the game",
   "Minimize the game window",
   "Show the help file",
   "Turn on or off the sound effects",
   "Turn on or off the music",
   "Show or hide the moves"};


// Game Globals //////////////////////////////////////////////////////////
DWORD g_settings    = SET_DEFAULT;      // game settings flags
DWORD g_startTime;                      // time the game was started
TEAM  g_teams;                          // the description of the current teams
int   g_gameState   = STATE_INTRO;      // current status of the game
int   g_nPlayers    = 2;                // number of players in this game
int   g_nSinkholes  = 30;               // number of sinkholese
int   g_cPlayer     = 0;                // number of the current player
int   g_cColor      = 0;                // color of the current player
int   g_marbles[4]  = {10, 10, 10, 10}; // number of remaining marbles for each player
long  g_score[4]    = {0, 0, 0, 0};     // the score for each player
char  g_name[4][32] = {"Player 1", "Player 2", "Player 3", "Player 4"};
int   g_pFlags[4]   = {PLAYER_DEF_1, PLAYER_DEF_2, PLAYER_DEF_3, PLAYER_DEF_4};
long  g_curx        = 320;
long  g_cury        = 240;

// Game Objects //////////////////////////////////////////////////////////
PINFO g_pInfo[4];          // the player info boxes
int   g_cMenu = D_INVALID; // current menu being clicked
BTN   g_menu[D_NUM_MENUS]; // buttons in the menubar
BOX   g_regtime;           // displays the time left if unregistered
BOX   g_menubar;           // the menubar
BOX   g_teamsBox;          // teams box
BOX   g_diceA[4];          // dice for seeing who goes first
BOX   g_diceB[2];          // main game dice


// General Globals ///////////////////////////////////////////////////////
HINSTANCE g_hInst;                        // handle to the main instance
HWND      g_hWnd;                         // handle to the main window
XMPDATA   g_introXMP;                     // intro screen image
char      g_appPath[256];                 // pathname of this application
DWORD     g_pauseTime;                    // time when applications was paused
DWORD     g_suspendStart;                 // time the application was suspended

#ifdef ELIM_DEBUG
BOOL show_debug_data = TRUE;              // display the framrate and data
BOOL error_detect = FALSE;                // user detected error
#endif


// GDI globals ///////////////////////////////////////////////////////////
HFONT g_fontBig = CreateFont(20, 0, 0 , 0, 400, FALSE, FALSE, FALSE,
  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
  FF_DONTCARE, "Arial");

HFONT g_fontSmall = CreateFont(8, 0, 0 , 0, 700, FALSE, FALSE, FALSE,
  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
  FF_DONTCARE, "MS Sans Serif");

HFONT g_fontHelp = CreateFont(15, 0, 0 , 0, 400, FALSE, FALSE, FALSE,
  ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
  FF_DONTCARE, "Arial");


// Palette Stuff /////////////////////////////////////////////////////////
LPDIRECTDRAWPALETTE ddpMain    = NULL; // main color palette
PALETTEENTRY        g_palIntro[256];   // palette entries for intro section
PALETTEENTRY        g_palMain[256];    // palette entries for main section


// DirectDraw Objects ////////////////////////////////////////////////////
LPDDS ddsFront   = NULL; // primary (displayed) surface
LPDDS ddsBack    = NULL; // back buffer for flipping
LPDDS ddsBoard   = NULL; // a picture of the board
LPDDS ddsDice    = NULL; // the dice
LPDDS ddsExplode = NULL; // the explosion animation
LPDDS ddsItems   = NULL; // misc. game items
LPDDS ddsMenu    = NULL; // the menu button pictures
LPDDS ddsSelect  = NULL; // the selection ring animation

LPDDS ddsWindow  = NULL; // surface for storing the window
LPDDS ddsMenubar = NULL; // stores the menubar
LPDDS ddsTeams   = NULL; // show's who is on what team
LPDDS ddsRegTime = NULL; // display the time left for unregistered versions
LPDDS ddsPInfo   = NULL; // display the player info

// DirectSound Objects ///////////////////////////////////////////////////
LPDSO dsoMenu      = NULL; // menuitem sound
LPDSO dsoMenuB     = NULL; // play when items in the get names box are clicked
LPDSO dsoQuestion  = NULL; // Question sound 
LPDSO dsoUnallowed = NULL; // not allowed sound
LPDSO dsoSink      = NULL; // a marble has sunk
LPDSO dsoMarbleUp  = NULL; // a marble has rissen
LPDSO dsoSelect    = NULL; // select a marble
LPDSO dsoExplode   = NULL; // explotion sound
LPDSO dsoFree      = NULL; // free marble sound
LPDSO dsoError     = NULL; // error sound


/*-----------------------------------------------------------------------------*/
// Window Management Functions
/*-----------------------------------------------------------------------------*/

// process messages sent to the window ///////////////////////////////////
LRESULT CALLBACK WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   DWORD thisTick = GetTickCount();

   switch(message) {
   case WM_ACTIVATEAPP:
      SET_FLAG(g_settings, SET_APP_PAUSED, wParam == 0); // set the app paused flag
      if(g_settings & SET_APP_PAUSED) {
         // record the time we were suspended
         g_suspendStart = thisTick;

         // pause the music so while we are suspened it dont keep playing
         if((g_settings & SET_MUSIC_ON) && g_gameState != STATE_INTRO) Music_Pause();
      } else {
         // resume the music if it was origionally on
         if((g_settings & SET_MUSIC_ON) && g_gameState != STATE_INTRO) Music_Resume();
      
         // add the suspend time to g_startTime
         g_startTime += (thisTick - g_suspendStart);
         g_pauseTime += (thisTick - g_suspendStart);
      }

   case WM_PAINT:
      ValidateRect(hWnd, NULL);
      break;
   
   case WM_DESTROY:
      PostQuitMessage(0);
      break;
   
   case MM_MCINOTIFY:
		if(wParam == MCI_NOTIFY_SUCCESSFUL) Music_PlayNext();
		break;

   case WM_CHECKMIDI:
      DM_Check();
      break;

   case WM_MOUSEMOVE:
      g_curx = LOWORD(lParam);
      g_cury = HIWORD(lParam);
      if(g_settings & SET_WINDOW)
         Win_Message(WM_MOUSEMOVE, wParam, lParam);
      else if(g_gameState == STATE_PLAYING)
         Menu_MouseMove(LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_LBUTTONDOWN:
      if(g_gameState == STATE_INTRO) {
         g_gameState = STATE_INITGAME; // close the intro and start the game
      } else {
         // see if a window is present
         if(g_settings & SET_WINDOW) {
            Win_Message(WM_LBUTTONDOWN, wParam, lParam);
         } else {
            // send the message to the board and menu
            Menu_MouseDown(LOWORD(lParam), HIWORD(lParam));
            Board_MouseDown(LOWORD(lParam), HIWORD(lParam));
         }
      }
      break;
   
   case WM_LBUTTONUP:
      if(g_settings & SET_WINDOW) Win_Message(WM_LBUTTONUP, wParam, lParam);
      else if(g_gameState == STATE_PLAYING) Menu_MouseUp(LOWORD(lParam), HIWORD(lParam));
   
   case WM_CHAR:
      // send this message to a dialog box if there is one
      if(g_settings & SET_WINDOW) Win_Message(message, wParam, lParam);
      break;
   
   case WM_KEYDOWN:
      // send this message to a dialog box if there is one
      if(g_settings & SET_WINDOW) Win_Message(message, wParam, lParam);

      // if this is the intro, move on to start the game
      else if(g_gameState == STATE_INTRO) g_gameState = STATE_INITGAME;

#ifdef ELIM_DEBUG
      if(wParam == VK_ESCAPE) {
         // make sure there is no window or the app wont terminate correctly
         if(!(g_settings & SET_WINDOW)) g_gameState = STATE_SHUTDOWN;
      }
      else if(wParam == VK_F1) show_debug_data = !show_debug_data;
#endif

      break;
   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }

   return 0;
}


// Application entry point ///////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine, int nCmdShow)
{
   WNDCLASS winClass;
   MSG      msg;

   // store the application instance
   g_hInst = hInst;

   // retreave the application path name
   GetCurrentDirectory(256, g_appPath);

   // setup the main window class
   winClass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   winClass.lpfnWndProc   = WinProc;
   winClass.cbClsExtra    = 0;
   winClass.cbWndExtra    = 0;
   winClass.hInstance     = hInst;
   winClass.hIcon         = LoadIcon(g_hInst, (LPCTSTR)ICO_MAIN);
   winClass.hCursor       = LoadCursor(g_hInst, (LPCTSTR)CUR_MAIN);
   winClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
   winClass.lpszMenuName  = NULL; 
   winClass.lpszClassName = TXT_WIN_CLASS;

   // register the window class
   if(!RegisterClass(&winClass)) return 0;

   // create the main window
   if(!(g_hWnd = CreateWindow(TXT_WIN_CLASS, TXT_WIN_TITLE,
      WS_POPUP | WS_VISIBLE, 0, 0,
      GetSystemMetrics(SM_CXSCREEN),
      GetSystemMetrics(SM_CYSCREEN),
      NULL, NULL, hInst, NULL)))
      return 0;

   if(!ResAvalable()) {
      MessageBox(g_hWnd, TXT_NORESOURCE, TXT_GENTITLE, MB_OK);
      return 0;
   }

   ElimInit();

   // main event loop
   while(1) {
      if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         if(msg.message == WM_QUIT) break;  // see if we should quit
         TranslateMessage(&msg);            // translate accelerator keys
         DispatchMessage(&msg);             // send message to window proc
      }
      if(!(g_settings & SET_APP_PAUSED)) ElimMain();
   }

   // close up resources and such
   ElimClose();

   // return to windows
   return msg.wParam;
}


// Main game processing //////////////////////////////////////////////////
void ElimMain(void)
{
   // if a window is present
   if(g_settings & SET_WINDOW) {
      RenderFrame();
      return;
   }

   switch(g_gameState) {
   // the intro is being shown
   case STATE_INTRO:
      XMP_Copy(ddsBack, &g_introXMP, 0, 0); // copy the image to the screen
      DD_Flip(ddsFront);                    // flip the buffers
      break;

   // the game is initilizing
   case STATE_INITGAME:
      if(g_settings & SET_FIRST_GAME) {
         // since this is the first game, get rid of
         // the intro screen and start the music
         if(g_settings & SET_MUSIC_ON) Music_PlayNext();
         XMP_Delete(&g_introXMP);
         DD_Effect_Fade();
      }
      else CloseGame();

      // set the game state to playing
      g_gameState = STATE_PLAYING;

      // see if the player want's to change anything
      if(!Dlg_GetNames()) {
         g_gameState = STATE_SHUTDOWN; // the user exited
         break;
      }

      // make sure we are not loading a game
      if(!(g_settings & SET_LOADING)) {
         Dlg_RollTurn(); // roll the dice to see who goes first
         NewGame();      // start the game
      }
      break;
      
   // the game is playing
   case STATE_PLAYING:
      RenderFrame();
      break;
   
   // the game is closing
   case STATE_SHUTDOWN:
      IO_Settings(WRITE);    // write the settings a file
      DestroyWindow(g_hWnd); // close the main window
      break;
   }
   return;
}


// Process the menu commands /////////////////////////////////////////////
void ElimMenu(UINT item, BOOL down)
{
   switch(item)
   {
   case 0: g_gameState = STATE_INITGAME;    break; // new game
   case 3: g_gameState = STATE_SHUTDOWN;    break; // exit
   case 4: ShowWindow(g_hWnd, SW_MINIMIZE); break; // minimize
   case 6: g_settings ^= SET_SOUND_ON;      break; // toggle sound
   case 8: g_settings ^= SET_SHOW_MOVES;    break; // toggle show moves
 
   // load a game
   case 1:
      if(g_settings & SET_REGISTERED) Dlg_LoadGame();
      else Dlg_MsgBox(TXT_GENERIC_UNREG, TXT_REGTITLE, TXT_OK, NULL);
      break;
      
   // save a game
   case 2:
      if(g_settings & SET_REGISTERED) Dlg_SaveGame();
      else Dlg_MsgBox(TXT_GENERIC_UNREG, TXT_REGTITLE, TXT_OK, NULL);
      break;
      
   // show the help file
   case 5:
      Dlg_Help((g_settings & SET_REGISTERED)? 1:0);
      break;

   // toggle music
   case 7:
      g_settings ^= SET_MUSIC_ON;
      if(g_settings & SET_MUSIC_ON)
      {
         Music_Init();
         Music_PlayNext();
      }
      else Music_Close();
      break;
   }
}


// Load game stuff ///////////////////////////////////////////////////////
void ElimInit(void)
{
   ShowCursor(FALSE);                      // hide the cursor while loading
   srand(GetTickCount());                  // seed the random generator
   CreateDirectory(TXT_DATA_FOLDER, NULL); // create the folder for data files
   
   // initilize DirectDraw
   if(!InitDirectDraw())
   {
      // DirectDraw could not be fully initilized, report the error and quit
      ShowCursor(TRUE);
      MessageBox(g_hWnd, TXT_DDINIT_FAILED, TXT_ERROR, MB_OK);
      DestroyWindow(g_hWnd);
      return;
   }
  
#ifdef ELIM_DEBUG
   IO_DebugOpen();
#endif

   //////////////////////////////////////////////////////////////////
   // Palette Loading
   
   // load the palettes
   XAL_Load(XAL_INTRO, g_palIntro);
   XAL_Load(XAL_ITEMS, g_palMain);

   // create the main palette
   if(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256,
      g_palIntro, &ddpMain, NULL) != DD_OK)
   {
      // an error occured
      ShowCursor(TRUE);
      MessageBox(g_hWnd, TXT_DDINIT_FAILED, TXT_ERROR, MB_OK);
      DestroyWindow(g_hWnd);
      return;
   }

   // set the palette of the primary display surface
   ddsFront->SetPalette(ddpMain);
   
   // draw the loading message
   GDI_DrawText(ddsFront, RGB(0, 255, 255), &R_SCREEN, TXT_LOADING, TXT_FMT_CC);

   // set the teams to their default state
   g_teams.nTeamA = 0;
   g_teams.nTeamB = 0;
   g_teams.teamA[0] = D_INVALID;
   g_teams.teamA[1] = D_INVALID;
   g_teams.teamB[0] = D_INVALID;
   g_teams.teamB[1] = D_INVALID;

   // load the settings
   IO_Settings(READ);

   // initilize DirectDraw surfaces with bitmaps
   DD_CreateSurface(&ddsBoard  , XMP_BOARD);
   DD_CreateSurface(&ddsDice   , XMP_DICE);
   DD_CreateSurface(&ddsExplode, XMP_EXPLODE);
   DD_CreateSurface(&ddsItems  , XMP_ITEMS);
   DD_CreateSurface(&ddsMenu   , XMP_MENU);
   DD_CreateSurface(&ddsSelect , XMP_SELECT);


   // initilize blank DirectDraw surfaces
   DD_CreateSurface(&ddsWindow , 482, 310, CLR_TRANSPARENT);
   DD_CreateSurface(&ddsMenubar, 640,  27, CLR_TRANSPARENT);
   DD_CreateSurface(&ddsTeams  , 128,  29, CLR_TRANSPARENT);
   DD_CreateSurface(&ddsRegTime, 139,  29, CLR_TRANSPARENT);
   DD_CreateSurface(&ddsPInfo  , 274, 168, CLR_BACKGROUND);

   // Initilize the menubar
   Menu_Init();


   //////////////////////////////////////////////////////////////////
   // Direct Sound Stuff
   
   // Initilize DirectSound
   InitDirectSound();
   
   // initilize DirectSound buffers
   DS_CreateBuffer(&dsoMenu     , WAV_MENU);
   DS_CreateBuffer(&dsoMenuB    , WAV_MENUB);
   DS_CreateBuffer(&dsoQuestion , WAV_QUESTION);
   DS_CreateBuffer(&dsoUnallowed, WAV_UNALLOWED);
   DS_CreateBuffer(&dsoSink     , WAV_SINK);
   DS_CreateBuffer(&dsoMarbleUp , WAV_RISE);
   DS_CreateBuffer(&dsoSelect   , WAV_SELECT);
   DS_CreateBuffer(&dsoExplode  , WAV_EXPLODE);
   DS_CreateBuffer(&dsoFree     , WAV_FREE);
   DS_CreateBuffer(&dsoError    , WAV_ERROR);

   //////////////////////////////////////////////////////////////////
   // Setup Misc. Objects
   Teams_Init();   // initilize the teams box
   RegTime_Init(); // initilize the registration time
   Dice_InitB();   // initilize the main dice
   Music_Init();   // Load the first song

   // close the first song if the music is off
   if(!(g_settings & SET_MUSIC_ON)) Music_Close();
   
   // load the game intro
   XMP_Load(&g_introXMP, XMP_INTRO);

}


// Release the game stuff ////////////////////////////////////////////////
void ElimClose(void)
{
#ifdef ELIM_DEBUG
   IO_DebugClose();
#endif

   // stop the music
   Music_Close();
  
   // delete the gdi fonts
   DeleteObject(g_fontBig);
   DeleteObject(g_fontSmall);
   DeleteObject(g_fontHelp);

   // Close DirectDraw
   if(lpdd != NULL)
   {
      // release all of the surfaces
      if(ddsFront   != NULL) {ddsFront->Release();   ddsFront   = NULL;}
      if(ddsBoard   != NULL) {ddsBoard->Release();   ddsBoard   = NULL;}
      if(ddsDice    != NULL) {ddsDice->Release();    ddsDice    = NULL;}
      if(ddsExplode != NULL) {ddsExplode->Release(); ddsExplode = NULL;}
      if(ddsItems   != NULL) {ddsItems->Release();   ddsItems   = NULL;}
      if(ddsMenu    != NULL) {ddsMenu->Release();    ddsMenu    = NULL;}
      if(ddsSelect  != NULL) {ddsSelect->Release();  ddsSelect  = NULL;}

      if(ddsWindow  != NULL) {ddsWindow->Release();  ddsWindow  = NULL;}
      if(ddsMenubar != NULL) {ddsMenubar->Release(); ddsMenubar = NULL;}
      if(ddsTeams   != NULL) {ddsTeams->Release();   ddsTeams   = NULL;}
      if(ddsRegTime != NULL) {ddsRegTime->Release(); ddsRegTime = NULL;}

      // release the DirectDraw object
      lpdd->Release();
      lpdd = NULL;
   }

   // Close DirectSound
   if(lpds != NULL)
   {
      // release all of the buffers
      if(dsoMenu      != NULL) {dsoMenu->Release();      dsoMenu      = NULL;}
      if(dsoMenuB     != NULL) {dsoMenuB->Release();     dsoMenuB     = NULL;}
      if(dsoQuestion  != NULL) {dsoQuestion->Release();  dsoQuestion  = NULL;}
      if(dsoUnallowed != NULL) {dsoUnallowed->Release(); dsoUnallowed = NULL;}
      if(dsoSink      != NULL) {dsoSink->Release();      dsoSink      = NULL;}
      if(dsoMarbleUp  != NULL) {dsoMarbleUp->Release();  dsoMarbleUp  = NULL;}
      if(dsoSelect    != NULL) {dsoSelect->Release();    dsoSelect    = NULL;}
      if(dsoExplode   != NULL) {dsoExplode->Release();   dsoExplode   = NULL;}
      if(dsoFree      != NULL) {dsoFree->Release();      dsoFree      = NULL;}
      if(dsoError     != NULL) {dsoError->Release();     dsoError     = NULL;}

      // release the DirectSound objecect
      lpds->Release();
      lpds = NULL;
   }

   // show the cursor
   ShowCursor(TRUE);
   return;
}


/*-----------------------------------------------------------------------------*/
// Main Frame Rendering function
/*-----------------------------------------------------------------------------*/

// Render the current frame //////////////////////////////////////////////
void RenderFrame(void)
{
   RECT r = {46, 89, 57, 108}; // cursor rect
   DDBLTFX bltFx;

   // setup bltFx
   ZeroMemory(&bltFx, sizeof(DDBLTFX));
   bltFx.dwSize      = sizeof(DDBLTFX);
   bltFx.dwFillColor = CLR_BACKGROUND;

   // Clear the back buffer
   DD_Blt(ddsBack, NULL, NULL, NULL, DDBLT_COLORFILL, &bltFx);

   // render objects to the screen
   Box_Render(&g_menubar);  // draw the menubar
   PInfo_Render();          // draw the player info boxes
   Dice_UpdateB();          // draw the main dice
   Board_Render();          // draw the board
   Box_Render(&g_teamsBox); // draw the teams box

   // draw the registration time
   if(!(g_settings & SET_REGISTERED)) RegTime_Update();

   // draw the window if there is one
   if(g_settings & SET_WINDOW) Win_Render();

   // draw the 'roll for turn' dice if needed
   if(g_settings & SET_ROLL_TURN) Dice_UpdateA();
   

   // dispaly vram and fps info
#ifdef ELIM_DEBUG 
   HDC hdc;
   static DWORD lastFrame = GetTickCount();
   static DWORD frames = 0;
   static DWORD fps = 0;

   if(GetTickCount() - lastFrame >= 1000)
   {
      fps = frames;
      frames = 0;
      lastFrame = GetTickCount();
   }

   if(show_debug_data)
   {
      // get v-ram data
      DDCAPS ddcaps;
      ZeroMemory(&ddcaps, sizeof(ddcaps));
      ddcaps.dwSize = sizeof(ddcaps);
      lpdd->GetCaps(&ddcaps, NULL);

      // get memory data
      MEMORYSTATUS memstat;
      memstat.dwLength = sizeof(MEMORYSTATUS);
      GlobalMemoryStatus(&memstat);

      DD_GetDC(ddsBack, &hdc);

      char szInfo[512];
     /* wsprintf(szInfo, "FPS: %d;\nVM - T: %d, F: %d, U: %d;"
         "\nMem - Load: %d, T: %d, F: %d;\nVMEM - T: %d, F: %d",
         fps,
         ddcaps.dwVidMemTotal,
         ddcaps.dwVidMemFree,
         (ddcaps.dwVidMemTotal - ddcaps.dwVidMemFree),
         memstat.dwMemoryLoad,
         memstat.dwTotalPhys,
         memstat.dwAvailPhys,
         memstat.dwTotalVirtual,
         memstat.dwAvailVirtual);*/
      wsprintf(szInfo, "FPS: %d%s", fps, error_detect? "; possible error detected :(":" ");

      SelectObject(hdc, g_fontSmall);
      DrawText(hdc, szInfo, strlen(szInfo), &R_SCREEN, DT_LEFT);

      DD_ReleaseDC(ddsBack, hdc);   
   }
   
   frames++;
#endif // end display vran and fps info

   // draw the cursor
   DD_BltClip(ddsBack, ddsItems, g_curx, g_cury, &R_SCREEN, r, DDBLTFAST_SRCCOLORKEY);

   // flip the buffer
   DD_Flip(ddsFront);
}


/*-----------------------------------------------------------------------------*/
// Misc. Game Functions
/*-----------------------------------------------------------------------------*/

// Start a game //////////////////////////////////////////////////////////
void NewGame(void)
{
   // initilize the game
   Board_NewGame();

   // initilize the pInfo boxes
   PInfo_Init();

   // re-draw the teams box
   Teams_Init();

   // start moving the boxes
   Box_Move(&g_menubar, TRUE);
   Box_Move(&g_regtime, TRUE);
   Box_Move(&g_teamsBox, TRUE);

   g_settings &= ~SET_FIRST_GAME; // remove the first game flag
   g_settings &= ~SET_GAME_OVER;  // remove the game over flag
   g_gameState = STATE_PLAYING;   // set the game state to playing

   // set the starting time
   g_startTime = GetTickCount();
 
   // set the check time flag
   g_settings |= SET_CHECK_TIME;
}


// setup a loaded game ///////////////////////////////////////////////////
void SetupLoadedGame(void)
{
   char szText[256]; // text used to say who is out

   g_cColor = PlayerToColor(g_cPlayer); // set the current color
   Teams_Init();                        // re-draw the teams box
   Dice_Hide();                         // hide the main dice

   // set the dice source rect
   for(int i = 0; i < 2; i++)
   {
      if(g_diceB[i].value == D_INVALID) Rect_SetDimen(&g_diceB[i].rectSource,
         g_cColor * 20, 120, 21, 21);

      else Rect_SetDimen(&g_diceB[i].rectSource, g_cColor * 20,
         g_diceB[i].value * 20, 21, 21);
   }

   g_settings &= ~SET_FIRST_GAME; // remove first game flag
   g_settings &= ~SET_GAME_OVER;  // remove game over flag
   g_gameState = STATE_PLAYING;   // set the game state to playing

   // tell user if any players have been taken out
   for(i = 0; i < 4; i++)
   {
      // make sure the user is was active and out
      if((g_pFlags[i] & PLAYER_ACTIVE) && (g_pFlags[i] & PLAYER_OUT))
      {
         if(g_pFlags[i] & PLAYER_IS_PC)
            wsprintf(szText, TXT_PC_PLAYER_OUT, g_name[i], g_score[i]);
         else wsprintf(szText, TXT_PLAYER_OUT, g_name[i], g_score[i]);

         // show the message box
         Dlg_MsgBox(szText, TXT_GENTITLE, "OK", NULL);
      }
   }

   // initilize the pInfo boxes
   PInfo_Init();
   
   // move the boxes
   Box_Move(&g_menubar, TRUE);
   Box_Move(&g_regtime, TRUE);
   Box_Move(&g_teamsBox, g_teams.nTeamA + g_teams.nTeamB > 0);

   // show the dice if a bullet or marble is in the air
   if(g_settings & SET_MARBLE_JUMPING) Dice_ShowB();

   // set the check time flag
   g_settings |= SET_CHECK_TIME;
}


// close the objects when a game is over or we are starting a new one ////
void CloseGame(void)
{
   // hide the boxes
   Box_Move(&g_menubar, FALSE);
   Box_Move(&g_regtime, FALSE);
   Box_Move(&g_teamsBox, FALSE);

   // hide the player info boxes
   PInfo_Close();

   // hide the dice
   Dice_Hide();

   // hide the marbles
   Board_CloseGame();
}


// change to the next player //////////////////////////////////////////////
void NextPlayer(void)
{
   // unselect the previous players box
   PInfo_UnSelect(g_cPlayer);

   do
   {
      // move to the next player
      if(++g_cPlayer >= g_nPlayers) g_cPlayer = 0;

      // find the next color
      // WARNING: this can potentually cause an infinate loop if somthing goes wrong
      do {if(++g_cColor > 3) g_cColor = 0;}
      while(!(g_pFlags[g_cColor] & PLAYER_ACTIVE));
   }
   while(g_pFlags[g_cColor] & PLAYER_OUT);

   // select the new player
   PInfo_Select(g_cPlayer);

   // roll the dice
   Dice_Roll();
}


// convert a color to a player ////////////////////////////////////////////
int ColorToPlayer(int color)
{
   int count = 0;

   // add to count if the player is active
   for(int i = 0; i < color; i++) if(g_pFlags[i] & PLAYER_ACTIVE) count++;

   // return the count
   return count;
}


// convert a player to a color ////////////////////////////////////////////
int PlayerToColor(int player)
{
   int count = 0;

   // add to count if the player is active
   for(int i = 0; i < 4; i++)
   {
      if(g_pFlags[i] & PLAYER_ACTIVE)
      {
         if(count >= player) return i;
         count++;
      }
   }

   // we should not get here
   return 0;
}


/*-----------------------------------------------------------------------------*/
// Generic Box Functions
/*-----------------------------------------------------------------------------*/

// start a 'moving box' on its way ///////////////////////////////////////
void Box_Move(BOX* box, BOOL show)
{
   // make sure the box is valid
   if(box->status == BOX_INACTIVE) return;

   // get the box moving
   box->lastTick = GetTickCount();
   box->status   = BOX_MOVING;

   // if (the box moves up to hide) or (the box moves up to show)
   if((!show && box->min < 0) || (show && box->max > 479))
   {
      // move box up

      // velocity should be negitive
      if(box->velocity > 0) box->velocity *= -1;

      // set the moving_min flag
      box->flags |= BOX_MOVING_MIN; 
   }
   else
   {
      // move box down

      // velocity should be positive
      if(box->velocity < 0) box->velocity *= -1;

      // remove the moving_min flag
      box->flags &= ~BOX_MOVING_MIN;
   }
}


// update a 'moving box' /////////////////////////////////////////////////
void Box_Render(BOX* box)
{
   // get the current tick
   DWORD thisTick = GetTickCount();

   // if the box is not active, exit
   if(box->status == BOX_INACTIVE) return;

   // make sure the box is moving
   if(box->status == BOX_MOVING)
   {
      // add the velocity * amout of ticks passed to the position
      box->variable += (long)(box->velocity * (double)(thisTick - box->lastTick));
      
      // set the last frame
      box->lastTick = thisTick;

      // should we test for max or min pos?
      if(box->flags & BOX_MOVING_MIN)
      {
         // is the box at or past its min point?
         if(box->variable <= box->min)
         {
            box->variable = box->min;
            box->status   = BOX_FINISHED_MOVING;
         }
      }
      else
      {
         // is the box past or at its max point?
         if(box->variable >= box->max)
         {
            box->variable = box->max;
            box->status   = BOX_FINISHED_MOVING;
         }
      }
   }

   if(box->flags & BOX_VERTICAL)
   {
      DD_BltClip(ddsBack, box->dds, box->constant, box->variable, &box->rectClip,
         box->rectSource, box->bltFlags);
   }
   else
   {
      DD_BltClip(ddsBack, box->dds, box->variable, box->constant, &box->rectClip,
         box->rectSource, box->bltFlags);
   }

}


/*-----------------------------------------------------------------------------*/
// Roll for turn functions
/*-----------------------------------------------------------------------------*/

// Setup the dice to see who goes first //////////////////////////////////
void Dice_SetUpA(void)
{
   int index = 0;
   int colors[4];

   // clipping rectangle for the dice
   RECT r = {200, 140, 235, 285};

   // make a list of the active colors
   for(int i = 0; i < 4; i++) if(g_pFlags[i] & PLAYER_ACTIVE) colors[index++] = i;

   // setup the dice
   for(i = 0; i < g_nPlayers; i++)
   {
      g_diceA[i].variable = 179;          // x chord
      g_diceA[i].constant = 32 * i + 167; // y chord
      g_diceA[i].velocity = VEL_DIEA;     // velocity
      g_diceA[i].max      = 209;          // max x
      g_diceA[i].min      = 179;          // min x

      g_diceA[i].dds      = ddsDice;
      g_diceA[i].bltFlags = DDBLTFAST_SRCCOLORKEY;

      g_diceA[i].owner    = colors[i];
      g_diceA[i].position = i;
      g_diceA[i].rectClip = r;
      g_diceA[i].status   = BOX_IDLE;
      g_diceA[i].value    = RAND(6);

      g_diceA[i].flags = 0; // clear the flags

      // set the source rectangle
      Rect_SetDimen(&g_diceA[i].rectSource, g_diceA[i].owner * 20,
         g_diceA[i].value * 20, 21, 21);
   }

   // start the first die moving
   g_diceA[0].lastTick = GetTickCount();
   g_diceA[0].status   = BOX_MOVING;

   // set the last die's die_last flag
   g_diceA[g_nPlayers - 1].flags |= DIE_LAST;

   // set the roll_turn flag in the main settings
   g_settings |= SET_ROLL_TURN;
}


// Roll the die to see who is the first player ///////////////////////////
void Dice_UpdateA(void)
{
   for(int i = 0; i < g_nPlayers; i++)
   {
      // render the die to the screen
      Box_Render(&g_diceA[i]);

      // the box is still moving, go on to the next box
      if(g_diceA[i].status != BOX_FINISHED_MOVING) continue;

      // set the status to idle so we dont end up here agan
      g_diceA[i].status = BOX_IDLE;

      // see if we were re-rolling
      if(g_diceA[i].flags & DIE_REROLLING)
      {
         // if we are the last die to hide, re-set up the dice and move them out
         if(g_diceA[i].flags & DIE_LAST) Dice_ReSetA();
      }
      else
      {
         if(g_diceA[i].flags & DIE_LAST)
         {
            // validate the roll
            Dice_Validate();
         }
         else
         {
            // find the id of the next die
            int nextDie = g_diceA[i].position;

            // make sure that the die should be rolled
            while(g_diceA[++nextDie].flags & DIE_DONT_ROLL) ;

            // move the next die
            g_diceA[nextDie].lastTick = GetTickCount();
            g_diceA[nextDie].velocity = VEL_DIEA;
            g_diceA[nextDie].status   = BOX_MOVING;
         }
      }
   }
}


// See if the dice are valad /////////////////////////////////////////////
void Dice_Validate(void)
{
   int high = 0;
   int num  = 0;
   int list[4];

   // find the highest roll, make sure the die is being rolled
   for(int i = 0; i < g_nPlayers; i++)
      if(g_diceA[i].value > high && !(g_diceA[i].flags & DIE_DONT_ROLL))
         high = g_diceA[i].value;

   // find the number of dice that have the highest roll, and record there id
   for(i = 0; i < g_nPlayers; i++)
      if(g_diceA[i].value == high && !(g_diceA[i].flags & DIE_DONT_ROLL))
         list[num++] = i;

   // if it is greater then one, there was doubles
   if(num > 1)
   {
      // ajust the message to say we are re-rolling
      DlgMsgExRT_ChangeText(TXT_DICE_TIE);
      

      // set the dont roll flag for all of the dice
      for(i = 0; i < g_nPlayers; i++) g_diceA[i].flags |= DIE_DONT_ROLL;

      // re-setup the dice that tied
      for(i = 0; i < num; i++)
      {
         g_diceA[list[i]].lastTick = GetTickCount(); // set the last tick
         g_diceA[list[i]].velocity = -VEL_DIEA;    // set the velosity
         
         g_diceA[list[i]].flags |= BOX_MOVING_MIN; // set the moving min flag
         g_diceA[list[i]].flags |= DIE_REROLLING;  // set the re-rolling flag
         g_diceA[list[i]].flags &= ~DIE_LAST;      // remove the last flag
         g_diceA[list[i]].flags &= ~DIE_DONT_ROLL; // remove the dont roll flag

         g_diceA[list[i]].status = BOX_MOVING;     // set the status to moving
      }

      // set the last double's die_last flag
      g_diceA[list[num - 1]].flags |= DIE_LAST;

      // 'disable' the other players names who still have a dont_roll flag
      for(i = 0; i < g_nPlayers; i++)
         if(g_diceA[i].flags & DIE_DONT_ROLL)
            DlgMsgExRT_DisablePlayer(i, g_diceA[i].owner);
   }
   else
   {
      char szText[128];
      wsprintf(szText, TXT_DICE_WINNER, g_name[g_diceA[list[0]].owner]);
      DlgMsgExRT_ChangeText(szText);
      DlgMsgExRT_Done();

      // set the player to who ever one
      g_cColor  = g_diceA[list[0]].owner;
      g_cPlayer = list[0];
   }
}


// Re-Setup the dice, and move them back out /////////////////////////////
void Dice_ReSetA(void)
{
   int list[4];
   int num = 0;

   // make a list of players to re-roll
   for(int i = 0; i < g_nPlayers; i++)
   {
      if(g_diceA[i].flags & DIE_REROLLING)
      {
         list[num++] = i;
      
         // remove the moving_min flag
         g_diceA[i].flags &= ~BOX_MOVING_MIN;

         // remove the re-rolling flag
         g_diceA[i].flags &= ~DIE_REROLLING;

         // give the die a new value
         g_diceA[i].value = RAND(6);

         // ajust the source rectangle
         Rect_SetDimen(&g_diceA[i].rectSource, g_diceA[i].owner * 20,
            g_diceA[i].value * 20, 21, 21);
      }
   }

   // start moving the first box
   g_diceA[list[0]].lastTick = GetTickCount();
   g_diceA[list[0]].velocity = VEL_DIEA;
   g_diceA[list[0]].status   = BOX_MOVING;
}


/*-----------------------------------------------------------------------------*/
// Main dice functions
/*-----------------------------------------------------------------------------*/

// Initilize the dice ////////////////////////////////////////////////////
void Dice_InitB(void)
{
   // the clipping rectangle for the dice
   RECT r = {0, 0, 640, 452};

   // setup the dice
   for(int i = 0; i < 2; i++)
   {
      g_diceB[i].constant = 25 * i + 297; // x chord
      g_diceB[i].variable = 453;          // y chord
      g_diceB[i].max      = 453;          // max x
      g_diceB[i].min      = 388;          // min x

      g_diceB[i].dds      = ddsDice;
      g_diceB[i].bltFlags = DDBLTFAST_SRCCOLORKEY;

      g_diceB[i].rectClip = r;
      g_diceB[i].status   = BOX_IDLE;

      g_diceB[i].flags = 0;               // clear the flags
      g_diceB[i].flags |= BOX_VERTICAL;   // set the vertical flag
   }
}


// Show the dice for the first turn //////////////////////////////////////
void Dice_Show(void)
{  
   for(int i = 0; i < 2; i++)
   {
      // set the moving_min flag
      g_diceB[i].flags |= BOX_MOVING_MIN;

      // set the location of the die to be safe
      g_diceB[i].variable = 453;

      // set the owner of the dice
      g_diceB[i].owner = g_cColor;

      // see if the game is being loaded, if so, don't change the dice value
      if(!(g_settings & SET_LOADING))
      {
         // set the value of the dice
         g_diceB[i].value = RAND(6);

         // set the source rectangle of the dice
         Rect_SetDimen(&g_diceB[i].rectSource, g_diceB[i].owner * 20, 
            g_diceB[i].value * 20, 21, 21);
      }
   }

   // start the first die moving
   g_diceB[0].lastTick = GetTickCount();
   g_diceB[0].status   = BOX_MOVING;
   g_diceB[0].velocity = -VEL_DIEB;

   // set the roll_dice flag in the main settings
   g_settings |= SET_ROLL_DICE;
}

// just show the regular dice without moving them ////////////////////////
void Dice_ShowB(void)
{
   for(int i = 0; i < 2; i++)
   {
      g_diceB[i].variable = 388;      // set the location of the die
      g_diceB[i].owner    = g_cColor; // set the owner of the dice
      g_diceB[i].status   = BOX_IDLE; // set the status of the die
   }
}


// Roll the dice /////////////////////////////////////////////////////////
void Dice_Roll(void)
{
   // remove the moving_min flag
   g_diceB[0].flags &= ~BOX_MOVING_MIN;
   g_diceB[1].flags &= ~BOX_MOVING_MIN;

   // start the first die moving
   g_diceB[0].lastTick = GetTickCount();
   g_diceB[0].status   = BOX_MOVING;
   g_diceB[0].velocity = VEL_DIEB;
   
   // set the dice rolling flag
   g_settings |= SET_ROLL_DICE;
}


// De-activate the dice //////////////////////////////////////////////////
void Dice_Hide(void)
{
   g_diceB[0].status = BOX_INACTIVE;
   g_diceB[1].status = BOX_INACTIVE;
}


// update the main dice //////////////////////////////////////////////////
void Dice_UpdateB(void)
{
   char szText[128];

   for(int i = 0; i < 2; i++)
   {
      // render the die
      Box_Render(&g_diceB[i]);

      // see if the die is done moving, if not, move on to the next die
      if(g_diceB[i].status != BOX_FINISHED_MOVING) continue;

      // set the status to idle
      g_diceB[i].status = BOX_IDLE;

      // see if the box is being shown
      if(g_diceB[i].flags & BOX_MOVING_MIN)
      {
         // see which die this is
         if(i == 0)
         {
            // start the other die moving out
            g_diceB[1].status   = BOX_MOVING;
            g_diceB[1].velocity = -VEL_DIEB;
            g_diceB[1].lastTick = GetTickCount();
         }
         
         // we are done rolling
         else
         {
            // remove the roll_dice flag
            g_settings &= ~SET_ROLL_DICE;

            // make sure the player can move
            if(Board_CanMove())
            {
#ifdef ELIM_DEBUG
               IO_DebugWrite("dice: execute move", 0);
#endif

               // start the computer player
               PC_ExecuteMove();
            }
            else
            {
               // tell the user there are no moves
               wsprintf(szText, TXT_NO_MOVES, g_name[g_cColor]);
               Dlg_MsgBox(szText, TXT_GENTITLE, "OK", NULL);
               NextPlayer();
            }
         }
      }
      else
      {
         // see which die it is
         if(i == 0)
         {
            // start the other die moving in
            g_diceB[1].lastTick = GetTickCount();
            g_diceB[1].status   = BOX_MOVING;
            g_diceB[1].velocity = VEL_DIEB;
         }

         // move the dice back out
         else Dice_Show();
      }
   }
}


/*-----------------------------------------------------------------------------*/
// Menubar Functions
/*-----------------------------------------------------------------------------*/

// Initilize the menubar /////////////////////////////////////////////////
void Menu_Init(void)
{
   RECT r = {0, 0, 640, 27};
   RECT ra = {2, 3, 24, 25};
   RECT rb = {2, 456, 24, 478};

   // initilize the menubar variables
   g_menubar.constant   = 0;         // x chord
   g_menubar.variable   = 480;       // y chord
   g_menubar.velocity   = VEL_MENU;  // velocity
   g_menubar.min        = 453;       // min y
   g_menubar.max        = 480;       // max y

   g_menubar.dds        = ddsMenubar;
   g_menubar.bltFlags   = DDBLTFAST_NOCOLORKEY;

   g_menubar.rectSource = r;
   g_menubar.rectClip   = R_SCREEN;

   g_menubar.status     = BOX_IDLE;
   g_menubar.flags     |= BOX_VERTICAL;   // set the vertical flag
   g_menubar.flags     |= MENUBAR_NOTEXT; // set the notext flag
   
   // initilize the menu items
   for(int i = 0; i < D_NUM_MENUS; i++)
   {
      g_menu[i].location    = i;
      g_menu[i].picture     = i;
      g_menu[i].rectSurface = ra;
      g_menu[i].rectScreen  = rb;
 
      // clear the flags
      g_menu[i].flags = 0;

      // set the enabled flag
      g_menu[i].flags |= BUTTON_ENABLED;

      // if the button is after the seperator, set the toggle flag
      if(i > 5) g_menu[i].flags |= BUTTON_TOGGLE;

      // move to the next button and ajust if it is after the seperator
      Rect_Offset(&ra, i == 5 ? 26 : 23, 0);
      Rect_Offset(&rb, i == 5 ? 26 : 23, 0);
   }

   // push the option menuitems if they are active
   if(g_settings & SET_SOUND_ON)
   {
      g_menu[6].flags |= BUTTON_DOWN;
      g_menu[6].flags |= BUTTON_TOGGLE_DOWN;
   }
   if(g_settings & SET_MUSIC_ON)
   {
      g_menu[7].flags |= BUTTON_DOWN;
      g_menu[7].flags |= BUTTON_TOGGLE_DOWN;
   }
   if(g_settings & SET_SHOW_MOVES)
   {
      g_menu[8].flags |= BUTTON_DOWN;
      g_menu[8].flags |= BUTTON_TOGGLE_DOWN;
   }

   Menu_Draw();
}


// Disable one of the menus //////////////////////////////////////////////
void Menu_Disable(int n)
{
   // change the picture associated with the button
   g_menu[n].picture = n + 3;
   
   g_menu[n].flags &= ~BUTTON_ENABLED;     // remove the enabled flag
   g_menu[n].flags &= ~BUTTON_DOWN;        // remove the down flag
   g_menu[n].flags &= ~BUTTON_TOGGLE_DOWN; // remove the toggle down flag

   // re-draw the button
   Menu_DrawMenu(n);
}


// Draw the menubar //////////////////////////////////////////////////////
void Menu_Draw(void)
{
   RECT r = {0, 1, 640, 27};

   // fill the background
   DD_Draw_Rect(ddsMenubar, &r, CLR_WINDOWFILL, 0, 0);

   // draw the line on top
   DD_Draw_Line(ddsMenubar, CLR_BORDER, 0, 0, 640, 0);

   // draw the other line on top
   DD_Draw_Line(ddsMenubar, CLR_MENU_TOP, 0, 1, 639, 1);

   // draw the line on the right
   DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM, 639, 2, 639, 26);

   // draw the line on the bottom
   DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM, 1, 26, 640, 26);

   // draw the line on the left
   DD_Draw_Line(ddsMenubar, CLR_MENU_TOP, 0, 1, 0, 26);

   // draw the sepperator
   DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM, 140, 3, 140, 25);
   DD_Draw_Line(ddsMenubar, CLR_MENU_TOP, 141, 3, 141, 25);

   // draw the buttons
   for(int i = 0; i < D_NUM_MENUS; i++) Menu_DrawMenu(i);
}


// Draw a particular menu ////////////////////////////////////////////////
void Menu_DrawMenu(int n)
{
   // find the source rectangle for the button face
   RECT ra = {0, 19 * g_menu[n].picture, 20, 19 * g_menu[n].picture + 20};
   
   // draw the menu's back
   if(g_menu[n].flags & BUTTON_DOWN)
   {
      // fill the rectangle
      DD_Draw_Rect(ddsMenubar, &g_menu[n].rectSurface, CLR_MENU_FILLDOWN, 0, 0);

      // draw the top line
      DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.top,
         g_menu[n].rectSurface.right, g_menu[n].rectSurface.top);

      // draw the left line
      DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.top,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.bottom);

      // blt the menu face
      DD_BltFast(ddsMenubar, ddsMenu,
         g_menu[n].rectSurface.left + 2, g_menu[n].rectSurface.top + 2,
         &ra, DDBLTFAST_SRCCOLORKEY);
   }
   else
   {
      // fill the rectangle
      DD_Draw_Rect(ddsMenubar, &g_menu[n].rectSurface, CLR_WINDOWFILL, 0, 0);

      // draw the top line
      DD_Draw_Line(ddsMenubar, CLR_MENU_TOP,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.top,
         g_menu[n].rectSurface.right - 1, g_menu[n].rectSurface.top);

      // draw the right side
      DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM,
         g_menu[n].rectSurface.right - 1, g_menu[n].rectSurface.top + 1,
         g_menu[n].rectSurface.right - 1, g_menu[n].rectSurface.bottom);

      // draw the bottom side
      DD_Draw_Line(ddsMenubar, CLR_MENU_BOTTOM,
         g_menu[n].rectSurface.left + 1, g_menu[n].rectSurface.bottom - 1,
         g_menu[n].rectSurface.right, g_menu[n].rectSurface.bottom - 1);

      // draw the left line
      DD_Draw_Line(ddsMenubar, CLR_MENU_TOP,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.top,
         g_menu[n].rectSurface.left, g_menu[n].rectSurface.bottom - 1);

      // blt the menu face
      DD_BltFast(ddsMenubar, ddsMenu,
         g_menu[n].rectSurface.left + 1, g_menu[n].rectSurface.top + 1,
         &ra, DDBLTFAST_SRCCOLORKEY);
   }
}


// Process mouse down ////////////////////////////////////////////////////
void Menu_MouseDown(long x, long y)
{
   // get the menuitem the mouse is within
   g_cMenu = Menu_MouseToBtn(x, y);

   // see if the mouse hit a button
   if(g_cMenu == D_INVALID) return;

   // see if the button was enabled
   if(!(g_menu[g_cMenu].flags & BUTTON_ENABLED))
   {
      g_cMenu = D_INVALID;
      return;
   }

   g_menu[g_cMenu].flags |= BUTTON_BEING_PUSHED; // set the being pushed flag
   g_menu[g_cMenu].flags |= BUTTON_DOWN;         // set the button down flag
   
   // play the sound
   DS_Play(dsoMenu, g_menu[g_cMenu].rectScreen.left + 5);

   // re-draw the menuitem
   Menu_DrawMenu(g_cMenu);
}


// Process mouse move ////////////////////////////////////////////////////
void Menu_MouseMove(long x, long y)
{
   RECT r = {215, 5, 502, 22};
   BOOL within, down;
   int  button;

   // if a menuitem is not being pressed, change message text
   // to say what the menu the mouse is over does
   if(g_cMenu == D_INVALID)
   {
      button = Menu_MouseToBtn(x, y);
      
      // see if there is any text there to begin with
      if(!(g_menubar.flags & MENUBAR_NOTEXT))
         DD_Draw_Rect(ddsMenubar, &r, CLR_WINDOWFILL, 0, 0);
      
      if(button == D_INVALID)
      {
         // set the no text flag
         g_menubar.flags |= MENUBAR_NOTEXT;
      }
      else
      {
         // remove the no text flag
         g_menubar.flags &= ~MENUBAR_NOTEXT;
         
         // draw the text that was there
         GDI_DrawText(ddsMenubar, RGB_MENUBAR_TEXT, &r,
            TXT_MENU_DESC[button], TXT_FMT_CL);
      }
   }
   else
   {
      within = Rect_Within(&g_menu[g_cMenu].rectScreen, x, y);
      down   = FLAG_SET(g_menu[g_cMenu].flags, BUTTON_DOWN);

      // only change if it is nessary
      if((down != within) && !(g_menu[g_cMenu].flags & BUTTON_TOGGLE_DOWN))
      {
         // set/remove the down flag
         SET_FLAG(g_menu[g_cMenu].flags, BUTTON_DOWN, within);

         // play the sound
         if(within) DS_Play(dsoMenu, g_menu[g_cMenu].rectScreen.left + 5);

         // re-draw the button
         Menu_DrawMenu(g_cMenu);
      }
   }
}


// Process mouse up //////////////////////////////////////////////////////
void Menu_MouseUp(long x, long y)
{
   // make sure a menuitem is being pressed
   if(g_cMenu == D_INVALID) return;

   // see if the mouse is still within the button
   UINT within = Rect_Within(&g_menu[g_cMenu].rectScreen, x, y);

   // execute different instructions if it is a toggeling button
   if(g_menu[g_cMenu].flags & BUTTON_TOGGLE)
   {
      if(within)
      {
         // invert the toggle down flag
         g_menu[g_cMenu].flags ^= BUTTON_TOGGLE_DOWN;

         // set/remove the down flag
         SET_FLAG(g_menu[g_cMenu].flags, BUTTON_DOWN,
            (g_menu[g_cMenu].flags & BUTTON_TOGGLE_DOWN));
      }
   }
   else
   {
      g_menu[g_cMenu].flags &= ~BUTTON_DOWN;         // remove the down flag
      g_menu[g_cMenu].flags &= ~BUTTON_BEING_PUSHED; // remove the being_pushed flag
   }

   // re-draw the menuitem
   Menu_DrawMenu(g_cMenu);

   // execute the menu messge if needed
   if(within) ElimMenu(g_cMenu, (g_menu[g_cMenu].flags & BUTTON_TOGGLE_DOWN));
   
   // clear the 'selected' menuitem
   g_cMenu = D_INVALID;
}


// return which button the mouse is over /////////////////////////////////
int Menu_MouseToBtn(long x, long y)
{
   // look through the rectangles of all of the menu buttons
   for(int i = 0; i < D_NUM_MENUS; i++) if(Rect_Within(&g_menu[i].rectScreen, x, y)) return i;
   
   // return the mouse was not within any button
   return D_INVALID;
}


/*-----------------------------------------------------------------------------*/
// Teams functions
/*-----------------------------------------------------------------------------*/

// Initilize the teams box ///////////////////////////////////////////////
void Teams_Init(void)
{

   RECT r = {0, 0, 129, 9};

   // initilize the teams variables
   g_teamsBox.constant   = 3;          // x chord
   g_teamsBox.variable   = -29;        // y chord
   g_teamsBox.velocity   = -VEL_TEAMS; // velocity
   g_teamsBox.min        = -29;        // min y
   g_teamsBox.max        = 3;          // max y

   g_teamsBox.dds        = ddsTeams;
   g_teamsBox.bltFlags   = DDBLTFAST_NOCOLORKEY;

   g_teamsBox.rectSource = r;
   g_teamsBox.rectClip   = R_SCREEN;

   g_teamsBox.status     = BOX_INACTIVE;

   g_teamsBox.flags      = 0;            // clear the flags
   g_teamsBox.flags     |= BOX_VERTICAL; // set the vertical flag
   
   // set this flag because it gets inverted before the object first moves
   g_teamsBox.flags     |= BOX_MOVING_MIN;
   
   // if there are no teams, deactivate the box
   if(g_teams.nTeamA + g_teams.nTeamB == 0) g_teamsBox.status = BOX_INACTIVE;

   // otherwise get the box moving
   else g_teamsBox.status = BOX_MOVING;

   // draw the box
   Teams_Draw();
}


// Draw the teams box ////////////////////////////////////////////////////
void Teams_Draw(void)
{
   // make sure the box is active
   if(g_teamsBox.status == BOX_INACTIVE) return;

   // source rectangel of the box
   RECT ra = {0, 0, 24 * (g_teams.nTeamA + g_teams.nTeamB) + 32, 29};
   RECT rb;

   // set the source rect of g_teamsBox
   g_teamsBox.rectSource = ra;

   // fill in the background
   DD_Draw_Rect(ddsTeams, &ra, CLR_WHITE, CLR_BORDER, 1);

   // draw the 'vs' text
   Rect_SetPoint(&rb, 46, 79, 62, 89);
   DD_BltFast(ddsTeams, ddsItems, 24 * g_teams.nTeamA + 8, 9, &rb, DDBLTFAST_NOCOLORKEY);

   // draw teamA's marbles
   for(int i = 0; i < g_teams.nTeamA; i++)
   {
      Rect_SetDimen(&rb, 25, 20 * g_teams.teamA[i] + 64, 21, 21);
      DD_BltFast(ddsTeams, ddsItems, 24 * i + 4, 4, &rb, DDBLTFAST_SRCCOLORKEY);
   }

   // draw teamB's marbles
   for(i = 0; i < g_teams.nTeamB; i++)
   {
      Rect_SetDimen(&rb, 25, 20 * g_teams.teamB[i] + 64, 21, 21);
      DD_BltFast(ddsTeams, ddsItems, 24 * (i + g_teams.nTeamA) + 31, 4, &rb, DDBLTFAST_SRCCOLORKEY);
   }
}


/*-----------------------------------------------------------------------------*/
// Registration time functions
/*-----------------------------------------------------------------------------*/

// Initilize the registration time box ///////////////////////////////////
void RegTime_Init(void)
{
   RECT r = {0, 0, 135, 29};

   // initilize the teams variables
   g_regtime.constant   = 502;          // x chord
   g_regtime.variable   = -29;          // y chord
   g_regtime.velocity   = -VEL_REGTIME; // velocity
   g_regtime.min        = -29;          // min y
   g_regtime.max        = 3;            // max y

   g_regtime.dds        = ddsRegTime;
   g_regtime.bltFlags   = DDBLTFAST_NOCOLORKEY;

   g_regtime.rectSource = r;
   g_regtime.rectClip   = R_SCREEN;

   g_regtime.flags      = 0;            // clear the flags
   g_regtime.flags |= BOX_VERTICAL;     // set the vertical flag
   g_regtime.flags |= BOX_MOVING_MIN;   // set this flag 'cause it will get inverted
   g_regtime.status = (g_settings & SET_REGISTERED)? BOX_INACTIVE : BOX_IDLE;

   // draw the registration time box
   RegTime_Draw();
}


// Update the registration time box //////////////////////////////////////
void RegTime_Update(void)
{
   // render the box
   Box_Render(&g_regtime);

   // the last time we re-drew the box
   static DWORD lastTick = GetTickCount() - 2000;

   // the current tick
   DWORD thisTick = GetTickCount();

   // see if the time should be changed
   if(thisTick - lastTick >= 1000)
   {
      lastTick = GetTickCount();
      RegTime_Draw();
   }
}


// draw the registration time box ////////////////////////////////////////
void RegTime_Draw(void)
{
   long  ticLeft;
   DWORD secLeft;
   DWORD minLeft;
   char  szSeconds[4];
   char  szFinal[64];

   // source rectangle
   RECT r = {0, 0, 135, 29};

   // fill in the rectangle
   DD_Draw_Rect(ddsRegTime, &r, CLR_WHITE, CLR_BORDER, 1);

   // see if we should check the time
   if(g_settings & SET_CHECK_TIME)
   {
      // create the string
      ticLeft = D_UNREG_TIME_ALLOWED - (GetTickCount() - g_startTime);
      secLeft = (DWORD)((ticLeft / 1000) % 60);
      minLeft = (DWORD)(ticLeft / 60000);

      if(secLeft < 10) wsprintf(szSeconds, "0%d", secLeft);
      else wsprintf(szSeconds, "%d", secLeft);

      wsprintf(szFinal, TXT_UNREG_TIME, minLeft, szSeconds);
   }
   else
   {
      // create the string
      wsprintf(szFinal, TXT_UNREG_TIME_PAUSE);
   }

   // draw the text
   GDI_DrawText(ddsRegTime, RGB_BLACK, &r, szFinal, TXT_FMT_CC);

   // see if there time is up
   if(ticLeft <= 0 && (g_settings & SET_CHECK_TIME))
   {
      g_settings |= SET_GAME_OVER;   // set the game over flag
      g_settings &= ~SET_CHECK_TIME; // remove the check time flag

      // show the message box
      Dlg_MsgBox(TXT_TIME_UP, TXT_REGTITLE, TXT_OK, NULL);
      g_gameState = STATE_INITGAME;
   }
}


/*-----------------------------------------------------------------------------*/
// Player Info boxes
/*-----------------------------------------------------------------------------*/

// Select a player info box //////////////////////////////////////////////
void PInfo_Select(int n)
{
   RECT r;

   // the source rectangle of the box
   Rect_SetDimen(&r, 0, 42 * n, 274, 40);

   // draw the border
   DD_Draw_Rect(ddsPInfo, &r, CLR_TRANSPARENT, CLR_PINFO, 2);

   // set the flashing flag
   g_pInfo[n].box.flags |= PINFO_FLASHING;

   // set the counters
   g_pInfo[n].cFFrameA = 200;
   g_pInfo[n].cFFrameB = 0;
}


// Unselect a player info box ////////////////////////////////////////////
void PInfo_UnSelect(int n)
{
   RECT r;

   // the source rectangle of the box
   Rect_SetDimen(&r, 0, 42 * n, 274, 40);

   // draw the background line
   DD_Draw_Rect(ddsPInfo, &r, CLR_TRANSPARENT, CLR_BACKGROUND, 1);

   // draw the border line
   Rect_Expand(&r, -1);
   DD_Draw_Rect(ddsPInfo, &r, CLR_TRANSPARENT, CLR_BORDER, 1);

}


// Update a player info score ////////////////////////////////////////////
void PInfo_UpdateScore(int n, int incr)
{
   RECT r;
   char szText[8];

   // rectangle of the text
   Rect_SetDimen(&r, 215, 42 * n + 19, 52, 18);

   // change the players score
   g_score[g_pInfo[n].box.owner] += incr;

   // clear the background
   DD_Draw_Rect(ddsPInfo, &r, CLR_WHITE, 0, 0);

   // draw the text
   wsprintf(szText, "%d", g_score[g_pInfo[n].box.owner]);
   GDI_DrawText(ddsPInfo, RGB_BLACK, &r, szText, TXT_FMT_CR);
}


// Update a player info graph ////////////////////////////////////////////
void PInfo_UpdateGraph(int n, int incr)
{
   // change the number of marbles the player has
   g_marbles[g_pInfo[n].box.owner] += incr;

   // set the scrolling flag
   g_pInfo[n].box.flags |= PINFO_SCROLLING;

   // set the graph velocity
   g_pInfo[n].velocity = incr;

   // set the max position
   double scale = (266 / (double)(g_nPlayers * 10));
   g_pInfo[n].widthMax = (long)(g_marbles[g_pInfo[n].box.owner] * scale);
}


// Remove a player from the game /////////////////////////////////////////
void PInfo_KillPlayer(int n)
{
   // set the box status to moving
   g_pInfo[n].box.status   = BOX_MOVING;
   g_pInfo[n].box.lastTick = GetTickCount();

   // see if the box is on the left or right
   if(g_pInfo[n].box.flags & PINFO_ON_LEFT)
   {
      g_pInfo[n].box.flags   |= BOX_MOVING_MIN; // set the moving_min flag
      g_pInfo[n].box.velocity = -VEL_PINFO;     // set the box velocity
   }
   else
   {
      g_pInfo[n].box.flags   &= ~BOX_MOVING_MIN; // set the moving_min flag
      g_pInfo[n].box.velocity = VEL_PINFO;       // set the box velocity
   }

   // set the player status to out
   g_pFlags[g_pInfo[n].box.owner] |= PLAYER_OUT;
}


// Init the player info boxes ////////////////////////////////////////////
void PInfo_Init(void)
{
   BOOL onLeft;
   DWORD thisTick = GetTickCount(); // get the current tick
   
   int i;     // used to determine color
   int j = 0; // used to determine turn

   // set all of the boxes to inactive
   for(i = 0; i < 4; i++) g_pInfo[i].box.status = BOX_INACTIVE;

   // set the atributes of the boxes
   for(i = 0; i < 4; i++)
   {
      // if the player is not active, move on to the next
      if(!(g_pFlags[i] & PLAYER_ACTIVE)) continue;
      
      // if the player was out (most likley in a loaded game)
      if(g_pFlags[i] & PLAYER_OUT)
      {
         j++;
         continue;
      }

      // determine if the box is on the left
      onLeft = (j == 0 || (j == 2 && g_nPlayers > 2));

      // if there is only two players and players two and four are active: invert onLeft
      if(g_nPlayers == 2 && (g_pFlags[1] & PLAYER_ACTIVE) &&
         (g_pFlags[3] & PLAYER_ACTIVE)) onLeft = !onLeft;

      // set the y chord
      if(g_nPlayers == 2 || (j == 1 && g_nPlayers == 3))
         g_pInfo[j].box.constant = 377; // center
      else if((g_nPlayers > 2 && j == 0) || (g_nPlayers == 4 && j == 1))
         g_pInfo[j].box.constant = 356; // top
      else
         g_pInfo[j].box.constant = 405; // bottom

      // set the x chord, see if a marble is already in the air
      if(g_settings & SET_MARBLE_JUMPING)
         g_pInfo[j].box.variable = onLeft?  7 : 358;
      else
         g_pInfo[j].box.variable = onLeft? -274 : 640;    
      
      g_pInfo[j].box.velocity = onLeft? VEL_PINFO : -VEL_PINFO; // velocity
      g_pInfo[j].box.lastTick = thisTick;                       // previous tick
      g_pInfo[j].box.max      = onLeft? 7 : 640;                // max x
      g_pInfo[j].box.min      = onLeft? -274 : 358;             // min x

      g_pInfo[j].box.dds      = ddsPInfo;
      g_pInfo[j].box.bltFlags = DDBLTFAST_NOCOLORKEY;

      g_pInfo[j].box.owner    = i;
      g_pInfo[j].box.position = j;
      g_pInfo[j].box.rectClip = R_SCREEN;
      g_pInfo[j].box.status   = BOX_MOVING;
      
      g_pInfo[j].box.flags    = 0; // clear the flags

      if(onLeft) g_pInfo[j].box.flags |= PINFO_ON_LEFT; // set the on_left flag
      else g_pInfo[j].box.flags |= BOX_MOVING_MIN;      // set the moving_min flag

      // if this is the current player, we must select it
      if(g_cPlayer == j) g_pInfo[j].box.flags |= PINFO_SELECT;

      // set the source rectangle
      Rect_SetDimen(&g_pInfo[j].box.rectSource, 0, 42 * j, 274, 42);

      // set the width postition of the graph
      double scale = (266 / (double)(g_nPlayers * 10));
      g_pInfo[j].width = (long)(g_marbles[i] * scale);
      
      // set the widthMax just to be safe
      g_pInfo[j].widthMax = g_pInfo[j].width;

      j++;
   }
   PInfo_Draw();
}


// Hide all of the player info boxes /////////////////////////////////////
void PInfo_Close(void)
{
   DWORD thisTick = GetTickCount();

   for(int i = 0; i < g_nPlayers; i++)
   {
      // set the status of all of the boxes to moving
      g_pInfo[i].box.lastTick = thisTick;
      g_pInfo[i].box.status   = BOX_MOVING;

      // set the velosity & direction of all the boxes
      if(g_pInfo[i].box.flags & PINFO_ON_LEFT)
      {
         g_pInfo[i].box.flags   |= BOX_MOVING_MIN;
         g_pInfo[i].box.velocity = -VEL_PINFO;
      }
      else
      {
         g_pInfo[i].box.velocity = VEL_PINFO;
         g_pInfo[i].box.flags   &= ~BOX_MOVING_MIN;
      }
   }
}


// Render the player info boxes //////////////////////////////////////////
void PInfo_Render(void)
{
   int i;     // represents color
   int j = 0; // represents turn
   RECT r;
   char szText[4];
   PALETTEENTRY color;
   UCHAR fill[4] = {CLR_L_RED, CLR_L_TEAL, CLR_L_PURPLE, CLR_L_TAN};
   UCHAR bord[4] = {CLR_D_RED, CLR_D_TEAL, CLR_D_PURPLE, CLR_D_TAN};

   for(i = 0; i < 4; i++)
   {
      // see if the player is active
      if(!(g_pFlags[i] & PLAYER_ACTIVE)) continue;

      // see if the graph is scrolling
      if(g_pInfo[j].box.flags & PINFO_SCROLLING)
      {
         // incriment the width
         g_pInfo[j].width += g_pInfo[j].velocity;

         // see if we are done
         if(g_pInfo[j].width >= g_pInfo[j].widthMax &&
            g_pInfo[j].width <= g_pInfo[j].widthMax)
         {            
            g_pInfo[j].width = g_pInfo[j].widthMax;   // make sure we don't over do it
            g_pInfo[j].box.flags &= ~PINFO_SCROLLING; // remove the scrolling flag
         }

         // draw the graph back
         Rect_SetDimen(&r, 4, 42 * j + 3, 266, 13);
         DD_Draw_Rect(ddsPInfo, &r, CLR_L_GREY, CLR_D_GREY, 1);

         // draw the graph front
         r.right = g_pInfo[j].width + 4;
         if(g_pInfo[j].width > 0) DD_Draw_Rect(ddsPInfo, &r, fill[i], bord[i], 1);

         // draw the graph text
         r.right = 270;
         wsprintf(szText, "%d", g_marbles[i]);
         GDI_DrawText(ddsPInfo, RGB_BLACK, &r, szText, TXT_FMT_CC);
      }

      // see if the box is flashing
      if(g_pInfo[j].box.flags & PINFO_FLASHING)
      {
         // increment counter 1
         if(g_pInfo[j].cFFrameB % 2 == 0) g_pInfo[j].cFFrameA -= 10;
         else g_pInfo[j].cFFrameA += 10;
         
         // determine if counter 1 is compete
         if(g_pInfo[j].cFFrameA >= 200 || g_pInfo[j].cFFrameA <= 0)
         {            
            // see if we should stop
            if(++g_pInfo[j].cFFrameB > 6)
            {
               g_pInfo[j].cFFrameA = 0; // change the color to only red
               g_pInfo[j].box.flags &= ~PINFO_FLASHING; // remove the flashing flag
            }
         }

         // set the colors
         color.peRed   = 255;
         color.peGreen = g_pInfo[j].cFFrameA;
         color.peBlue  = 0;
         color.peFlags = PC_NOCOLLAPSE;

         // set the palette entry
         ddpMain->SetEntries(0, CLR_PINFO, 1, &color);
      }

      // render the box to the screen
      Box_Render(&g_pInfo[j].box);

      // see if we should select the box
      if(g_pInfo[j].box.status == BOX_FINISHED_MOVING &&
         (g_pInfo[j].box.flags & PINFO_SELECT))
      {
         
         PInfo_Select(j); // select this box
         
         // if a marble or bullet is already in the air, dont do anything
         if(!(g_settings & SET_MARBLE_JUMPING)) Dice_Show();
         
         // remove the loading flag so the dice will be given a value next time
         g_settings &= ~SET_LOADING;
         
         // remove the select flag
         g_pInfo[j].box.flags &= ~PINFO_SELECT;
      }

      // increment the turn
      j++;
   }
}


// Draw the player info boxes ////////////////////////////////////////////
void PInfo_Draw(void)
{
   DDBLTFX bltFx;
   RECT ra = {1, 1, 273, 39};
   RECT rb = {4, 3, 270, 16};
   RECT rc;
   HDC hdc;
   char szText[8];
   UCHAR fill[4] = {CLR_L_RED, CLR_L_TEAL, CLR_L_PURPLE, CLR_L_TAN};
   UCHAR bord[4] = {CLR_D_RED, CLR_D_TEAL, CLR_D_PURPLE, CLR_D_TAN};

   // setup bltFx
   ZeroMemory(&bltFx, sizeof(DDBLTFX));
   bltFx.dwSize      = sizeof(DDBLTFX);
   bltFx.dwFillColor = CLR_BACKGROUND;

   // clear the surface
   DD_Blt(ddsPInfo, NULL, NULL, NULL, DDBLT_COLORFILL, &bltFx);

   // do the directDraw stuff
   for(int i = 0; i < g_nPlayers; i++)
   {
      // draw the backs
      DD_Draw_Rect(ddsPInfo, &ra, CLR_WHITE, CLR_BORDER, 1);

      // draw the marble
      Rect_SetDimen(&rc, 50, 11 * g_pInfo[i].box.owner, 12, 12);
      DD_BltFast(ddsPInfo, ddsItems, 6, 42 * i + 21, &rc, DDBLTFAST_SRCCOLORKEY);

      // draw the graph back
      DD_Draw_Rect(ddsPInfo, &rb, CLR_L_GREY, CLR_D_GREY, 1);

      // draw the graph front
      Rect_SetDimen(&rc, 4, 42 * i + 3, g_pInfo[i].width, 13);
      DD_Draw_Rect(ddsPInfo, &rc, fill[g_pInfo[i].box.owner],
         bord[g_pInfo[i].box.owner], 1);

      // offset the rectangels
      Rect_Offset(&ra, 0, 42);
      Rect_Offset(&rb, 0, 42);
   }

   // do the gdi stuff
   DD_GetDC(ddsPInfo, &hdc);
   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, g_fontSmall);

   Rect_SetPoint(&ra, 4, 3, 270, 16);
   Rect_SetPoint(&rb, 21, 19, 266, 35);
   
   for(i = 0; i < g_nPlayers; i++)
   {
      // draw the graph text
      wsprintf(szText, "%d", g_marbles[g_pInfo[i].box.owner]);
      DrawText(hdc, szText, strlen(szText), &ra, TXT_FMT_CC);

      // draw the name
      DrawText(hdc, g_name[g_pInfo[i].box.owner], strlen(g_name[g_pInfo[i].box.owner]),
         &rb, TXT_FMT_CL);

      // draw the score
      wsprintf(szText, "%d", g_score[g_pInfo[i].box.owner]);
      DrawText(hdc, szText, strlen(szText), &rb, TXT_FMT_CR);

      // offset the rectangles
      Rect_Offset(&ra, 0, 42);
      Rect_Offset(&rb, 0, 42);
   }

   DD_ReleaseDC(ddsPInfo, hdc);
}
