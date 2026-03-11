/************************************************************************/
// user.cpp - User interface code
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "game.h"
#include "user.h"
#include "network.h"


//————————————————————————————————————————————————————————————————————————
// Globals
//————————————————————————————————————————————————————————————————————————

// misc. globals /////////////////////////////////////////////////////////
char  TRANS_KEYSL[] = "~~1234567890-=?~qwertyuiop[]~~asdfghjkl;'~~\\zxcvbnm,./~*~ ";
char  TRANS_KEYSU[] = "~~!@#$%^&*()_+?~QWERTYUIOP{}~~ASDFGHJKL:\"~~|ZXCVBNM<>?~*~ ";
UCHAR CLRS_LHT[4] = {225, 221, 227, 246};
UCHAR CLRS_MED[4] = {137, 102, 166, 196};
UCHAR CLRS_DRK[4] = {124, 91 , 153, 178};
UCHAR CLRS_BLK[4] = {48 , 40 , 56 , 66};

RECT  R_SCREEN = {0, 0, SCREEN_W, SCREEN_H};
long  gnMenus  = 0;
void  (*NameDraw)(void);

// pop-up message box varables ///////////////////////////////////////////
char *gsMsgPrev = 0;
char *gsMsgNext = 0;
long  gMsgX     = 0;
long  gMsgY     = 0;
long  gMsgW     = 0;
DWORD gMsgTimer = 0;

// chat varables /////////////////////////////////////////////////////////
char gsChatText[4][64];
char gsChatSend[64];
long gsChatSendPos = 0;

// fonts /////////////////////////////////////////////////////////////////
FNT gFntTitle   ; // title font
FNT gFntNames   ; // player names font
FNT gFntTxt     ; // window text font
FNT gFntTxtHil  ; // 'hilighted' font
FNT gFntTxtDim  ; // 'disabled' font
FNT gFntTxtSmall; // small text

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
char *gsMenuHelp[9] = {
   "start a new game",
   "load a different game",
   "save this game",
   "save this game with a different name",
   "view the help file",
   "return to the real world",
   "set the music options",
   "turn the sound on or off"
};


//————————————————————————————————————————————————————————————————————————
// Game User Interface Code
//————————————————————————————————————————————————————————————————————————

// Update user interface /////////////////////////////////////////////////
void GameUpdateUI(void)
{
   RECT r;
   long i, j;

   // clear the current message
   gsMsgNext = 0;

   // update all of the utility bars
   BarUpdate(&gBarTeams);
   BarUpdate(&gBarChat);
   BarUpdate(&gBarMenu);
   BarUpdate(&gBarRoll);
   BarUpdate(&gBarDice[0]);
   BarUpdate(&gBarDice[1]);

   for(i = 0; i < nplrs; i++) {
      BarUpdate(&gBarNames[i]);

      // show the next bar
      if(game_state == GAME_WAITNAMEBARS) {
         j = i + 1; // get index of next bar
 
         if(gBarNames[i].state == BOX_FINISHED) {
            if(j < nplrs) BarShow(&gBarNames[j], (long)(gBarNames[j].x) == gBarNames[j].xmax, 0);
            else if(j == nplrs) {
               NameSelect();
               GameRoll();
            }
         }
      }
   }

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
   if(gBarMenu.state == BOX_FINISHED) BtnDraw(&gBtnShowMenu);

   // check keyboard for chat
   if((game_flags & GAME_NETWORKED) && (game_flags & GAME_MSGBOX) == 0) {

      // update the chat text
      if(TxtCheckKeys(gsChatSend, &gsChatSendPos, 32)) {
         ChatDrawSendText();
      }

      // see if text should be sent
      if(KEY_CLICK(DIK_RETURN) && gsChatSendPos > 0) {

         // send the message
         GameSendChat();

         // add message to screen
         ChatAddText((UCHAR)gThisPlr, gsChatSend);
         gsChatSend[0] = '\0';
         gsChatSendPos = 0;
         ChatDrawSendText();
      }
   }

   // update a players selecting name
   if(game_flags & GAME_SELECTNAME) {
      if(gThisTick - pdat[pcur].ltick >= WAIT_NAMEFLASH) {

         // see if we are done selecting the name
         if(++pdat[pcur].frame > 8) {
            BREM(game_flags, GAME_SELECTNAME);
         } else {

            // update the name bar
            pdat[pcur].ltick = gThisTick;
            NameSelectDraw((UCHAR)pcur, (pdat[pcur].frame + 1) % 2);
         }
      }
   }

   // update a players scrolling graph
   for(i = 0; i < 4; i++) {
      if(pdat[i].flags & PLR_SCROLLGRAPH) {
         if(gThisTick - pdat[i].ltick >= WAIT_GRAPH) {
            pdat[i].ltick  = gThisTick;
            pdat[i].frame += pdat[i].fdir;

            // draw the graph
            NameDrawGraph((UCHAR)i);

            // see if we are done scrolling
            if(pdat[i].frame == 0 || pdat[i].frame == 20) {
               BREM(pdat[i].flags, PLR_SCROLLGRAPH);
            }
         }
      }
   }

   // update the window
   if(game_flags & GAME_MSGBOX) WndUpdate();

   // update pop-up mesage
   if(gsMsgNext != gsMsgPrev) {
      if(gsMsgNext == 0) {
         gMsgW = 0;
      } else {

         // set the timer
         gMsgTimer = gThisTick;

         // re-draw the pop-up msg box
         gMsgW = 7 + FntStrWidth(&gFntTxtSmall, gsMsgNext, strlen(gsMsgNext));
         
         RSET(r, 0, 0, gMsgW, 17);
         DDRect(ddsMsg, CLR_BLACK, CLR_BTNPUSHED, &r, 0);
         
         REXP(r, -1);
         DDRect(ddsMsg, CLR_WHITE, 0, &r, DDRECT_NOFILL);
         FntDrawCentered(&gFntTxtSmall, ddsMsg, gsMsgNext, &r); 
      }

      // save locatoin of this message for next frame
      gsMsgPrev = gsMsgNext;
   }
}

// Render user interface /////////////////////////////////////////////////
void GameRenderUI(void)
{
   long i;
   RECT r;

   // render all of the utility bars
   BarRender(&gBarTeams);
   BarRender(&gBarChat);
   BarRender(&gBarMenu);
   BarRender(&gBarRoll);
   BarRender(&gBarDice[0]);
   BarRender(&gBarDice[1]);

   for(i = 0; i < nplrs; i++) BarRender(&gBarNames[i]);

   // render x's over the dice if necessary
   RSET(r, 25, 91, 41, 108);

   if(gBarDice[0].extra == 50)
      DDBltClip(ddsBack, ddsItems, (long)(gBarDice[0].x) + 5, (long)(gBarDice[0].y) + 5,
         &gBarDice[0].rclip, &r, SRCCK);

   if(gBarDice[1].extra == 50)
      DDBltClip(ddsBack, ddsItems, (long)(gBarDice[1].x) + 5, (long)(gBarDice[1].y) + 5,
         &gBarDice[1].rclip, &r, SRCCK);

   // render top of menubar
   RSET(r, 0, 0, 26, 15);
   DDBltFast(ddsBack, ddsMenuBar, 607, 7, &r, SRCCK);

   // render the window
   if(game_flags & GAME_MSGBOX) {
      RSET(r, 0, 0, gWndA.width, gWndA.hight);
      DDBltFast(ddsBack, ddsWnd, gWndA.xscrn, gWndA.yscrn, &r, NOCK);
   }

   // render the pop-up box
   if(gMsgW > 0 && gThisTick - gMsgTimer > WAIT_SHOWMSG) {
      RSET(r, 0, 0, gMsgW, 17);
      DDBltFast(ddsBack, ddsMsg, gMsgX - gMsgW, gMsgY, &r, NOCK);
   }

   // render the cursor
   RSET(r, 179, 0, 190, 19);
   DDBltClip(ddsBack, ddsItems, cur_x, cur_y, &R_SCREEN, &r, SRCCK);
}


//————————————————————————————————————————————————————————————————————————
// Setup functions
//————————————————————————————————————————————————————————————————————————

// initilize constant utility bars at the begining of a game /////////////
void GameInitBarsA(void)
{
   // clear the bars
   memset(&gBarTeams, 0, sizeof(gBarTeams));
   memset(&gBarChat , 0, sizeof(gBarTeams));
   memset(&gBarMenu , 0, sizeof(gBarTeams));
   memset(&gBarRoll , 0, sizeof(gBarTeams));

   //--------------------------------------------
   // team bar
   //--------------------------------------------
   gBarTeams.x     = 7;
   gBarTeams.xmax  = 7;
   gBarTeams.xmin  = 7;

   gBarTeams.y     = -32;
   gBarTeams.ymax  = 7;
   gBarTeams.ymin  = -32;
   gBarTeams.yvel  = VEL_TEAMBAR;

   gBarTeams.state = BOX_IDLE;
   gBarTeams.surf  = ddsTeams;
   gBarTeams.rclip = R_SCREEN;

   gBarTeams.rsrc.right  = 137;
   gBarTeams.rsrc.bottom = 31;

   //--------------------------------------------
   // chat bar
   //--------------------------------------------
   gBarChat.x     = 12;
   gBarChat.xmax  = 12;
   gBarChat.xmin  = 12;

   gBarChat.y     = 480;
   gBarChat.ymax  = 480;
   gBarChat.ymin  = 376;
   gBarChat.yvel  = VEL_CHATBAR;

   gBarChat.state = BOX_IDLE;
   gBarChat.surf  = ddsChat;
   gBarChat.rclip = R_SCREEN;

   gBarChat.rsrc.right  = 336;
   gBarChat.rsrc.bottom = 89;

   //--------------------------------------------
   // menu bar
   //--------------------------------------------
   gBarMenu.x     = 607;
   gBarMenu.xmax  = 607;
   gBarMenu.xmin  = 607;

   gBarMenu.y     = 22;
   gBarMenu.ymax  = 22;
   gBarMenu.yvel  = VEL_MENUBAR;

   gBarMenu.state = BOX_IDLE;
   gBarMenu.surf  = ddsMenuBar;
   RSET(gBarMenu.rclip, 607, 22, 633, 216);
   RSET(gBarMenu.rsrc , 0  , 17, 26 , 189);

   //--------------------------------------------
   // roll bar
   //--------------------------------------------
   gBarRoll.x     = 640;
   gBarRoll.xmax  = 640;
   gBarRoll.xmin  = 524;
   gBarRoll.xvel  = VEL_ROLLBAR;

   gBarRoll.y     = 320;
   gBarRoll.ymax  = 320;
   gBarRoll.ymin  = 320;

   gBarRoll.state = BOX_IDLE;
   gBarRoll.surf  = ddsDiceBar;
   gBarRoll.rclip = R_SCREEN;

   gBarRoll.rsrc.right  = 104;
   gBarRoll.rsrc.bottom = 35;
}

// initilize game specific bars //////////////////////////////////////////
void GameInitBarsB(void)
{
   long i;

   // clear the dice bars
   memset(gBarDice, 0, sizeof(gBarDice));

   //--------------------------------------------
   // game is in network mode
   //--------------------------------------------
   if(game_flags & GAME_NETWORKED) {

      // menu bar
      gBarMenu.ymin        = -64;
      gBarMenu.rsrc.bottom = 105;

      // dice
      for(i = 0; i < 2; i++) {
         gBarDice[i].x     = 29*i + 568;
         gBarDice[i].xmax  = (long)gBarDice[i].x;
         gBarDice[i].xmin  = (long)gBarDice[i].x;

         gBarDice[i].y     = 354;
         gBarDice[i].ymax  = 354;
         gBarDice[i].ymin  = 324;
         gBarDice[i].yvel  = VEL_NETDICE;

         gBarDice[i].state = BOX_IDLE;
         gBarDice[i].surf  = ddsDice;

         RSET(gBarDice[i].rclip, 568, 324, 623, 352);
      }

   //--------------------------------------------
   // game is in standard mode
   //--------------------------------------------
   } else {

      // menu bar
      gBarMenu.ymin        = -148;
      gBarMenu.rsrc.bottom = 189;

      // dice
      for(i = 0; i < 2; i++) {
         gBarDice[i].x     = 32*i + 291;
         gBarDice[i].xmax  = (long)gBarDice[i].x;
         gBarDice[i].xmin  = (long)gBarDice[i].x;

         gBarDice[i].y     = 480;
         gBarDice[i].ymax  = 480;
         gBarDice[i].ymin  = 408;
         gBarDice[i].yvel  = VEL_NORMALDICE;

         gBarDice[i].state = BOX_IDLE;
         gBarDice[i].surf  = ddsDice;
         gBarDice[i].rclip = R_SCREEN;
      }
   }
}


//————————————————————————————————————————————————————————————————————————
// Menu functions
//————————————————————————————————————————————————————————————————————————

// Enable a menu item ////////////////////////////////////////////////////
void MenuEnable(UCHAR id, bool on)
{
   // if we are in network mode, subtract 4 from the menuID
   if(game_flags & GAME_NETWORKED) id -= 4;
   BtnEnable(&gBtnMenus[id], on);
}

// Push a menu item down /////////////////////////////////////////////////
void MenuPush(UCHAR id)
{
   // if we are in network mode, subtract 4 from the menuID
   if(game_flags & GAME_NETWORKED) id -= 4;
   BSET(gBtnMenus[id].flags, BTN_TDOWN);
   BtnDraw(&gBtnMenus[id]);
}

// Draw the menu bar /////////////////////////////////////////////////////
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

// initilize the menubar /////////////////////////////////////////////////
void MenuInit(void)
{
   long i;
   long start;

   // get the number of menus & what menu to start at
   if(game_flags & GAME_NETWORKED) {
      gnMenus = 4;
      start   = 4;
   } else {
      gnMenus = 8;
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
      RSETW(gBtnMenus[i].rdest, 3, 21*i + 19, 20, 20);
   }

   // make the last button a toggle button
   gBtnMenus[gnMenus - 1].flags = BTN_TOGGLE;

   // draw the menubar
   MenuDraw();
}


//————————————————————————————————————————————————————————————————————————
// Chat Bar Functions
//————————————————————————————————————————————————————————————————————————

// draw the chat bar /////////////////////////////////////////////////////
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

// add text to the chat box //////////////////////////////////////////////
void ChatAddText(UCHAR p, char *sText)
{
   // move lines up
   strcpy(gsChatText[0], gsChatText[1]); 
   strcpy(gsChatText[1], gsChatText[2]); 
   strcpy(gsChatText[2], gsChatText[3]);

   // write to bottom line & re-draw text
   if(p == 4) wsprintf(gsChatText[3], ">> %s", sText);
   else       wsprintf(gsChatText[3], "%s: %s", pdat[p].name, sText);

   // re-draw the chat box
   ChatDrawText();
}

// re-draw the text in the chat box //////////////////////////////////////
void ChatDrawText(void)
{
   long i;

   // clear the old text
   RECT r = {5, 5, 330, 64};
   DDRect(ddsChat, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw the new text
   for(i = 0; i < 4; i++)
      FntDraw(&gFntTxtSmall, ddsChat, gsChatText[i], strlen(gsChatText[i]), 8, 14*i + 9);
}

// draw the send text ////////////////////////////////////////////////////
void ChatDrawSendText(void)
{
   long x;

   // clear the old text
   RECT r = {5, 68, 330, 83};
   DDRect(ddsChat, 0, CLR_LTAN, &r, DDRECT_NOBORDER);

   // draw the current send text followed by an underscore
   x = FntDraw(&gFntTxtSmall, ddsChat, gsChatSend, gsChatSendPos, 8, 72);
   FntDraw(&gFntTxtSmall, ddsChat, "_", 1, x, 72);
}

// initilize the chat box ////////////////////////////////////////////////
void ChatInit(void)
{
   // clear the button structures & chat text
   memset(gsChatText, 0, sizeof(gsChatText));
   memset(gsChatSend, 0, sizeof(gsChatSend));
   gsChatSendPos = 0;

   // draw the chat bar
   ChatDraw();
}



//————————————————————————————————————————————————————————————————————————
// Name bar functions
//————————————————————————————————————————————————————————————————————————

// Setup bars according to player statuses ///////////////////////////////
void NameSetup(void)
{
   long i, j;
   long a, b;

   // clear the name bars
   memset(gBarNames, 0, sizeof(gBarNames));

   //--------------------------------------------
   // network game
   //--------------------------------------------
   if(game_flags & GAME_NETWORKED) {

      NameDraw = NameDrawB;

      switch(nplrs) {
      case 2: a = 45; b = 387; break;
      case 3: a = 31; b = 379; break;
      case 4: a = 23; b = 375; break;
      }

      for(i = j = 0; i < nplrs; i++) {

         // move to the next active player
         while((pdat[j].flags & PLR_ON) == 0) j++;

         gBarNames[i].x     = 640;
         gBarNames[i].xmax  = 640;
         gBarNames[i].xmin  = 356;
         gBarNames[i].xvel  = VEL_CHATNAMEBAR;

         gBarNames[i].y     = a*i + b;
         gBarNames[i].ymin  = (long)gBarNames[i].y;
         gBarNames[i].ymax  = (long)gBarNames[i].y;

         gBarNames[i].extra = j;
         gBarNames[i].state = BOX_IDLE;
         gBarNames[i].surf  = ddsNames;
         gBarNames[i].rclip = R_SCREEN;
         RSETW(gBarNames[i].rsrc, 0, 22*j, 273, 22);

         j++;
      }

   //--------------------------------------------
   // standard game
   //--------------------------------------------
   } else {

      NameDraw = NameDrawA;

      for(i = j = 0; i < nplrs; i++) {

         // move to the next active player
         while((pdat[j].flags & PLR_ON) == 0) j++;
  
         a = i % 2; // if 1 then bar is on right
         b = nplrs == 2? 400 : i < 2? 372 : 425; // get bar y chord

         gBarNames[i].x     = a? 640 : -251;
         gBarNames[i].xmax  = a? 640 : 17;
         gBarNames[i].xmin  = a? 372 : -251;
         gBarNames[i].xvel  = VEL_NORMALNAMEBAR;

         gBarNames[i].y     = b;
         gBarNames[i].ymin  = (long)b;
         gBarNames[i].ymax  = (long)b;

         gBarNames[i].extra = j;
         gBarNames[i].state = BOX_IDLE;
         gBarNames[i].surf  = ddsNames;
         gBarNames[i].rclip = R_SCREEN;
         RSETW(gBarNames[i].rsrc, 0, 44*j, 251, 44);

         j++;
      }

      // if there are only 3 players, center the right box
      if(nplrs == 3) {
         gBarNames[1].y    = 400;
         gBarNames[1].ymin = 400;
         gBarNames[1].ymax = 400;
      }
   }

   // draw the names
   NameDraw();
}

// select the current players name ///////////////////////////////////////
void NameSelect(void)
{
   BSET(game_flags, GAME_SELECTNAME);
   pdat[pcur].ltick = gThisTick;
   pdat[pcur].frame = 0;
}

// scroll a players marble graph /////////////////////////////////////////
void NameChangeGraph(UCHAR p, long x)
{
   // add the change to the current number of marbles
   pdat[p].nmarbles += x;

   // we are loosing a marble
   if(x == -1) {
      pdat[p].fdir  = -1;
      pdat[p].frame = 20;

   // we are gaining a marble
   } else {
      pdat[p].fdir  = 1;
      pdat[p].frame = 0;
   }

   // set global varables
   BSET(pdat[p].flags, PLR_SCROLLGRAPH);
   pdat[p].ltick = gThisTick;
}

// draw a different border around a players name /////////////////////////
void NameSelectDraw(UCHAR p, UCHAR mode)
{
   RECT  r;
   UCHAR ca;
   UCHAR cb;

   // create the rectangle
   if(game_flags & GAME_NETWORKED) {
      RSETW(r, 0, 22*p, 273, 22);
   } else {
      RSETW(r, 0, 44*p, 250, 44);
   }

   // determine the colors
   switch(mode) {
   case 0: ca = CLRS_MED[p]; cb = CLRS_MED[p]; break;
   case 1: ca = CLR_BLUE   ; cb = CLR_BLUE   ; break;
   case 2: ca = CLR_BKGND  ; cb = CLR_BLACK  ; break;
   }

   // draw the first line
   DDRect(ddsNames, ca, 0, &r, DDRECT_NOFILL);
   
   // draw the second line
   REXP(r, -1);
   DDRect(ddsNames, cb, 0, &r, DDRECT_NOFILL);
}

// Draw all or a selected name (standard mode) ///////////////////////////
void NameDrawA(void)
{
   long i, j, x;
   RECT ra, rb;

   RSET(ra, 1, 1, 250, 43);
   RSET(rb, 2, 2, 249, 42);

   // clear the entire surface
   DDClear(ddsNames, CLR_BKGND);

   for(i = 0; i < 4; i++) {

      // cashe this common value
      x = 44*i;

      // fill back
      DDRect(ddsNames, CLR_BLACK, CLR_LGREY, &ra, 0);
      DDRect(ddsNames, CLR_WHITE, 0, &rb, DDRECT_NOFILL);

      // draw the player names
      FntDraw(&gFntNames, ddsNames, pdat[i].name, strlen(pdat[i].name), 7, x + 24);

      // draw the marble graph
      NameDrawGraph((UCHAR)i);

      // draw the spaces for number of games won
      for(j = 0; j < gnGames; j++) {
         NameDrawGreyBar(CLR_NGAMEMARKBORDER, CLR_LGREY, -5*j + 238, x + 23, 5, 13);
      }

      // draw the actual number of games won
      for(j = 0; j < pdat[i].games_won; j++) {
         NameDrawColorBar((UCHAR)i, -5*j + 238, x + 23, 5, 13);
      }

      // offset the rectangles
      ROFSTY(ra, 44);
      ROFSTY(rb, 44);
   }
}

// Draw all or a selected name (network mode) ////////////////////////////
void NameDrawB(void)
{
   long x;
   long i, j;
   RECT ra, rb;

   RSET(ra, 1, 1, 272, 21);
   RSET(rb, 2, 2, 271, 20);

   // clear the entire surface
   DDClear(ddsNames, CLR_BKGND);

   for(i = 0; i < 4; i++) {

      // cash this common value
      x = 22*i;

      // fill back
      DDRect(ddsNames, CLR_BLACK, CLR_LGREY, &ra, 0);
      DDRect(ddsNames, CLR_WHITE, 0, &rb, DDRECT_NOFILL);

      // draw the name
      FntDraw(&gFntTxtSmall, ddsNames, pdat[i].name, strlen(pdat[i].name), 6, x + 7);

      // draw the marble graph
      NameDrawGraph((UCHAR)i);

      // draw the spaces for number of games won
      for(j = 0; j < gnGames; j++) {
         NameDrawGreyBar(CLR_NGAMEMARKBORDER, CLR_LGREY, -5*j + 263, x + 4, 5, 13);
      }

      // draw the actual number of games won
      for(j = 0; j < pdat[i].games_won; j++) {
         NameDrawColorBar((UCHAR)i, -5*j + 263, x + 4, 5, 13);
      }

      // offset the rectangles
      ROFSTY(ra, 22);
      ROFSTY(rb, 22);
   }
}

// draw the number of marbles bar ////////////////////////////////////////
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

   if(game_flags & GAME_NETWORKED) {
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
   wb = (long)(scale * (double)(pdat[p].nmarbles));

   // ajust for scrolling
   if(pdat[p].flags & PLR_SCROLLGRAPH) {
      wb += (long)(scale * ( (double)(pdat[p].frame) / 20.00 ));
      if(pdat[p].fdir == 1) wb -= (long)scale;
   }

   // draw the bars & clear part of the graph border if necessary
   NameDrawGreyBar(CLR_GRAPHBORDER, CLR_GRAPHFILL, x, y, wa, 13);
   if(wb > 1) NameDrawColorBar(p, x, y, wb, 13);

   // clear the lines from the edge of the bar if necessary
   if( ((game_flags & GAME_NETWORKED)? (wb > 70 && wb < 84) : (wb > 114 && wb < 128)) ) {
      linex  = x + wb;
      lineya = y + 2;
      lineyb = y + 12;
      DDLine(ddsNames, CLRS_MED[p], linex - 1, lineya, linex - 1, lineyb);
      DDLine(ddsNames, CLRS_MED[p], linex    , lineya, linex    , lineyb);
   }

   // draw the text on top of the bars
   RSETW(r, x, y, wa+1, 14);
   wsprintf(sText, "%d", pdat[p].nmarbles);
   FntDrawCentered(&gFntTxtSmall, ddsNames, sText, &r);
}

// draw a colored bar ////////////////////////////////////////////////////
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
   buffer[x + 1 + yb*lpitch - lpitch] = CLRS_MED[p];
   buffer[xb - 1 + y*lpitch + lpitch] = CLRS_MED[p];

   // unlock the surface
   DDUnlock(ddsNames);

   if(w > 3) {
      // fill the remaining area
      RSET(r, x + 2, y + 2, xb - 1, yb - 1);
      DDRect(ddsNames, 0, CLRS_MED[p], &r, DDRECT_NOBORDER);
   }
}

// draw a grey bar ///////////////////////////////////////////////////////
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

//————————————————————————————————————————————————————————————————————————
// Dice & Team bars
//————————————————————————————————————————————————————————————————————————

// draw the dice bar /////////////////////////////////////////////////////
void DiceDraw(void)
{
   RECT r;

   // fill dice bar
   RSET(r, 0, 0, 104, 35);
   DDRect(ddsDiceBar, CLR_BLACK, CLR_LGREY, &r, 0);

   REXP(r, -1);
   DDRect(ddsDiceBar, CLR_WHITE, 0, &r, DDRECT_NOFILL);

   // draw text on dice bar
   FntDraw(&gFntNames, ddsDiceBar, TXT_ROLL, TXTLEN_ROLL, 7, 12);
}

// draw the team bar /////////////////////////////////////////////////////
void TeamDraw(void)
{
}


//————————————————————————————————————————————————————————————————————————
// Utility bars functions
//————————————————————————————————————————————————————————————————————————

// start a bar moving ////////////////////////////////////////////////////
void BarShow(BAR *bar, bool xneg, bool yneg)
{
   // set the status and last tick
   bar->ltick = gThisTick;
   bar->state = BOX_MOVING;

   // set the x & y velocity
   bar->xvel = ABS(bar->xvel);
   bar->yvel = ABS(bar->yvel);

   // make velocities negitive if necessary
   if(xneg) bar->xvel *= -1;
   if(yneg) bar->yvel *= -1;
}

// update a bar //////////////////////////////////////////////////////////
void BarUpdate(BAR *bar)
{
   switch(bar->state) {

   // the box isn't even visible
   case BOX_INACTIVE:
      break;
   
   // box is moving
   case BOX_MOVING:

      DWORD tickdiff;

      // update boxes position
      tickdiff   = gThisTick - bar->ltick; 
      bar->x    += bar->xvel * (double)(tickdiff);
      bar->y    += bar->yvel * (double)(tickdiff);
      bar->ltick = gThisTick;

      // test to see if box x chord is done moving
      if(bar->xvel > 0 && bar->x >= bar->xmax) {
         bar->state = BOX_FINISHED;
         bar->x     = bar->xmax;
      } else if(bar->xvel < 0 && bar->x <= bar->xmin) {
         bar->state = BOX_FINISHED;
         bar->x     = bar->xmin;
      }

      // test to see if box y chord is done moving
      if(bar->yvel > 0 && bar->y >= bar->ymax) {
         bar->state = BOX_FINISHED;
         bar->y     = bar->ymax;
      } else if(bar->yvel < 0 && bar->y <= bar->ymin) {
         bar->state = BOX_FINISHED;
         bar->y     = bar->ymin;
      }
      break;

   // box is done moving
   case BOX_FINISHED:
      bar->state = BOX_IDLE;
      break;
   }
}

// render a bar //////////////////////////////////////////////////////////
void BarRender(BAR *bar)
{
   if(bar->state != BOX_INACTIVE)
      DDBltClip(ddsBack, bar->surf, (long)bar->x,
         (long)bar->y, &bar->rclip, &bar->rsrc, SRCCK);
}


//————————————————————————————————————————————————————————————————————————
// Window functions
//————————————————————————————————————————————————————————————————————————

// Display a window //////////////////////////////////////////////////////
long Wnd(UCHAR id, WNDLOOP wndloop)
{
   UCHAR k;
   RES   res;
   long  i, j;
   char  mbtns[3][16];

   // set current window & its call loop
   gWndCurN++;
   gWndCur = gWndCurN > 1? &gWndB : &gWndA;
   gWndCur->loop = wndloop;

   // open window script file
   ResOpen(&res, id);

   // read winodw header
   ResRead(&res, gWndCur->title , sizeof(gWndCur->title));
   ResRead(&res, &gWndCur->width, sizeof(gWndCur->width));
   ResRead(&res, &gWndCur->hight, sizeof(gWndCur->hight));
   ResRead(&res,  mbtns         , sizeof(mbtns));
   ResRead(&res, &gWndCur->nnums, sizeof(gWndCur->nnums));
   ResRead(&res, &gWndCur->nbtns, sizeof(gWndCur->nbtns));
   ResRead(&res, &gWndCur->ntxts, sizeof(gWndCur->ntxts));

   // allocate window data
   if(gWndCur->nnums > 0) gWndCur->pnums = new NUM[gWndCur->nnums];
   if(gWndCur->nbtns > 0) gWndCur->pbtns = new BTN[gWndCur->nbtns];
   if(gWndCur->ntxts > 0) gWndCur->ptxts = new TXT[gWndCur->ntxts];

   // read number control data
   for(k = 0; k < gWndCur->nnums; k++) {
      ResRead(&res,  gWndCur->pnums[k].title , sizeof(gWndCur->pnums[k].title));
      ResRead(&res, &gWndCur->pnums[k].x     , sizeof(gWndCur->pnums[k].x));
      ResRead(&res, &gWndCur->pnums[k].y     , sizeof(gWndCur->pnums[k].y));
      ResRead(&res, &gWndCur->pnums[k].labelw, sizeof(gWndCur->pnums[k].labelw));
      ResRead(&res, &gWndCur->pnums[k].textw , sizeof(gWndCur->pnums[k].textw));
      ResRead(&res, &gWndCur->pnums[k].max   , sizeof(gWndCur->pnums[k].max));
      ResRead(&res, &gWndCur->pnums[k].min   , sizeof(gWndCur->pnums[k].min));
      ResRead(&res, &gWndCur->pnums[k].incr  , sizeof(gWndCur->pnums[k].incr));
   }

   // read custom button data
   for(k = 0; k < gWndCur->nbtns; k++) {
      ResRead(&res, &gWndCur->pbtns[k].type , sizeof(gWndCur->pbtns[k].type));
      ResRead(&res, &gWndCur->pbtns[k].rdest, sizeof(gWndCur->pbtns[k].rdest));
   }

   // read text edit box data
   for(k = 0; k < gWndCur->ntxts; k++) {
      ResRead(&res, &gWndCur->ptxts[k].rdest, sizeof(gWndCur->ptxts[k].rdest));
   }

   // close resource file
   ResClose(&res);

   // calculate window surface chordinates
   if(gWndCurN > 1) {
      gWndCur->xsurf = (long)((gWndA.width - gWndCur->width) / 2);
      gWndCur->ysurf = (long)((gWndA.hight - gWndCur->hight) / 2);
   } else {
      gWndCur->xsurf = 0;
      gWndCur->ysurf = 0;
   }

   // calculate window screen chordinates
   gWndCur->xscrn = (long)((SCREEN_W - gWndCur->width + 1) / 2);
   gWndCur->yscrn = (long)((SCREEN_H - gWndCur->hight + 1) / 2);

   // create main buttons
   for(i = 0; i < 3; i++) {

      // setup button data
      wsprintf(gWndCur->mbtns[i].text, mbtns[i]);
      gWndCur->mbtns[i].state  = mbtns[i][0] == '#'? BTN_INACTIVE : BTN_IDLE;
      gWndCur->mbtns[i].type   = BTN_TEXT;
      gWndCur->mbtns[i].ddsdst = ddsWnd;
      gWndCur->mbtns[i].flags  = 0;
      gWndCur->mbtns[i].msg    = 0;

      // set surface rectangle of buttons
      RSETW(gWndCur->mbtns[i].rdest,
         gWndCur->xsurf + gWndCur->width - 238 + 79*i,
         gWndCur->ysurf + gWndCur->hight - 27, 75, 22);

      // set screen chord of buttons
      gWndCur->mbtns[i].xscrn = gWndCur->xscrn + gWndCur->width - 238 + 79*i;
      gWndCur->mbtns[i].yscrn = gWndCur->yscrn + gWndCur->hight - 30;
   }

   // translate number control data
   for(i = 0; i < gWndCur->nnums; i++) {
      gWndCur->pnums[i].disabled = 0;
      gWndCur->pnums[i].ltick    = 0;
      gWndCur->pnums[i].numcust  = 0;
      gWndCur->pnums[i].value    = gWndCur->pnums[i].min;

      // setup up arrows
      for(j = 0; j < 2; j++) {
         gWndCur->pnums[i].btns[j].msg    = 0;
         gWndCur->pnums[i].btns[j].flags  = 0;
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
      wsprintf(gWndCur->pbtns[i].text, "");
      gWndCur->pbtns[i].msg    = 0;
      gWndCur->pbtns[i].flags  = 0;
      gWndCur->pbtns[i].ddsdst = ddsWnd;
      gWndCur->pbtns[i].state  = BTN_IDLE;

      // calculate screen chords
      gWndCur->pbtns[i].xscrn = gWndCur->pbtns[i].rdest.left + gWndCur->xscrn - gWndCur->xsurf;
      gWndCur->pbtns[i].yscrn = gWndCur->pbtns[i].rdest.top  + gWndCur->yscrn - gWndCur->ysurf;
   }

   // translate text box data
   for(i = 0; i < gWndCur->ntxts; i++) {

      // set text & state
      wsprintf(gWndCur->ptxts[i].text, "");
      gWndCur->ptxts[i].state  = TXT_IDLE;
      gWndCur->ptxts[i].center = 0;
      gWndCur->ptxts[i].key    = 0;

      // calculate screen rect
      gWndCur->ptxts[i].rscrn = gWndCur->ptxts[i].rdest;
      ROFST(gWndCur->ptxts[i].rscrn,
         gWndCur->xscrn - gWndCur->xsurf,
         gWndCur->yscrn - gWndCur->ysurf);
   }

   BSET(game_flags, GAME_MSGBOX);       // set msg box flag
   gWndCur->loop(gWndCur, WND_INIT, 0); // send initilize message to window
   WndDraw(&gWndA);                     // draw bottom window
   if(gWndCurN > 1) WndDraw(&gWndB);    // draw top window
   else WndLoop();                      // call window processing loop

   return gWndA.rval;
}

// Main window event loop ////////////////////////////////////////////////
void WndLoop(void)
{
   MSG msg;

   // main event loop
   while(game_flags & GAME_MSGBOX) {
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

// Update the windows ////////////////////////////////////////////////////
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

// Close the top window //////////////////////////////////////////////////
void WndClose(long rval)
{
   // set return value of window
   gWndCur->rval = rval;

   // release window data
   if(gWndCur->nbtns > 0) delete [] gWndCur->pbtns;
   if(gWndCur->nnums > 0) delete [] gWndCur->pnums;
   if(gWndCur->ntxts > 0) delete [] gWndCur->ptxts;

   // if top window was closed, re-draw the bottom one
   if(gWndCurN > 1) {
      gWndCurN = 1;
      gWndCur  = &gWndA;
      WndDraw(&gWndA);
   } else {
      gWndCurN = 0;
      gWndCur  = 0;
      BREM(game_flags, GAME_MSGBOX);
   }
}

// Draw the windows //////////////////////////////////////////////////////
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
   FntDraw(&gFntTitle, ddsWnd, wnd->title, strlen(wnd->title), wnd->xsurf + 7, wnd->ysurf + 8);

   // draw line above main buttons
   WndHLine(wnd, wnd->hight - 32);

   for(i = 0; i < 3; i++)          BtnDraw(&wnd->mbtns[i]); // main buttons
   for(i = 0; i < wnd->nbtns; i++) BtnDraw(&wnd->pbtns[i]); // custom buttons
   for(i = 0; i < wnd->nnums; i++) NumDraw(&wnd->pnums[i]); // number controls
   for(i = 0; i < wnd->ntxts; i++) TxtDraw(&wnd->ptxts[i]); // text boxes
   
   // send draw msg
   wnd->loop(wnd, WND_DRAW, 0);
}

// Re-draw the windows ///////////////////////////////////////////////////
void WndRedraw(void)
{
   WndDraw(&gWndA);
   if(gWndCurN > 1) WndDraw(&gWndB);
}

// Draw a group label ////////////////////////////////////////////////////
void WndGroup(WND *wnd, char *szText, long xa, long ya, long xb, long yb)
{
   RECT r   = {xa, ya, xb, yb};
   long len = strlen(szText);
   long w;

   // draw the group box
   DDRect(ddsWnd, CLR_DTAN, 1, &r, DDRECT_NOFILL);

   // get the widht of the string
   w = FntStrWidth(&gFntTxt, szText, len);

   // clear some of top line of group box then draw the text
   if(w > 0) DDLine(ddsWnd, CLR_LTAN, r.left + 4, r.top, w + r.left + 5, r.top);

   // draw the text
   FntDraw(&gFntTxt, ddsWnd, szText, len, xa + 5, ya - 5);
}

// Draw an h-line on the window //////////////////////////////////////////
void WndHLine(WND *wnd, long y)
{
   DDLine(ddsWnd, CLR_DTAN, wnd->xsurf + 6, wnd->ysurf + y,
      wnd->xsurf + wnd->width - 5, wnd->ysurf + y);
}


//————————————————————————————————————————————————————————————————————————
// Number control functions
//————————————————————————————————————————————————————————————————————————

// Set max & min value of a control //////////////////////////////////////
void NumSetBounds(NUM *num, long min, long max)
{
   // set max & min values
   num->max = max;
   num->min = min;

   // see if current value excedes limits
   if(num->value < min) {
      num->value = min;
      NumDrawData(num);
   } else if(num->value > max) {
      num->value = max;
      NumDrawData(num);
   }
}

// Set value of an edit box //////////////////////////////////////////////
void NumSetValue(NUM *num, long n)
{
   num->value = n;
   NumDrawData(num);
}

// Update a number control ///////////////////////////////////////////////
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
            /*DsbPlay(dsbNoNo);*/
         }
      } else if(num->btns[1].flags & BTN_PUSHED) {
         if((num->value -= num->incr) < num->min) {
            num->value = num->min;
            /*DsbPlay(dsbNoNo);*/
         }
      }

      // draw data item if a change occured
      if(save != num->value) NumDrawData(num);
   }
}

// Draw a number control /////////////////////////////////////////////////
void NumDraw(NUM *num)
{
   RECT r;

   // draw label
   RSETW(r, num->x, num->y, num->labelw, 21);
   DDRect(ddsWnd, 0, CLR_LBLUE, &r, DDRECT_NOBORDER);
   FntDraw(&gFntTxtHil, ddsWnd, num->title, strlen(num->title), num->x + 4, num->y + 6);

   // draw data item
   NumDrawData(num);

   // draw buttons
   BtnDraw(&num->btns[0]);
   BtnDraw(&num->btns[1]);
}

// Draw text in a number control /////////////////////////////////////////
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
   FntDrawCentered(&gFntTxt, ddsWnd, szText, &r);
}


//————————————————————————————————————————————————————————————————————————
// Button functions
//————————————————————————————————————————————————————————————————————————

// Enable/disable a button ///////////////////////////////////////////////
void BtnEnable(BTN *btn, bool on)
{
   // set the button flags
   if(!on) {
      BREM(btn->flags, BTN_PUSHED);
      BREM(btn->flags, BTN_TDOWN);
   }

   // set the button state
   btn->state = on? BTN_IDLE : BTN_DISABLED;

   // re-draw the button
   BtnDraw(btn);
}

// Update a button ///////////////////////////////////////////////////////
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
         /* play mouse within sound */
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
      /*if(CUR_CLICK && w) DsbPlay(dsbDefault);*/
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

// Draw a button /////////////////////////////////////////////////////////
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

   switch(btn->type) {

   //--------------------------------------------
   // Draw a text button
   //--------------------------------------------
   case BTN_TEXT:

      // draw back of button
      DDRect(btn->ddsdst, (d? CLR_BLUE : w? CLR_LBLUE : CLR_DTAN),
         (d? CLR_LBLUE : CLR_LTAN), &btn->rdest, 0);

      // draw text of button
      FntDrawCentered(d? &gFntTxtHil : btn->state == BTN_DISABLED? &gFntTxtDim : &gFntTxt,
         ddsWnd, btn->text, &btn->rdest);
      break;

   //--------------------------------------------
   // Draw a menu button
   //--------------------------------------------
   case BTN_MENUA:
      if(d) {

         // fill back, draw top line, then draw right line
         DDRect(btn->ddsdst, 0, CLR_BTNPUSHED, &btn->rdest, DDRECT_NOBORDER);
         DDLine(btn->ddsdst, CLR_DTAN,
            btn->rdest.left , btn->rdest.top,
            btn->rdest.right, btn->rdest.top);
         DDLine(btn->ddsdst, CLR_DTAN,
            btn->rdest.left, btn->rdest.top,
            btn->rdest.left, btn->rdest.bottom);

      } else {
         
         // fill back, then draw top, right, bottom, & left
         DDRect(btn->ddsdst, 0, CLR_LTAN, &btn->rdest, DDRECT_NOBORDER);

         // top line
         DDLine(btn->ddsdst, CLR_WHITE, 
            btn->rdest.left, btn->rdest.top,
            btn->rdest.right, btn->rdest.top);
         
         // right line
         DDLine(btn->ddsdst, CLR_DTAN,
            btn->rdest.right - 1, btn->rdest.top,
            btn->rdest.right - 1, btn->rdest.bottom);

         // bottom line
         DDLine(btn->ddsdst, CLR_DTAN,
            btn->rdest.left , btn->rdest.bottom - 1,
            btn->rdest.right, btn->rdest.bottom - 1);

         // left line
         DDLine(btn->ddsdst, CLR_WHITE,
            btn->rdest.left, btn->rdest.top,
            btn->rdest.left, btn->rdest.bottom);

         // draw the blue line if the cursor is within the button
         if(btn->flags & BTN_CURWITHIN) {
            r = btn->rdest;
            REXP(r, -1);
            DDRect(btn->ddsdst, CLR_BTNPUSHED, 0, &r, DDRECT_NOFILL);
         }
      }

      // draw the button image
      RSETW(r, 0, 16*btn->extra, 16, 16);
      DDBltFast(btn->ddsdst, ddsMenuItems, btn->rdest.left + d + 2,
         btn->rdest.top + d + 2, &r, SRCCK);

      // if the button is disabled, draw an X over it
      if(btn->state == BTN_DISABLED) {
         RSET(r, 168, 94, 184, 110);
         DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left + 3,
            btn->rdest.top + 3, &r, SRCCK);
      }
      break;

   //--------------------------------------------
   // Draw a show menu button
   //--------------------------------------------
   case BTN_SHOWMENU:
      if(d)      n = 2;
      else if(w) n = 1;
      else       n = 0;

      // if the menu is visible, increase value by 3
      if((long)(gBarMenu.y) == gBarMenu.ymax) n += 3;

      RSETW(r, 96, 12*n, 22, 12);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;

   //--------------------------------------------
   // Draw a check button
   //--------------------------------------------
   case BTN_CHECKBOX:
      if(d)      n = 2;
      else if(w) n = 1;
      else       n = 0;

      // draw check image
      RSETW(r, 10*n + 113, 90, 10, 10);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);

      // draw the button text
      FntDraw(btn->state == BTN_DISABLED? &gFntTxtDim : &gFntTxt, btn->ddsdst,
         btn->text, strlen(btn->text), btn->rdest.left + 15, btn->rdest.top);
      break;
   
   //--------------------------------------------
   // Draw an up arrow
   //--------------------------------------------
   case BTN_ARROW:
      if(d)      n = 2;
      else if(w) n = 1;
      else       n = 0;

      // draw check image
      RSETW(r, 12*n + 76, 10*btn->extra + 84, 13, 11);
      DDBltFast(btn->ddsdst, ddsItems, btn->rdest.left, btn->rdest.top, &r, NOCK);
      break;
   }
}


//————————————————————————————————————————————————————————————————————————
// Text box functions
//————————————————————————————————————————————————————————————————————————

// Send Keyboard input to a text box ////////////////////////////////////
long TxtCheckKeys(char *sText, long *len, long maxlen)
{
   long s;      // the shift keys are down
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
         sText[0] = '\0';
         *len     = 0;

      // delete one character
      } else if(*len > 0) {
         *len       -= 1;
         sText[*len] = '\0';
         return 1;
      }

   // any other key was used
   } else if(j > 0 && (*len) < maxlen) {

      // add the key to the string
      sText[*len] = s? TRANS_KEYSU[j] : TRANS_KEYSL[j];
 
      // increment the string lenght
      (*len)++;

      // add a null terminator
      sText[*len] = '\0';

   // return no change
   } else return 0;

   // return a change has occured
   return 1;
}

// Update a text box /////////////////////////////////////////////////////
void TxtUpdate(TXT *txt)
{
   // save the current state
   long  slen;
   UCHAR save = txt->state;
   bool  w    = RISIN(txt->rscrn, cur_x, cur_y);

   // if the mouse is within the button
   if(CUR_CLICK) {
      txt->state = w? TXT_SELECTED : TXT_IDLE;
   } else if(txt->state != TXT_SELECTED) {
      txt->state = w? TXT_CURWITHIN : TXT_IDLE;
   }

   // see if the button should be drawn
   if(save != txt->state) {
      /*if(txt->state == TXT_CURWITHIN) DsbPlay(dsbButton);*/
      TxtDraw(txt);
   }

   // check key input
   if(txt->state == TXT_SELECTED) {
      if(txt->key == 0) {

         slen = strlen(txt->text);
         if(TxtCheckKeys(txt->text, &slen, 16)) TxtDraw(txt);

      } else txt->key(txt);
   }
}

// Draw a text box ///////////////////////////////////////////////////////
void TxtDraw(TXT *txt)
{
   RECT r;
   char szText[32];

   // fill in the back
   DDRect(ddsWnd, txt->state == TXT_SELECTED? CLR_BLUE :
      txt->state == TXT_CURWITHIN? CLR_LBLUE : CLR_DTAN,
      txt->state == TXT_SELECTED? CLR_LBLUE : CLR_LTAN, &txt->rdest, 0);

   // setup text rect
   r = txt->rdest;
   if(!txt->center) r.left += 4;

   // see if a "_" should be appended to the string
   wsprintf(szText, txt->state == TXT_SELECTED? "%s_" : "%s", txt->text);

   // draw the text
   FntDraw(txt->state == TXT_SELECTED? &gFntTxtHil : &gFntTxt,
      ddsWnd, szText, strlen(szText), r.left, r.top + 5);
}


//————————————————————————————————————————————————————————————————————————
// Text Rendering functions
//————————————————————————————————————————————————————————————————————————

// load a font from the resource file ////////////////////////////////////
void FntLoad(FNT *fnt, UCHAR resid)
{
   RES res;
   XMP xmp;
   
   ResOpen(&res, resid);
   
   // read font information
   ResRead(&res, &fnt->cellw , sizeof(fnt->cellw));
   ResRead(&res, &fnt->cellh , sizeof(fnt->cellh));
   ResRead(&res, &fnt->charh , sizeof(fnt->charh));
   ResRead(&res, fnt->lwidths, sizeof(fnt->lwidths));
   
   // read font image
   DdsCreate(&fnt->dds, fnt->cellw, 91*fnt->cellh);
   XmpRead(&xmp, &res);
   XmpCopy(&xmp, fnt->dds, 0, 0); 
   XmpDelete(&xmp);

   ResClose(&res);
}

// release a fonts direct draw surface ///////////////////////////////////
void FntDelete(FNT *fnt)
{
   DXOBJ_DELETE(fnt->dds);
}

// restore a font ////////////////////////////////////////////////////////
long FntRestore(FNT *fnt, UCHAR resid)
{
   RES res;
   XMP xmp;

   // restore the surface
   if(fnt->dds->Restore() != DD_OK) return 0;

   // read font image
   ResOpen(&res, resid);
   ResSeek(&res, sizeof(fnt->cellw) + sizeof(fnt->cellh) + sizeof(fnt->charh) + sizeof(fnt->lwidths), SEEK_CUR);
   XmpRead(&xmp, &res);
   XmpCopy(&xmp, fnt->dds, 0, 0); 
   XmpDelete(&xmp);
   ResClose(&res);
   return 1;
}

// return the width of a string with a given font ////////////////////////
long FntStrWidth(FNT *fnt, char *sText, long len)
{
   long i, w;

   // calculate string length in pixels
   for(i = w = 0; i < len; i++) w += fnt->lwidths[ sText[i] - 32 ];
   return w;
}

// render a font to the screen given an x & y ////////////////////////////
long FntDraw(FNT *fnt, LPDDS dds, char *sText, long len, long x, long y)
{
   RECT r;
   char c;
   long i;

   for(i = 0; i < len; i++) {

      // get array element ID
      c = sText[i] - 32;

      // make sure we dont process characters out of range
      if(0 > c || c > 90) continue;

      // render letter
      RSETW(r, 0, c*fnt->cellh, fnt->cellw, fnt->cellh);
      DDRVAL(dds->BltFast(x, y, fnt->dds, &r, SRCCK | DDBLTFAST_WAIT));

      // update spaceing
      x += fnt->lwidths[c];
   }

   return x;
}

// draw centered text ////////////////////////////////////////////////////
void FntDrawCentered(FNT *fnt, LPDDS dds, char *sText, RECT *r)
{
   long len = strlen(sText);

   FntDraw(fnt, dds, sText, len,
      (long)((r->right + r->left - FntStrWidth(fnt, sText, len)) / 2),  // x chord
      (long)((r->bottom + r->top - fnt->charh) / 2));                   // y chord
}

// draw text with wrapping ///////////////////////////////////////////////
void FntDrawWrap(FNT *fnt, LPDDS dds, char *sText, RECT *r)
{
   char sWord[32];
   long wdth = 0;
   long wpos = 0;
   long spos = 0;
   long slen = strlen(sText);
   long x    = r->left;
   long y    = r->top;

   for(; spos <= slen; spos++) {

      // we have reached the end of a word
      if(sText[spos] == ' ' || sText[spos] == '\0') {

         // get the width of the word
         wdth = FntStrWidth(fnt, sWord, wpos);

         // determine if we should go to the next line
         if(x + wdth > r->right) {
            y += 2;
            y += fnt->cellh;
            x  = r->left;
         }

         // draw the word
         FntDraw(fnt, dds, sWord, wpos, x, y);
       
         // update x position
         x += wdth;
         x += fnt->lwidths[0];

         // reset word position
         wpos = 0;
         continue;
      }

      // copy character from string into word
      sWord[wpos] = sText[spos];
      wpos++;
   }
}