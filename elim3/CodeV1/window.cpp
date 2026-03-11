/************************************************************************/
// window.cpp - Window callback & utilities
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

#include "utility.h"
#include "resdef.h"
#include "window.h"
#include "network.h"
#include "user.h"
#include "game.h"


//————————————————————————————————————————————————————————————————————————
// New Game Window
//————————————————————————————————————————————————————————————————————————

// main loop /////////////////////////////////////////////////////////////
long WndNewGame(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:
      strcpy(wnd->ptxts[0].text, pdat[0].name);
      strcpy(wnd->ptxts[1].text, pdat[1].name);
      strcpy(wnd->ptxts[2].text, pdat[2].name);
      strcpy(wnd->ptxts[3].text, pdat[3].name);
      wnd->pnums[0].value = gnSinkHoles;
      wnd->pnums[1].value = gnGames;
      break;

   // the window needs to be drawn
   case WND_DRAW:
      WndGroup(wnd, "Player Options:", 9, 39, 292, 156);
      WndGroup(wnd, "Game Options:", 9, 166, 292, 203);
      break;

   // one of the main buttons was used
   case WND_MAINBTN:
      strcpy(pdat[0].name, wnd->ptxts[0].text);
      strcpy(pdat[1].name, wnd->ptxts[1].text);
      strcpy(pdat[2].name, wnd->ptxts[2].text);
      strcpy(pdat[3].name, wnd->ptxts[3].text);
      gnSinkHoles = wnd->pnums[0].value;
      gnGames     = wnd->pnums[1].value;
      BSETB(game_flags, GAME_NETWORKED, extra == 0);
      WndClose(extra);
      return 1;
   }

   return 0;
}


//————————————————————————————————————————————————————————————————————————
// Message Box Code
//————————————————————————————————————————————————————————————————————————

// global data ///////////////////////////////////////////////////////////
static char *WndMsgBoxText;
static char *WndMsgBoxTitle;
static char *WndMsgBoxBtnA;
static char *WndMsgBoxBtnB;

// master function ///////////////////////////////////////////////////////
long MsgBox(char *text, char *title, char *btna, char *btnb)
{
   WndMsgBoxText  = text;
   WndMsgBoxTitle = title;
   WndMsgBoxBtnA  = btna;
   WndMsgBoxBtnB  = btnb;
   return Wnd(WNDID_MSGBOX, WndMsgBox) - 1;
}

// main window loop //////////////////////////////////////////////////////
long WndMsgBox(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:

      if(WndMsgBoxBtnB == 0) {
         memset(wnd->mbtns[1].text, 0, sizeof(wnd->mbtns[1].text));
         strcpy(wnd->mbtns[2].text, WndMsgBoxBtnA);
         wnd->mbtns[1].state = BTN_INACTIVE;
      } else {
         strcpy(wnd->mbtns[1].text, WndMsgBoxBtnA);
         strcpy(wnd->mbtns[2].text, WndMsgBoxBtnB);
      }

      strcpy(wnd->title, WndMsgBoxTitle);
      break;

   // the window needs to be drawn
   case WND_DRAW:
      RECT r;
      RSETW(r, wnd->xsurf + 5, wnd->ysurf + 32, 231, 66);
      FntDrawWrap(&gFntTxt, ddsWnd, WndMsgBoxText, &r);
      break;

   // one of the main buttons was used
   case WND_MAINBTN:
      WndClose(extra);
      return 1;
   }

   return 0;
}


//————————————————————————————————————————————————————————————————————————
// Join Network Window
//————————————————————————————————————————————————————————————————————————

// main loop /////////////////////////////////////////////////////////////
long WndJoinNet(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:
      wnd->pbtns[0].extra = 8;
      wnd->pbtns[1].extra = 9;
      wnd->pbtns[0].msg   = "Paste an IP from the clipboard";
      wnd->pbtns[1].msg   = "Minimize the game screen";
      break;

   // the window needs to be drawn
   case WND_DRAW:
      RECT r;
      RSET(r, 208, 33, 230, 55); DDRect(ddsWnd, CLR_BLACK, 0, &r, DDRECT_NOFILL);
      RSET(r, 233, 33, 255, 55); DDRect(ddsWnd, CLR_BLACK, 0, &r, DDRECT_NOFILL);

      FntDraw(&gFntTxt, ddsWnd, TXT_HOSTIP  , TXTLEN_HOSTIP  , 32, 39);
      FntDraw(&gFntTxt, ddsWnd, TXT_PASSWORD, TXTLEN_PASSWORD, 22, 65);
      FntDraw(&gFntTxt, ddsWnd, TXT_USERNAME, TXTLEN_USERNAME, 8 , 91);
      break;

   // one of the custom buttons was used
   case WND_CUSTBTN:
      if(extra == 0) WndJoinNetPaste(wnd); // paste button
      else prog_state = PROG_MINIMIZE; // minimize button
      break;

   // one of the main buttons was used
   case WND_MAINBTN:

      // copy hosthood & player name
      dpHost = extra == 1;
      strcpy(dpThisPlrName, wnd->ptxts[2].text);

      // user canceled
      if(extra == 2) {
         WndClose(extra < 2);

      // start a net game
      } else {

         // make sure name is valid
         if(wnd->ptxts[2].text[0] == '\0') {
            MsgBox("Please enter a screen name.", TXT_GENTITLE, TXT_OKAY, 0);

         // attempt to connect
         } else {

            // draw a connecting message
            WndJoinNetDrawConnect(dpHost);

            // attempt to connect
            if(!DpConnect(wnd->ptxts[0].text)) {
               MsgBox(dpHost? "Could Not Host" : "Could not connect", "Error", "Ok", 0);
            } else {
               WndClose(extra < 2);
            }
         }
      }
      return 1;
   }

   return 0;
}

// draw a connecting message to the front buffer /////////////////////////
static void WndJoinNetDrawConnect(long host)
{
   RECT r;
   RSETW(r, 232, 225, 175, 30);
   DDRect(ddsFront, CLR_BLACK, CLR_BLUE, &r, 0);
   REXP(r, -1);
   DDRect(ddsFront, CLR_WHITE, 0, &r, DDRECT_NOFILL);
   FntDrawCentered(&gFntTitle, ddsFront, host? "Hosting..." : "Connecting...", &r);
}

// paste data from the clipboard /////////////////////////////////////////
static void WndJoinNetPaste(WND *wnd)
{
   HANDLE hClipData;
   LPVOID pClipData;
   DWORD  nClipSize;

   // open the clipboard
   OpenClipboard(gHWnd);

   // make sure clip board data valid
   if((hClipData = GetClipboardData(CF_TEXT)) == 0) {
      MsgBox("Invalid clipboard data.", "Error", TXT_OKAY, 0);
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


//————————————————————————————————————————————————————————————————————————
// Start Network Window
//————————————————————————————————————————————————————————————————————————

// global varables ///////////////////////////////////////////////////////
static char WndStartNetChatText[5][64];
static char WndStartNetChatSend[32];
static long WndStartNetSendLen = 0;
long        WndStartNetNewGame = 0;
long        WndStartNetNPlrs   = 1;
PDATB       WndStartNetPlrs[4];


// main loop /////////////////////////////////////////////////////////////
long WndStartNet(WND *wnd, UCHAR msg, UCHAR extra)
{
   switch(msg) {

   // initilize the window
   case WND_INIT:

      // initilize chat varables
      memset(WndStartNetChatText, 0, sizeof(WndStartNetChatText));
      memset(WndStartNetChatSend, 0, sizeof(WndStartNetChatSend));
      memset(WndStartNetPlrs    , 0, sizeof(WndStartNetPlrs));
      WndStartNetNPlrs = 1;

      // setup the first player
      if(!WndStartNetNewGame) {
         WndStartNetPlrs[0].ready  = 1;
         WndStartNetPlrs[0].active = 1;
         WndStartNetPlrs[0].team   = 0;
         strcpy(WndStartNetPlrs[0].sName, dpThisPlrName);
      
      // re-list all active players
      } else if(dpHost) {
         WndStartNetReList();
      }

      // make sure the session is unlocked
      DpLock(0);

      // set the menu buttons up
      wnd->pbtns[0].extra = 8;
      wnd->pbtns[1].extra = 9;

      // attach messages to the menu buttons
      wnd->pbtns[0].msg = "Copy your IP to the clipboard";
      wnd->pbtns[1].msg = "Minimize the game screen";

      // disable start button
      wnd->mbtns[1].state = BTN_DISABLED;

      // setup the ready button
      wnd->pbtns[2].flags = BTN_TOGGLE;
      strcpy(wnd->pbtns[2].text, "Ready");

      // setup number controls
      wnd->pnums[0].value = gnSinkHoles;
      wnd->pnums[1].value = gnGames;

      // disable buttons based on hosthood
      if(dpHost) {
         wnd->pbtns[2].state    = BTN_INACTIVE;
      } else {
         wnd->mbtns[1].state    = BTN_INACTIVE;
         wnd->pbtns[0].state    = BTN_INACTIVE;
         wnd->pbtns[1].state    = BTN_INACTIVE;
         wnd->pnums[0].disabled = 1;
         wnd->pnums[1].disabled = 1;
      }
      break;

   // a window loop is to be processed
   case WND_UPDATE:

      // check the chat keys
      if(TxtCheckKeys(WndStartNetChatSend, &WndStartNetSendLen, 32)) WndStartNetDrawSend();

      // check to see if chat text should be sent
      if(KEY_CLICK(DIK_RETURN) && WndStartNetSendLen > 0) {

         // send the chat message
         WndStartNetSendChat();

         // add the text to this players window
         WndStartNetAddText((UCHAR)gThisPlr, WndStartNetChatSend);
         WndStartNetChatSend[0] = '\0';
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
      break;

   // the window needs to be drawn
   case WND_DRAW:
      WndStartNetDraw(wnd);
      WndStartNetDrawChat();
      WndStartNetDrawSend();
      WndStartNetDrawPlrs();
      break;

   // one of the custom buttons was used
   case WND_CUSTBTN:
      if(extra == 0)      WndStartNetCopy();          // copy the IP
      else if(extra == 1) prog_state = PROG_MINIMIZE; // minimize the game
      
      // change this players ready state
      else {
         gNetData[0] = NETMSG_READYPLR;
         gNetData[1] = BTN_DOWN(wnd->pbtns[2]);
         DpSend(gNetData, sizeof(gNetData));
         WndStartNetReadyPlr((UCHAR)gThisPlr, gNetData[1]? 1 : 0);
      }
      break;

   // one of the main buttons was used
   case WND_MAINBTN:

      // get num data values
      gnSinkHoles = wnd->pnums[0].value;
      gnGames     = wnd->pnums[1].value;

      // get the game going
      if(extra == 1) {
         DpLock(1);
         WndStartNetStart();
         GameInitNet();

      // exit window & direct play session
      } else if(extra == 2) {
         DpQuit();
      }

      WndClose(extra < 2);
      return 1;
   }

   return 0;
}

// draw main window parts ////////////////////////////////////////////////
static void WndStartNetDraw(WND *wnd)
{
   RECT r;
   char sText[32];

   // draw the groups
   WndGroup(wnd, "Chat:"    , 9  , 39 , 259, 144);
   WndGroup(wnd, "Players:" , 267, 39 , 460, 144);
   WndGroup(wnd, "Settings:", 9  , 156, 322, 193);

   // draw the lines above chat send text
   DDLine(ddsWnd, CLR_DTAN, 12, 121, 256, 121);
   DDLine(ddsWnd, CLR_DTAN, 12, 123, 256, 123);

   if(dpHost) {

      // draw rectangles around the menu buttons
      RSET(r, 174, 206, 196, 228); DDRect(ddsWnd, CLR_BLACK, 0, &r, DDRECT_NOFILL);
      RSET(r, 200, 206, 222, 228); DDRect(ddsWnd, CLR_BLACK, 0, &r, DDRECT_NOFILL);

      // draw the hosts IP
      wsprintf(sText, "Your IP: %s", dpThisPlrIp);
      FntDraw(&gFntTxt, ddsWnd, sText, strlen(sText), 8, 212);
   }
}

// draw the chat area ////////////////////////////////////////////////////
static void WndStartNetDrawChat(void)
{
   long i;
   RECT r = {13, 45, 258, 120};
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   for(i = 0; i < 5; i++) {
      FntDraw(&gFntTxtSmall, ddsWnd, WndStartNetChatText[i],
         strlen(WndStartNetChatText[i]), 14, 14*i+50);
   }
}

// draw the send chat text ///////////////////////////////////////////////
static void WndStartNetDrawSend(void)
{
   long x;
   RECT r = {14, 129, 258, 141};
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);
   x = FntDraw(&gFntTxtSmall, ddsWnd, WndStartNetChatSend, WndStartNetSendLen, r.left, r.top);
   FntDraw(&gFntTxtSmall, ddsWnd, "_", 1, x, r.top);
}

// copy ip to clipboard //////////////////////////////////////////////////
static void WndStartNetCopy(void)
{
   HANDLE hMem;
   LPVOID pMem;
   long   len;

   // copy the data into a global memory place
   len  = strlen(dpThisPlrIp) + 1;
   hMem = GlobalAlloc(GHND, len);
   pMem = GlobalLock(hMem);
   memcpy(pMem, dpThisPlrIp, len);
   GlobalUnlock(hMem);

   // set the clipboard data
   OpenClipboard(gHWnd);
   SetClipboardData(CF_TEXT, hMem);
   CloseClipboard();
}

// send a chat message ///////////////////////////////////////////////////
static void WndStartNetSendChat(void)
{
   BYTE* data;

   // create the message
   data    = new BYTE[WndStartNetSendLen + 3];
   data[0] = NETMSG_CHATA;

   // copy the chat text
   memcpy(data + 1, WndStartNetChatSend, WndStartNetSendLen);

   // add a null terminator
   data[WndStartNetSendLen + 2] = '\0';

   // send the data
   DpSend(data, WndStartNetSendLen + 3);
   delete [] data;
}

// draw the player info //////////////////////////////////////////////////
void WndStartNetDrawPlrs(void)
{
   long i;
   long j;
   long y;
   RECT r;

   // clear the players area
   RSET(r, 272, 48, 457, 141);
   DDRect(ddsWnd, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   for(i = j = 0; i < 4; i++) {
      if(WndStartNetPlrs[i].active) {

         y = 23*j+54;
      
         // draw the sphere next to their name
         if(WndStartNetPlrs[i].ready) { RSETW(r, 118, i*9, 10, 10); }
         else                         { RSETW(r, 118, i*10+37, 10, 10); }
         DDBltFast(ddsWnd, ddsItems, 278, y, &r, SRCCK);

         // draw their name
         FntDraw(&gFntTxt, ddsWnd, WndStartNetPlrs[i].sName,
            strlen(WndStartNetPlrs[i].sName), 295, y);

         /* draw their team */

         j++;
      }
   }
}

// add chat text /////////////////////////////////////////////////////////
void WndStartNetAddText(UCHAR p, char *sText)
{
   // move the chat text up
   for(long i = 0; i < 4; i++) strcpy(WndStartNetChatText[i], WndStartNetChatText[i+1]);
  
   // add the new line
   if(p == 4) wsprintf(WndStartNetChatText[4], ">> %s", sText);
   else       wsprintf(WndStartNetChatText[4], "%s: %s", WndStartNetPlrs[p].sName, sText);

   // re-draw the chat area
   if(prog_state != PROG_SUSPENDED) WndStartNetDrawChat();
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
   if(prog_state != PROG_SUSPENDED) WndStartNetDrawPlrs();

   // if host, then check to see if the game can start
   if(dpHost) {
      
      // determine if ALL the ACTIVE players are ready
      for(i = 0; i < 4; i++) {
         if(WndStartNetPlrs[i].active) {
            bReady = (bReady && WndStartNetPlrs[i].ready);
            nReady++;
         }
      }

      // set the ready button state & re-draw
      gWndA.mbtns[1].state = (bReady && nReady > 1)? BTN_IDLE : BTN_DISABLED;
      if(prog_state != PROG_SUSPENDED) BtnDraw(&gWndA.mbtns[1]);
   }
}

// re-list the players ///////////////////////////////////////////////////
void WndStartNetReList(void)
{
   BYTE *data;

   // get new data
   DpListPlayers();
   WndStartNetNPlrs = dpCurEnumPlr;

   // create the message to send
   data    = new BYTE[sizeof(WndStartNetPlrs) + 1];
   data[0] = NETMSG_RELISTPLRS;

   // send the message to re-list
   memcpy(data + 1, WndStartNetPlrs, sizeof(WndStartNetPlrs));
   DpSend(data, sizeof(WndStartNetPlrs) + 1);
   delete [] data;

   // re-draw the player list
   if((game_flags & GAME_MSGBOX) && prog_state != PROG_SUSPENDED)
      WndStartNetDrawPlrs();
}

// start the game ////////////////////////////////////////////////////////
void WndStartNetStart(void)
{
   long i;

   // copy the number of players
   nplrs = WndStartNetNPlrs;

   // translate data
   for(i = 0; i < 4; i++) {

      // copy direct play ID and team
      pdat[i].dpId = WndStartNetPlrs[i].dpId;
      pdat[i].team = WndStartNetPlrs[i].team;

      // copy names
      strcpy(pdat[i].name, WndStartNetPlrs[i].sName);

      // set option flags
      BSETB(pdat[i].flags, PLR_ON, WndStartNetPlrs[i].active);
      BSETB(pdat[i].flags, PLR_ACTIVE, WndStartNetPlrs[i].active);

      // record current player ID
      if(pdat[i].dpId == dpThisPlrId) gThisPlr = i;
   }
}