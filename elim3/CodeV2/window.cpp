/************************************************************************/
// window.cpp - Code for custom message boxes
//
// Author: Justin Hoffman
// Date:   6/9/00 - 7/18/2000
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "network.h"
#include "resdef.h"
#include "game.h"
#include "user.h"
#include "file.h"
#include "window.h"

//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// misc globals //////////////////////////////////////////////////////////
char  *gsTeamNameText[]    = {"No Team", "Team A", "Team B"};
char   WndStartNetChatText[5][80];
PDATB  WndStartNetPlrs[4];
bool   WndStartNetGotNames = 0;

// error list for connection /////////////////////////////////////////////
char *dpErrorListText[] = {
   "could not create game session.",
   "the software security package cannot be loaded.",
   "Security Support Provider Interface cannot be loaded.",
   "the connection was reset.",
   "invalid password.",
   "could not detect a network connection.",
   "this session is not accepting new players.",
   "no Elimination III sessions could be found.",
   "connection timed out.",
   "unknown error."
};

// error id list /////////////////////////////////////////////////////////
DWORD dpErrorList[] = {
   DPERR_CANNOTCREATESERVER,
   DPERR_CANTLOADSECURITYPACKAGE,
   DPERR_CANTLOADSSPI,
   DPERR_CONNECTIONLOST,
   DPERR_INVALIDPASSWORD,
   DPERR_NOCONNECTION,
   DPERR_NONEWPLAYERS,
   DPERR_NOSESSIONS,
   DPERR_TIMEOUT
};



//————————————————————————————————————————————————————————————————————————
// Generic Message Box
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static char *WndMsgBoxText;
static char *WndMsgBoxTitle;
static char *WndMsgBoxBtnA;
static char *WndMsgBoxBtnB;

// window wrapper ////////////////////////////////////////////////////////
long MsgBox(char* text, char* title, char* btna, char* btnb)
{
   // copy the texts
   WndMsgBoxText  = text;
   WndMsgBoxTitle = title;
   WndMsgBoxBtnA  = btna;
   WndMsgBoxBtnB  = btnb;

   // show the window
   return Wnd(WNDID_MSGBOX, WndMsgBox) - 1;
}

// message box loop //////////////////////////////////////////////////////
long WndMsgBox(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:

      if(WndMsgBoxBtnB == 0) {
         wnd->mbtns[1].state = BTN_INACTIVE;
         lstrcpy(wnd->mbtns[2].text, WndMsgBoxBtnA);
      } else {
         lstrcpy(wnd->mbtns[1].text, WndMsgBoxBtnA);
         lstrcpy(wnd->mbtns[2].text, WndMsgBoxBtnB);
      }

      lstrcpy(wnd->title, WndMsgBoxTitle);
      break;

   // the window needs to be drawn
   case WND_DRAW:
      RECT r;
      RSETW(r, wnd->xsurf + 7, wnd->ysurf + 35, 235, 83);
      FntDrawWrap(&gFntWndTxt, ddsWnd, WndMsgBoxText, &r);
      break;

   // one of the main buttons was used
   case WND_MAINBTN:
      WndClose(extra);
      return 1;
   }

   return 0;
}


//————————————————————————————————————————————————————————————————————————
// start a new game
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
UCHAR WndNewGamePlrsOn[4]   = {1, 0, 1, 0};
UCHAR WndNewGamePlrsType[4] = {0, 0, 0, 0};
UCHAR WndNewGamePlrsTeam[4] = {0, 0, 0, 0};

// main window loop //////////////////////////////////////////////////////
long WndNewGame(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:

      long i;

      // copy player names
      for(i = 0; i < 4; i++) lstrcpy(wnd->ptxts[i].text, pdat[i].sName);

      // load new game information
      IoSettingsNewGameInfo(1);

      // copy values into number controls
      wnd->pnums[0].value = gnSinkHoles;
      wnd->pnums[1].value = gnGames;

      // setup help button
      wnd->pbtns[1].extra = 4;
      wnd->pbtns[1].msg   = TXT_HELP;

      // get text lengths
      for(i = 0; i < 4; i++) wnd->ptxts[i].slen = lstrlen(wnd->ptxts[i].text);

      // copy the play text into the button text
      lstrcpy(wnd->pbtns[0].text, TXT_PLAY);
      break;

   // the window needs to be drawn
   case WND_DRAW:
      WndDrawGroup(TXT_PLAYEROPTIONS, 9, 39, 339, 166);
      WndDrawGroup(TXT_GAMEOPTIONS  , 9, 178, 339, 216);
      WndNewGameDrawPlayerInfo();
      break;

   // check for mouse input
   case WND_UPDATE:
      WndNewGameUpdate();
      break;

   // the play button was used
   case WND_CUSTBTN:
      if(extra == 0) {
         if(!WndNewGameValidateData()) return 0;
         IoSettingsNewGameInfo(0);
         WndNewGameTranslateData(wnd);
         gbNetwork = 0;
         WndClose(3);
      } else {
         WndClose(4);
      }

      return 1;

   // one of the main buttons was used
   case WND_MAINBTN:
      IoSettingsNewGameInfo(0);
      gbNetwork = extra == 0;
      WndClose(extra);
      return 1;
   }

   return 0;
}

// draw the player information ///////////////////////////////////////////
static void WndNewGameDrawPlayerInfo(void) 
{
   RECT r;
   long i;
   long y;

   // clear the back
   RSET(r, 16, 53, 162, 158);
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   for(i = 0; i < 4; i++) {

      // cashe value
      y = 26*i + 56;

      // draw the players "on/off" marble
      RSETW(r, 21*i, -21*WndNewGamePlrsOn[i] + 21, 21, 21);
      DDBltFast(ddsWnd, ddsMarbles, 20, y, &r, SRCCK);

      // draw the players type icon
      RSETWX(r, 0, 22*WndNewGamePlrsType[i], 19, 22);
      DDBltFast(ddsWnd, ddsPlrType, 58, y, &r, SRCCK);

      // draw the players team
      RSETWX(r, 87, y + 1, 71, 20);
      FntDrawCenter(&gFntWndTxt, ddsWnd, gsTeamNameText[ WndNewGamePlrsTeam[i] ], &r);
   }
}

// get the location of the item the mouse is within //////////////////////
static void WndNewGameUpdate(void)
{
   RECT r;
   long cashe;
   long x, y;
   bool click = CUR_CLICK;

   // re-draw the player information
   WndNewGameDrawPlayerInfo();

   // NOTE: window choordinates are offset by 146,112
   // get which column the mouse is near
   if     (cur_x > 164 && cur_x < 188) x = 0;
   else if(cur_x > 202 && cur_x < 223) x = 1;
   else if(cur_x > 232 && cur_x < 304) x = 2;
   else                                x = -1;

   // determine what row the mouse is near
   y = (long)((cur_y - 165) / 26);

   // make sure the position is valid
   if(x == -1 || y < 0 || y > 3 || cur_y < 165) return;

   // cashe common value
   cashe = 26*y + 55;

   // mouse is within player on/off button
   if(x == 0) {

      RSET(r, 73, 20, 96, 43);
      DDBltFast(ddsWnd, ddsItems, 19, cashe, &r, SRCCK);

      if(click) WndNewGamePlrsOn[y] = 1 - WndNewGamePlrsOn[y];

   // mouse is within player type button
   } else if(x == 1) {

      if(WndNewGamePlrsType[y] == 0) { RSET(r, 62, 43, 81, 67); }
      else                           { RSET(r, 81, 43, 102, 67);}
      DDBltFast(ddsWnd, ddsItems, 57, cashe, &r, SRCCK);

      if(click) {
         if(++WndNewGamePlrsType[y] > 1) WndNewGamePlrsType[y] = 0;
      }

   // mouse is within player team button
   } else {
      RSETWX(r, 87, cashe + 2, 71, 20);
      DDRect(ddsWnd, CLR_WHITE, 0, &r, DDRECT_NOFILL);

      if(click) {
         if(++WndNewGamePlrsTeam[y] > 2) WndNewGamePlrsTeam[y] = 0;
      }
   }
}

// make sure we can actually move ////////////////////////////////////////
static long WndNewGameValidateData(void)
{
   long   i;
   char  *msg          = 0;
   UCHAR  PlayerCount  = 0;
   UCHAR  TeamCount[3] = {0, 0, 0};

   // calculate the number of active players
   // and the number of players on each team
   for(i = 0; i < 4; i++) {
      if(WndNewGamePlrsOn[i]) {
         PlayerCount++;
         TeamCount[ WndNewGamePlrsTeam[i] ]++;
      }
   }

   // there arent enough players
   if(PlayerCount < 2)
      msg = TXT_NOTENOUGHPLRS;
   
   // there aren't enough players for teams
   else if(PlayerCount < 3 && (TeamCount[1] > 0 || TeamCount[2] > 0))
      msg = TXT_NOTENOUGHFORTEAMS;
   
   // not everyone has choosen a team
   else if(TeamCount[0] > 0 && (TeamCount[1] > 0 || TeamCount[2] > 0))
      msg = TXT_EVERYONENEEDSTEAM;
   
   // everyone is either on a single team
   else if(TeamCount[1] == PlayerCount || TeamCount[2] == PlayerCount)
      msg = TXT_ALLSAMETEAM;

   // if data is invalid, show a message box telling problem
   if(msg) MsgBox(msg, TXT_GENTITLE, TXT_OKAY, 0);
   return msg == 0;
}

// translate data for the game ///////////////////////////////////////////
static void WndNewGameTranslateData(WND *wnd)
{
   UCHAR i;
   UCHAR j[2] = {0, 0};

   // clear game data
   nplrs         = 0;
   gnTeamPlrs[0] = 0;
   gnTeamPlrs[1] = 0;
   memset(gTeams, 0, sizeof(gTeams));

   for(i = 0; i < 4; i++) {

      // copy player information
      lstrcpy(pdat[i].sName, wnd->ptxts[i].text);
      pdat[i].type      = WndNewGamePlrsType[i];
      pdat[i].team      = WndNewGamePlrsTeam[i] - 1;
      pdat[i].nGamesWon = 0;

      // copy player activation
      if(WndNewGamePlrsOn[i]) {

         // turn player on & increment number of players
         pdat[i].flags |= PLR_ON;
         nplrs++;

         // edit team data
         if(WndNewGamePlrsTeam[i] != 0) {
            gnTeamPlrs[ pdat[i].team ]++;
            gTeams[ pdat[i].team ][ j[ pdat[i].team ] ] = i;
            j[ pdat[i].team ]++;
         }
      } else pdat[i].flags &= ~PLR_ON;
   }

   // copy game data
   gnSinkHoles = (UCHAR)wnd->pnums[0].value;
   gnGames     = (UCHAR)wnd->pnums[1].value;
}


//————————————————————————————————————————————————————————————————————————
// connect to a network game
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static long WndConnectPasswordLen;


// connect to network game loop //////////////////////////////////////////
long WndConnect(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:

      // setup password stuff
      WndConnectPasswordLen = 0;
      dpPassword[0]         = TXT_NULLCHAR;
      wnd->ptxts[1].key     = WndConnectPasswordKeys;

      // setup the menu buttons
      wnd->pbtns[0].extra = 9;
      wnd->pbtns[1].extra = 5;
      wnd->pbtns[3].extra = 4;
      wnd->pbtns[0].msg   = TXT_PASTEIP;
      wnd->pbtns[1].msg   = TXT_MINIMIZEBTN;
      wnd->pbtns[3].msg   = TXT_HELP;

      // setup the LAN check button
      BSET(wnd->pbtns[2].flags, BTN_TOGGLE); 
      lstrcpy(wnd->pbtns[2].text, TXT_USINGLAN);

      // read saved information
      IoSettingsNetInfo(1);
      break;

   // the window needs to be drawn
   case WND_DRAW:

      bool down;
      down = BTN_DOWN(wnd->pbtns[2]);

      FntDraw(&gFntWndTxt, ddsWnd, down? TXT_SESSION : TXT_HOSTIP, 8, down? 28 : 33, 41);
      FntDraw(&gFntWndTxt, ddsWnd, TXT_PASSWORD, TXTLEN_PASSWORD, 21, 70);
      FntDraw(&gFntWndTxt, ddsWnd, TXT_USERNAME, TXTLEN_USERNAME,  9, 99);
      break;

   // one of the custom buttons was used
   case WND_CUSTBTN:
      if(extra == 0)      WndConnectPaste(wnd);       // paste button
      else if(extra == 1) prog_state = PROG_MINIMIZE; // minimize button
      else if(extra == 2) WndReDraw();                // using LAN button
      else {                                          // help button
         WndClose(2);
         return 1;
      }
      break;

   // one of the main buttons was used
   case WND_MAINBTN:

      // set the host & lan booleans
      dpHost   = extra == 1;
      dpUseLan = BTN_DOWN(wnd->pbtns[2]);

      // save info & copy hosthood & player name
      lstrcpy(dpThisPlrName, wnd->ptxts[2].text);
      IoSettingsNetInfo(0);

      // user canceled
      if(extra == 2) {
         WndClose(0);

      // start a net game
      } else {

         // make sure name is valid
         if(wnd->ptxts[2].text[0] == TXT_NULLCHAR) {
            MsgBox(TXT_ENTERSCREENNAME, TXT_GENTITLE, TXT_OKAY, 0);

         // make sure the session name is valid
         } else if(dpUseLan && wnd->ptxts[0].text[0] == TXT_NULLCHAR) {
            MsgBox(TXT_ENTERSESSIONNAME, TXT_GENTITLE, TXT_OKAY, 0);

         // attempt to connect
         } else WndConnectConnect(wnd);
      }
      return 1;
   }

   return 0;
}

// process character input to the password box ///////////////////////////
static long WndConnectPasswordKeys(TXT *txt)
{
   // if we need to re-draw password, re-set displayed text
   if(TxtCheckKeys(dpPassword, &WndConnectPasswordLen, 16)) {
      memset(txt->text, 0, sizeof(txt->text));
      memset(txt->text, '*', WndConnectPasswordLen);
      return 1;
   }

   return 0;
}

// draw a connecting message to the front buffer /////////////////////////
static void WndConnectDrawConnect(long host)
{
   RECT r;
   RSET(r, 232, 225, 407, 255);
   DDRect(ddsFront, CLR_BLACK, CLR_BLUE, &r, 0);
   REXP(r, -1);
   DDRect(ddsFront, CLR_WHITE, 0, &r, DDRECT_NOFILL);
   FntDrawCenter(&gFntTitle, ddsFront, host? TXT_HOSTING : TXT_CONNECTING, &r);
}

// paste data from the clipboard /////////////////////////////////////////
static void WndConnectPaste(WND *wnd)
{
   HANDLE hClipData;
   LPVOID pClipData;
   DWORD  nClipSize;

   // open the clipboard
   OpenClipboard(gHWnd);

   // make sure clip board data valid
   if((hClipData = GetClipboardData(CF_TEXT)) == 0) {
      MsgBox(TXT_INVALIDCLIPBOARD, TXT_GENERROR, TXT_OKAY, 0);
   } else {

      // get the size of the data
      nClipSize = GlobalSize(hClipData);
      if(nClipSize > 16) nClipSize = 16;

      // copy the text
      pClipData = GlobalLock(hClipData);
      memcpy(wnd->ptxts[0].text, pClipData, nClipSize);
      GlobalUnlock(hClipData);
      
      // re-draw the text box
      TxtDraw(&wnd->ptxts[0]);
   }

   // close the clipboard
   CloseClipboard();
}

// attempt the actual connection /////////////////////////////////////////
static void WndConnectConnect(WND *wnd)
{
   long errorId = 0;
   char sText[128];

   // draw a connecting message
   WndConnectDrawConnect(dpHost);

   // attempt to connect
   if(DpConnect(wnd->ptxts[0].text)) {

      // add the user name to the window text
      wsprintf(sText, "%s - %s", TXT_WINTITLE, dpThisPlrName);
      SetWindowText(gHWnd, sText);

      // clear the chat text & player data
      memset(WndStartNetChatText, 0, sizeof(WndStartNetChatText));
      memset(WndStartNetPlrs    , 0, sizeof(WndStartNetPlrs));

      // clear the chat text
      memset(gsChatText, 0, sizeof(gsChatText));
      memset(gsChatSend, 0, sizeof(gsChatSend));
      gsChatSendLen = 0;

      // set first game & got names booleans & close the window
      WndStartNetGotNames  = dpHost;
      WndStartNetFirstGame = 1;
      WndClose(1);

   // failed to connect to game
   } else {

      // determine which error it was
      while(dpErrorList[errorId] != dpError && errorId < (sizeof(dpErrorList) / sizeof(dpErrorList[0]))) errorId++;

#ifdef GAME_DEBUG
      DebugWrite("Error connecting/hosting: %d", dpError);
#endif

      // create & display the error string
      wsprintf(sText, "%s %s", dpHost? TXT_COULDNOTHOST : TXT_COULDNOTJOIN, dpErrorListText[errorId]);
      MsgBox(sText, TXT_NETERROR, TXT_OKAY, 0);
   }
}


//————————————————————————————————————————————————————————————————————————
// start a network game
//————————————————————————————————————————————————————————————————————————

// global varables ///////////////////////////////////////////////////////
static char WndStartNetChatSend[64];
static long WndStartNetSendLen    = 0;
bool        WndStartNetFirstGame  = 1;
bool        WndStartNetShowWinner = 0;
UCHAR       WndStartNetNPlrs      = 1;

// start network game loop ///////////////////////////////////////////////
long WndStartNet(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:
      WndStartNetInit(wnd);
      break;

   // a window loop is to be processed
   case WND_UPDATE:
      WndStartNetUpdate(wnd);
      break;

   // the window needs to be drawn
   case WND_DRAW:
      WndStartNetDraw();
      WndStartNetDrawChat();
      WndStartNetDrawSend();
      WndStartNetDrawPlrs();
      break;

   // one of the custom buttons was used
   case WND_CUSTBTN:
      switch(extra) {
      case 0: WndStartNetCopy();          break; // copy the IP
      case 1: prog_state = PROG_MINIMIZE; break; // minimize the game

      // change this players ready state
      case 2:
         gNetData[0] = NETMSG_READYPLR;
         gNetData[1] = BTN_DOWN(wnd->pbtns[2]);
         DpSend(gNetData, sizeof(gNetData));
         WndStartNetReadyPlr(gThisPlr, gNetData[1]? 1 : 0);
         break;
      }
      break;

   // one of the main buttons was used
   case WND_MAINBTN:

      // get num data values
      gnSinkHoles = (UCHAR)wnd->pnums[0].value;
      gnGames     = (UCHAR)wnd->pnums[1].value;

      // get the game going
      switch(extra) {
      
      // play button (host only)
      case 0:
         if(WndStartNetValidateTeams()) {
            DpLock(1);
            WndStartNetStart();
            GameSendNewGame();
            WndClose(1);
            return 1;
         }
         break;

      // exit application (notice fall trough)
      case 2:
         prog_state = PROG_CLOSE;

      // exit direct play session
      case 1:
         DpQuit();
         WndClose(0);
         return 1;
      }
   }

   return 0;
}

// initlize the window ///////////////////////////////////////////////////
static void WndStartNetInit(WND *wnd)
{
   // initilize chat varables
   WndStartNetNPlrs    = 1;
   memset(WndStartNetChatSend, 0, sizeof(WndStartNetChatSend));

   // setup the first player
   if(!WndStartNetGotNames || dpHost) {
      gThisPlr                  = 0;
      WndStartNetPlrs[0].ready  = 1;
      WndStartNetPlrs[0].active = 1;
      WndStartNetPlrs[0].team   = 0;
      WndStartNetPlrs[0].dpId   = dpThisPlrId;
      lstrcpy(WndStartNetPlrs[0].sName, dpThisPlrName);
   }

   // re-list all active players
   if(!WndStartNetFirstGame && dpHost) WndStartNetReList();

   // make sure the session is unlocked
   DpLock(0);

   // set the menu buttons up
   wnd->pbtns[0].extra = 9;
   wnd->pbtns[1].extra = 5;

   // attach messages to the menu buttons
   wnd->pbtns[0].msg = dpUseLan? TXT_COPYSESSION : TXT_COPYIP;
   wnd->pbtns[1].msg = TXT_MINIMIZEBTN;

   // setup the ready button
   wnd->pbtns[2].flags = BTN_TOGGLE;
   lstrcpy(wnd->pbtns[2].text, TXT_READY);

   // setup number controls
   wnd->pnums[0].value = gnSinkHoles;
   wnd->pnums[1].value = gnGames;

   // disable buttons based on hosthood
   if(dpHost) {
      wnd->mbtns[0].state = BTN_DISABLED;
      wnd->pbtns[0].state = BTN_IDLE;
      wnd->pbtns[2].state = BTN_INACTIVE;
   } else {
      wnd->mbtns[0].state = BTN_INACTIVE;
      wnd->pbtns[0].state = BTN_INACTIVE;
      wnd->pbtns[2].state = BTN_IDLE;
   }

   // set common values to both host and not host
   wnd->pbtns[1].state    = wnd->pbtns[0].state;
   wnd->pnums[0].disabled = !dpHost;
   wnd->pnums[1].disabled = !dpHost;

   // clear first game varable
   WndStartNetFirstGame = 0;
}

// update the window /////////////////////////////////////////////////////
static void WndStartNetUpdate(WND *wnd)
{
   long i, y;
   bool w;
   RECT r;

   // display game winner
   if(WndStartNetShowWinner) {
      WndStartNetAddText(4, gsWinText);
      WndStartNetShowWinner = 0;
   }

   // check the chat keys
   if(TxtCheckKeys(WndStartNetChatSend, &WndStartNetSendLen, 64) || gCurChanged)
      WndStartNetDrawSend();

   // check to see if chat text should be sent
   if(KEY_CLICK(DIK_RETURN) && WndStartNetSendLen > 0) {

      // send the chat message
      WndStartNetSendChat();

      // add the text to this players window
      WndStartNetAddText(gThisPlr, WndStartNetChatSend);
      WndStartNetChatSend[0] = TXT_NULLCHAR;
      WndStartNetSendLen     = 0;
      WndStartNetDrawSend();
   }

   // check to see if we have changed the number of sinkholes
   if(BTN_USED(wnd->pnums[0].btns[0]) || BTN_USED(wnd->pnums[0].btns[1])) {
      gNetData[0] = NETMSG_CHANGESINK;
      gNetData[1] = (BYTE)wnd->pnums[0].value;
      DpSend(gNetData, sizeof(gNetData));
   }

   // check to see if we have changed the number of games
   if(BTN_USED(wnd->pnums[1].btns[0]) || BTN_USED(wnd->pnums[1].btns[1])) {
      gNetData[0] = NETMSG_CHANGEGAMES;
      gNetData[1] = (BYTE)wnd->pnums[1].value;
      DpSend(gNetData, sizeof(gNetData));
   }

   // make sure we have a player list first
   if(!WndStartNetGotNames) return;

   // NOTE: window chordinates offset by 86, 124
   // determine if the user is pointing at their team name
   i = WndStartNetNPlrs == 2 && gThisPlr == 2? 1 : gThisPlr;
   y = 23*i + 172;
   w = cur_x > 472 && cur_x < 540 && cur_y > y && cur_y < y + 21;

   // draw the rectangle around the team name
   RSETWX(r, 387, y - 124, 67, 21);
   DDRect(ddsWnd, w? CLR_LBLUE : CLR_DTAN, 0, &r, DDRECT_NOFILL);

   // the cursor was clicked within the box
   if(CUR_CLICK && w) {

      // change this players team
      if(++WndStartNetPlrs[gThisPlr].team > 2) WndStartNetPlrs[gThisPlr].team = 0;

      // send the message
      gNetData[0] = NETMSG_CHANGETEAM;
      gNetData[1] = WndStartNetPlrs[gThisPlr].team;
      DpSend(gNetData, sizeof(gNetData));

      // re-draw the player area
      WndStartNetDrawPlrs();
   }
}

// draw main window parts ////////////////////////////////////////////////
static void WndStartNetDraw(void)
{
   char sText[32];

   // draw the groups
   WndDrawGroup(TXT_CHAT       , 9  , 39 , 259, 144);
   WndDrawGroup(TXT_PLAYERS    , 267, 39 , 460, 144);
   WndDrawGroup(TXT_GAMEOPTIONS, 9  , 156, 322, 193);

   // draw the lines above chat send text
   DDLine(ddsWnd, CLR_DTAN, 12, 121, 256, 121);
   DDLine(ddsWnd, CLR_DTAN, 12, 123, 256, 123);

   // draw the hosts IP
   if(dpHost) {
      if(dpUseLan) wsprintf(sText, TXT_DISPLAYSESSION, dpSessionName);
      else         wsprintf(sText, TXT_DISPLAYIP     , dpThisPlrIp); 
      FntDraw(&gFntWndTxt, ddsWnd, sText, lstrlen(sText), 8, 212);
   }
}

// draw the chat area ////////////////////////////////////////////////////
static void WndStartNetDrawChat(void)
{
   long i;

   // clear the back
   RECT r = {13, 45, 258, 120};
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw the text
   for(i = 0; i < 5; i++)
      FntDrawClip(&gFntSmall, ddsWnd, WndStartNetChatText[i], 14, 14*i + 50, 256);
}

// draw the send chat text ///////////////////////////////////////////////
static void WndStartNetDrawSend(void)
{
   long x;
   RECT r = {14, 129, 258, 141};
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);
   x = FntDrawClip(&gFntSmall, ddsWnd, WndStartNetChatSend, r.left, r.top, 256);
   if(gCurVisible) FntDrawClip(&gFntSmall, ddsWnd, TXT_CURCHAR, x, r.top, 256);
}

// copy ip to clipboard //////////////////////////////////////////////////
static void WndStartNetCopy(void)
{
   HANDLE hMem;
   LPVOID pMem;
   long   len;

   // copy the data into a global memory place
   len  = lstrlen(dpThisPlrIp) + 1;
   hMem = GlobalAlloc(GHND, len);
   pMem = GlobalLock(hMem);
   memcpy(pMem, dpUseLan? dpSessionName : dpThisPlrIp, len);
   GlobalUnlock(hMem);

   // set the clipboard data
   OpenClipboard(gHWnd);
   SetClipboardData(CF_TEXT, hMem);
   CloseClipboard();
}

// send a chat message ///////////////////////////////////////////////////
static void WndStartNetSendChat(void)
{
   BYTE *data;

   // create the message
   data    = (BYTE*)MoreMem(WndStartNetSendLen + 2);
   data[0] = NETMSG_CHATA;

   // copy the chat text & add a null terminator
   memcpy(data + 1, WndStartNetChatSend, WndStartNetSendLen);
   data[WndStartNetSendLen + 1] = TXT_NULLCHAR;

   // send the data
   DpSend(data, WndStartNetSendLen + 2);
   LessMem(data);
}

// make sure we can actually move ////////////////////////////////////////
static long WndStartNetValidateTeams(void)
{
   long   i;
   char  *msg          = 0;
   UCHAR  PlayerCount  = 0;
   UCHAR  TeamCount[3] = {0, 0, 0};

   // calculate the number of active players
   // and the number of players on each team
   for(i = 0; i < 4; i++) {
      if(WndStartNetPlrs[i].active) {
         PlayerCount++;
         TeamCount[ WndStartNetPlrs[i].team ]++;
      }
   }

   // there aren't enough players for teams
   if(PlayerCount < 3 && (TeamCount[1] > 0 || TeamCount[2] > 0))
      msg = TXT_NOTENOUGHFORTEAMS;

   // not everyone has choosen a team
   else if(TeamCount[0] > 0 && (TeamCount[1] > 0 || TeamCount[2] > 0))
      msg = TXT_EVERYONENEEDSTEAM;

   // everyone is either on a single team
   else if(TeamCount[1] == PlayerCount || TeamCount[2] == PlayerCount)
      msg = TXT_ALLSAMETEAM;

   // if we have made a message, display it
   if(msg) WndStartNetAddText(4, msg);
   return msg == 0;
}

// draw the player info //////////////////////////////////////////////////
void WndStartNetDrawPlrs(void)
{
   RECT r;
   long i, j, y;

   // clear the players area
   RSET(r, 272, 46, 457, 138);
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw loading text
   if(!WndStartNetGotNames) {
      FntDrawCenter(&gFntWndTxt, ddsWnd, TXT_GETTINGPLRLIST, &r);
      return;
   }

   // draw the player information
   for(i = j = 0; i < 4; i++) {
      if(WndStartNetPlrs[i].active) {

         y = 23*j + 54;
      
         // draw the sphere next to their name
         if(WndStartNetPlrs[i].ready) r.top = 9*i;
         else                         r.top = 10*i + 37;

         r.left   = 0;
         r.bottom = r.top + 10;
         r.right  = 10;

         DDBltFast(ddsWnd, ddsItems, 278, y, &r, SRCCK);

         // draw their name
         FntDraw(&gFntWndTxt, ddsWnd, WndStartNetPlrs[i].sName,
            lstrlen(WndStartNetPlrs[i].sName), 295, y);

         // draw their team
         RSETWX(r, 387, 23*j + 48, 67, 21);
         FntDrawCenter(&gFntWndTxt, ddsWnd, gsTeamNameText[ WndStartNetPlrs[i].team ], &r);

         // draw a rectangle around their team (if it's this players)
         if(i == gThisPlr) DDRect(ddsWnd, CLR_DTAN, 0, &r, DDRECT_NOFILL);

         j++;
      }
   }
}

// add chat text /////////////////////////////////////////////////////////
void WndStartNetAddText(UCHAR p, char *sText)
{
   // move the chat text up
   for(long i = 0; i < 4; i++)
      lstrcpy(WndStartNetChatText[i], WndStartNetChatText[i+1]);
  
   // add the new line
   if(p == 4) wsprintf(WndStartNetChatText[4], TXT_SYSTEMCHAT, sText);
   else       wsprintf(WndStartNetChatText[4], TXT_PLAYERCHAT, WndStartNetPlrs[p].sName, sText);

   // re-draw the chat area
   if(gbVisible && gbMsgBox) WndStartNetDrawChat();
}

// a player has changed their ready status ///////////////////////////////
void WndStartNetReadyPlr(UCHAR p, bool ready)
{
   long i;
   long nReady = 0;
   bool bReady = 1;

   // set the ready state of the player
   WndStartNetPlrs[p].ready = ready;

   // re-draw the names
   if(gbVisible) WndStartNetDrawPlrs();

   // if host, then check to see if the game can start
   if(dpHost) {

      // determine if ALL the ACTIVE players are ready
      for(i = 0; i < 4; i++) {
         if(WndStartNetPlrs[i].active) {
            bReady = (bReady && WndStartNetPlrs[i].ready);
            nReady++;
         }
      }

      // enable the play button
      BtnEnable(&gWndA.mbtns[0], bReady && nReady > 1);
   }
}

// change a players team /////////////////////////////////////////////////
void WndStartNetChangeTeam(UCHAR p, UCHAR team)
{
   WndStartNetPlrs[p].team = team;
   if(gbVisible) WndStartNetDrawPlrs();
}

// re-list the players ///////////////////////////////////////////////////
void WndStartNetReList(void)
{
   BYTE *data;

   // get new data
   DpListPlayers();
   WndStartNetNPlrs = (UCHAR)dpCurEnumPlr;

   // create the message to send
   data    = (BYTE*)MoreMem(sizeof(WndStartNetPlrs) + 4);
   data[0] = NETMSG_RELISTPLRS;

   // send the message to re-list
   memcpy((data + 1), WndStartNetPlrs, sizeof(WndStartNetPlrs));
   data[ sizeof(WndStartNetPlrs) + 2 ] = (BYTE)gWndA.pnums[0].value;
   data[ sizeof(WndStartNetPlrs) + 3 ] = (BYTE)gWndA.pnums[1].value;

   DpSend(data, sizeof(WndStartNetPlrs) + 4);
   LessMem(data);

   // re-draw the player list
   if(gbMsgBox && gbVisible) WndStartNetDrawPlrs();
}

// start the game ////////////////////////////////////////////////////////
void WndStartNetStart(void)
{
   UCHAR i, j[2] = {0, 0};

   // copy the number of players
   nplrs = WndStartNetNPlrs;

   // clear team data
   gnTeamPlrs[0] = 0;
   gnTeamPlrs[1] = 0;
   memset(gTeams, 0, sizeof(gTeams));

   // translate data
   for(i = 0; i < 4; i++) {

      // copy direct play ID and team
      pdat[i].dpId      = WndStartNetPlrs[i].dpId;
      pdat[i].team      = WndStartNetPlrs[i].team - 1;
      pdat[i].type      = 0;
      pdat[i].nGamesWon = 0;

      // edit team data
      if(pdat[i].team >= 0) {
         gnTeamPlrs[ pdat[i].team ]++;
         gTeams[ pdat[i].team ][ j[ pdat[i].team ] ] = i;
         j[ pdat[i].team ]++;
      }

      // copy name
      lstrcpy(pdat[i].sName, WndStartNetPlrs[i].sName);

      // set option flag
      BSETB(pdat[i].flags, PLR_ON, WndStartNetPlrs[i].active);

      // record current player ID
      if(pdat[i].dpId == dpThisPlrId) gThisPlr = (UCHAR)i;
   }
}


//————————————————————————————————————————————————————————————————————————
// load a game
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static long  WndLoadSelected   = 0;
static long  WndLoadListStart  = 0;
static long  WndLoadListLength = 0;
static char *WndLoadData       = 0;

// load game loop ////////////////////////////////////////////////////////
long WndLoad(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {
   
   // initlize window information
   case WND_INIT:

      // load the game information
      WndLoadData = IoGameGetInfo(&WndLoadListLength);

      // set extra numbers for up & down arrows
      wnd->pbtns[0].extra = 0;
      wnd->pbtns[1].extra = 1;

      // disable load & delete buttons if there are no games
      if(WndLoadListLength == 0) {
         wnd->mbtns[0].state = BTN_DISABLED;
         wnd->mbtns[1].state = BTN_DISABLED;
      }

      // disable scroll buttons if there arent enough games
      wnd->pbtns[1].state = (WndLoadListLength < 8 || WndLoadListStart == WndLoadListLength - 7)? BTN_DISABLED : BTN_IDLE;

      // disable the top scroll button cause we at top of list
      wnd->pbtns[0].state = (WndLoadListStart == 0)? BTN_DISABLED : BTN_IDLE;

   // draw "client" area of the window
   case WND_DRAW:
      WndLoadDraw();
      WndLoadDrawGames(wnd);
      break;

   // see if the user clicked on an item in the list
   // window chorrdinates offset by: x = 88, y = 132
   case WND_UPDATE:
      if(CUR_CLICK && cur_x > 97 && cur_x < 529) {

         long max, y;

         max = WndLoadListLength > 7? 7 : WndLoadListLength; 
         y   = (long)((cur_y - 186) / 17);

         if(y >= 0 && y < max) {
            WndLoadSelected = WndLoadListStart + y;
            WndLoadDrawGames(wnd);
         }
      }

      break;

   // one of the custom buttons were used
   case WND_CUSTBTN:

      // increment list start
      if(extra == 0) WndLoadListStart--;
      else           WndLoadListStart++;

      // disable/enable top & bottom scroll buttons if we can move no further
      wnd->pbtns[0].state = WndLoadListStart == 0? BTN_DISABLED : BTN_IDLE;
      wnd->pbtns[1].state = WndLoadListStart == WndLoadListLength - 7? BTN_DISABLED : BTN_IDLE;

      // re-draw the list
      WndLoadDrawGames(wnd);
      break;

   // one of the main buttons were used
   case WND_MAINBTN:

      // get loaded file name
      if(extra == 0) wsprintf(FILE_LOADED, FILE_SAVEDGAME, WndLoadData + 80*WndLoadSelected);

      // delete game information
      if(extra != 1) DELETE_ARRAY(WndLoadData);

      switch(extra) {
      case 0: WndClose(WndLoadSelected);               break; // load file
      case 1: WndLoadDeleteFile(wnd, WndLoadSelected); break; // delete
      case 2: WndClose(-1);                            break; // cancel
      }

      return 1;
   }

   return 0;
}

// draw the main window stuff ////////////////////////////////////////////
static void WndLoadDraw(void)
{
   RECT r;
   long i;

   // draw file name text
   FntDraw(&gFntSmall, ddsWnd, TXT_FILENAME, 9, 14, 35);

   // draw the player marbles
   for(i = 0; i < 4; i++) {
      RSETWX(r, 0, 9*i, 10, 10);
      DDBltFast(ddsWnd, ddsItems, 86*i + 133, 34, &r, SRCCK);
   }
}

// draw the game data ////////////////////////////////////////////////////
static void WndLoadDrawGames(WND *wnd)
{
   RECT r;
   long i, j;
   long y, z, end;

   // fill the background
   RSET(r, 9, 52, 456, 176); 
   DDRect(ddsWnd, CLR_DTAN, CLR_LTAN, &r, 0);

   // draw the back of the scroll bar
   RSET(r, 441, 54, 454, 174);
   DDRect(ddsWnd, CLR_DTAN, 0, &r, DDRECT_NOFILL);
   
   // draw the scroll bar arrows
   BtnDraw(&wnd->pbtns[0]);
   BtnDraw(&wnd->pbtns[1]);

   // if there are not saved games,
   // display a message saying so
   if(WndLoadListLength <= 0) {
      RSET(r, 9, 52, 456, 176);
      FntDrawCenter(&gFntWndTxt, ddsWnd, TXT_NOSAVEDGAMES, &r);
      return;
   }

   // draw the selected line
   y = WndLoadSelected - WndLoadListStart;

   if(y >= 0 && y < 7) {
      RSETWX(r, 11, 17*y + 54, 429, 18);
      DDRect(ddsWnd, CLR_LBLUE, CLR_BTNPUSHED, &r, 0);
   }

   // set top & bottom dimensions of rect
   r.top    = 54;
   r.bottom = 72;

   // determine how many lines need drawn
   end = WndLoadListLength < 7? WndLoadListLength : 7;

   // draw the data items
   for(i = 0; i < end; i++) {

      // cache common values
      y = 17*i + 59;
      z = 5*16*(i+WndLoadListStart);

      // draw game title
      FntDraw(&gFntSmall, ddsWnd, WndLoadData + z, lstrlen(WndLoadData + z), 15, y);

      // reset left & right dimensions of rect
      r.left  = 94;
      r.right = 181;

      // draw player names
      for(j = 0; j < 4; j++) {
         FntDrawCenter(&gFntSmall, ddsWnd, WndLoadData + 16*j + 16 + z, &r);
         ROFSTX(r, 86);
      }

      ROFSTY(r, 17);
   }
}

// delete a file /////////////////////////////////////////////////////////
static void WndLoadDeleteFile(WND *wnd, long index)
{
   char sFile[32];

   // remove the file
   SetCurrentDirectory(gszAppPath);
   wsprintf(sFile, FILE_SAVEDGAME, WndLoadData + 80*index);
   DeleteFile(sFile);

   // change selected item if the bottom one was selected
   WndLoadSelected  -= (WndLoadSelected == WndLoadListLength - 1);
   WndLoadListStart -= (WndLoadListLength > 7 && WndLoadListStart >= WndLoadListLength - 7);

   // clear the list & re-initlize it
   DELETE_ARRAY(WndLoadData);
   WndLoad(wnd, WND_INIT, 0);
   WndReDraw();
}


//————————————————————————————————————————————————————————————————————————
// save a game
//————————————————————————————————————————————————————————————————————————

// save game loop ////////////////////////////////////////////////////////
long WndSaveAs(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // "client" area needs to be drawn
   case WND_DRAW:
      FntDraw(&gFntWndTxt, ddsWnd, TXT_SAVEGAMEAS, TXTLEN_SAVEGAMEAS, 63, 55);
      break;

   // one of the main buttons was used
   case WND_MAINBTN:

      if(extra == 1) {
         wsprintf(FILE_LOADED, FILE_SAVEDGAME, wnd->ptxts[0].text);
         WndClose(1);
      } else {
         WndClose(0);
      }

      return 1;
   }

   return 0;
}


//————————————————————————————————————————————————————————————————————————
// help window
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static char   WndHelpTitle[32] = "";
static char   WndHelpNext      = -1;
static char   WndHelpPrev      = -1;
static ULONG  WndHelpTextLen   = 0;
static UCHAR  WndHelpNImages   = 0;
static char  *WndHelpText      = 0;
static UCHAR *WndHelpImageIds  = 0;
static POINT *WndHelpImages    = 0;

// help wrapper //////////////////////////////////////////////////////////
void Help(char topic)
{
   // pause the game if we are not in network mode
   if(!gbNetwork) GamePause();

   // load the topic & show the window
   WndHelpLoadTopic(topic);
   Wnd(WNDID_HELP, WndHelp);

   // resume the game
   if(!gbNetwork) GameResume();
}

// help loop /////////////////////////////////////////////////////////////
long WndHelp(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {
   
   // initilize the window
   case WND_INIT:
      wsprintf(wnd->title, WndHelpTitle);
      wnd->mbtns[0].state = (WndHelpPrev < 0? BTN_DISABLED : BTN_IDLE);
      wnd->mbtns[1].state = (WndHelpNext < 0? BTN_DISABLED : BTN_IDLE);
      break;

   // draw the window
   case WND_DRAW:

      RECT  r;
      UCHAR i;

      // draw the images
      for(i = 0; i < WndHelpNImages; i++) {
         XmpLoad(WndHelpImageIds[i]);
         XmpCopy(ddsWnd, WndHelpImages[i].x + wnd->xsurf, WndHelpImages[i].y + wnd->ysurf + 28);
         XmpDelete();
      }

      // draw the text
      RSETW(r, wnd->xsurf + 7, wnd->ysurf + 34, 443, 176);
      FntDrawWrap(&gFntWndTxt, ddsWnd, WndHelpText, &r);
      break;

   // one of the main buttons was pressed
   case WND_MAINBTN:

      // release help data
      LessMem(WndHelpText);

      // release image data
      if(WndHelpNImages > 0) {
         LessMem(WndHelpImages);
         LessMem(WndHelpImageIds);
      }

      // load the next/prev topic
      if(extra == 0 || extra == 1) {
         WndHelpLoadTopic(extra == 0? WndHelpPrev : WndHelpNext);
         WndHelp(wnd, WND_INIT, 0);
         WndDraw(wnd);
      }

      // close button was used
      if(extra == 2) {
         WndClose(0);
         return 1;
      }

      break;
   }
   return 0;
}

// Load a help topic /////////////////////////////////////////////////////
static void WndHelpLoadTopic(char topic)
{
   // open topic data
   ResOpen(HELP_MAIN + topic);

   // read topic header
   ResRead(WndHelpTitle   , sizeof(WndHelpTitle));
   ResRead(&WndHelpNext   , sizeof(WndHelpNext));
   ResRead(&WndHelpPrev   , sizeof(WndHelpPrev));
   ResRead(&WndHelpTextLen, sizeof(WndHelpTextLen));
   ResRead(&WndHelpNImages, sizeof(WndHelpNImages));

   // load image data
   if(WndHelpNImages > 0) {

      // allocate space for image data
      WndHelpImages   = (POINT*)MoreMem(WndHelpNImages*sizeof(POINT));
      WndHelpImageIds = (UCHAR*)MoreMem(WndHelpNImages);

      // read the data
      for(UCHAR i = 0; i < WndHelpNImages; i++) {
         ResRead(&WndHelpImageIds[i], sizeof(UCHAR));
         ResRead(&WndHelpImages[i].x, sizeof(long));
         ResRead(&WndHelpImages[i].y, sizeof(long));
      }
   }

   // read text data
   WndHelpText = (char*)MoreMem(WndHelpTextLen);
   ResRead(WndHelpText, WndHelpTextLen);
   ResClose();
}


//————————————————————————————————————————————————————————————————————————
// Music Options Window
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static char WndMusicOptionsList[MUSIC_MAXCD][16];
static long WndMusicOptionsListStart;
static long WndMusicOptionsSelected;

// main loop /////////////////////////////////////////////////////////////
long WndMusicOptions(WND *wnd, UCHAR msg, UCHAR extra)
{ 
   long i;

   switch(msg) {

   // initlize the window
   case WND_INIT:

      // load song list
      WndMusicOptionsLoadList();

      // set the music type button names
      lstrcpy(wnd->pbtns[0].text, TXT_STANDARDMIDI);
      lstrcpy(wnd->pbtns[1].text, TXT_DIRECTMUSIC);
      lstrcpy(wnd->pbtns[2].text, TXT_CDMUSIC);
      lstrcpy(wnd->pbtns[3].text, TXT_NOMUSIC);

      // set the extra value in the up & down arrows
      wnd->pbtns[9].extra  = 0;
      wnd->pbtns[10].extra = 1;

      // set all the toggle buttons
      for(i = 0; i < 8; i++) wnd->pbtns[i].flags |= BTN_TOGGLE;
      
      // put a check by the current music type
      wnd->pbtns[mMusicType].flags |= BTN_TDOWN;

      // disable unabalable formats
      if(!gbMciAval)         wnd->pbtns[0].state = BTN_DISABLED;
      if(!gbDirectMusicAval) wnd->pbtns[1].state = BTN_DISABLED;
      if(!gbCdAudioAval)     wnd->pbtns[2].state = BTN_DISABLED;

      // update the song list
      WndMusicOptionsUpdateList(wnd);
      break;

   // draw the window
   case WND_DRAW:
      WndMusicOptionsDrawList(wnd);
      FntDraw(&gFntWndTxt, ddsWnd, TXT_PLAYSELECTED, TXTLEN_PLAYSELECTED, 162, 117);
      break;

   // see if the user clicked on song list
   // screen chords offset by 164, 156
   case WND_UPDATE:
      if(CUR_CLICK && cur_x > 301 && cur_x < 448) {

         i = (long)((cur_y - 196) / 17);

         if(i >= 0 && i < 4) {
            WndMusicOptionsSelected = WndMusicOptionsListStart + i;
            WndMusicOptionsDrawList(wnd);
         }
      }
      break;

   // a custom button was used
   case WND_CUSTBTN:
      WndMusicOptionsCustomBtn(wnd, extra);
      break;

   // one the main button was used
   case WND_MAINBTN:
      IoSettingsSongInfo(0);
      MusicReList();
      WndClose(0);
      return 1;
   }

   return 0;
}

// a custom button was used //////////////////////////////////////////////
static void WndMusicOptionsCustomBtn(WND *wnd, UCHAR extra)
{
   long i;
   long j;

   // one of the music type buttons was used
   if(extra < 4) {

      // set the check marks of the buttons
      for(i = 0; i < 4; i++) {
         if(i == extra) wnd->pbtns[i].flags |=  BTN_TDOWN;
         else           wnd->pbtns[i].flags &= ~BTN_TDOWN;
         BtnDraw(&wnd->pbtns[i]);
      }

      // if the same music type was picked, quit now
      if(mMusicType == extra) return;

      // change the music type
      IoSettingsSongInfo(0);
      MusicChangeType(extra);
      WndMusicOptionsLoadList();
      WndMusicOptionsUpdateList(wnd);
      WndMusicOptionsDrawList(wnd);

      // set the music on boolean
      gbMusicOn = extra != 3;

   // one of the song on/off buttons was used
   } else if(extra < 8) {

      // count the number of active songs
      for(i = j = 0; i < mNumSongs; i++) if(mSongFlags[i] & MUSIC_SONGACTIVE) j++;

      // figure out which song to edit
      i = WndMusicOptionsListStart + extra - 4;
 
      if(BTN_DOWN(wnd->pbtns[extra])) {
         mSongFlags[i] |=  MUSIC_SONGACTIVE;
      } else {

         // not enough songs to be removing the last one
         if(j < 2) {
            wnd->pbtns[extra].flags |= BTN_TDOWN;
            MsgBox(TXT_MUSTHAVEONESONG, TXT_GENTITLE, TXT_OKAY, 0);
         } else mSongFlags[i] &= ~MUSIC_SONGACTIVE;
      }

   // the play song button was used
   } else if(extra == 8) {

      if(mSongFlags[WndMusicOptionsSelected] & MUSIC_SONGAVALABLE)
         MusicPlaySong((UCHAR)(WndMusicOptionsSelected + 1));

   // one of the scroll buttons was used
   } else {

      // increment list start
      if(extra == 9) WndMusicOptionsListStart--;
      else           WndMusicOptionsListStart++;

      // update the list
      WndMusicOptionsUpdateList(wnd);
      WndMusicOptionsDrawList(wnd);
   }
}

// move the list up or down //////////////////////////////////////////////
static void WndMusicOptionsUpdateList(WND *wnd)
{
   long i;
   long y;
   long z;

   // update the song on/off buttons
   for(i = 0; i < 4; i++) {

      // cashe common values
      y = i + WndMusicOptionsListStart;
      z = i + 4;
 
      // set the toggle & enabled states
      if(mSongFlags[y] & MUSIC_SONGAVALABLE) {
         wnd->pbtns[z].state = BTN_IDLE;

         if(mSongFlags[y] & MUSIC_SONGACTIVE) wnd->pbtns[z].flags |=  BTN_TDOWN;
         else                                 wnd->pbtns[z].flags &= ~BTN_TDOWN;

      } else {
         wnd->pbtns[z].state  = BTN_DISABLED;
         wnd->pbtns[z].flags &= ~BTN_TDOWN;
      }
   }

   // disable/enable top & bottom scroll buttons if we can move no further
   wnd->pbtns[9].state  = WndMusicOptionsListStart == 0? BTN_DISABLED : BTN_IDLE;
   wnd->pbtns[10].state = WndMusicOptionsListStart == mNumSongs - 4? BTN_DISABLED : BTN_IDLE;
}

// draw the song list ////////////////////////////////////////////////////
static void WndMusicOptionsDrawList(WND *wnd)
{
   long i;
   long y;
   RECT r;

   // draw the border
   RSET(r, 137, 38, 299, 107);
   DDRect(ddsWnd, CLR_DTAN, CLR_LTAN, &r, 0);

   // draw the scroll bar border
   RSET(r, 284, 40, 297, 105);
   DDRect(ddsWnd, CLR_DTAN, 0, &r, DDRECT_NOFILL);

   // draw the selected item
   y = WndMusicOptionsSelected - WndMusicOptionsListStart;

   if(y >= 0 && y < 4) {
      RSETWX(r, 139, 17*y + 40, 144, 14);
      DDRect(ddsWnd, CLR_LBLUE, CLR_BTNPUSHED, &r, 0);
   }

   // update the song on/off buttons
   for(i = 0; i < 4; i++) {

      // cashe common values
      y = i + WndMusicOptionsListStart;
     
      // draw the text
      FntDraw(&gFntSmall, ddsWnd, WndMusicOptionsList[y],
         lstrlen(WndMusicOptionsList[y]), 155, 17*i + 43);
   }

   // re-draw the buttons
   for(i = 4; i < 11; i++) BtnDraw(&wnd->pbtns[i]);
}

// load the song list ////////////////////////////////////////////////////
static void WndMusicOptionsLoadList(void)
{
   long i;

   // set the list position
   WndMusicOptionsSelected  = mThisSong - 1;
   WndMusicOptionsListStart = WndMusicOptionsSelected;

   // make sure we dont excede the list length when setting position
   if(WndMusicOptionsListStart > mNumSongs - 4)
      WndMusicOptionsListStart = mNumSongs - 4;

   // write CD song titles
   if(mMusicType == MUSIC_CD) {

      for(i = 0; i < mNumSongs; i++)
         wsprintf(WndMusicOptionsList[i], TXT_CDTRACK, i+1);

   // load MIDI song titles
   } else {
      ResOpen(BIN_TRACKS);
      ResSeek(sizeof(long), SEEK_CUR);
      ResRead(WndMusicOptionsList, 16*mNumSongs);
      ResClose();
   }
}