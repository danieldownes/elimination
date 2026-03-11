/*******************************************************************************/
// window.cpp - Custom Window Functions
//
// Written By : Justin Hoffman
// Date       : July 12, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "main.h"
#include "elimres.h"
#include "window.h"
#include "graph.h"
#include "sound.h"
#include "file.h"


// Globals ///////////////////////////////////////////////////////////////
static WINDOW* g_windows[2];
static BOOL    g_active[2];
static int     g_current;
static char*   TXT_GETNAMES_HELP[4] = {
   "Change the activation of a player",
   "Switch between computer and human player",
   "Change the players team",
   "Change the players name"};


/*-----------------------------------------------------------------------------*/
// Window Creation Wrappers
/*-----------------------------------------------------------------------------*/

// Generic Message box ///////////////////////////////////////////////////
int Dlg_MsgBox(LPSTR text, LPSTR title, char *btnA, char* btnB)
{
   // set the paused flag
   g_settings |= SET_PAUSED;

   // set the text of the buttons
   char btns[3][32];
   wsprintf(btns[0], "");
   wsprintf(btns[1], btnB == NULL? "" : btnA);
   wsprintf(btns[2], btnB == NULL? btnA : btnB);

   // setup the window atributes
   WINDOW windat;
   windat.szTitle   = title;
   windat.width     = 245;
   windat.height    = 205;
   windat.nTxtBoxes = 0;
   windat.pTxtBoxes = NULL;
   windat.pfnMsg    = DlgMsg_MsgBox;
   windat.exData    = (DWORD)text;
   
   // show the window
   Win_Create(&windat, btns);

   // remove the paused flag
   g_settings &= ~SET_PAUSED;

   // return what button was clicked
   return windat.returnVal - 1;
}


// Get Names box /////////////////////////////////////////////////////////
BOOL Dlg_GetNames(void)
{
   RECT ra, rb;
   EDAT_GETNAMES data;

   //////////////////////////////////////////////////////////////////
   // translate g_teams to data.teams

   // get the number of players
   data.nPlayers = g_nPlayers;

   // clear data.teams
   for(int i = 0; i < 4; i++) data.teams[i] = 0;

   // make sure there are teams
   if(g_teams.nTeamA + g_teams.nTeamB != 0)
   {
      for(i = 0; i < 4; i++)
      {
         // see if they are on teamA
         if(g_teams.teamA[0] == i || g_teams.teamA[1] == i) data.teams[i] = 1;

         // see if they are on teamB
         if(g_teams.teamB[0] == i || g_teams.teamB[1] == i) data.teams[i] = 2;
      }
   }


   //////////////////////////////////////////////////////////////////
   // Setup the window

   // set the text of the buttons
   char btns[3][32];
   wsprintf(btns[0], (g_settings & SET_FIRST_GAME)? "Play" : "");
   wsprintf(btns[1], (g_settings & SET_FIRST_GAME)? "Load Game" : "");
   wsprintf(btns[2], (g_settings & SET_FIRST_GAME)? "Exit" : "Play");

   // create the surface rectangle for the text boxes
   Rect_SetPoint(&ra, 286, 72, 471, 91);
   
   // offset the screen rectangle for the text boxes
   rb = ra; Rect_Offset(&rb, 79, 85);

   // setup the text boxes
   TEXTBOX tBoxes[4];
   for(i = 0; i < 4; i++)
   {
      TxtBox_Create(&tBoxes[i], g_name[i], &ra, &rb);
      Rect_Offset(&ra, 0, 46);
      Rect_Offset(&rb, 0, 46);
   }

   // setup the window atributes
   WINDOW windat;
   windat.szTitle   = "Select Game Options";
   windat.width     = 482;
   windat.height    = 310;
   windat.nTxtBoxes = 4;
   windat.pTxtBoxes = tBoxes;
   windat.pfnMsg    = DlgMsg_GetNames;
   windat.exData    = (DWORD)&data;
   
   // show the window
   Win_Create(&windat, btns);


   //////////////////////////////////////////////////////////////////
   // Translate data gotten back into the games formats

   // change the number of players
   g_nPlayers = data.nPlayers;

   // translate data.teams to g_teams
   g_teams.nTeamA = 0;
   g_teams.nTeamB = 0;
   
   for(i = 0; i < 4; i++)
   {
      // see if the player is on teamA
      if((g_pFlags[i] & PLAYER_ACTIVE) && data.teams[i] == 1)
         g_teams.teamA[g_teams.nTeamA++] = i;

      // see if they are on teamB
      else if((g_pFlags[i] & PLAYER_ACTIVE)&& data.teams[i] == 2)
         g_teams.teamB[g_teams.nTeamB++] = i;
   }

   //////////////////////////////////////////////////////////////////
   // figure out what button was pushed and what to do
   if(windat.returnVal == 1)
   {
      // make sure the game is registered
      if(g_settings & SET_REGISTERED)
      {
         // show the load game box
         if(!Dlg_LoadGame()) return Dlg_GetNames();
      }
      else
      {
         Dlg_MsgBox(TXT_GENERIC_UNREG, TXT_REGTITLE, TXT_OK, NULL);
         return Dlg_GetNames();
      }
   }
   else if(windat.returnVal == 3) return FALSE; // the user exited

   // return that we want the game to continue
   return TRUE;
}


// Roll dice to see who is first /////////////////////////////////////////
void Dlg_RollTurn(void)
{
   // the message text
   char szMsg[128];
   wsprintf(szMsg, NULL);

   // set the text of the buttons
   char btns[3][32];
   wsprintf(btns[0], "");
   wsprintf(btns[1], "");
   wsprintf(btns[2], "OK#");

   // setup the window atributes
   WINDOW windat;
   windat.szTitle   = TXT_GENTITLE;
   windat.width     = 244;
   windat.height    = 233;
   windat.nTxtBoxes = 0;
   windat.pTxtBoxes = NULL;
   windat.pfnMsg    = DlgMsg_RollTurn;
   windat.exData    = (DWORD)szMsg;
   
   // set the dice in motion
   Dice_SetUpA();

   // show the window
   Win_Create(&windat, btns);
}


// Save a game ///////////////////////////////////////////////////////////
void Dlg_SaveGame(void)
{
   RECT ra = {13, 47, 202, 66};    // surface rect
   RECT rb = {123, 161, 312, 180}; // screen rect
 
   DWORD selected = 0; // the selected game
   char names[10][32]; // saved games names
   TEXTBOX boxes[10];  // the text boxes

   // load the saved games names
   if(!IO_GameNames(names, READ))
   {
      // tell the user there was an error
      DS_Play(dsoError, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_ERR_READ, TXT_ERROR, TXT_OK, NULL);
      return;
   }

   // set the paused flag
   g_settings |= SET_PAUSED;

   // set up the text boxes
   for(int i = 0; i < 10; i++)
   {
      // create the box
      TxtBox_Create(&boxes[i], names[i], &ra, &rb);

      if(i == 4)
      {
         // switch to the other side
         Rect_SetPoint(&ra, 217, 47, 406, 66);
         Rect_SetPoint(&rb, 327, 161, 518, 180);
      }
      else
      {
         // offset the rectangels
         Rect_Offset(&ra, 0, 32);
         Rect_Offset(&rb, 0, 32);
      }
   }

   // set the text of the buttons
   char btns[3][32];
   wsprintf(btns[0], "");
   wsprintf(btns[1], "Save");
   wsprintf(btns[2], "Cancel");

   // setup the window atributes
   WINDOW windat;
   windat.szTitle   = "Save a Game";
   windat.width     = 419;
   windat.height    = 245;
   windat.nTxtBoxes = 10;
   windat.pTxtBoxes = boxes;
   windat.pfnMsg    = DlgMsg_SaveGame;
   windat.exData    = (DWORD)&selected;

   // show the window
   Win_Create(&windat, btns);

   // remove the paused flag
   g_settings &= ~SET_PAUSED;

   // check to make sure 'save' was clicked
   if(windat.returnVal == 1)
   {
      // save the game names & save the game in slot <selected>
      if(!IO_GameNames(names, WRITE) || !IO_Game(selected, WRITE))
      {
         // an error occured while saving
         DS_Play(dsoError, D_CENTERCHANNEL);
         Dlg_MsgBox(TXT_ERR_WRITE, TXT_ERROR, TXT_OK, NULL);
      }
   }
}


// Load a game ///////////////////////////////////////////////////////////
BOOL Dlg_LoadGame(void)
{
   // determine if this is the first game
   bool firstGame = FLAG_SET(g_settings, SET_FIRST_GAME);

   EDAT_LOADGAME data;            // setup the extra data for the window
   data.selected = 0;             // set the selected line to 0

   // load the games settings
   if(!IO_LoadGameStats(&data))
   {
      // report an error
      DS_Play(dsoError, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_ERR_READ, TXT_ERROR, TXT_OK, NULL);
      return FALSE;
   }
   
   // pause the current game
   g_settings |= SET_PAUSED;      

   // set the text of the buttons
   char btns[3][32];
   wsprintf(btns[0], "");
   wsprintf(btns[1], data.active[0]? "Load" : "Load#");
   wsprintf(btns[2], "Cancel");

   // setup the window atributes
   WINDOW windat;
   windat.szTitle   = "Select A Game To Load";
   windat.width     = 481;
   windat.height    = 309;
   windat.nTxtBoxes = 0;
   windat.pTxtBoxes = NULL;
   windat.pfnMsg    = DlgMsg_LoadGame;
   windat.exData    = (DWORD)&data;

   Win_Create(&windat, btns); // show the window
   g_settings &= ~SET_PAUSED; // remove the paused flag

   // if the user clicked 'load'
   if(windat.returnVal == 1)
   {  
      // load the game
      if(!IO_Game(data.selected, READ))
      {
         // report an error
         DS_Play(dsoError, D_CENTERCHANNEL);
         Dlg_MsgBox(TXT_ERR_READ, TXT_ERROR, TXT_OK, NULL);
         return FALSE;
      }

      g_settings |= SET_LOADING; // set the loading flag
      SetupLoadedGame();         // show the changes
   }

   // return the user clicked 'load'
   return windat.returnVal == 1;
}


// The Help File /////////////////////////////////////////////////////////
void Dlg_Help(int index)
{
   // set the paused flag
   g_settings |= SET_PAUSED;

   // set the text of the buttons
   char btns[3][32];

   if(g_settings & SET_REGISTERED)
      wsprintf(btns[0], index == 1? "< Prev#" : "< Prev");
   else
      wsprintf(btns[0], index == 0? "< Prev#" : "< Prev");

   wsprintf(btns[1], index == 6? "Next >#" : "Next >");
   wsprintf(btns[2], "Close");

   // setup the window atributes
   WINDOW windat;
   
   switch(index)
   {
   case 0:
      windat.szTitle = TXT_REGTITLE;
      break;
   case 1:
   case 6:
      windat.szTitle = TXT_ABOUT_ELIM;
      break;
   default:
      windat.szTitle = TXT_HELP;
      break;
   }   
   
   windat.width     = 481;
   windat.height    = 309;
   windat.nTxtBoxes = 0;
   windat.pTxtBoxes = NULL;
   windat.pfnMsg    = DlgMsg_Help;
   windat.exData    = index;

   // show the window
   Win_Create(&windat, btns);

   if(windat.returnVal == 0)      Dlg_Help(index - 1); // prev
   else if(windat.returnVal == 1) Dlg_Help(index + 1); // next

   // remove the paused flag
   g_settings &= ~SET_PAUSED;
}


/*-----------------------------------------------------------------------------*/
// Window Messaging functions
/*-----------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////////
// Generic Message box ///////////////////////////////////////////////////
static void DlgMsg_MsgBox(DWORD text, UINT msg, WPARAM, LPARAM)
{
   RECT r = {7, 37, 240, 150};

   switch(msg)
   {
   case MSG_DRAW:

      // make sure to ajust the rectangle if it is on the second window
      if(g_active[1]) Rect_Offset(&r, g_windows[1]->x, g_windows[1]->y);

      // draw the message text
      GDI_DrawText(ddsWindow, RGB_BLACK, &r, (LPSTR)text, DT_WORDBREAK | DT_LEFT);
      break;
   case MSG_BUTTON1:
   case MSG_BUTTON2:
   case MSG_BUTTON3:
      Win_Close();
      break;
   }
}



//////////////////////////////////////////////////////////////////////////
// Get Names Message box /////////////////////////////////////////////////
static void DlgMsg_GetNames(DWORD pData, UINT msg, WPARAM wParam, LPARAM lParam)
{
   int line; // used to see what player the mouse is over
   char szText[4];
   RECT ra = {199, 368, 214, 383}; // used to see if the mouse is over the arrows
   RECT rb = {91, 280, 120, 303};  // used to redraw the number of sinkholes
   EDAT_GETNAMES *data = (EDAT_GETNAMES*)pData;

   switch(msg)
   {
   // re-draw the window
   case MSG_DRAW:
      DlgExGN_Draw(data);
      break;

   // one of the buttons was clicked
   case MSG_BUTTON1:
      // make sure that the teams are valad
      if(g_settings & SET_FIRST_GAME) if(DlgExGN_CheckTeamB(data)) Win_Close();
      break;

   case MSG_BUTTON2:
      Win_Close();
      break;

   case MSG_BUTTON3:
      if(g_settings & SET_FIRST_GAME)
      {
         // close the game
         g_windows[0]->returnVal = 3;
         Win_Close();
      }
      else
      {
         // make sure the teams are valad
         if(DlgExGN_CheckTeamB(data)) Win_Close();
      }
      break;

   case WM_KEYDOWN:
      if(wParam == VK_UP || wParam == VK_DOWN)
      {
         if(wParam == VK_DOWN && g_nSinkholes > 0) g_nSinkholes--; // move it down
         else if(wParam == VK_UP && g_nSinkholes < 75) g_nSinkholes++; // move it up
 
         // re-draw the number of sinkholes
         wsprintf(szText, "%d", g_nSinkholes);
         DD_Draw_Rect(ddsWindow, &rb, CLR_WINDOWFILL, 0, 0);
         GDI_DrawText(ddsWindow, RGB_BLACK, &rb, szText, TXT_FMT_CC);
      }
      break;

   // the mouse was clicked in the window
   case WM_LBUTTONDOWN:
      // check to see if the mouse was within either of the
      // arrows to change the number of sinkholes
      if(Rect_Within(&ra, LOWORD(lParam), HIWORD(lParam)))
      {
         if(HIWORD(lParam) > 377 && g_nSinkholes > 0) g_nSinkholes--; // move it down
         else if(HIWORD(lParam) < 377 && g_nSinkholes < 75) g_nSinkholes++; // move it up

         // re-draw the number of sinkholes
         wsprintf(szText, "%d", g_nSinkholes);
         DD_Draw_Rect(ddsWindow, &rb, CLR_WINDOWFILL, 0, 0);
         GDI_DrawText(ddsWindow, RGB_BLACK, &rb, szText, TXT_FMT_CC);
      }

      // get the line (player) that was clicked on
      line = (int)((HIWORD(lParam) - 143) / 46);
      switch(DlgExGN_GetQuad(LOWORD(lParam), HIWORD(lParam)))
      {
      // change player activation
      case 0:
         g_pFlags[line] ^= PLAYER_ACTIVE;
         
         // increment the number of players
         if(g_pFlags[line] & PLAYER_ACTIVE) data->nPlayers++;
         else data->nPlayers--;

         // check to see if there are two players present
         DlgExGN_CheckAct(line, data);
         Win_Draw();
         break;

      // change player type
      case 1:
         g_pFlags[line] ^= PLAYER_IS_PC;

         // play the menu sound
         DS_Play(dsoMenuB, D_CENTERCHANNEL);
         Win_Draw();
         break;

      // change player team
      case 2:
         if(++data->teams[line] > 2) data->teams[line] = 0;
         
         // check to see if this can be done
         DlgExGN_CheckTeamA(line, data);
         Win_Draw();
         break;
      }
      break;

   // the mouse moved in the window
   case WM_MOUSEMOVE:
      DlgExGN_ChangeText(DlgExGN_GetQuad(LOWORD(lParam), HIWORD(lParam)));
      break;
   }
}


// draw to the window ////////////////////////////////////////////////////
static void DlgExGN_Draw(EDAT_GETNAMES *data)
{
   HDC hdc;
   int i, j;
   RECT r;

   char szText[64];
   char titles[4][32] = {"Active", "Type", "Team", "Name"};
   RECT rects[4] = {
      {36 , 21, 82 , 68},  // player status
      {81 , 21, 175, 68},  // player type
      {174, 21, 262, 68},  // player team
      {273, 21, 343, 68}}; // Name Title
    
   // Draw the horizontal lines
   Win_DrawHLine(g_windows[0], 55);
   Win_DrawHLine(g_windows[0], 249);

   // draw the arrow for the number of sinkholes
   Rect_SetPoint(&r, 46, 64, 61, 79);
   DD_BltFast(ddsWindow, ddsItems, 120, 284, &r, DDBLTFAST_SRCCOLORKEY);

   // Draw the marbles
   Rect_SetPoint(&r, 25, 64, 46, 85);
   for(i = 0; i < 4; i++)
   {
      DD_BltFast(ddsWindow, ddsItems, 10, 46 * i + 71, &r, DDBLTFAST_SRCCOLORKEY);
      Rect_Offset(&r, 0, 20);
   }

   // get a divice context of the surface
   DD_GetDC(ddsWindow, &hdc);
   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, g_fontSmall);

   // draw the titles
   for(i = 0; i < 4; i++)
   {
      DrawText(hdc, titles[i], strlen(titles[i]), &rects[i], TXT_FMT_CC);
      Rect_Offset(&rects[i], 0, 37);
   }

   // draw the data
   for(i = 0; i < 4; i++)
   {
      // draw the status
      SetTextColor(hdc, (g_pFlags[i] & PLAYER_ACTIVE)? RGB_BLACK : RGB_RED);
      wsprintf(szText, (g_pFlags[i] & PLAYER_ACTIVE)? "Yes" : "No");
      DrawText(hdc, szText, strlen(szText), &rects[0], TXT_FMT_CC);
      SetTextColor(hdc, RGB_BLACK); // reset the text color

      // draw the player type
      wsprintf(szText, (g_pFlags[i] & PLAYER_IS_PC)? "Computer" : "Human");
      DrawText(hdc, szText, strlen(szText), &rects[1], TXT_FMT_CC);

      // draw the player team
      if(data->teams[i] == 0) wsprintf(szText, "No Team");
      else if(data->teams[i] == 1) wsprintf(szText, "Team A");
      else wsprintf(szText, "Team B");
      DrawText(hdc, szText, strlen(szText), &rects[2], TXT_FMT_CC);

      // offset the rectangles
      for(j = 0; j < 3; j++) Rect_Offset(&rects[j], 0, 46);
   }

   // draw the 'sinkholes:' text
   Rect_SetPoint(&r, 26, 274, 92, 309);
   DrawText(hdc, "Sinkholes:", strlen("Sinkholes:"), &r, TXT_FMT_CC);

   // draw the number of sinkholes
   wsprintf(szText, "%d", g_nSinkholes);
   Rect_SetPoint(&r, 91, 274, 120, 309);
   DrawText(hdc, szText, strlen(szText), &r, TXT_FMT_CC);

   // release the dc
   DD_ReleaseDC(ddsWindow, hdc);
}


// return what section of the window the mouse is over ///////////////////
static int DlgExGN_GetQuad(long x, long y)
{
   if(143 > y || y > 327) return D_INVALID; // the mouse y is not within the window
   else if(115 < x && x < 160) return 0;    // the mouse is within the player status
   else if(160 < x && x < 253) return 1;    // the mouse is within the player type
   else if(253 < x && x < 340) return 2;    // the mouse is witin the team
   else if(363 < x && x < 516) return 3;    // the moues is witin the player names

   // the mouse was not within any of those,
   // report an invalid value
   return D_INVALID;
}


// change the message text ///////////////////////////////////////////////
static void DlgExGN_ChangeText(int n)
{
   RECT r = {8, 253, 370, 271};

   // clear the text
   DD_Draw_Rect(ddsWindow, &r, CLR_WINDOWFILL, 0, 0);

   if(n != D_INVALID)
   {
      // change to the new text
      GDI_DrawText(ddsWindow, RGB_BLACK, &r, TXT_GETNAMES_HELP[n], TXT_FMT_CL);
   }
}


// check the player activation ///////////////////////////////////////////
void DlgExGN_CheckAct(int line, EDAT_GETNAMES* data)
{
   // see if the game is registered
   if(!(g_settings & SET_REGISTERED))
   {
      // reset the players activation
      g_pFlags[line] ^= PLAYER_ACTIVE;

      // set the number of players back
      if(g_pFlags[line] & PLAYER_ACTIVE) data->nPlayers++;
      else data->nPlayers--;

      // show a message box telling the error
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_ACT_UNREGISTERED, TXT_REGTITLE, TXT_OK, NULL);
   }

   // see if there are enough players
   else if(data->nPlayers < 2)
   {
      // reset the players activation
      g_pFlags[line] ^= PLAYER_ACTIVE;

      // set the number of players back
      if(g_pFlags[line] & PLAYER_ACTIVE) data->nPlayers++;
      else data->nPlayers--;

      // show a message box telling the error
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_NEED_TWO, TXT_GENTITLE, TXT_OK, NULL);
   }
   else
   {
      // play the menu sound
      DS_Play(dsoMenuB, D_CENTERCHANNEL);

      // see if we should deactivate the teams
      if(data->nPlayers < 3) for(int i = 0; i < 4; i++) data->teams[i] = 0;
   }
}


// check the teams ///////////////////////////////////////////////////////
static void DlgExGN_CheckTeamA(int line, EDAT_GETNAMES* data)
{
   if(data->nPlayers < 3)
   {
      // reset the players team
      if(--data->teams[line] < 0) data->teams[line] = 2;

      // show a message box reporting the error
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_NEED_THREE, TXT_GENTITLE, TXT_OK, NULL);
   }
   else DS_Play(dsoMenuB, D_CENTERCHANNEL);
}


// check the teams win the window is closed //////////////////////////////
static BOOL DlgExGN_CheckTeamB(EDAT_GETNAMES* data)
{
   int nTeamA = 0, nTeamB = 0;

   // figure the number of players on each team
   for(int i = 0; i < 4; i++)
   {
      // if they are on team A
      if((g_pFlags[i] & PLAYER_ACTIVE) && data->teams[i] == 1) nTeamA++;
      
      // if they are on team b
      else if((g_pFlags[i] & PLAYER_ACTIVE) && data->teams[i] == 2) nTeamB++;
   }

   // make sure there are only 2 on each team
   if(nTeamA > 2 || nTeamB > 2)
   {
      // now now, no ganging up in big numbers - it is not fair you know
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_TWO_ON_TEAM, TXT_GENTITLE, TXT_OK, NULL);

      // return the users failure
      return FALSE;
   }

   // make sure the number of players on the teams added together is equal
   // to the number of accual players or is equal to zero
   else if(nTeamA + nTeamB != data->nPlayers && nTeamA + nTeamB != 0)
   {
      // the numbers did not check out, show a message box saying so
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      Dlg_MsgBox(TXT_ALL_ON_TEAM, TXT_GENTITLE, TXT_OK, NULL);
      return FALSE;
   }
   return TRUE;
}



//////////////////////////////////////////////////////////////////////////
// Roll Turn Messaging ///////////////////////////////////////////////////
static void DlgMsg_RollTurn(DWORD text, UINT msg, WPARAM, LPARAM)
{
   int i;
   RECT ra = {8, 178, 235, 193}; // message text
   RECT rb = {43, 47, 230, 63};  // player names

   switch(msg)
   {
   case MSG_DRAW:
      Win_DrawHLine(g_windows[0], 173);
      GDI_DrawText(ddsWindow, RGB_BLACK, &ra, (LPSTR)text, TXT_FMT_CL);

      // draw the player names
      for(i = 0; i < 4; i++)
      {
         if(g_pFlags[i] & PLAYER_ACTIVE)
         {
            GDI_DrawText(ddsWindow, RGB_BLACK, &rb, g_name[i], TXT_FMT_CL);
            Rect_Offset(&rb, 0, 32);
         }
      }
      break;
   case MSG_BUTTON3:
      g_settings &= ~SET_ROLL_TURN; // terminate the dice
      Win_Close();
      break;
   }
}


// change the message text ///////////////////////////////////////////////
void DlgMsgExRT_ChangeText(char* text)
{
   // Note: we know that this will be the only window active,
   // so i can refer to it through g_windos[0] without trouble
   RECT r = {8, 178, 235, 193};

   // change the message text
   wsprintf((LPSTR)g_windows[0]->exData, text);

   // clear the back of the text
   DD_Draw_Rect(ddsWindow, &r, CLR_WINDOWFILL, 0, 0);

   // draw the text
   GDI_DrawText(ddsWindow, RGB_BLACK, &r, text, TXT_FMT_CL);
}


// disable a player name //////////////////////////////////////////////////
void DlgMsgExRT_DisablePlayer(int location, int owner)
{
   RECT r = {43, 32 * location + 47, 230, 32 * location + 63};
   GDI_DrawText(ddsWindow, RGB_GREY, &r, g_name[owner],  TXT_FMT_CL);
}


// enable the button //////////////////////////////////////////////////////
void DlgMsgExRT_Done(void)
{
   TxtBtn_Enable(&g_windows[0]->buttons[2], TRUE);
}



//////////////////////////////////////////////////////////////////////////
// Save Game Messaging ///////////////////////////////////////////////////
static void DlgMsg_SaveGame(DWORD data, UINT msg, WPARAM wParam, LPARAM lParam)
{
   RECT r;
   DWORD oldBox = *(DWORD*)data;
   DWORD newBox = g_windows[0]->selBox;

   // makesure we didn't get a D_INVALID value
   if(newBox == D_INVALID) newBox = 0;

   switch(msg)
   {
   case MSG_BUTTON2:
   case MSG_BUTTON3:
      Win_Close();
      break;
   
   // select a text box
   case MSG_TEXT_BOX:
   case MSG_DRAW:
      // clear the previous box
      Rect_SetDimen(&r, oldBox > 4? 214 : 10, 32 * (oldBox % 5) + 44, 195, 25);
      DD_Draw_Rect(ddsWindow, &r, CLR_TRANSPARENT, CLR_WINDOWFILL, 2);

      // draw the new box
      Rect_SetDimen(&r, newBox > 4? 214 : 10, 32 * (newBox % 5) + 44, 195, 25);
      DD_Draw_Rect(ddsWindow, &r, CLR_TRANSPARENT, CLR_RED, 2);

      // set the value of the old box to the value of the new one
      *(DWORD*)data = newBox;
      break;
   }
}


//////////////////////////////////////////////////////////////////////////
// Load Game /////////////////////////////////////////////////////////////
static void DlgMsg_LoadGame(DWORD pData, UINT msg, WPARAM wParam, LPARAM lParam)
{
   EDAT_LOADGAME* data = (EDAT_LOADGAME*)pData;
   RECT r = {79, 85, 560, 394};
   int selected;

   switch(msg)
   {
   // a button was pushed
   case MSG_BUTTON2:
   case MSG_BUTTON3:
      Win_Close();
      break;

   // re-draw the window
   case MSG_DRAW:
      DlgExLG_Draw(data);
      break;

   // process key messages
   case WM_KEYDOWN:
      if(wParam == VK_DOWN || wParam == VK_UP)
      {
         if(wParam == VK_DOWN)
         {
            if(++data->selected > 9) data->selected = 0;
         }
         else
         {
            if(--data->selected < 0) data->selected = 9;
         }

         // enable or disable the 'load' button
         TxtBtn_Enable(&g_windows[0]->buttons[1], data->active[data->selected]);
         Win_Draw();
      }
      break;

   // process mouse clicks
   case WM_LBUTTONDOWN:
      // make sure the mouse is within the window
      if(Rect_Within(&r, LOWORD(lParam), HIWORD(lParam)))
      {
         // set the new selected line
         selected = (int)((HIWORD(lParam) - 143) / 21);

         // make sure the value was acceptable
         if(-1 < selected && selected < 10) data->selected = selected;

         // enable or disable the 'load' button
         TxtBtn_Enable(&g_windows[0]->buttons[1], data->active[data->selected]);

         // re-draw the window
         Win_Draw();
      }
      break;
   }
}


// Draw the load game window /////////////////////////////////////////////
static void DlgExLG_Draw(EDAT_LOADGAME* data)
{
   HDC hdc;
   int i, j;
   RECT r;
   RECT table[6] = {
      {10 , 32, 85 , 55},  // the file names
      {85 , 32, 162, 55},  // name 1
      {162, 32, 239, 55},  // name 2
      {239, 32, 316, 55},  // name 3
      {316, 32, 393, 55},  // name 4
      {425, 32, 443, 55}}; // vs text
   
   char szTitle[32] = "File Name";

   // draw the seperator
   Win_DrawHLine(g_windows[0], 55);

   // Draw the selection box
   Rect_SetDimen(&r, 7, 21 * data->selected + 60, 468, 20);
   DD_Draw_Rect(ddsWindow, &r, CLR_WINDOWBORDER, CLR_BLACK, 1);

   // draw the marbles on the top
   Rect_SetPoint(&r, 50, 0, 62, 12);
   for(i = 0; i < 4; i++)
   {
      DD_BltFast(ddsWindow, ddsItems, 76 * i + 119, 38, &r, DDBLTFAST_SRCCOLORKEY);
      Rect_Offset(&r, 0, 11);
   }

   // draw the team marbles
   for(i = 0; i < 10; i++)
   {
      // makesure the game is not empty
      if(!data->active[i]) continue;

      // draw teamA's marbles
      for(j = 0; j < data->teams[i].nTeamA; j++)
      {
         Rect_SetDimen(&r, 50, 11 * data->teams[i].teamA[j], 12, 12);
         DD_BltFast(ddsWindow, ddsItems,
            (-14 * data->teams[i].nTeamA + j * 14 + 425), (i * 21 + 64),
            &r, DDBLTFAST_SRCCOLORKEY);
      }

      // draw teamB's marbles
      for(j = 0; j < data->teams[i].nTeamB; j++)
      {
         Rect_SetDimen(&r, 50, 11 * data->teams[i].teamB[j], 12, 12);
         DD_BltFast(ddsWindow, ddsItems, (j * 14 + 445), (i * 21 + 64),
            &r, DDBLTFAST_SRCCOLORKEY);
      }
   }

   // get the device context
   DD_GetDC(ddsWindow, &hdc);
   SetBkMode(hdc, TRANSPARENT);
   SelectObject(hdc, g_fontSmall);

   // draw the title
   DrawText(hdc, szTitle, strlen(szTitle), &table[0], TXT_FMT_CL);

   // offset the titles rects
   for(i = 0; i < 6; i++) Rect_Offset(&table[i], 0, 26);

   // draw the game data
   for(i = 0; i < 10; i++)
   {
      if(i == data->selected) SetTextColor(hdc, RGB_WHITE);

      // draw the file name
      DrawText(hdc, data->gameName[i], strlen(data->gameName[i]), &table[0], TXT_FMT_CL);
      
      // makesure the game is not empty
      if(data->active[i])
      {
         // draw the players names
         for(j = 0; j < 4; j++) DrawText(hdc, data->names[i][j],
            strlen(data->names[i][j]), &table[j + 1], TXT_FMT_CC);

         // draw the vs text
         if(data->teams[i].nTeamA + data->teams[i].nTeamB == 0)
            DrawText(hdc, "----", 4, &table[5], TXT_FMT_CC);
         else DrawText(hdc, "vs", 2, &table[5], TXT_FMT_CC);
      }

      // offset the rectangles
      for(j = 0; j < 6; j++) Rect_Offset(&table[j], 0, 21);

      // reset the text color
      SetTextColor(hdc, RGB_BLACK);
   }

   // release the dc
   DD_ReleaseDC(ddsWindow, hdc);
}


//////////////////////////////////////////////////////////////////////////
// Help Window messaging /////////////////////////////////////////////////
static void DlgMsg_Help(DWORD index, UINT msg, WPARAM wParam, LPARAM lParam)
{
   HDC hdc;
   XMPDATA xmp;
   char szText[2048];
   RECT r = {7, 37, 475, 265};

   switch(msg)
   {
   // a button was pushed
   case MSG_BUTTON1:
   case MSG_BUTTON2:
   case MSG_BUTTON3:
      Win_Close();
      break;

   // draw the window
   case MSG_DRAW:
      switch(index)
      {
      // title, names & beta testers
      case 1:
      case 6:
         XMP_Load(&xmp, index == 1? XMP_ABOUT : XMP_NAMES); // load the image
         XMP_Copy(ddsWindow, &xmp, 105, 82); // copy image to window
         XMP_Delete(&xmp);                   // delete the image
         break;

      // registration & help texts (index 0 through 4)
      default:
         LoadString(g_hInst, TXT_HELP1 + index, szText, 2048);

         DD_GetDC(ddsWindow, &hdc);     // get the dc
         SelectObject(hdc, g_fontHelp); // select the help font into the dc
         SetBkMode(hdc, TRANSPARENT);   // make the text transparent

         // draw the text
         DrawText(hdc, szText, strlen(szText), &r, DT_EXPANDTABS | DT_LEFT | DT_WORDBREAK);

         // release the dc
         DD_ReleaseDC(ddsWindow, hdc);
         break;
      }
      break;
   }
}


/*-----------------------------------------------------------------------------*/
// Main Window Functoins
/*-----------------------------------------------------------------------------*/

// Create a window ///////////////////////////////////////////////////////
void Win_Create(WINDOW* window, char btns[3][32])
{
   window->returnVal = D_INVALID; // clear the return value
   window->selBox    = D_INVALID; // clear the selected box
   
   // set the window flag
   g_settings |= SET_WINDOW;

   // remove the check time flag
   g_settings &= ~SET_CHECK_TIME;

   // record the time we started pausing
   g_pauseTime = GetTickCount();

   // see if it will go in the first window
   if(!g_active[0])
   {
      g_current    = 0;      // set the current window
      g_windows[0] = window; // set the window's data
      g_active[0]  = TRUE;   // activate the window

      // set the x and y chords
      g_windows[0]->x = 0;
      g_windows[0]->y = 0;
      
      // create the buttons for the window
      Win_CreateBtns(g_windows[0], 0, 0, (long)((SCREEN_W - g_windows[0]->width) / 2),
         (long)((SCREEN_H - g_windows[0]->height) / 2), btns);
      
      // draw the window
      Win_Draw();

      // start the windows event loop
      Win_Loop(&g_active[0]);
   }
   else
   {
      // the screen x and y of the bottom window
      long sx = (long)((SCREEN_W - g_windows[0]->width) / 2);
      long sy = (long)((SCREEN_H - g_windows[0]->height) / 2);
     
      g_current    = 1;      // set the current window
      g_windows[1] = window; // set the window data
      g_active[1]  = TRUE;   // ativate the window

      // data does not include the x and y positions, because it does not
      // know if it will have another window under it.
      g_windows[1]->x = (long)((g_windows[0]->width - g_windows[1]->width) / 2);
      g_windows[1]->y = (long)((g_windows[0]->height - g_windows[1]->height) / 2);

      // create the buttons for the window
      Win_CreateBtns(g_windows[1], g_windows[1]->x, g_windows[1]->y,
         sx + g_windows[1]->x, sy + g_windows[1]->y, btns);
           
      // draw the window
      Win_Draw();

      // start a second event loop
      Win_Loop(&g_active[1]);
   }
}


// Create the buttons for a window ///////////////////////////////////////
void Win_CreateBtns(
WINDOW *data,
long surfX,
long surfY,
long screenX,
long screenY, 
char btns[3][32])
{
   RECT ra, rb;
   BOOL enabled; // is the button enabled?

   // set the surface rectangle
   Rect_SetDimen(&ra, surfX + data->width - 308,
      surfY + data->height - 31, 98, 25);

   // set the screen rectangle
   Rect_SetDimen(&rb, screenX + data->width - 308,
      screenY + data->height - 31, 98, 25);

   // create the three buttons
   for(int i = 0; i < 3; i++)
   {
      enabled = btns[i][strlen(btns[i]) - 1] != '#';

      if(!enabled)
      {
         // remove the enabled flag
         data->buttons[i].flags &= ~BUTTON_ENABLED;

         // pound symbol from the end of the string
         btns[i][strlen(btns[i]) - 1] = NULL;
      }

      // set the enabled flag
      else data->buttons[i].flags |= BUTTON_ENABLED;

      // set the active flag
      if(strlen(btns[i]) > 0) data->buttons[i].flags |= BUTTON_ACTIVE; 

      // remove the active flag
      else data->buttons[i].flags &= ~BUTTON_ACTIVE;

      data->buttons[i].flags &= ~BUTTON_BEING_PUSHED; // remove being_pushed flag
      data->buttons[i].flags &= ~BUTTON_DOWN;         // remove down flag

      // set the rectangles of the button
      data->buttons[i].rectSurface = ra;
      data->buttons[i].rectScreen  = rb;

      // move the text into the buttons structure
      wsprintf(data->buttons[i].szText, btns[i]);

      // offset the rectangles for the next button
      Rect_Offset(&ra, 102, 0);
      Rect_Offset(&rb, 102, 0);
   }
}


// Close a window ////////////////////////////////////////////////////////
void Win_Close(void)
{
   // see if the second window is active
   if(g_active[1])
   {
      // get rid of the window
      g_active[1] = FALSE;
      g_windows[1] = NULL;

      // set the current window to 0
      g_current = 0;

      // re-draw so the bottom window looks correct
      Win_Draw();
   }
   else
   {
      // get rid of the window
      g_active[0] = FALSE;
      g_windows[0] = NULL;

      // remove the window flag
      g_settings &= ~SET_WINDOW;

      // set the check time flag
      g_settings |= SET_CHECK_TIME;

      // add the amout of time we were paused to the start time
      g_startTime += (GetTickCount() - g_pauseTime);
   }
}


// Render the windows to the screen //////////////////////////////////////
void Win_Render(void)
{
   // make sure a window is active
   if(!g_active[0]) return;

   // find the x & y of the window
   long x = (long)((SCREEN_W - g_windows[0]->width) / 2);
   long y = (long)((SCREEN_H - g_windows[0]->height) / 2);

   RECT r = {0, 0, g_windows[0]->width, g_windows[0]->height};
   DD_BltFast(ddsBack, ddsWindow, x, y, &r, DDBLTFAST_NOCOLORKEY);
}


// Draw the windows //////////////////////////////////////////////////////
void Win_Draw(void)
{
   RECT r;
   HDC hdc;
   long x, y;

   for(int i = 0; i < 2; i++)
   {
      if(!g_active[i]) continue;

      // get the x and y of the window
      x = g_windows[i]->x;
      y = g_windows[i]->y;

      // determine the rectangle the window occupies in ddsWindow
      Rect_SetDimen(&r, x, y, g_windows[i]->width, g_windows[i]->height);

      // draw the white border
      DD_Draw_Rect(ddsWindow, &r, CLR_TRANSPARENT, CLR_WINDOWBORDER, 1);

      // fill in the main rectangle
      Rect_Expand(&r, -1);
      DD_Draw_Rect(ddsWindow, &r, CLR_WINDOWFILL, CLR_WHITE, 1);

      // draw the seperation lines
      Win_DrawHLine(g_windows[i], 32);
      Win_DrawHLine(g_windows[i], g_windows[i]->height - 36);

      // get the rectangle for the title
      Rect_SetDimen(&r, x + 7, y + 1, g_windows[i]->width, 32);

      // get the gdi device context
      DD_GetDC(ddsWindow, &hdc);
      SelectObject(hdc, g_fontBig);
      SetBkMode(hdc, TRANSPARENT);

      // draw the title
      DrawText(hdc, g_windows[i]->szTitle, strlen(g_windows[i]->szTitle), &r, TXT_FMT_CL);
      DD_ReleaseDC(ddsWindow, hdc);

      // draw the buttons
      for(int j = 0; j < 3; j++) if(g_windows[i]->buttons[j].flags & BUTTON_ACTIVE)
         TxtBtn_Draw(&g_windows[i]->buttons[j]);

      // draw the text boxes
      for(j = 0; j < g_windows[i]->nTxtBoxes; j++)
         TxtBox_Draw(&g_windows[i]->pTxtBoxes[j]);

      // send a message to the window to draw
      g_windows[i]->pfnMsg(g_windows[i]->exData, MSG_DRAW, 0, 0);
   }
}


// Draw a horizontal line ////////////////////////////////////////////////
void Win_DrawHLine(WINDOW* window, long y)
{
   DD_Draw_Line(ddsWindow, CLR_WINDOWBORDER, window->x + 7, window->y + y,
      window->x + window->width - 7, window->y + y);
}


// Process messages to the window ////////////////////////////////////////
void Win_Message(UINT msg, WPARAM wParam, LPARAM lParam)
{
   int i;
   WINDOW* window = g_windows[g_current];

   switch(msg)
   {
   case WM_MOUSEMOVE:
      // test the buttons
      for(i = 0; i < 3; i++)
         TxtBtn_MouseMove(&window->buttons[i], LOWORD(lParam), HIWORD(lParam));
      break;

   case WM_LBUTTONDOWN:
      // test the buttons
      for(i = 0; i < 3; i++)
         TxtBtn_MouseDown(&window->buttons[i], LOWORD(lParam), HIWORD(lParam));

      // make sure to clear the selected text box
      window->selBox = D_INVALID;

      for(i = 0; i < window->nTxtBoxes; i++)
      {
         // test to see if the box was selected
         if(TxtBox_MouseDown(&window->pTxtBoxes[i], LOWORD(lParam), HIWORD(lParam)))
         {
            window->selBox = i;
            window->pfnMsg(window->exData, MSG_TEXT_BOX, 0, 0);
         }
      }
      break;

   case WM_LBUTTONUP:
      // test the buttons
      for(i = 0; i < 3; i++)
      {
         if(TxtBtn_MouseUp(&window->buttons[i], LOWORD(lParam), HIWORD(lParam)))
         {
            // set the return value to the button that was released
            window->returnVal = i;

            // tell the window that a button was clicked and released
            window->pfnMsg(window->exData, MSG_BUTTON1 + i, 0, 0);

            // return so we dont send any other messages to the window
            return;
         }
      }
      break;

   case WM_CHAR:
      if(window->selBox != D_INVALID)
      {
         TxtBox_Char(&window->pTxtBoxes[ window->selBox ], wParam);
      }
      break;

   case WM_KEYDOWN:
      // select the next text box
      if(wParam == VK_TAB && window->nTxtBoxes > 0)
      {
         // unselect the previous box if there is one
         if(window->selBox != D_INVALID)
         {
            window->pTxtBoxes[ window->selBox ].active = FALSE;
            TxtBox_Draw(&window->pTxtBoxes[ window->selBox ]);
         }

         // update selectedBox
         if(++window->selBox >= window->nTxtBoxes) window->selBox = 0;

         // select the new box
         window->pTxtBoxes[ window->selBox ].active = TRUE;
         TxtBox_Draw(&window->pTxtBoxes[ window->selBox ]);

         // send the message to the window
         window->pfnMsg(window->exData, MSG_TEXT_BOX, 0, 0);
      }

      // if it is the delete key, clear all text from the selected text box
      else if(wParam == VK_DELETE && window->selBox != D_INVALID)
      {
         window->pTxtBoxes[ window->selBox ].szText[0] = NULL;
         TxtBox_Draw(&window->pTxtBoxes[ window->selBox ]);
      }
      break;
   }

   // send the message to the window message processor
   window->pfnMsg(window->exData, msg, wParam, lParam);
}


// Main window event loop ////////////////////////////////////////////////
void Win_Loop(BOOL *active)
{
   MSG msg;
   while(*active)
   {
      if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if(msg.message == WM_QUIT) break;  // see if we should quit
         TranslateMessage(&msg);            // translate accelerator keys
         DispatchMessage(&msg);             // send message to window proc
      }
      if(!(g_settings & SET_APP_PAUSED)) ElimMain();
   }
}


/*-----------------------------------------------------------------------------*/
// Text Button Functions
/*-----------------------------------------------------------------------------*/

// Draw a text button ////////////////////////////////////////////////////
static void TxtBtn_Draw(BTN *button)
{
   // fill in the rectangle
   DD_Draw_Rect(ddsWindow, &button->rectSurface, 
      (button->flags & BUTTON_DOWN)? CLR_WINDOWBORDER : CLR_WINDOWFILL,
      (button->flags & BUTTON_DOWN)? CLR_BLACK : CLR_WINDOWBORDER, 1);

   // Draw the text on the button
   GDI_DrawText(ddsWindow,
      (button->flags & BUTTON_ENABLED)? ((button->flags & BUTTON_DOWN)? RGB_WHITE : RGB_BLACK) : RGB_WINDOWBORDER,
      &button->rectSurface, button->szText, TXT_FMT_CC);
}


// The mouse was clicked /////////////////////////////////////////////////
static void TxtBtn_MouseDown(BTN *button, long x, long y)
{
   // make sure that the button is active and enabled
   if(!(button->flags & BUTTON_ACTIVE) || !(button->flags & BUTTON_ENABLED)) return;

   if(Rect_Within(&button->rectScreen, x, y))
   {
      // set the down and being pushed flags
      button->flags |= BUTTON_DOWN;
      button->flags |= BUTTON_BEING_PUSHED;

      // re-draw the button
      TxtBtn_Draw(button);
   }
}


// The mouse was moved ///////////////////////////////////////////////////
static void TxtBtn_MouseMove(BTN *button, long x, long y)
{
   // make sure the button is being pushed
   if(!(button->flags & BUTTON_BEING_PUSHED)) return;

   // determine if the mouse is within the button
   UINT within = Rect_Within(&button->rectScreen, x, y);
   
   // don't bother drawing unless somthing has changed
   if((button->flags & BUTTON_DOWN) != within)
   {
      // set/reomve the down flag
      SET_FLAG(button->flags, BUTTON_DOWN, within);
      
      // re-draw the button
      TxtBtn_Draw(button);
   }
}

// The mouse was released ////////////////////////////////////////////////
static BOOL TxtBtn_MouseUp(BTN *button, long x, long y)
{
   // make sure the button is being pushed
   if(!(button->flags & BUTTON_BEING_PUSHED)) return FALSE;

   // remove the being_pushed and down flags
   button->flags &= ~BUTTON_BEING_PUSHED;
   button->flags &= ~BUTTON_DOWN;

   // re-draw the button
   TxtBtn_Draw(button);

   // return weather the mouse was within the button
   return Rect_Within(&button->rectScreen, x, y);
}


// Enable/disable the button /////////////////////////////////////////////
static void TxtBtn_Enable(BTN *button, BOOL enable)
{
   // set/remove the enabled flag
   SET_FLAG(button->flags, BUTTON_ENABLED, enable);

   // re-draw the button
   TxtBtn_Draw(button);
}


/*-----------------------------------------------------------------------------*/
// Text Box functions
/*-----------------------------------------------------------------------------*/

// Create a text box /////////////////////////////////////////////////////
static void TxtBox_Create(TEXTBOX *box, LPSTR szText, RECT *rSurface, RECT *rScreen)
{
   // set the atributes of the text box
   box->szText      = szText;
   box->enabled     = TRUE;
   box->active      = FALSE;
   box->rectSurface = *rSurface;
   box->rectScreen  = *rScreen;
}


// Draw a text box ///////////////////////////////////////////////////////
static void TxtBox_Draw(TEXTBOX *box)
{
   // modify the string so that when the box
   // is selected there will be an underscore at the end
   char szText[32];
   if(box->active) wsprintf(szText, "%s_", box->szText);
   else wsprintf(szText, box->szText);

   // modify a rectangle so the text is not
   // all the way to the left
   RECT rect = box->rectSurface; rect.left += 4;

   // draw the box
   DD_Draw_Rect(ddsWindow, &box->rectSurface,
      box->active? CLR_WINDOWBORDER : CLR_WINDOWFILL,
      box->active? CLR_BLACK : CLR_WINDOWBORDER, 1);

   // draw the text, make sure there is text to draw
   // otherwise an error occurs at runtime
   if(strlen(szText) > 0) GDI_DrawText(ddsWindow,
      box->enabled? (box->active? RGB_WHITE : RGB_BLACK) : RGB_WINDOWBORDER,
      &rect, szText, TXT_FMT_CL);
}


// Process mouse down ////////////////////////////////////////////////////
static BOOL TxtBox_MouseDown(TEXTBOX *box, long x, long y)
{
   BOOL cWithin = Rect_Within(&box->rectScreen, x, y);
   
   // don't bother drawing if nothing has changed
   if(box->active != cWithin)
   {
      box->active = cWithin;
      TxtBox_Draw(box);
   }
   return cWithin;
}


// Process WM_CHAR ///////////////////////////////////////////////////////
static void TxtBox_Char(TEXTBOX *box, int letter)
{
   // if it is the delete key
   if(letter == 8) box->szText[strlen(box->szText) - 1] = NULL;

   // else it is a regular letter - also make sure we dont overflow the char array
   else if(strlen(box->szText) < 21 && letter != '\t')
      wsprintf(box->szText, "%s%c", box->szText, letter);

   // re-draw the box
   TxtBox_Draw(box);
}
