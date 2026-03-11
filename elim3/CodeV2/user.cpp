/************************************************************************/
// user.cpp - Main user/information display interfaces
//
// Author: Justin Hoffman
// Date:   6/7/00 - 7/18/2000
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "game.h"
#include "user.h"
#include "resdef.h"


//————————————————————————————————————————————————————————————————————————
// Globals
//————————————————————————————————————————————————————————————————————————

// repeated texts ////////////////////////////////////////////////////////
char *TXT_SYSTEMCHAT        = ">> %s";
char *TXT_PLAYERCHAT        = "%s: %s";
char *TXT_PLRDISCONNECT     = "%s has been disconnected";
char *TXT_MINIMIZEBTN       = "Minimize the game screen";
char *TXT_ALLSAMETEAM       = "Everyone cannot be on the same team.";
char *TXT_EVERYONENEEDSTEAM = "Not everyone has choosen a team.";
char *TXT_NOTENOUGHFORTEAMS = "There are not enough players to have teams.";
char *TXT_HELP              = "Show help window";
char *TXT_GAMEOPTIONS       = "Game Options:";
char *TXT_GENTITLE          = "Elimination III";
char *TXT_NETERROR          = "Network Error";
char *TXT_GENERROR          = "Error";
char *TXT_OKAY              = "Okay";
char *TXT_CANCEL            = "Cancel";

// misc. globals /////////////////////////////////////////////////////////
char  TRANS_KEYSL[] = "~~1234567890-=?~qwertyuiop[\\~~asdfghjkl;'~~]zxcvbnm,./~*~ ";
char  TRANS_KEYSU[] = "~~!@#$%^&*()_+?~QWERTYUIOP{}~~ASDFGHJKL:\"~~|ZXCVBNM<>?~*~ ";
UCHAR CLRS_LHT[4]   = {225, 221, 227, 246};
UCHAR CLRS_MED[4]   = {137, 102, 166, 196};
UCHAR CLRS_DRK[4]   = {124, 91 , 153, 178};
UCHAR CLRS_BLK[4]   = {48 , 40 , 56 , 66};
long  gnMenus       = 0;

// pop-up message box varables ///////////////////////////////////////////
char *gsMsgPrev = 0;
char *gsMsgNext = 0;
long  gMsgX     = 0;
long  gMsgY     = 0;
long  gMsgW     = 0;
DWORD gMsgTimer = 0;

// flashing cursor data //////////////////////////////////////////////////
DWORD gCurLTick   = 0;
bool  gCurChanged = 0;
bool  gCurVisible = 0;

// chat varables /////////////////////////////////////////////////////////
long gsChatSendLen = 0;
char gsChatText[4][80];
char gsChatSend[64];

// utility bars & buttons ////////////////////////////////////////////////
BAR gBarTeams;     // player teams bar
BAR gBarMenu;      // menu bar
BAR gBarChat;      // chat bar
BAR gBarRoll;      // roll bar
BAR gBarDice[2];   // main dice
BAR gBarNames[4];  // player name bars
BTN gBtnShowMenu;  // show menubar button
BTN gBtnMenus[9];  // menubar buttons

// window globals ////////////////////////////////////////////////////////
WND   gWndA;
WND   gWndB;
WND  *gWndCur  = 0;
long  gWndCurN = 0;

// menu bar messages /////////////////////////////////////////////////////
char *gsMenuHelp[10] = {
   "start a new game",
   "load a different game",
   "save this game",
   "save this game with a different name",
   "view the help file",
   "minimize the game window",
   "return to the real world",
   "set the music options",
   "turn the sound on or off"
};


//————————————————————————————————————————————————————————————————————————
// User Interface Code
//————————————————————————————————————————————————————————————————————————

// update the cursor flash ///////////////////////////////////////////////
void GameUpdateCursor(void)
{
   // clear the "next" message &  re-set the changed boolean
   gsMsgNext   = 0;
   gCurChanged = 0;

   // make sure enough time has passed
   if(gThisTick - gCurLTick < WAIT_CURFLASH) return;

   // re-set the last tick
   gCurLTick = gThisTick;

   // invert the cursor status
   gCurVisible = !gCurVisible;
   gCurChanged = 1;
}

// update the utility bars ///////////////////////////////////////////////
void GameUpdateBars(void)
{
   long i, j;

   // update all of the utility bars
   BarUpdate(&gBarTeams);
   BarUpdate(&gBarChat);
   BarUpdate(&gBarMenu);
   BarUpdate(&gBarRoll);
   BarUpdate(&gBarDice[0]);
   BarUpdate(&gBarDice[1]);

   // update the name bars
   for(i = 0; i < nplrs; i++) {

      BarUpdate(&gBarNames[i]);

      // show the next bar
      if(game_state == GAME_WAITNAMEBARS && gBarNames[i].state == BAR_FINISHED) {
 
         // get index of next bar
         j = i + 1;

         // move the next bar
         if(j < nplrs) {
            BarShow(&gBarNames[j], gBarNames[j].xmin > 320, 0);

         // we are done showing the names, move on
         } else {
            NameSelect();
            GameRoll();
         }
      }
   }
}

// update the menu bar ///////////////////////////////////////////////////
void GameUpdateMenu(void)
{
   long i;

   // check the menu items if the menubar is visible
   if((long)(gBarMenu.y) == gBarMenu.ymax) {
      for(i = 0; i < gnMenus; i++) {
         BtnUpdate(&gBtnMenus[i]);
         if(BTN_USED(gBtnMenus[i])) ProgMenu(gBtnMenus[i].extra, BTN_DOWN(gBtnMenus[i]));
      }
   }

   // update the show/hide menubar button
   BtnUpdate(&gBtnShowMenu);
   if(BTN_USED(gBtnShowMenu)) {
      BREM(gBtnShowMenu.flags, BTN_CURWITHIN);
      BarShow(&gBarMenu, 0, (long)(gBarMenu.y) == gBarMenu.ymax);
   }

   // repaint the show/hide menubar button when the menu is done moving
   if(gBarMenu.state == BAR_FINISHED) BtnDraw(&gBtnShowMenu);
}

// update the chat box ///////////////////////////////////////////////////
void GameUpdateChat(void)
{
   // update the chat text
   if(TxtCheckKeys(gsChatSend, &gsChatSendLen, 64) || gCurChanged)
      ChatDrawSendText();

   // see if text should be sent
   if(KEY_CLICK(DIK_RETURN) && gsChatSendLen > 0) {

      // send the message
      GameSendChat();

      // add message to screen
      ChatAddText(gThisPlr, gsChatSend);
      gsChatSend[0] = TXT_NULLCHAR;
      gsChatSendLen = 0;
      ChatDrawSendText();
   }
}

// update the player name boxes //////////////////////////////////////////
void GameUpdateName(void)
{
   UCHAR i;

   // update the players flashing name
   if(gbSelectName && gThisTick - pdat[pcur].ltick_a >= WAIT_NAMEFLASH) {

      // see if we are done selecting the name
      if(++pdat[pcur].frame_a > NFRAMES_NAMEFLASH) {
         gbSelectName = 0;

      // update the flash
      } else {
         pdat[pcur].ltick_a = gThisTick;
         NameDrawSelect(pcur, (pdat[pcur].frame_a + 1) % 2);
      }
   }

   // update a players scrolling graph
   for(i = 0; i < 4; i++) {
      if((pdat[i].flags & PLR_SCROLLGRAPH) && gThisTick - pdat[i].ltick_b >= WAIT_GRAPH) {
         pdat[i].ltick_b  = gThisTick;
         pdat[i].frame_b += pdat[i].frameDir;

         // draw the graph
         NameDrawGraph(i);

         // see if we are done scrolling
         if(pdat[i].frame_b == 0 || pdat[i].frame_b == 20) {
            BREM(pdat[i].flags, PLR_SCROLLGRAPH);
         }
      }
   }
}

// update the popup help text ////////////////////////////////////////////
void GameUpdatePopUp(void)
{
   RECT r;

   if(gsMsgNext == gsMsgPrev) return;

   if(gsMsgNext) {

      // set the timer
      gMsgTimer = gThisTick;

      // re-draw the pop-up msg box
      gMsgW = 9 + FntStrWidth(&gFntSmall, gsMsgNext, lstrlen(gsMsgNext));

      RSET(r, 0, 0, gMsgW, 17);
      DDRect(ddsMsg, CLR_BLACK, CLR_BTNPUSHED, &r, 0);

      REXP(r, -1);
      DDRect(ddsMsg, CLR_WHITE, 0, &r, DDRECT_NOFILL);
      FntDrawCenter(&gFntSmall, ddsMsg, gsMsgNext, &r);

   } else gMsgW = 0;

   // save location of this message for next frame
   gsMsgPrev = gsMsgNext;
}

// render the user interface /////////////////////////////////////////////
void GameRenderUi(void)
{
   RECT r;
   long i;

   // render all of the utility bars
   BarRender(&gBarTeams);
   BarRender(&gBarChat);
   BarRender(&gBarMenu);
   BarRender(&gBarRoll);
   BarRender(&gBarDice[0]);
   BarRender(&gBarDice[1]);

   // render the player name bars
   for(i = 0; i < nplrs; i++) BarRender(&gBarNames[i]);

   // render top of menubar
   RSET(r, 0, 0, 26, 15);
   DDBltFast(ddsBack, ddsMenuBar, 607, 7, &r, SRCCK);

   // render x's over the dice if necessary
   RSET(r, 35, 56, 51, 73);

   for(i = 0; i < 2; i++) {
      if(gBarDice[i].extra == 25) {
         DDBltClip(ddsBack, ddsItems, (long)(gBarDice[i].x) + 5,
            (long)(gBarDice[i].y) + 5, &gBarDice[i].rclip, &r, SRCCK);
      }
   }

   // render the message box
   if(gbMsgBox) WndRender();

   // render the pop-up box
   if(gMsgW > 0 && gThisTick - gMsgTimer > WAIT_SHOWMSG) {
      RSET(r, 0, 0, gMsgW, 17);
      DDBltFast(ddsBack, ddsMsg, gMsgX - gMsgW, gMsgY, &r, NOCK);
   }

   // render the cursor
   RSET(r, 51, 53, 62, 72);
   DDBltClip(ddsBack, ddsItems, cur_x, cur_y, &R_SCREEN, &r, SRCCK);
}


//————————————————————————————————————————————————————————————————————————
// Chat Bar Functions
//————————————————————————————————————————————————————————————————————————

// add a line of text to the chat box ////////////////////////////////////
void ChatAddText(UCHAR p, char *sText)
{
   // move lines up
   lstrcpy(gsChatText[0], gsChatText[1]); 
   lstrcpy(gsChatText[1], gsChatText[2]); 
   lstrcpy(gsChatText[2], gsChatText[3]);

   // write to bottom line & re-draw text
   if(p == 4) wsprintf(gsChatText[3], TXT_SYSTEMCHAT, sText);
   else       wsprintf(gsChatText[3], TXT_PLAYERCHAT, pdat[p].sName, sText);

   // re-draw the chat box
   if(gbVisible) ChatDrawText();
}

// draw the chat box /////////////////////////////////////////////////////
void ChatDraw(void)
{
   RECT r;

   // fill the surface
   RSET(r, 0, 0, 336, 89);
   DDRect(ddsChat, CLR_BLACK, CLR_LTAN, &r, 0);

   // draw the white inside line
   REXP(r, -1);
   DDRect(ddsChat, CLR_WHITE, 0, &r, DDRECT_NOFILL);

   // draw the chat area box
   RSET(r, 4, 4, 332, 85);
   DDRect(ddsChat, CLR_DTAN, CLR_LTAN, &r, 0);

   // draw lines in chat box
   DDLine(ddsChat, CLR_DTAN, 7, 65, 329, 65);
   DDLine(ddsChat, CLR_DTAN, 7, 67, 329, 67);

   // draw chat text
   ChatDrawText();
   ChatDrawSendText();
}

// draw the text in the chat box /////////////////////////////////////////
void ChatDrawText(void)
{
   long i;
   RECT r;

   // clear the old text
   RSET(r, 5, 5, 330, 64);
   DDRect(ddsChat, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw the new text
   for(i = 0; i < 4; i++)
      FntDrawClip(&gFntSmall, ddsChat, gsChatText[i], 8, 14*i + 9, 330);
}

// draw the text to be sent in the chat box //////////////////////////////
void ChatDrawSendText(void)
{
   long x;
   RECT r;

   // clear the old text
   RSET(r, 5, 68, 330, 83);
   DDRect(ddsChat, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw the current send text followed by an underscore
   x = FntDrawClip(&gFntSmall, ddsChat, gsChatSend, 8, 72, 330);
   if(gCurVisible) FntDraw(&gFntSmall, ddsChat, TXT_CURCHAR, 1, x, 72);
}

// initilize the chat box ////////////////////////////////////////////////
void ChatInit(void)
{
   // clear the chat text
   memset(gsChatText, 0, sizeof(gsChatText));
   memset(gsChatSend, 0, sizeof(gsChatSend));
   gsChatSendLen = 0;

   // draw the chat bar
   ChatDraw();
}


//————————————————————————————————————————————————————————————————————————
// Menu Functions
//————————————————————————————————————————————————————————————————————————

// enable/disable a menu item ////////////////////////////////////////////
void MenuEnable(UCHAR menuId, bool enable)
{
   // if we are in network mode, subtract 4 from the menuID
   if(gbNetwork) menuId -= 4;
   BtnEnable(&gBtnMenus[menuId], enable);
}

// "push" a toggle menu item /////////////////////////////////////////////
void MenuPush(UCHAR menuId)
{
   // if we are in network mode, subtract 4 from the menuID
   if(gbNetwork) menuId -= 4;
   BSET(gBtnMenus[menuId].flags, BTN_TDOWN);
   if(gbVisible) BtnDraw(&gBtnMenus[menuId]);
}

// draw the menubar //////////////////////////////////////////////////////
void MenuDraw(void)
{
   RECT r;
   long i;
   long b = 21*gnMenus + 19; // location of menubar's bottom pixel

   // clear the background
   RSET(r, 0, 0, gBarMenu.rsrc.right, gBarMenu.rsrc.bottom);
   DDRect(ddsMenuBar, CLR_BLACK, CLR_LTAN, &r, 0);

   // draw the '3D ish' lines
   DDLine(ddsMenuBar, CLR_WHITE, 1 , 1, 24, 1); // top
   DDLine(ddsMenuBar, CLR_DTAN , 24, 2, 24, b); // left
   DDLine(ddsMenuBar, CLR_DTAN , 2 , b, 25, b); // bottom
   DDLine(ddsMenuBar, CLR_WHITE, 1 , 1, 1 , b); // right

   // draw the buttons
   for(i = 0; i < gnMenus; i++) BtnDraw(&gBtnMenus[i]);

   // draw the showmenu button
   BtnDraw(&gBtnShowMenu);
}

// initilize the menu bar ////////////////////////////////////////////////
void MenuInit(void)
{
   long i;
   long start;

   // get the number of menus & what menu to start at
   if(gbNetwork) {
      gnMenus = 5;
      start   = 4;
   } else {
      gnMenus = 9;
      start   = 0;
   }

   // initilize show/hide menu button
   memset(&gBtnShowMenu, 0, sizeof(gBtnShowMenu));
   gBtnShowMenu.msg    = TXT_SHOWMENU;
   gBtnShowMenu.ddsdst = ddsMenuBar;
   gBtnShowMenu.state  = BTN_IDLE;
   gBtnShowMenu.type   = BTN_SHOWMENU;
   gBtnShowMenu.xscrn  = 609;
   gBtnShowMenu.yscrn  = 9;
   RSET(gBtnShowMenu.rdest, 2, 2, 22, 12);

   // clear menu structurs
   memset(gBtnMenus, 0, sizeof(gBtnMenus));

   // initilize menuitems
   for(i = 0; i < gnMenus; i++) {
      gBtnMenus[i].msg    = gsMenuHelp[i+start];
      gBtnMenus[i].ddsdst = ddsMenuBar;
      gBtnMenus[i].state  = BTN_IDLE;
      gBtnMenus[i].type   = BTN_MENUA;
      gBtnMenus[i].extra  = i + start;
      gBtnMenus[i].xscrn  = 611;
      gBtnMenus[i].yscrn  = 21*i + 26;
      RSETWX(gBtnMenus[i].rdest, 3, 21*i + 19, 20, 20);
   }

   // make the last button (sound on/off) a toggle button
   gBtnMenus[gnMenus - 1].flags = BTN_TOGGLE;

   // make music on/off button toggle if we are networked
   if(gbNetwork) gBtnMenus[gnMenus - 2].flags = BTN_TOGGLE;

   // draw the menubar
   if(gbVisible) MenuDraw();
}


//————————————————————————————————————————————————————————————————————————
// Name Bar Functions
//————————————————————————————————————————————————————————————————————————

// select the current players name ///////////////////////////////////////
void NameSelect(void)
{
   gbSelectName       = 1;
   pdat[pcur].ltick_a = gThisTick;
   pdat[pcur].frame_a = 0;
}

// change the graph of a player //////////////////////////////////////////
void NameReGraph(UCHAR p, long incr)
{
   // add the change to the current number of marbles
   pdat[p].nMarbles += incr;

   // we are loosing a marble
   if(incr == -1) {
      pdat[p].frameDir  = -1;
      pdat[p].frame_b   = 20;

   // we are gaining a marble
   } else {
      pdat[p].frameDir = 1;
      pdat[p].frame_b  = 0;
   }

   // set global varables
   BSET(pdat[p].flags, PLR_SCROLLGRAPH);
   pdat[p].ltick_b = gThisTick;
}

// draw the name bars ////////////////////////////////////////////////////
void NameDraw(void)
{
   RECT   r;
   UCHAR  i, j;
   FNT   *pFnt;
   long   cashe;
   long   offset;
   long   nameOfstX;
   long   nameOfstY;
   long   nGamesOfstX;
   long   nGamesOfstY;

   // clear the entire surface
   DDClear(ddsNames, CLR_BKGND);

   // setup top left corner of rect
   r.left = r.top = 1;

   // set network specific data
   if(gbNetwork) {

      r.right     = 272;
      r.bottom    = 21;
      offset      = 22;
      nameOfstX   = 6;
      nameOfstY   = 7;
      nGamesOfstX = 263;
      nGamesOfstY = 4;
      pFnt        = &gFntSmall;

   // set offline specific data
   } else {

      r.right     = 250;
      r.bottom    = 43;
      offset      = 44;
      nameOfstX   = 8;
      nameOfstY   = 24;
      nGamesOfstX = 238;
      nGamesOfstY = 23;
      pFnt        = &gFntNames;
   }

   for(i = 0; i < 4; i++) {

      // cash this common value
      cashe = offset*i;

      // fill the back
      DDRect(ddsNames, CLR_BLACK, CLR_LGREY, &r, 0);

      // draw the white inner border
      REXP(r, -1);
      DDRect(ddsNames, CLR_WHITE, 0, &r, DDRECT_NOFILL);
      
      // move the rectangle for the next item
      r.left   -= 1;
      r.right  += 1;
      r.top    += offset - 1;
      r.bottom += offset + 1;

      // draw the name
      FntDraw(pFnt, ddsNames, pdat[i].sName, lstrlen(pdat[i].sName), nameOfstX, cashe + nameOfstY);

      // draw the marble graph
      NameDrawGraph(i);

      // draw the spaces for number of games won
      for(j = 0; j < gnGames; j++) {
         NameDrawGreyBar(CLR_NGAMEMARKBORDER, CLR_LGREY, -5*j + nGamesOfstX, cashe + nGamesOfstY, 5, 13);
      }

      // draw the actual number of games won
      for(j = 0; j < pdat[i].nGamesWon; j++) {
         NameDrawColorBar(i, -5*j + nGamesOfstX, cashe + nGamesOfstY, 5, 13);
      }
   }
}

// re-draw the selection rectangle ///////////////////////////////////////
void NameDrawSelect(UCHAR p, UCHAR mode)
{
   RECT  r;
   long  x;
   UCHAR ca;
   UCHAR cb;

   // create the rectangle
   x        = gbNetwork? 22 : 44;
   r.left   = 0;
   r.top    = x*p;
   r.right  = r.left + 22*gbNetwork + 251;
   r.bottom = r.top + x;

   // determine the colors
   switch(mode) {
   case 0: ca = CLRS_MED[p]; cb = CLRS_MED[p]; break;
   case 1: ca = CLR_BLUE   ; cb = CLR_BLUE   ; break;
   case 2: ca = CLR_BKGND  ; cb = CLR_BLACK  ; break;
   }

   // draw the first rect
   DDRect(ddsNames, ca, 0, &r, DDRECT_NOFILL);

   // draw the second rect
   REXP(r, -1);
   DDRect(ddsNames, cb, 0, &r, DDRECT_NOFILL);
}

// draw a players graph //////////////////////////////////////////////////
void NameDrawGraph(UCHAR p)
{
   RECT   r;
   long   linex;
   long   lineya;
   long   lineyb;
   long   x, y;
   long   wa, wb;
   char   sText[3];
   double scale;

   if(gbNetwork) {
      x  = 89;
      y  = 22*p + 4;
      wa = 151;
   } else {
      x  = 5;
      y  = 44*p + 4;
      wa = 240;
   }

   // determine the scale of the graph
   scale = ((double)(wa) / (double)(nplrs * 10.00));

   // determine how much of the graph to fill
   wb = (long)(scale * (double)(pdat[p].nMarbles));

   // ajust for scrolling
   if(pdat[p].flags & PLR_SCROLLGRAPH) {
      wb += (long)(scale * ( (double)(pdat[p].frame_b) / 20.00 ));
      if(pdat[p].frameDir == 1) wb -= (long)scale;
   }

   // draw the bars & clear part of the graph border if necessary
   NameDrawGreyBar(CLR_GRAPHBORDER, CLR_GRAPHFILL, x, y, wa, 13);
   if(wb > 1) NameDrawColorBar(p, x, y, wb, 13);

   // clear the lines from the edge of the bar if necessary
   if( (gbNetwork? (wb > 70 && wb < 84) : (wb > 114 && wb < 128)) ) {
      linex  = x + wb;
      lineya = y + 2;
      lineyb = y + 12;
      DDLine(ddsNames, CLRS_MED[p], linex - 1, lineya, linex - 1, lineyb);
      DDLine(ddsNames, CLRS_MED[p], linex    , lineya, linex    , lineyb);
   }

   // draw the text on top of the bars
   RSETW(r, x, y, wa + 1, 14);
   wsprintf(sText, "%d", pdat[p].nMarbles);
   FntDrawCenter(&gFntSmall, ddsNames, sText, &r);
}

// draw the colord part of a graph ///////////////////////////////////////
void NameDrawColorBar(UCHAR p, long x, long y, long w, long h)
{
   RECT   r;
   UCHAR *buffer;
   ULONG  lpitch;
   long   a , b;
   long   xb, yb;
   long   i;

   // detirmine stoping points
   xb = x + w;
   yb = y + h;

   // fill the area
   if(w > 3) {
      RSET(r, x + 1, y, xb, yb);
      DDRect(ddsNames, 0, CLRS_MED[p], &r, DDRECT_NOBORDER);
   }

   // lock the names surface
   DDLock(ddsNames, &buffer, &lpitch);

   // draw the top & bottom lines
   a = y  + 1;
   b = yb - 1;

   for(i = x + 1; i < xb; i++) {
      buffer[i +  y*lpitch] = CLRS_BLK[p];
      buffer[i +  a*lpitch] = CLRS_LHT[p];
      buffer[i +  b*lpitch] = CLRS_DRK[p];
      buffer[i + yb*lpitch] = CLRS_BLK[p];
   }

   // draw the right & left lines
   a = x  + 1;
   b = xb - 1;

   for(i = y + 1; i < yb; i++) {
      buffer[x  + i*lpitch] = CLRS_BLK[p];
      buffer[a  + i*lpitch] = CLRS_LHT[p];
      buffer[b  + i*lpitch] = CLRS_DRK[p];
      buffer[xb + i*lpitch] = CLRS_BLK[p];
   }

   // draw the corner pixels
   buffer[x  + 1 + yb*lpitch - lpitch] = CLRS_MED[p];
   buffer[xb - 1 + y*lpitch  + lpitch] = CLRS_MED[p];

   // unlock the surface
   DDUnlock(ddsNames);
}

// draw the grey part of a graph /////////////////////////////////////////
void NameDrawGreyBar(UCHAR border, UCHAR fill, long x, long y, long w, long h)
{
   RECT   r;
   UCHAR *buffer;
   ULONG  lpitch;
   long   xb, yb; 

   // fill the area
   RSETW(r, x, y, w + 1, h + 1);
   DDRect(ddsNames, border, fill, &r, 0);

   // lock the buffer
   DDLock(ddsNames, &buffer, &lpitch);

   // cash values
   xb = x + w;
   yb = (y+h)*lpitch;
   y *= lpitch;

   // clear the pixels on the 4 corners
   buffer[x  + y]  = CLR_LGREY;
   buffer[xb + y]  = CLR_LGREY;
   buffer[x  + yb] = CLR_LGREY;
   buffer[xb + yb] = CLR_LGREY;
   
   // unlock the surface
   DDUnlock(ddsNames);
}

// setup the name bars ///////////////////////////////////////////////////
void NameSetup(void)
{
   long i, j;
   long a[5] = {0, 0  , 45 , 31 , 23};
   long b[5] = {0, 375, 387, 379, 375};

   // clear the name bars
   memset(gBarNames, 0, sizeof(gBarNames));

   for(i = j = 0; i < nplrs; i++) {

      // move to the next active player
      while((pdat[j].flags & PLR_ON) == 0) j++;

      // set network specific paramiters
      if(gbNetwork) {

         gBarNames[i].x     = 640;
         gBarNames[i].xmax  = 640;
         gBarNames[i].xmin  = 356;
         gBarNames[i].xvel  = VEL_CHATNAMEBAR;
         gBarNames[i].y     = a[nplrs]*i + b[nplrs];
         RSETWX(gBarNames[i].rsrc, 0, 22*j, 273, 22);

      // set offline specific paraimters
      } else {

         a[0] = i % 2; // if 1 then bar is on right
         b[0] = nplrs == 2? 400 : i < 2? 372 : 425; // get bar y chord

         gBarNames[i].x     = a[0]? 640 : -251;
         gBarNames[i].xmax  = a[0]? 640 : 17;
         gBarNames[i].xmin  = a[0]? 372 : -251;
         gBarNames[i].xvel  = VEL_NORMALNAMEBAR;
         gBarNames[i].y     = b[0];
         RSETWX(gBarNames[i].rsrc, 0, 44*j, 251, 44);
      }

      // set simmilar data
      gBarNames[i].ymin = (long)gBarNames[i].y;
      gBarNames[i].ymax = (long)gBarNames[i].y;
      gBarNames[i].extra = j;
      gBarNames[i].state = BAR_IDLE;
      gBarNames[i].surf  = ddsNames;
      gBarNames[i].rclip = R_SCREEN;
      j++;
   }

   // if there are only 3 players, center the right box
   if(!gbNetwork && nplrs == 3) {
      gBarNames[1].y    = 400;
      gBarNames[1].ymin = 400;
      gBarNames[1].ymax = 400;
   }

   // draw the names
   if(gbVisible) NameDraw();
}


//————————————————————————————————————————————————————————————————————————
// Misc. Utility Bar Functions
//————————————————————————————————————————————————————————————————————————

// draw the dice bar /////////////////////////////////////////////////////
void BarsDrawDice(void)
{
   RECT r;

   // fill dice bar
   RSET(r, 0, 0, 104, 35);
   DDRect(ddsDiceBar, CLR_BLACK, CLR_LGREY, &r, 0);

   // draw the white inner border
   REXP(r, -1);
   DDRect(ddsDiceBar, CLR_WHITE, 0, &r, DDRECT_NOFILL);

   // draw text on dice bar
   FntDraw(&gFntNames, ddsDiceBar, TXT_ROLL, TXTLEN_ROLL, 7, 12);
}

// draw the team bar /////////////////////////////////////////////////////
void BarsDrawTeam(void)
{
   RECT r;
   long i, j;

   // clear the surface
   DDClear(ddsTeams, CLR_BKGND);

   // fill the back
   RSET(r, 0, 0, 25*nplrs + 37, 31);
   DDRect(ddsTeams, CLR_BLACK, CLR_LGREY, &r, 0);
   REXP(r, -1);
   DDRect(ddsTeams, CLR_WHITE, 0, &r, DDRECT_NOFILL);

   // draw the marbles
   for(i = 0; i < 2; i++) {
      for(j = 0; j < gnTeamPlrs[i]; j++) {
         RSETWY(r, 21*gTeams[i][j], 0, 21, 21);
         DDBltFast(ddsTeams, ddsMarbles, 25*j + i*(gnTeamPlrs[0]*25 + 31) + 5, 5, &r, SRCCK);
      }
   }

   // draw "vs" text
   RSET(r, 62, 67, 79, 77);
   DDBltFast(ddsTeams, ddsItems, 25*gnTeamPlrs[0] + 9, 11, &r, NOCK);
}

// initilize the constant varables in the bars ///////////////////////////
void BarsInitA(void)
{
   // clear the bars
   memset(&gBarTeams, 0, sizeof(gBarTeams));
   memset(&gBarChat , 0, sizeof(gBarTeams));
   memset(&gBarMenu , 0, sizeof(gBarTeams));
   memset(&gBarRoll , 0, sizeof(gBarTeams));

   //-------------------------------------------------
   // team bar
   //-------------------------------------------------
   gBarTeams.x     = 7;
   gBarTeams.xmax  = 7;
   gBarTeams.xmin  = 7;

   gBarTeams.y     = -32;
   gBarTeams.ymax  = 7;
   gBarTeams.ymin  = -32;
   gBarTeams.yvel  = VEL_TEAMBAR;

   gBarTeams.state = BAR_IDLE;
   gBarTeams.surf  = ddsTeams;
   gBarTeams.rclip = R_SCREEN;

   gBarTeams.rsrc.right  = 137;
   gBarTeams.rsrc.bottom = 31;

   //-------------------------------------------------
   // chat bar
   //-------------------------------------------------
   gBarChat.x     = 12;
   gBarChat.xmax  = 12;
   gBarChat.xmin  = 12;

   gBarChat.y     = 480;
   gBarChat.ymax  = 480;
   gBarChat.ymin  = 376;
   gBarChat.yvel  = VEL_CHATBAR;

   gBarChat.state = BAR_IDLE;
   gBarChat.surf  = ddsChat;
   gBarChat.rclip = R_SCREEN;

   gBarChat.rsrc.right  = 336;
   gBarChat.rsrc.bottom = 89;

   //-------------------------------------------------
   // menu bar
   //-------------------------------------------------
   gBarMenu.x     = 607;
   gBarMenu.xmax  = 607;
   gBarMenu.xmin  = 607;

   gBarMenu.y     = 22;
   gBarMenu.ymax  = 22;
   gBarMenu.yvel  = VEL_MENUBAR;

   gBarMenu.state = BAR_IDLE;
   gBarMenu.surf  = ddsMenuBar;
   RSET(gBarMenu.rclip, 0, 22, 640, 480);
   RSET(gBarMenu.rsrc , 0, 17, 26 , 210);

   //-------------------------------------------------
   // roll bar
   //-------------------------------------------------
   gBarRoll.x     = 640;
   gBarRoll.xmax  = 640;
   gBarRoll.xmin  = 524;
   gBarRoll.xvel  = VEL_ROLLBAR;

   gBarRoll.y     = 320;
   gBarRoll.ymax  = 320;
   gBarRoll.ymin  = 320;

   gBarRoll.state = BAR_IDLE;
   gBarRoll.surf  = ddsDiceBar;
   gBarRoll.rclip = R_SCREEN;

   gBarRoll.rsrc.right  = 104;
   gBarRoll.rsrc.bottom = 35;
}

// initilize the game specific varables in the bars //////////////////////
void BarsInitB(void)
{
   long i;

   // clear the dice bars
   memset(gBarDice, 0, sizeof(gBarDice));

   // setup menu bar items
   if(gbNetwork) {
      gBarMenu.ymin        = -85;
      gBarMenu.rsrc.bottom = 126;
   } else {
      gBarMenu.ymin        = -169;
      gBarMenu.rsrc.bottom = 210;
   }

   // setup the dice
   for(i = 0; i < 2; i++) {

      // setup network specific information
      if(gbNetwork) {
         gBarDice[i].x    = 29*i + 568;
         gBarDice[i].ymax = 354;
         gBarDice[i].ymin = 324;
         gBarDice[i].yvel = VEL_NETDICE;
         RSET(gBarDice[i].rclip, 568, 324, 623, 352);

      // setup offline specific information
      } else {
         gBarDice[i].x     = 32*i + 291;
         gBarDice[i].ymax  = 480;
         gBarDice[i].ymin  = 408;
         gBarDice[i].yvel  = VEL_NORMALDICE;
         gBarDice[i].rclip = R_SCREEN;
      }

      // setup common information
      gBarDice[i].y     = gBarDice[i].ymax;
      gBarDice[i].xmax  = (long)gBarDice[i].x;
      gBarDice[i].xmin  = (long)gBarDice[i].x;
      gBarDice[i].state = BAR_IDLE;
      gBarDice[i].surf  = ddsDice;
   }
}


//————————————————————————————————————————————————————————————————————————
// Utility Bar Functions
//————————————————————————————————————————————————————————————————————————

// start a bar moving ////////////////////////////////////////////////////
void BarShow(BAR *bar, bool xneg, bool yneg)
{
   // set the status and last tick
   bar->ltick = gThisTick;
   bar->state = BAR_MOVING;

   // set the x & y velocity
   bar->xvel = ABS(bar->xvel);
   bar->yvel = ABS(bar->yvel);

   // make velocities negitive if necessary
   if(xneg) bar->xvel *= -1;
   if(yneg) bar->yvel *= -1;
}

// set a bar to one of the end positions /////////////////////////////////
void BarSetEnd(BAR *bar, bool xmin, bool ymin)
{
   bar->x = xmin? bar->xmin : bar->xmax;
   bar->y = ymin? bar->ymin : bar->ymax;
}

// update a bar //////////////////////////////////////////////////////////
void BarUpdate(BAR *bar)
{
   switch(bar->state) {
   
   // box is moving
   case BAR_MOVING:

      DWORD tickdiff;

      // update boxes position
      tickdiff   = gThisTick - bar->ltick; 
      bar->ltick = gThisTick;

      if(!gbPaused) {
         bar->x += bar->xvel * (double)(tickdiff);
         bar->y += bar->yvel * (double)(tickdiff);
      }

      // test to see if box x chord is done moving
      if(bar->xvel > 0 && bar->x >= bar->xmax) {
         bar->state = BAR_FINISHED;
         bar->x     = bar->xmax;
      } else if(bar->xvel < 0 && bar->x <= bar->xmin) {
         bar->state = BAR_FINISHED;
         bar->x     = bar->xmin;
      }

      // test to see if box y chord is done moving
      if(bar->yvel > 0 && bar->y >= bar->ymax) {
         bar->state = BAR_FINISHED;
         bar->y     = bar->ymax;
      } else if(bar->yvel < 0 && bar->y <= bar->ymin) {
         bar->state = BAR_FINISHED;
         bar->y     = bar->ymin;
      }
      break;

   // bar is done moving
   case BAR_FINISHED:
      bar->state = BAR_IDLE;
      break;
   }
}

// render a bar //////////////////////////////////////////////////////////
void BarRender(BAR *bar)
{
   if(bar->state != BAR_INACTIVE)
      DDBltClip(ddsBack, bar->surf, (long)bar->x, (long)bar->y, &bar->rclip, &bar->rsrc, SRCCK);
}


//————————————————————————————————————————————————————————————————————————
// Window Functions
//————————————————————————————————————————————————————————————————————————

// setup a window ////////////////////////////////////////////////////////
long Wnd(UCHAR resId, WNDLOOP wndloop)
{
   UCHAR k;
   long  i, j;
   long  x, y;
   char  mbtns[3][16];

   // set current window & its call loop
   gWndCurN++;
   gWndCur = gWndCurN > 1? &gWndB : &gWndA;
   memset(gWndCur, 0, sizeof(WND));
   gWndCur->loop = wndloop;

   // open window script file
   ResOpen(resId);

   // read winodw header
   ResRead(gWndCur->title , sizeof(gWndCur->title));
   ResRead(&gWndCur->width, sizeof(gWndCur->width));
   ResRead(&gWndCur->hight, sizeof(gWndCur->hight));
   ResRead(mbtns          , sizeof(mbtns));
   ResRead(&gWndCur->nnums, sizeof(gWndCur->nnums));
   ResRead(&gWndCur->nbtns, sizeof(gWndCur->nbtns));
   ResRead(&gWndCur->ntxts, sizeof(gWndCur->ntxts));

   // allocate window data
   if(gWndCur->nbtns > 0) gWndCur->pbtns = (BTN*)MoreMem(sizeof(BTN)*gWndCur->nbtns);
   if(gWndCur->nnums > 0) gWndCur->pnums = (NUM*)MoreMem(sizeof(NUM)*gWndCur->nnums);
   if(gWndCur->ntxts > 0) gWndCur->ptxts = (TXT*)MoreMem(sizeof(TXT)*gWndCur->ntxts);

   // read number control data
   for(k = 0; k < gWndCur->nnums; k++) {
      ResRead(gWndCur->pnums[k].title  , sizeof(gWndCur->pnums[k].title));
      ResRead(&gWndCur->pnums[k].x     , sizeof(gWndCur->pnums[k].x));
      ResRead(&gWndCur->pnums[k].y     , sizeof(gWndCur->pnums[k].y));
      ResRead(&gWndCur->pnums[k].labelw, sizeof(gWndCur->pnums[k].labelw));
      ResRead(&gWndCur->pnums[k].textw , sizeof(gWndCur->pnums[k].textw));
      ResRead(&gWndCur->pnums[k].max   , sizeof(gWndCur->pnums[k].max));
      ResRead(&gWndCur->pnums[k].min   , sizeof(gWndCur->pnums[k].min));
      ResRead(&gWndCur->pnums[k].incr  , sizeof(gWndCur->pnums[k].incr));
   }

   // read custom button data
   for(k = 0; k < gWndCur->nbtns; k++) {
      ResRead(&gWndCur->pbtns[k].type , sizeof(gWndCur->pbtns[k].type));
      ResRead(&gWndCur->pbtns[k].rdest, sizeof(gWndCur->pbtns[k].rdest));
   }

   // read text edit box data
   for(k = 0; k < gWndCur->ntxts; k++) {
      ResRead(&gWndCur->ptxts[k].rdest, sizeof(gWndCur->ptxts[k].rdest));
   }

   // close resource file
   ResClose();

   // calculate window surface chordinates
   if(gWndCurN > 1) {
      gWndCur->xsurf = (long)((gWndA.width - gWndCur->width) / 2);
      gWndCur->ysurf = (long)((gWndA.hight - gWndCur->hight) / 2);
   }

   // calculate window screen chordinates
   gWndCur->xscrn = (long)((SCREEN_W - gWndCur->width + 1) / 2);
   gWndCur->yscrn = (long)((SCREEN_H - gWndCur->hight + 1) / 2);

   // create main buttons
   for(i = 0; i < 3; i++) {

      // cache values
      x = gWndCur->width - 238 + 79*i;
      y = gWndCur->hight - 27;

      // setup button data
      wsprintf(gWndCur->mbtns[i].text, mbtns[i]);
      gWndCur->mbtns[i].state  = mbtns[i][0] == '#'? BTN_INACTIVE : BTN_IDLE;
      gWndCur->mbtns[i].type   = BTN_TEXT;
      gWndCur->mbtns[i].ddsdst = ddsWnd;

      // set surface rectangle of buttons
      RSETW(gWndCur->mbtns[i].rdest, gWndCur->xsurf + x, gWndCur->ysurf + y, 75, 22);

      // set screen chord of buttons
      gWndCur->mbtns[i].xscrn = gWndCur->xscrn + x;
      gWndCur->mbtns[i].yscrn = gWndCur->yscrn + y;
   }

   // translate number control data
   for(i = 0; i < gWndCur->nnums; i++) {

      // set the numbers value to the minimum
      gWndCur->pnums[i].value = gWndCur->pnums[i].min;

      // setup up arrows
      for(j = 0; j < 2; j++) {
         gWndCur->pnums[i].btns[j].type   = BTN_ARROW;
         gWndCur->pnums[i].btns[j].extra  = j;
         gWndCur->pnums[i].btns[j].state  = BTN_IDLE;
         gWndCur->pnums[i].btns[j].ddsdst = ddsWnd;

         // calculate surface rectangle
         RSETW(gWndCur->pnums[i].btns[j].rdest,
            gWndCur->xsurf + gWndCur->pnums[i].x + gWndCur->pnums[i].labelw + gWndCur->pnums[i].textw - 1,
            gWndCur->ysurf + gWndCur->pnums[i].y + 10*j, 13, 9 - j);

         // calculate screen chord
         gWndCur->pnums[i].btns[j].xscrn = gWndCur->pnums[i].btns[j].rdest.left + gWndCur->xscrn - gWndCur->xsurf;
         gWndCur->pnums[i].btns[j].yscrn = gWndCur->pnums[i].btns[j].rdest.top  + gWndCur->yscrn - gWndCur->ysurf;
      }
   }

   // translate custom button data
   for(i = 0; i < gWndCur->nbtns; i++) {
      gWndCur->pbtns[i].ddsdst = ddsWnd;
      gWndCur->pbtns[i].state  = BTN_IDLE;

      // calculate screen chords
      gWndCur->pbtns[i].xscrn = gWndCur->pbtns[i].rdest.left + gWndCur->xscrn - gWndCur->xsurf;
      gWndCur->pbtns[i].yscrn = gWndCur->pbtns[i].rdest.top  + gWndCur->yscrn - gWndCur->ysurf;
   }

   // translate text box data
   for(i = 0; i < gWndCur->ntxts; i++) {

      // set state
      gWndCur->ptxts[i].state = TXT_IDLE;

      // calculate screen rect
      gWndCur->ptxts[i].rscrn = gWndCur->ptxts[i].rdest;
      ROFST(gWndCur->ptxts[i].rscrn,
         gWndCur->xscrn - gWndCur->xsurf,
         gWndCur->yscrn - gWndCur->ysurf);
   }

   gbMsgBox = 1;                         // set msg box flag
   gWndCur->loop(gWndCur, WND_INIT, 0); // send initilize message to window
   WndDraw(&gWndA);                     // draw bottom window
   if(gWndCurN > 1) WndDraw(&gWndB);    // draw top window
   else WndLoop();                      // call window processing loop

   return gWndA.rval;
}

// the loop used when a window is being processed ////////////////////////
void WndLoop(void)
{
   MSG msg;

   // main event loop
   while(gbMsgBox) {
      if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
         if(msg.message == PROG_KILL) {
            prog_state = PROG_CLOSE;
            return;
         }
         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      ProgLoop();
   }
}

// update the current window /////////////////////////////////////////////
void WndUpdate(void)
{
   UCHAR i;

   // make sure cursor doesnt go outside window
   long right = gWndCur->xscrn + gWndCur->width - 1;
   long botom = gWndCur->yscrn + gWndCur->hight - 1;

   if(cur_x < gWndCur->xscrn) cur_x = gWndCur->xscrn;
   if(cur_y < gWndCur->yscrn) cur_y = gWndCur->yscrn;
   if(cur_x > right) cur_x = right;
   if(cur_y > botom) cur_y = botom;

   // update number boxes
   for(i = 0; i < gWndCur->nnums; i++) {
      NumUpdate(&gWndCur->pnums[i]);
   }

   // update custom buttons
   for(i = 0; i < gWndCur->nbtns; i++) {
      BtnUpdate(&gWndCur->pbtns[i]);
      if(BTN_USED(gWndCur->pbtns[i])) {
         if(gWndCur->loop(gWndCur, WND_CUSTBTN, i)) return;
      }
   }

   // update main buttons
   for(i = 0; i < 3; i++) {
      BtnUpdate(&gWndCur->mbtns[i]);
      if(BTN_USED(gWndCur->mbtns[i])) {
         if(gWndCur->loop(gWndCur, WND_MAINBTN, i)) return;
      }
   }
   
   // update text boxes
   gWndCur->cbox = -1;
   for(i = 0; i < gWndCur->ntxts; i++) {
      TxtUpdate(&gWndCur->ptxts[i]);
      if(gWndCur->ptxts[i].state == TXT_SELECTED) {
         gWndCur->cbox = i;
         gWndCur->loop(gWndCur, WND_TEXTBOX, i);
      }
   }

   // see if the user is tabing through the text boxes
   if(KEY_CLICK(DIK_TAB) && gWndCur->ntxts > 0) {
      if(gWndCur->cbox > -1) {
         gWndCur->ptxts[gWndCur->cbox].state = TXT_IDLE;
         TxtDraw(&gWndCur->ptxts[ gWndCur->cbox ]);
      }

      if(++gWndCur->cbox >= gWndCur->ntxts) gWndCur->cbox = 0;
      gWndCur->loop(gWndCur, WND_TEXTBOX, gWndCur->cbox);
      gWndCur->ptxts[ gWndCur->cbox ].state = TXT_SELECTED;
      TxtDraw(&gWndCur->ptxts[ gWndCur->cbox ]);
   }

   // send an update message to window
   gWndCur->loop(gWndCur, WND_UPDATE, 0);
}

// render the current window /////////////////////////////////////////////
void WndRender(void)
{
   RECT r;
   RSET(r, 0, 0, gWndA.width, gWndA.hight);
   DDBltFast(ddsBack, ddsWnd, gWndA.xscrn, gWndA.yscrn, &r, NOCK);
}

// close the top most window /////////////////////////////////////////////
void WndClose(long rval)
{
   // set return value of window
   gWndCur->rval = rval;

   // release window data
   if(gWndCur->nbtns > 0) LessMem(gWndCur->pbtns);
   if(gWndCur->nnums > 0) LessMem(gWndCur->pnums);
   if(gWndCur->ntxts > 0) LessMem(gWndCur->ptxts);

   // if top window was closed, re-draw the bottom one
   if(gWndCurN > 1) {
      gWndCurN = 1;
      gWndCur  = &gWndA;
      WndDraw(&gWndA);
   } else {
      gWndCurN = 0;
      gWndCur  = 0;
      gbMsgBox = 0;
   }
}

// draw a window /////////////////////////////////////////////////////////
void WndDraw(WND *wnd)
{
   int  i;
   RECT r;
   
   // fill the back of bottom window
   RSETW(r, wnd->xsurf, wnd->ysurf, wnd->width, wnd->hight);
   DDRect(ddsWnd, CLR_BLACK, CLR_LTAN, &r, 0);

   // draw the white line on the inside of the border
   REXP(r, -1); DDRect(ddsWnd, CLR_WHITE, 0, &r, DDRECT_NOFILL);

   // fill the title bar of back window
   RSETW(r, wnd->xsurf + 4, wnd->ysurf + 4, wnd->width - 8, 22);
   DDRect(ddsWnd, 0, CLR_BLUE, &r, DDRECT_NOBORDER);

   // draw the text for the title bar
   FntDraw(&gFntTitle, ddsWnd, wnd->title, lstrlen(wnd->title), wnd->xsurf + 7, wnd->ysurf + 8);

   // draw line above main buttons
   WndDrawLine(wnd, wnd->hight - 32);

   for(i = 0; i < 3; i++)          BtnDraw(&wnd->mbtns[i]); // main buttons
   for(i = 0; i < wnd->nbtns; i++) BtnDraw(&wnd->pbtns[i]); // custom buttons
   for(i = 0; i < wnd->nnums; i++) NumDraw(&wnd->pnums[i]); // number controls
   for(i = 0; i < wnd->ntxts; i++) TxtDraw(&wnd->ptxts[i]); // text boxes
   
   // send draw msg
   wnd->loop(wnd, WND_DRAW, 0);
}

// re-draw both windows on application restore ///////////////////////////
void WndReDraw(void)
{
   WndDraw(&gWndA);
   if(gWndCurN > 1) WndDraw(&gWndB);
}

// draw a group box on a window //////////////////////////////////////////
void WndDrawGroup(char *sText, long xa, long ya, long xb, long yb)
{
   RECT r   = {xa, ya, xb, yb};
   long len = strlen(sText);
   long w;

   // draw the group box
   DDRect(ddsWnd, CLR_DTAN, 1, &r, DDRECT_NOFILL);

   // get the widht of the string
   w = FntStrWidth(&gFntWndTxt, sText, len);

   // clear some of top line of group box then draw the text
   if(w > 0) DDLine(ddsWnd, CLR_LTAN, r.left + 4, r.top, w + r.left + 5, r.top);

   // draw the text
   FntDraw(&gFntWndTxt, ddsWnd, sText, len, xa + 5, ya - 5);
}

// draw a horizontal line on a window ////////////////////////////////////
void WndDrawLine(WND *wnd, long y)
{
   DDLine(ddsWnd, CLR_DTAN, wnd->xsurf + 6, wnd->ysurf + y,
   wnd->xsurf + wnd->width - 4, wnd->ysurf + y);
}


//————————————————————————————————————————————————————————————————————————
// Number Control Functions
//————————————————————————————————————————————————————————————————————————

// set the value of a number box /////////////////////////////////////////
void NumSetValue(NUM *num, long value)
{
   num->value = value;
   if(gbVisible) NumDrawData(num);
}

// update a number box ///////////////////////////////////////////////////
void NumUpdate(NUM *num)
{
   long save;

   if(num->disabled) return;

   // update buttons
   BtnUpdate(&num->btns[0]);
   BtnUpdate(&num->btns[1]);

   // make sure enough time has lapsed
   if(gThisTick - num->ltick > WAIT_NUMUPDATE) {
      num->ltick = gThisTick;

      // save current value
      save = num->value;

      // take input from buttons
      if(num->btns[0].flags & BTN_PUSHED) {
         if((num->value += num->incr) > num->max) {
            num->value = num->max;
            DsbPlay(dsbDefault, SOUND_CENTER);
         }
      } else if(num->btns[1].flags & BTN_PUSHED) {
         if((num->value -= num->incr) < num->min) {
            num->value = num->min;
            DsbPlay(dsbDefault, SOUND_CENTER);
         }
      }

      // draw data item if a change occured
      if(save != num->value) NumDrawData(num);
   }
}

// draw a number box /////////////////////////////////////////////////////
void NumDraw(NUM *num)
{
   RECT r;

   // draw label
   RSETW(r, num->x, num->y, num->labelw, 21);
   DDRect(ddsWnd, 0, CLR_LBLUE, &r, DDRECT_NOBORDER);
   FntDraw(&gFntWndTxtHil, ddsWnd, num->title, lstrlen(num->title), num->x + 4, num->y + 6);

   // draw data item
   NumDrawData(num);

   // draw buttons
   BtnDraw(&num->btns[0]);
   BtnDraw(&num->btns[1]);
}

// draw the data in a number box /////////////////////////////////////////
void NumDrawData(NUM *num)
{
   RECT r;
   char szText[32];

   // create text string
   if(num->numcust != 0) num->numcust(num->value, szText);
   else                  wsprintf(szText, "%d", num->value);

   // draw data item
   RSETW(r, num->x + num->labelw, num->y, num->textw, 21);
   DDRect(ddsWnd, CLR_LBLUE, CLR_LTAN, &r, 0);
   FntDrawCenter(&gFntWndTxt, ddsWnd, szText, &r);
}


//————————————————————————————————————————————————————————————————————————
// Button Functions
//————————————————————————————————————————————————————————————————————————

// enable/disable a button ///////////////////////////////////////////////
void BtnEnable(BTN *btn, bool enable)
{
   // set the button flags
   if(!enable) {
      BREM(btn->flags, BTN_PUSHED);
      BREM(btn->flags, BTN_TDOWN);
   }

   // set the button state
   btn->state = enable? BTN_IDLE : BTN_DISABLED;

   // re-draw the button
   if(gbVisible) BtnDraw(btn);
}

// update a button ///////////////////////////////////////////////////////
void BtnUpdate(BTN *btn)
{
   bool w;

   if(btn->state == BTN_INACTIVE) return;

   // determine if the mouse is within the button
   w = (cur_x >= btn->xscrn && cur_y >= btn->yscrn &&
      cur_x <= btn->xscrn + btn->rdest.right  - btn->rdest.left &&
      cur_y <= btn->yscrn + btn->rdest.bottom - btn->rdest.top);

   if(w) {

      // display a pop-up message text box 
      gsMsgNext = btn->msg;
      gMsgX     = btn->xscrn;
      gMsgY     = btn->yscrn;

      // see if the button should be re-drawn because of mouse movement
      if(btn->state != BTN_TRACKMOUSE && (btn->flags & BTN_CURWITHIN) == 0) {
         BSET(btn->flags, BTN_CURWITHIN);
         BtnDraw(btn);
         DsbPlay(dsbCurWithin, SOUND_CENTER);
      }

   // else if the mouse is not within a button but the CURWITHIN flag is still set
   } else if(btn->flags & BTN_CURWITHIN) {
      BREM(btn->flags, BTN_CURWITHIN);
      BtnDraw(btn);
   }

   switch(btn->state) {

   //--------------------------------------------
   // The button is disabled
   //--------------------------------------------
   case BTN_DISABLED:
      if(CUR_CLICK && w) DsbPlay(dsbDefault, SOUND_CENTER);
      break;

   //--------------------------------------------
   // Determine if the mouse should be watched
   //--------------------------------------------
   case BTN_IDLE:
      if(CUR_CLICK && w) btn->state = BTN_TRACKMOUSE;
      else break;

   //--------------------------------------------
   // Check the mouse for movement
   //--------------------------------------------
   case BTN_TRACKMOUSE:
      if(cur_down) {

         // if mouse has moved outside button
         if(btn->flags & BTN_PUSHED) {
            if(!w && (btn->flags & BTN_TDOWN) == 0) {
               BREM(btn->flags, BTN_PUSHED);
               BtnDraw(btn);
            }

         // if mouse has moved inside button
         } else if(w) {
            BSET(btn->flags, BTN_PUSHED);
            BtnDraw(btn);
         }

      } else {

         // if the mouse is still within the button, set the state to
         // BTN_WASUSED so other modules know the button was used        
         if(w) {
            if(btn->flags & BTN_TOGGLE) BINV(btn->flags, BTN_TDOWN);
            btn->state = BTN_WASUSED;
         } else btn->state = BTN_IDLE;

         // unpush button and re-draw
         BREM(btn->flags, BTN_PUSHED);
         BtnDraw(btn);
      }
      break;

   //--------------------------------------------
   // Button has been used
   //--------------------------------------------
   case BTN_WASUSED:
      btn->state = BTN_IDLE;
      break;
   }
}

// draw a button /////////////////////////////////////////////////////////
void BtnDraw(BTN *btn)
{
   RECT r;
   long n;
   bool d, w;

   // don't do anything if the button is inactive
   // or if there is no destination surface
   if(btn->state == BTN_INACTIVE) return;
 
   // determine if the button is down & if the mouse is within
   d = ((btn->flags & BTN_PUSHED) && btn->state == BTN_TRACKMOUSE) || (btn->flags & BTN_TDOWN);
   w = BISSETB(btn->flags, BTN_CURWITHIN);

   // figure out the draw mode
   if(d)      n = 2;
   else if(w) n = 1;
   else       n = 0;

   switch(btn->type) {

   // Draw a text button
   case BTN_TEXT:

      // draw back of button
      DDRect(btn->ddsdst, (d? CLR_BLUE : w? CLR_LBLUE : CLR_DTAN),
         (d? CLR_LBLUE : CLR_LTAN), &btn->rdest, 0);

      // draw text of button
      FntDrawCenter(d? &gFntWndTxtHil : btn->state == BTN_DISABLED?
         &gFntWndTxtDim : &gFntWndTxt, ddsWnd, btn->text, &btn->rdest);
      break;

   // Draw a menu button
   case BTN_MENUA:
      BtnDrawMenu(btn, d, w);
      break;

   // draw a window menu button
   case BTN_MENUB:

      // draw the standard menu stuff
      BtnDrawMenu(btn, d, w);

      // draw blue border
      RSET(r, btn->rdest.left - 1, btn->rdest.top - 1, btn->rdest.right + 1, btn->rdest.bottom + 1);
      DDRect(btn->ddsdst, w || d? CLR_BLUE : CLR_MENUBORDER, 0, &r, DDRECT_NOFILL);

      // re-draw the top & left lines
      if(d) {
         DDLine(btn->ddsdst, CLR_LBLUE, btn->rdest.left, btn->rdest.top, btn->rdest.right, btn->rdest.top);
         DDLine(btn->ddsdst, CLR_LBLUE, btn->rdest.left, btn->rdest.top, btn->rdest.left, btn->rdest.bottom);
      }

      break;

   // draw a show menu button
   case BTN_SHOWMENU:

      // if the menu is visible, increase value by 3
      if((long)(gBarMenu.y) == gBarMenu.ymax) n += 3;

      RSETWX(r, 0, 12*n, 22, 12);
      DDBltFast(btn->ddsdst, ddsSMenu, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;

   // Draw a check button
   case BTN_CHECKBOX:

      // draw check image
      RSETWY(r, 10*n + 35, 30, 10, 10);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);

      // draw the button text
      FntDraw(btn->state == BTN_DISABLED? &gFntWndTxtDim : &gFntWndTxt, btn->ddsdst,
         btn->text, strlen(btn->text), btn->rdest.left + 15, btn->rdest.top);
      break;

   // draw a play music button
   case BTN_PLAY:
      RSETWY(r, 10*n + 35, 21, 10, 9);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;
   
   // draw an up arrow
   case BTN_ARROW:
      RSETW(r, 12*n + 35, 10*btn->extra, 13, 11);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;

   // draw a scroll bar arrow
   case BTN_SCROLLARROW:
      RSETW(r, 12*n + 36, 10*btn->extra + 1, 11, 9);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;
   }
}

// draw a menu button ////////////////////////////////////////////////////
void BtnDrawMenu(BTN *btn, bool d, bool w)
{
   RECT r;

   if(d) {

      // fill back, draw top line, then draw right line
      DDRect(btn->ddsdst, 0, CLR_BTNPUSHED, &btn->rdest, DDRECT_NOBORDER);
      DDLine(btn->ddsdst, CLR_DTAN, btn->rdest.left , btn->rdest.top, btn->rdest.right, btn->rdest.top);
      DDLine(btn->ddsdst, CLR_DTAN, btn->rdest.left, btn->rdest.top, btn->rdest.left, btn->rdest.bottom);

   } else {

      // fill back, then draw top, right, bottom, & left
      DDRect(btn->ddsdst, 0, CLR_LTAN, &btn->rdest, DDRECT_NOBORDER);
      DDLine(btn->ddsdst, CLR_WHITE, btn->rdest.left   , btn->rdest.top     , btn->rdest.right-1, btn->rdest.top);
      DDLine(btn->ddsdst, CLR_DTAN , btn->rdest.right-1, btn->rdest.top   +1, btn->rdest.right-1, btn->rdest.bottom);
      DDLine(btn->ddsdst, CLR_DTAN , btn->rdest.left +1, btn->rdest.bottom-1, btn->rdest.right  , btn->rdest.bottom-1);
      DDLine(btn->ddsdst, CLR_WHITE, btn->rdest.left   , btn->rdest.top     , btn->rdest.left   , btn->rdest.bottom-1);

      // draw the blue line if the cursor is within the button
      if(w) {
         r = btn->rdest;
         REXP(r, -1);
         DDRect(btn->ddsdst, CLR_BTNPUSHED, 0, &r, DDRECT_NOFILL);
      }
   }

   // draw the button image
   RSETWX(r, 0, 16*btn->extra, 16, 16);
   DDBltFast(btn->ddsdst, ddsMenus, btn->rdest.left + d + 2, btn->rdest.top + d + 2, &r, SRCCK);

   // if the button is disabled, draw an X over it
   if(btn->state == BTN_DISABLED) {
      RSET(r, 35, 40, 51, 56);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left + 2, btn->rdest.top + 2, &r, SRCCK);
   }
}


//————————————————————————————————————————————————————————————————————————
// Text Box Functions
//————————————————————————————————————————————————————————————————————————

// check keyboard keys for input to a string /////////////////////////////
long TxtCheckKeys(char *pText, long *pTextLen, long maxLen)
{
   bool s;      // the shift keys are down
   long i;
   long j = -1;

   // determine which valid key is down
   for(i = 0; i <= DIK_SPACE; i++) {
      if(KEY_CLICK(i) && TRANS_KEYSL[i] != '~') j = i;
   }

   // determine wheater the shift keys are down
   s = key_down[DIK_LSHIFT] || key_down[DIK_RSHIFT];

   // backspace key was used
   if(j == DIK_BACKSPACE) {

      // shift key was used
      if(s) {
         *pTextLen = 0;
         pText[0]  = TXT_NULLCHAR;

      // delete one character
      } else if(*pTextLen > 0) {
         *pTextLen       -= 1;
         pText[*pTextLen] = TXT_NULLCHAR;
         return 1;
      }

   // any other key was used
   } else if(j > 0 && (*pTextLen) < maxLen - 1) {

      // add the key to the string
      pText[*pTextLen] = s? TRANS_KEYSU[j] : TRANS_KEYSL[j];
 
      // increment the string lenght
      (*pTextLen)++;

      // add a null terminator
      pText[*pTextLen] = TXT_NULLCHAR;

   // return no change
   } else return 0;

   // return a change has occured
   return 1;
}

// set the value of a text box ///////////////////////////////////////////
void TxtSetValue(TXT *txt, char *sText)
{
   txt->slen = lstrlen(sText);
   lstrcpy(txt->text, sText);
   if(gbVisible) TxtDraw(txt);
}

// update a text box /////////////////////////////////////////////////////
void TxtUpdate(TXT *txt)
{
   // save the current state
   UCHAR save = txt->state;
   bool  w    = RISIN(txt->rscrn, cur_x, cur_y);

   // if the mouse is within the button
   if(CUR_CLICK) {
      txt->state = w? TXT_SELECTED : TXT_IDLE;
   } else if(txt->state != TXT_SELECTED) {
      txt->state = w? TXT_CURWITHIN : TXT_IDLE;
   }

   // see if the text box should be drawn
   if(save != txt->state) {
      if(txt->state == TXT_CURWITHIN) DsbPlay(dsbCurWithin, SOUND_CENTER);
      TxtDraw(txt);
   }

   // check key input
   if(txt->state == TXT_SELECTED) {
      if( (txt->key? txt->key(txt) : TxtCheckKeys(txt->text, &txt->slen, 16)) || gCurChanged) TxtDraw(txt);
   }
}

// draw a text box ///////////////////////////////////////////////////////
void TxtDraw(TXT *txt)
{
   FNT  *fnt;
   long  x;

   // fill in the back
   DDRect(ddsWnd, txt->state == TXT_SELECTED? CLR_BLUE :
      txt->state == TXT_CURWITHIN? CLR_LBLUE : CLR_DTAN,
      txt->state == TXT_SELECTED? CLR_LBLUE : CLR_LTAN, &txt->rdest, 0);

   // determine which font to use
   fnt = txt->state == TXT_SELECTED? &gFntWndTxtHil : &gFntWndTxt;

   // draw the text
   x = FntDrawClip(fnt, ddsWnd, txt->text, txt->rdest.left + 4, txt->rdest.top + 5, txt->rdest.right);

   // draw the cursor after the text
   if(gCurVisible && txt->state == TXT_SELECTED)
      FntDrawClip(fnt, ddsWnd, TXT_CURCHAR, x, txt->rdest.top + 5, txt->rdest.right);
}


//————————————————————————————————————————————————————————————————————————
// Font Functions
//————————————————————————————————————————————————————————————————————————

// draw a font given an x & y choordinate pair ///////////////////////////
long FntDraw(FNT *fnt, LPDDS dds, char *sText, long sLen, long x, long y)
{
   RECT r;
   char c;
   long i;

   for(i = 0; i < sLen; i++) {

      // get array element ID
      c = sText[i] - 32;

      // make sure we dont process characters out of range
      if(0 > c || c > 90) continue;

      // render letter
      RSETWX(r, 0, c*fnt->cellh, fnt->cellw, fnt->cellh);
      DDRVAL(dds->BltFast(x, y, fnt->dds, &r, SRCCK | DDBLTFAST_WAIT));

      // update spaceing
      x += fnt->lwidths[c];
   }

   return x;
}

// draw a font given an x & y choordinate pair (dont excede xclip) ///////
long FntDrawClip(FNT *fnt, LPDDS dds, char *sText, long x, long y, long xclip)
{
   RECT r;
   char c;
   long i;
   long sLen = lstrlen(sText);

   for(i = 0; i < sLen; i++) {

      // get array element ID
      c = sText[i] - 32;

      // make sure we dont process characters out of range
      if(0 > c || c > 90) continue;

      // make sure the character will fit before xclip
      if(x + fnt->lwidths[c] > xclip) return x;

      // render letter
      RSETWX(r, 0, c*fnt->cellh, fnt->cellw, fnt->cellh);
      DDRVAL(dds->BltFast(x, y, fnt->dds, &r, SRCCK | DDBLTFAST_WAIT));

      // update spaceing
      x += fnt->lwidths[c];
   }

   return x;
}

// draw a font centered //////////////////////////////////////////////////
void FntDrawCenter(FNT *fnt, LPDDS dds, char *sText, RECT *r)
{
   long len = lstrlen(sText);

   FntDraw(fnt, dds, sText, len,
      (long)((r->right  + r->left - FntStrWidth(fnt, sText, len)) / 2),  // x chord
      (long)((r->bottom + r->top  - fnt->charh) / 2));                   // y chord
}

// draw a font with wrapping /////////////////////////////////////////////
void FntDrawWrap(FNT *fnt, LPDDS dds, char *sText, RECT *r)
{
   char sWord[32];
   long wdth;
   long wpos = 0;
   long spos = 0;
   long slen = lstrlen(sText);
   long x    = r->left;
   long y    = r->top;

   for(; spos <= slen; spos++) {

      // we have reached the end of a word
      if(sText[spos] == ' ' || sText[spos] == TXT_NULLCHAR || sText[spos] == '\r') {

         // get the width of the word
         wdth = FntStrWidth(fnt, sWord, wpos);

         // determine if we should go to the next line
         if(x + wdth > r->right) {
            y += fnt->cellh + 5;
            x  = r->left;
         }

         // draw the word
         FntDraw(fnt, dds, sWord, wpos, x, y);

         // see if we've hit a newline character
         if(sText[spos] == '\r') {
            y += fnt->cellh + 5;
            x  = r->left;
            spos++; // skip \n character

         // update x position
         } else {
            x += wdth;
            x += fnt->lwidths[0];
         }

         // reset word position
         wpos = 0;
         continue;
      }

      // copy character from string into word
      sWord[wpos] = sText[spos];
      wpos++;
   }
}

// calculate a strings width with a given font ///////////////////////////
long FntStrWidth(FNT *fnt, char *sText, long sLen)
{
   long i, w;

   // calculate string length in pixels
   for(i = w = 0; i < sLen; i++) {
      if(31 < sText[i] && sText[i] < 123) w += fnt->lwidths[ sText[i] - 32 ];
   }

   return w;
}

// load a font resource //////////////////////////////////////////////////
void FntLoad(FNT *fnt, UCHAR resId)
{
   ResOpen(resId);

   // read font information
   ResRead(&fnt->cellw , sizeof(fnt->cellw));
   ResRead(&fnt->cellh , sizeof(fnt->cellh));
   ResRead(&fnt->charh , sizeof(fnt->charh));
   ResRead(fnt->lwidths, sizeof(fnt->lwidths));

   // read font image
   DdsCreate(&fnt->dds, fnt->cellw, 91*fnt->cellh);
   XmpRead();
   XmpCopy(fnt->dds, 0, 0); 
   XmpDelete();

   ResClose();
}

// restore a font resource ///////////////////////////////////////////////
long FntRestore(FNT *fnt, UCHAR resId)
{
   // restore the surface
   if(fnt->dds->Restore() != DD_OK) return 0;

   // read font image
   ResOpen(resId);
   ResSeek(sizeof(fnt->cellw) + sizeof(fnt->cellh) + sizeof(fnt->charh) + sizeof(fnt->lwidths), SEEK_CUR);
   XmpRead();
   XmpCopy(fnt->dds, 0, 0); 
   XmpDelete();
   ResClose();
   return 1;
}

// delete a font resource ////////////////////////////////////////////////
void FntDelete(FNT *fnt)
{
   DXOBJ_DELETE(fnt->dds);
}