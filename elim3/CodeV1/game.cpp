/************************************************************************/
// game.cpp - Main game code
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

// local headers /////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "resdef.h"
#include "user.h"
#include "window.h"
#include "network.h"
#include "computer.h"
#include "game.h"


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// direct draw varables //////////////////////////////////////////////////
LPDDP ddpMain      = 0; // main palette
LPDDS ddsFront     = 0; // front buffer
LPDDS ddsBack      = 0; // back buffer

LPDDS ddsItems     = 0; // misc. graphics items
LPDDS ddsExplode   = 0; // marble explode animations
LPDDS ddsMenuItems = 0; // menu item drawings
LPDDS ddsDice      = 0; // dice drawings
LPDDS ddsSelect    = 0; // select marble drawing
LPDDS ddsBoard     = 0; // board image

LPDDS ddsMenuBar   = 0; // menu bar
LPDDS ddsTeams     = 0; // teams bar
LPDDS ddsChat      = 0; // chat bar
LPDDS ddsNames     = 0; // names bars
LPDDS ddsDiceBar   = 0; // dice bar
LPDDS ddsMsg       = 0; // pop-up message text
LPDDS ddsWnd       = 0; // message box

// direct sound buffers //////////////////////////////////////////////////
LPDSB dsbError     = 0;
LPDSB dsbExplode   = 0;
LPDSB dsbFree      = 0;
LPDSB dsbRise      = 0;
LPDSB dsbSelect    = 0;
LPDSB dsbSink      = 0;
LPDSB dsbNoNo      = 0;
LPDSB dsbMHide     = 0;
LPDSB dsbMShow     = 0;
LPDSB dsbCWithin   = 0;

// splash screen data ////////////////////////////////////////////////////
XMP    gXmpSplash;
LPDDS  ddsSplash;

// game varables /////////////////////////////////////////////////////////
PALETTEENTRY gPalette[256];               // palette for temp use
DWORD        gThisTick      = 0;          // tick at beginging of frame
DWORD        gLastTick      = 0;          // last tick counter for misc. jobs
DWORD        gTimer         = 0;          // timer for misc. jobs
double       gFrame         = 0;          // frame counter for misc. jobs
double       gGlowFrame     = 0;          // glowing "f" frame
DWORD        gGlowLTick     = 0;          // last tick "f"'s were updated

UCHAR        prog_state     = PROG_SPLASHA; // current program state
UCHAR        game_state     = GAME_IDLE;    // current game state
UCHAR        game_nxtst     = GAME_IDLE;    // next state
ULONG        game_flags     = GAME_SOUNDON | GAME_MUSICON; // game flags

long         gWinner        = 0;          // winner of the current game
long         gnGames        = 3;          // number of games to play
long         gnSinkHoles    = 25;         // number of sinkholes
long         gMoveDataValid = 0;
long         gThisPlr       = 0;         // player # for network game
long         gnTeamPlrs[2]  = {0,0};      // number of players on a team
long         gTeams[3][2]   = {{0,0},{0,0},{0,0}};
BYTE         gNetData[2]    = {0,0};     // buffer for simple network message
long         gnTeamPlrsRemain[2] = {0,0}; // number of remaining players on a team
BYTE         gMoveData[8];               // data used for a move
char         g_board[16][16];            // the location of things on the board

// player data ///////////////////////////////////////////////////////////
PLR  pdat[4];                           // player data
long nplrs       = 4;                   // number of players in game
long pcur        = 0;                   // ID of current player
long gnPlrsReady = 0;                   // how many players are ready for next game

// input data ////////////////////////////////////////////////////////////
double gSelectFrame = 0;                   // current selection frame
DWORD  gSelectLTick = 0;                   // last time of update
long   gCurWithinX  = -1;                  // x of marble cursor is near to
long   gCurWithinY  = -1;                  // y of marble cursor is near to
long   gSelectedX   = -1;                  // current selected marble
long   gSelectedY   = -1;
long   gMovesX[8]   = {-1,-1,-1,-1,-1,-1,-1,-1}; // possible moves
long   gMovesY[8]   = {-1,-1,-1,-1,-1,-1,-1,-1};                
long   gRiseSinkX   = -1;                  // current sinking/rising marble
long   gRiseSinkY   = -1;
long   gExplodeX    = -1;                  // current exploding marble
long   gExplodeY    = -1;

// move data /////////////////////////////////////////////////////////////
long   gMoveSrcX    = -1;
long   gMoveSrcY    = -1;
long   gMoveDstX    = -1;
long   gMoveDstY    = -1;
long   gMoveDstVal  = -1;

// flying object data ////////////////////////////////////////////////////
double gObjFrame    = 0;                   // current object frame
double gObjWScale   = 0;                   // object width scale
double gObjHScale   = 0;                   // object height scale
DWORD  gObjLTick    = 0;                   // tick of last frame
long   gObjValue    = 0;                   // the player who "owns" the marble
long   gObjX        = 0;                   // current x
long   gObjY        = 0;                   // current y
long   gObjSrcX     = -1;                  // source position
long   gObjSrcY     = -1;
long   gObjDstX     = -1;                  // destination position
long   gObjDstY     = -1;


//————————————————————————————————————————————————————————————————————————
// Main Game Loops
//————————————————————————————————————————————————————————————————————————

// Update the game ///////////////////////////////////////////////////////
void GameUpdate(void)
{
   switch(game_state) {

   //--------------------------------------------
   // a new game has been requested
   //--------------------------------------------
   case GAME_REQUESTNEW:

      // hide the bars
      GameHideBars();

      // change the state to avoid recursion
      game_state = GAME_IDLE;

      // request a new game
      switch(Wnd(WNDID_NEWGAME, WndNewGame)) {
      case 0: game_state = GAME_JOINNET; break; // go to a network game
      case 1: /*break;*/// load a game

      // start an offline game
      case 2:
         GameInitNew();

         // clear the players number of games won
         pdat[0].games_won = 0;
         pdat[1].games_won = 0;
         pdat[2].games_won = 0;
         pdat[3].games_won = 0;
         break;
      }

      break;


   //--------------------------------------------
   // We are thinking about joining a network game
   //--------------------------------------------
   case GAME_JOINNET:
      game_state = GAME_IDLE;
      
      switch(Wnd(WNDID_NETWORKA, WndJoinNet)) {
      case 0: game_state = GAME_REQUESTNEW; break; // cancel
      case 1: game_state = GAME_STARTNET;   break; // game has been hosted
      }
      break;

   //--------------------------------------------
   // We are are waiting to begin a new network round
   //--------------------------------------------
   case GAME_WAITNEWNET:

      // we are supposed to wait for the other
      // players before starting a new network game
      if(dpHost && gnPlrsReady == nplrs - 1) {
         GameInitNet();
         GameInitNew();
      }
      break;

   //--------------------------------------------
   // We are about to begin a network game
   //--------------------------------------------
   case GAME_STARTNET:
      game_state = GAME_IDLE;

      switch(Wnd(WNDID_NETWORKB, WndStartNet)) {
      case 0: game_state = GAME_JOINNET; break; // cancelel

      // play
      case 1:
         GameInitNew();

         // clear the players number of games won
         pdat[0].games_won = 0;
         pdat[1].games_won = 0;
         pdat[2].games_won = 0;
         pdat[3].games_won = 0;
         break;
      }
      break;

   //--------------------------------------------
   // we are showing all the marbles
   //--------------------------------------------
   case GAME_SHOWMARBLES:

      gFrame   += ((double)(gThisTick - gLastTick)) * VEL_SHOWMARBLE;
      gLastTick = gThisTick;

      // the animation has completed
      if(gFrame > 20) {

         // change game states
         game_state = game_nxtst;

         // we are done moving all the marbles in the beginng of the game
         if(gRiseSinkX == -1 && gRiseSinkY == -1) {

            // this will be undone if any bars are going to move
            gTimer = 0;

            // show the team bars
            if(gnTeamPlrs[0]) {
               BarShow(&gBarTeams, 0, 0);
               gTimer = gThisTick;
            }

            // show the chat & roll bars
            if(game_flags & GAME_NETWORKED) {
               BarShow(&gBarChat , 0, 1);
               BarShow(&gBarRoll , 1, 0);
               gTimer = gThisTick;
            }

            game_state = GAME_WAITBARS;

         // we were just creating a single marble
         } else {
            /* play the sound */
            GameEndTurn();
         }
      }
      break;

   //--------------------------------------------
   // we are showing the utility bars
   //--------------------------------------------
   case GAME_WAITBARS:
      if(gThisTick - gTimer > WAIT_UTILBARS) {
         BarShow(&gBarNames[0], gBarNames[0].x > 320, 0);
         game_state = GAME_WAITNAMEBARS;
      }
      break;

   //--------------------------------------------
   // the dice are being rolled
   //--------------------------------------------
   case GAME_ROLLDICE:
      if(gBarDice[0].state == BOX_FINISHED) BarShow(&gBarDice[1], 0, 1);
      if(gBarDice[1].state == BOX_FINISHED) {

         if((game_flags & GAME_NETWORKED) && gThisPlr != pcur) {
            game_state = GAME_WAITNETINPUT;
         } else {
            game_state = GAME_GETINPUT;
         }
      }
      break;

   //--------------------------------------------
   // waiting for the user to choose his move
   //--------------------------------------------
   case GAME_WAITNETINPUT:
      if(gMoveDataValid) {

         /* DEBUG */
         DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d", pcur,
            gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
            gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
         /*********/

         GameMoveMarble(gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3]);
         gMoveDataValid = 0;
      }
      break;

   //--------------------------------------------
   // waiting for the user to choose his move
   //--------------------------------------------
   case GAME_GETINPUT:

      // update the selection ring frame
      gSelectFrame += ((double)(gThisTick - gSelectLTick) * VEL_SELECTRING);
      gSelectLTick = gThisTick;
      if(gSelectFrame >= 24) gSelectFrame = 0;

      // update the moves frame
      gFrame    += ((double)(gThisTick - gLastTick) * VEL_MOVE);
      gLastTick  = gThisTick;
      if(gFrame > 8) gFrame = 8;

      // update the current "within" marble x
      if     (cur_x < 571 && cur_x > 545) gCurWithinX = 15;
      else if(cur_x > 106 && cur_x < 527) gCurWithinX = (long)((cur_x - 78) / 30);
      else if(cur_x > 68  && cur_x < 93 ) gCurWithinX = 0;
      else                                gCurWithinX = -1;

      // update the current "within" marble y
      if(cur_y < 340 && cur_y > 16) {
         if     (cur_y > 322) gCurWithinY = 15;
         else if(cur_y > 36)  gCurWithinY = (long)((cur_y - 16) / 20);
         else                 gCurWithinY = 0;
      } else gCurWithinY = -1;

      if(CUR_CLICK) {

         // see if a marble should be moved
         if(GameIsMove(gCurWithinX, gCurWithinY) >= 0) {

            // if the move will land on a free guy, get the chords of the new marble
            if(g_board[gCurWithinX][gCurWithinY] == MARBLE_FREE)
               MarbleNewGetPos();

            // save data in move data
            gMoveData[0] = (BYTE)gSelectedX;
            gMoveData[1] = (BYTE)gSelectedY;
            gMoveData[2] = (BYTE)gCurWithinX;
            gMoveData[3] = (BYTE)gCurWithinY;

            // get the new dice
            gMoveData[6] = RAND(6);
            gMoveData[7] = RAND(6);

            // send data via network
            if(game_flags & GAME_NETWORKED) GameSendMove();

            /* DEBUG */
            DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d", pcur,
               gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
               gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
            /*********/

            // move the marble then unselect it (sounds backwards, huh?)
            GameMoveMarble(gSelectedX, gSelectedY, gCurWithinX, gCurWithinY);
            GameUnselect();

         // see if a marble should be selected
         } else if(gCurWithinX != -1 && gCurWithinY != -1) {

            // make sure its the proper player
            if(g_board[gCurWithinX][gCurWithinY] == MARBLE_SINKHOLE + 1 + pcur) {

               // set the selected marble
               gSelectedX = gCurWithinX;
               gSelectedY = gCurWithinY;

               // get the possible moves
               GameSelectMoves();

            // player clicked on another players marble
            } else if(g_board[gCurWithinX][gCurWithinY] > MARBLE_SINKHOLE) {

            // de-select the marble
            } else if(g_board[gCurWithinX][gCurWithinY] == MARBLE_INVALID) {
               GameUnselect();
            }

         // unselect everything
         } else GameUnselect();
      }

      break;
 
   //--------------------------------------------
   // a bullet is flying through the air
   //--------------------------------------------
   case GAME_BULLETMOVE:

      // update the frame number
      gObjFrame += gThisTick - gObjLTick;
      gObjLTick  = gThisTick;

      // update the marble x & y chords
      gObjX = (long)(gObjWScale * gObjFrame + MarbleX(gObjSrcX) + 10);
      gObjY = (long)((.0012*gObjFrame*gObjFrame)-(0.6*gObjFrame)+(double)(gObjHScale*gObjFrame))+MarbleY(gObjSrcY) - 8;

      // the bullet is done moving, change the graph & blow up the marble
      if(gObjFrame >= 500) {
         NameChangeGraph((UCHAR)(g_board[gObjDstX][gObjDstY] - MARBLE_SINKHOLE - 1), -1);
         MarbleExplode(gObjDstX, gObjDstY);
      }
      break;

   //--------------------------------------------
   // the marble got it
   //--------------------------------------------
   case GAME_MARBLEEXPLODE:
      
      gFrame   += ((double)(gThisTick - gLastTick)) * VEL_EXPLODE;
      gLastTick = gThisTick;

      // the animation has completed, set the space to a free guy & try to move again
      if(gFrame > 11) {
         g_board[gObjDstX][gObjDstY] = MARBLE_FREE;
         GameMoveMarble(gMoveSrcX, gMoveSrcY, gMoveDstX, gMoveDstY);
      }
      break;

   //--------------------------------------------
   // da' marble, its flying!!!!!!
   //--------------------------------------------
   case GAME_MARBLEMOVE:

      // update the frame number
      gObjFrame += gThisTick - gObjLTick;
      gObjLTick  = gThisTick;

      // update the marble x & y chords
      gObjX = (long)(gObjWScale * gObjFrame + MarbleX(gObjSrcX) + 2);
      gObjY = (long)((.0012*gObjFrame*gObjFrame)-(0.6*gObjFrame)+(double)(gObjHScale*gObjFrame))+MarbleY(gObjSrcY) - 5;

      // the marble is done moving
      if(gObjFrame >= 500) {

         // ajust the dice
         if(abs(gObjDstX - gObjSrcX) + abs(gObjDstY - gObjSrcY) - 1 == gBarDice[0].extra) {
            gBarDice[0].extra = 50;
         } else {
            gBarDice[1].extra = 50;
         }

         // set the board item
         g_board[gObjDstX][gObjDstY] = (UCHAR)gObjValue;

         // decide what to do after landing
         if(gMoveDstVal == MARBLE_SINKHOLE) {
            NameChangeGraph((UCHAR)pcur, -1);
            MarbleSink(gObjDstX, gObjDstY);
         } else if(gMoveDstVal == MARBLE_FREE) {
            MarbleNew();
         } else {
            GameEndTurn();
         }
      }

      break;

   //--------------------------------------------
   // darn sink holes
   //--------------------------------------------
   case GAME_MARBLESINK:

      // update the frame
      gFrame   -= ((double)(gThisTick - gLastTick)) * VEL_HIDEMARBLE;
      gLastTick = gThisTick;

      // the animation has completed
      if(gFrame < 0) {
         if(gRiseSinkX == -1 && gRiseSinkY == -1) {

            // move to the next game state & clear all the marbles
            game_state = game_nxtst;
            memset(g_board, 0, sizeof(g_board));

         } else {

            // remove the marble from the board and end the turn
            g_board[gRiseSinkX][gRiseSinkY] = MARBLE_SPACE;
            GameEndTurn();
         }
      }
      break;

   //--------------------------------------------
   // the dice are going away
   //--------------------------------------------
   case GAME_HIDEDICE:
      if(gBarDice[0].state == BOX_FINISHED) BarShow(&gBarDice[1], 0, 0);
      if(gBarDice[1].state == BOX_FINISHED) game_state = GAME_NEXTPLAYER;
      break;
      
   //--------------------------------------------
   // the next player is up
   //--------------------------------------------
   case GAME_NEXTPLAYER:

      // remove the selection around the current player
      NameSelectDraw((UCHAR)pcur, 2);

      // go to the next player
      do {
         if(++pcur > 3) pcur = 0;
      } while((pdat[pcur].flags & PLR_ACTIVE) == 0);

      // start the next turn
      NameSelect();
      GameRoll();
      break;

   //--------------------------------------------
   // the round is over
   //--------------------------------------------
   case GAME_ROUNDOVER:

      // start a new network round
      if(game_flags & GAME_NETWORKED) {

         // prepair for the next game
         ChatAddText(4, "round over");
         game_state = GAME_WAITNEWNET;

         // send the host message that we are ready
         if(!dpHost) {
            gNetData[0] = NETMSG_READYFORNEW;
            DpSend(&gNetData[0], sizeof(gNetData[0]));
         }

      // start a new standard round
      } else {
         GameHideBars();
         game_state = GAME_IDLE;
         MsgBox("Round Over", TXT_GENTITLE, TXT_OKAY, 0);
         GameInitNew();
      }

      break;

   //--------------------------------------------
   // the game is over
   //--------------------------------------------
   case GAME_OVER:
      GameHideBars();
      game_state = GAME_IDLE;

      // end a network game
      if(game_flags & GAME_NETWORKED) {
         WndStartNetNewGame = 1;
         game_state = GAME_STARTNET;

      // end a standard game
      } else {
         if(MsgBox("Game Over", TXT_GENTITLE, "New", "Cancel") == 0) game_state = GAME_REQUESTNEW;
         else prog_state = PROG_CLOSE;
      }
      break;
   }

   // update the glow frame
   GameUpdateGlow();
}

// render the game ///////////////////////////////////////////////////////
void GameRender(void)
{
   RECT r;
   RECT rb;
   long i, j;
   long x, y;

   // render the board
   RSET(r, 0, 0, 517, 344);
   DDBltFast(ddsBack, ddsBoard, 61, 17, &r, NOCK);

   // "select" the moves
   for(i = 0; i < 8; i++) {
      if(gMovesX[i] != -1 && gMovesY[i] != -1) {
         RSETW(r, 0, 11*(long)(gFrame), 25, 11);
         DDBltFast(ddsBack, ddsItems, MarbleX(gMovesX[i]), MarbleY(gMovesY[i]), &r, NOCK);
      }
   }

   // the space is a possible move
   if(GameIsMove(gCurWithinX, gCurWithinY) >= 0) {
      RSET(r, 0, 99, 25, 110);
      DDBltFast(ddsBack, ddsItems, MarbleX(gCurWithinX), MarbleY(gCurWithinY), &r, SRCCK);
   }

   // render the marbles
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) {

         x = MarbleX(i);
         y = MarbleY(j);

         // the space is a marble
         if(g_board[i][j] > MARBLE_SINKHOLE) {

            // determine if the marble is rising/sinking
            if(((gRiseSinkX == i && gRiseSinkY == j) || (gRiseSinkX == -1 && gRiseSinkY == -1)) &&
               (game_state == GAME_SHOWMARBLES || game_state == GAME_MARBLESINK)) {

               // we are rendering the hole moving
               if(gFrame <= 5) {
                  RSETW(r, 25, 11*(long)(gFrame), 25, 11);
                  DDBltFast(ddsBack, ddsItems, x, y, &r, NOCK);

               // we are rendring the sinking marble
               } else {

                  // render the back
                  RSET(r, 25, 44, 50, 55);
                  DDBltFast(ddsBack, ddsItems, x, y, &r, NOCK);

                  // render the marble
                  RSET(rb, 0, 0, 640, y + 11);
                  RSETW(r, 133, (g_board[i][j] - MARBLE_SINKHOLE - 1)*20, 21, 21);
                  DDBltClip(ddsBack, ddsItems, x + 2, (long)(y - gFrame + 15), &rb, &r, SRCCK);

                  // render the front
                  RSET(r, 143, 88, 168, 93);
                  DDBltFast(ddsBack, ddsItems, x, y + 6, &r, SRCCK);
               }

            // determine if the marble is exploding
            } else if(game_state == GAME_MARBLEEXPLODE && gExplodeX == i && gExplodeY == j) {

               // render the marble
               if(gFrame < 6) {
                  RSETW(r, 154, (g_board[i][j] - MARBLE_SINKHOLE - 1)*16, 25, 16);
                  DDBltFast(ddsBack, ddsItems, x, y - 5, &r, SRCCK);
               
               // render a free guy
               } else {
                  RSET(r, 50, 97, 61, 110);
                  DDBltFast(ddsBack, ddsItems, x + 8, y - 5, &r, SRCCK);
               }

               // render the explosion
               RSETW(r, 29*(g_board[i][j] - MARBLE_SINKHOLE - 1), 24*(long)(gFrame), 29, 24);
               DDBltFast(ddsBack, ddsExplode, x - 2, y - 11, &r, SRCCK);

            // render a normal marble
            } else {

               // see if the marble is located on the top row
               if(((i < 3 || i > 12) && j == 3) || j == 0) {
                  DDLine(ddsBack, CLR_BKGND, x + 5, y - 4, x + 20, y - 4);
               }

               // render the marble
               RSETW(r, 154, (g_board[i][j] - MARBLE_SINKHOLE - 1)*16, 25, 16);
               DDBltFast(ddsBack, ddsItems, x, y - 5, &r, SRCCK);
            }
         
         // the space is a free marble
         } else if(g_board[i][j] == MARBLE_FREE) {
            RSET(r, 50, 97, 61, 110);
            DDBltFast(ddsBack, ddsItems, x + 8, y - 5, &r, SRCCK);
         }
      }
   }

   // render the "mouse within marble" stuff
   if(gCurWithinX != -1 && gCurWithinY != -1 &&
      (gCurWithinX != gSelectedX || gCurWithinY != gSelectedY)) {

      // the space is this players marble
      if(g_board[gCurWithinX][gCurWithinY] == MARBLE_SINKHOLE + pcur + 1) {
         RSET(r, 154, 64, 183, 84);
         DDBltFast(ddsBack, ddsItems, MarbleX(gCurWithinX) - 2, MarbleY(gCurWithinY) - 7, &r, SRCCK);
      }
   }

   // render the selection ring
   if(gSelectedX != -1 && gSelectedY != -1) {
      RSETW(r, 0, 18*(long)(gSelectFrame), 27, 18);
      DDBltFast(ddsBack, ddsSelect, MarbleX(gSelectedX) - 1, MarbleY(gSelectedY) - 7, &r, SRCCK);
   }

   // render the "flying marble" if there is one
   if(game_state == GAME_MARBLEMOVE) {
      RSETW(r, 133, 20*(gObjValue-MARBLE_SINKHOLE-1), 21, 21);
      RSET(rb, 0, 0, 640, 480);

      // edit the clipping rect if needed
      if(gObjFrame < 25)  rb.bottom = MarbleY(gObjSrcY) + 11;
      if(gObjFrame > 475) rb.bottom = MarbleY(gObjDstY) + 11;

      DDBltClip(ddsBack, ddsItems, gObjX, gObjY, &rb, &r, SRCCK);

      // acount for closeness to the starting/ending points
      RSET(r, 143, 88, 168, 93);
      if(gObjFrame < 25) {
         DDBltFast(ddsBack, ddsItems, MarbleX(gObjSrcX), MarbleY(gObjSrcY) + 6, &r, SRCCK);
      } else if(gObjFrame > 475) {
         DDBltFast(ddsBack, ddsItems, MarbleX(gObjDstX), MarbleY(gObjDstY) + 6, &r, SRCCK);
      }
   }
   
   // render the bullet if there is one
   if(game_state == GAME_BULLETMOVE) {
      RSETW(r, 5*(gObjValue-MARBLE_SINKHOLE-1) + 93, 105, 5, 5);
      DDBltClip(ddsBack, ddsItems, gObjX, gObjY, &R_SCREEN, &r, SRCCK);
   }
}

//————————————————————————————————————————————————————————————————————————
// Move functions
//————————————————————————————————————————————————————————————————————————

// move a marble /////////////////////////////////////////////////////////
void GameMoveMarble(long ax, long ay, long bx, long by)
{
   long dist;
   long dirx;
   long diry;
   long i, a, b;

   // save the move chordinates
   gMoveSrcX   = ax;
   gMoveSrcY   = ay;
   gMoveDstX   = bx;
   gMoveDstY   = by;
   gMoveDstVal = g_board[bx][by];

   // setup data common to bullets & the moving marble
   gObjValue = g_board[ax][ay];
   gObjX     = MarbleX(ax);
   gObjY     = MarbleY(ay);
   gObjLTick = gThisTick;
   gObjSrcX  = ax;
   gObjSrcY  = ay;
   gObjFrame = 0;

   // see if there are any enemy marbles in the way
   dist = abs(ax - bx) + abs(ay - by);
   dirx = ax > bx? -1 : ax == bx? 0 : 1;
   diry = ay > by? -1 : ay == by? 0 : 1;

   for(i = 1; i < dist; i++) {
      a = dirx * i + ax;
      b = diry * i + ay;

      if(g_board[a][b] > MARBLE_SINKHOLE) {
         if(GameIsEnemy(pcur, g_board[a][b] - MARBLE_SINKHOLE - 1)) {

            // setup the bullet to move
            gObjWScale  = (double)(MarbleX(a) - gObjX) / 500.00;
            gObjHScale  = (double)(MarbleY(b) - gObjY) / 500.00;
            gObjX      += 10;
            gObjY      -= 8;
            gObjDstX    = a;
            gObjDstY    = b;

            // change the game state & exit
            game_state = GAME_BULLETMOVE;
            return;
         }
      }
   }

   // setup the marble to move
   gObjWScale  = (double)(MarbleX(bx) - gObjX) / 500.00;
   gObjHScale  = (double)(MarbleY(by) - gObjY) / 500.00;
   gObjX      += 2;
   gObjY      -= 5;
   gObjDstX    = bx;
   gObjDstY    = by;

   // remove the marble from the board
   g_board[ax][ay] = MARBLE_SPACE;

   // change the game state
   game_state = GAME_MARBLEMOVE;
}

// unselect the current marble ///////////////////////////////////////////
void GameUnselect(void)
{
   gSelectedX  = -1;
   gSelectedY  = -1;
   gCurWithinX = -1;
   gCurWithinY = -1;
   GameSelectMoves();
}

// determine weather a locatoin is a possible move ///////////////////////
long GameIsMove(long a, long b)
{
   long i;

   // count it out if the location is invalid
   if(a == -1 || b == -1) return -1;
   for(i = 0; i < 8; i++) if(gMovesX[i] == a && gMovesY[i] == b) return i;
   return -1;
}

// select the possible moves /////////////////////////////////////////////
void GameSelectMoves(void)
{
   long i;
   long x;
   long dirX[] = {0, 1, 0, -1};
   long dirY[] = {-1, 0, 1, 0};

   gFrame = 0;

   // deselect the moves
   if(gSelectedX == -1 || gSelectedY == -1) {
      for(i = 0; i < 8; i++) {
         gMovesX[i] = -1;
         gMovesY[i] = -1;
      }
   
   // select the moves
   } else {

      // get the moves
      for(i = 0; i < 4; i++) {

         x = 2*i;
         gMovesX[x] = gSelectedX + dirX[i]*(gBarDice[0].extra+1);
         gMovesY[x] = gSelectedY + dirY[i]*(gBarDice[0].extra+1);

         x++;
         gMovesX[x] = gSelectedX + dirX[i]*(gBarDice[1].extra+1);
         gMovesY[x] = gSelectedY + dirY[i]*(gBarDice[1].extra+1);
      }

      // validate the moves
      for(i = 0; i < 8; i++) {
         if(gMovesX[i] > 14 || gMovesX[i] < 1 || gMovesY[i] > 14 || gMovesY[i] < 1) {
            gMovesX[i] = -1;
            gMovesY[i] = -1;
         } else if(g_board[ gMovesX[i] ][ gMovesY[i] ] > MARBLE_SINKHOLE || 
            g_board[ gMovesX[i] ][ gMovesY[i] ] == MARBLE_INVALID) {
            gMovesX[i] = -1;
            gMovesY[i] = -1;
         }
      }
   }
}


//————————————————————————————————————————————————————————————————————————
// Network Functions
//————————————————————————————————————————————————————————————————————————

// initilize a network game //////////////////////////////////////////////
void GameInitNet(void)
{
   BYTE* data;

   // re-initilize stuff
   GameInitBoard();

   // choose a random player
   do { pcur = RAND(4); } while((pdat[pcur].flags & PLR_ON) == 0);

   // give the first set of dice a value
   gMoveData[6] = RAND(6);
   gMoveData[7] = RAND(6);

   // create the message
   data    = new BYTE[sizeof(g_board) + 6];
   data[0] = NETMSG_STARTGAME;
   
   // copy the board
   memcpy(data + 1, g_board, sizeof(g_board));

   // copy in misc. data
   data[sizeof(g_board) + 2] = gMoveData[6];
   data[sizeof(g_board) + 3] = gMoveData[7];
   data[sizeof(g_board) + 4] = (BYTE)pcur;
   data[sizeof(g_board) + 5] = (BYTE)gnGames;

   // send the data
   DpSend(data, sizeof(g_board) + 6);
   delete [] data;

   // clear the number of players ready
   gnPlrsReady = 0;
}

// send a chat message ///////////////////////////////////////////////////
void GameSendChat(void)
{
   BYTE *data;

   // create the message
   data    = new BYTE[gsChatSendPos + 3];
   data[0] = NETMSG_CHATB;

   // copy the string
   memcpy(data + 1, gsChatSend, gsChatSendPos);

   // add a null terminator to string
   data[gsChatSendPos + 2] = '\0';

   // send & release data
   DpSend(data, gsChatSendPos + 3);
   delete [] data;
}

// send a move ///////////////////////////////////////////////////////////
void GameSendMove(void)
{
   BYTE *data;

   // create the message
   data    = new BYTE[sizeof(gMoveData) + 1];
   data[0] = NETMSG_MOVEMARBLE;

   // copy the move data
   memcpy(data + 1, gMoveData, sizeof(gMoveData));

   // send & release the message
   DpSend(data, sizeof(gMoveData) + 1);
   delete [] data;
}

// one of the network players has disconnected ///////////////////////////
void GameKillNetPlr(DPID dpId)
{
   long p = -1;
   char sText[64];

   // determine which player quit
   for(long i = 0; i < 4; i++) if(pdat[i].dpId == dpId) p = i;

   // make sure we even found a player
   if(p == -1) return;

   // post the message in the chat
   wsprintf(sText, TXT_PLRDISCONNECT, pdat[p].name);
   ChatAddText(4, sText);

   // disable the player
   BREM(pdat[p].flags, PLR_ON);
   BREM(pdat[p].flags, PLR_ACTIVE);
   nplrs--;

   // if there are no more players, the game is over
   if(nplrs < 2) {
      gWinner    = gThisPlr;
      game_state = GAME_OVER;

   // if it was the players turn, go to the next turn
   } else if(p == pcur) {
      game_state = GAME_HIDEDICE;
      BarShow(&gBarDice[0], 0, 0);
   }
}


//————————————————————————————————————————————————————————————————————————
// Misc. Functions
//————————————————————————————————————————————————————————————————————————

// see if the game is over ///////////////////////////////////////////////
long GameIsOver(void)
{
   long i;
   long teamgone = 1;  // is the team gone?
   long over     = 0;  // is the game over?
   long remain   = -1; // ID of player that remains
   long nout     = 0;  // number of out players

   // see how many people are out
   for(i = 0; i < 4; i++) {
      if(pdat[i].flags & PLR_ON) {
         if(pdat[i].flags & PLR_ACTIVE) remain = i;
         else nout++;
      }
   }

   // check to see if everyone is gone
   if(nout == nplrs) {
      over       = 1;
      gWinner    = -1;
      game_nxtst = gnGames > 1? GAME_ROUNDOVER : GAME_OVER;

   // there are no teams to check
   } else if(gnTeamPlrs[0] <= 0) {

      // decide if the game is over
      if(nout + 1 == nplrs) {
         pdat[remain].games_won++;

         over       = 1;
         gWinner    = remain;
         game_nxtst = (pdat[remain].games_won >= gnGames)? GAME_OVER : GAME_ROUNDOVER;
      }

   // check teams
   } else {

      // team A is gone, team B wins
      if(gnTeamPlrsRemain[0] == 0) {

         // ajust their games won varables
         for(i = 0; i < gnTeamPlrs[1]; i++) pdat[ gTeams[i][1] ].games_won++;

         over       = 1;
         gWinner    = 1;
         game_nxtst = (pdat[ gTeams[1][0] ].games_won >= gnGames)? GAME_OVER : GAME_ROUNDOVER;
      
      // team B is gone, team A wins
      } else if(gnTeamPlrsRemain[1] == 0) {

         // ajust their games won varables
         for(i = 0; i < gnTeamPlrs[0]; i++) pdat[ gTeams[i][0] ].games_won++;

         over       = 1;
         gWinner    = 0;
         game_nxtst = (pdat[ gTeams[0][0] ].games_won >= gnGames)? GAME_OVER : GAME_ROUNDOVER;
      }
   }

   // set the marbles to sink into the board
   if(over) {

      // hide the bars now if its a network game
      if(game_flags & GAME_NETWORKED) {
         BarShow(&gBarNames[0], 0, 0);
         BarShow(&gBarNames[1], 0, 0);
         BarShow(&gBarNames[2], 0, 0);
         BarShow(&gBarNames[3], 0, 0);
         BarShow(&gBarDice[0], 0, 0);
         BarShow(&gBarDice[1], 0, 0);
      }

      NameDraw();
      gFrame     = 20;
      gRiseSinkX = -1;
      gRiseSinkY = -1;
      gLastTick  = gThisTick;
      game_state = GAME_MARBLESINK;
   }

   return over;
}

// check to see if any players are out ///////////////////////////////////
long GameCheckPlayers(void)
{
   long i, j, r;

   j = r = 0;

   for(i = 0; i < 4; i++) {
      if(pdat[i].flags & PLR_ON) {

         // the players is out, hide their name
         if(pdat[i].nmarbles <= 0) {
            r = 1;
            BREM(pdat[i].flags, PLR_ACTIVE);
            gnTeamPlrsRemain[ pdat[i].team ]--;
            BarShow(&gBarNames[j], gBarNames[j].xmin < 320, 0);
         }

         j++;
      }
   }

   // return weather any players were knocked out
   return r;
}

// wrap up a turn ////////////////////////////////////////////////////////
void GameEndTurn(void)
{
   if(GameCheckPlayers()) {
      if(GameIsOver()) return;
      else {

         // the current player was knocked out
         if((pdat[pcur].flags & PLR_ACTIVE) == 0) {
            game_state = GAME_HIDEDICE;
            BarShow(&gBarDice[0], 0, 0);
            return;
         }
      }
   }

   // see if we need to change players, hide the dice
   if(gBarDice[0].extra == 50 && gBarDice[1].extra == 50) {
      game_state = GAME_HIDEDICE;
      BarShow(&gBarDice[0], 0, 0);

   // get input for second move
   } else {

      if((game_flags & GAME_NETWORKED) && gThisPlr != pcur) {
         game_state = GAME_WAITNETINPUT;
      /* } else(is computer player) { */
      } else {
         game_state = GAME_GETINPUT;
      }
   }
}

// update the "glowing f" palette entry //////////////////////////////////
void GameUpdateGlow(void)
{
   UCHAR        color;
   PALETTEENTRY pe;

   // update the frame
   gGlowFrame += (double)(gThisTick - gGlowLTick) * VEL_GLOW;
   if(gGlowFrame > 50) gGlowFrame = 0;

   gGlowLTick = gThisTick;

   // create the color
   color = (UCHAR)(gGlowFrame > 25? 50 - gGlowFrame : gGlowFrame);
   pe.peRed   = color;
   pe.peGreen = 7 * color;
   pe.peBlue  = 10 * color;
   pe.peFlags = PC_NOCOLLAPSE;

   // edit the palette
   ddpMain->SetEntries(0, CLR_FREEBORDER, 1, &pe);
}

// see if a marble is an enemy ///////////////////////////////////////////
long GameIsEnemy(long a, long b)
{
   if(gnTeamPlrs[0] > 0) return (pdat[a].team == pdat[b].team); 
   else                  return a != b;
}

// roll the dice /////////////////////////////////////////////////////////
void GameRoll(void)
{
   // setup the dice
   for(long i = 0; i < 2; i++) {
      gBarDice[i].extra = gMoveData[i + 6];
      RSETW(gBarDice[i].rsrc, 26*pcur, 26*gBarDice[i].extra, 26, 27);
   }

   // start the first die moving
   BarShow(&gBarDice[0], 0, 1);
   game_state = GAME_ROLLDICE;
}


//————————————————————————————————————————————————————————————————————————
// Board Initilization
//————————————————————————————————————————————————————————————————————————

// hide the bars /////////////////////////////////////////////////////////
void GameHideBars(void)
{
   BarShow(&gBarTeams, 0, 1);
   BarShow(&gBarChat , 0, 0);
   BarShow(&gBarRoll , 0, 0);
   BarShow(&gBarNames[0], gBarNames[0].x < 320, 0);
   BarShow(&gBarNames[1], gBarNames[1].x < 320, 0);
   BarShow(&gBarNames[2], gBarNames[2].x < 320, 0);
   BarShow(&gBarNames[3], gBarNames[3].x < 320, 0);
   BarShow(&gBarDice[0], 0, 0);
   BarShow(&gBarDice[1], 0, 0);
}

// initilize a new game //////////////////////////////////////////////////
void GameInitNew(void)
{
   long i;

   // initilize player data
   for(i = 0; i < 4; i++) {
      BSETB(pdat[i].flags, PLR_ACTIVE, ((pdat[i].flags & PLR_ON) > 0));
      pdat[i].nmarbles = 10;
   }

   // make sure the game is not a network one
   if((game_flags & GAME_NETWORKED) == 0) {

      // re-initilize stuff
      GameInitBoard();

      // choose a random player
      do { pcur = RAND(4); } while((pdat[pcur].flags & PLR_ACTIVE) == 0);

      // give the first set of dice a value
      gMoveData[6] = RAND(6);
      gMoveData[7] = RAND(6);
   }

   // initilize game stuff
   GameInitBarsB();
   NameSetup();
   MenuInit();

   // push the sound on menu
   if(game_flags & GAME_SOUNDON) MenuPush(7);

   // initilize varables
   gRiseSinkX = -1;
   gRiseSinkY = -1;
   gSelectedX = -1;
   gSelectedY = -1;
   gFrame     = 0;
   gLastTick  = gThisTick;
   game_state = GAME_SHOWMARBLES;
}

// start a new game //////////////////////////////////////////////////////
void GameNew(void)
{
   // set the marbles to sink into the board
   gFrame     = 20;
   gRiseSinkX = -1;
   gRiseSinkY = -1;
   gLastTick  = gThisTick;
   game_nxtst = GAME_REQUESTNEW;
   game_state = GAME_MARBLESINK;
}

// initilize the board ///////////////////////////////////////////////////
void GameInitBoard(void)
{
   long i, j;
   long a, b;
   
   // clear the board
   memset(g_board, 0, sizeof(g_board));

   // set the invalid corner regions
   for(i = 0; i < 3; i++) {
      for(j = 0; j < 3; j++) {

         // cash values
         a = i + 13;
         b = j + 13;

         // set an item in each corner to invalid
         g_board[i][j] = MARBLE_INVALID;
         g_board[i][b] = MARBLE_INVALID;
         g_board[a][j] = MARBLE_INVALID;
         g_board[a][b] = MARBLE_INVALID;
      }
   }

   // set the player's marbles up
   if(pdat[0].flags & PLR_ON) for(i = 3; i < 13; i++) g_board[i][0]  = MARBLE_SINKHOLE + 1; // red
   if(pdat[1].flags & PLR_ON) for(i = 3; i < 13; i++) g_board[15][i] = MARBLE_SINKHOLE + 2; // teal
   if(pdat[2].flags & PLR_ON) for(i = 3; i < 13; i++) g_board[i][15] = MARBLE_SINKHOLE + 3; // purple
   if(pdat[3].flags & PLR_ON) for(i = 3; i < 13; i++) g_board[0][i]  = MARBLE_SINKHOLE + 4; // tan

   // setup random sink holes
   for(i = 0; i < gnSinkHoles; i++) {

      // find an empty location
      do {
         a = RAND(14) + 1;
         b = RAND(14) + 1;
      } while(g_board[a][b] != MARBLE_SPACE);

      // set the location to be a sinkhole
      g_board[a][b] = MARBLE_SINKHOLE;
   }
}


//————————————————————————————————————————————————————————————————————————
// Marble functions
//————————————————————————————————————————————————————————————————————————

// make a marble explode /////////////////////////////////////////////////
void MarbleExplode(long x, long y)
{
   gFrame     = 0;
   gExplodeX  = x;
   gExplodeY  = y;
   gLastTick  = gThisTick;
   game_state = GAME_MARBLEEXPLODE;
}

// make a marble sink ////////////////////////////////////////////////////
void MarbleSink(long x, long y)
{
   gFrame     = 20;
   gRiseSinkX = x;
   gRiseSinkY = y;
   gLastTick  = gThisTick;
   game_state = GAME_MARBLESINK;
}

// create the new marble /////////////////////////////////////////////////
void MarbleNew(void)
{
   // see if there was a place for the marble to go
   if(gMoveData[4] < 16 && gMoveData[5] < 16) {
      /*play the sound*/

      // update the graph
      NameChangeGraph((UCHAR)pcur, 1);

      // create the new marble
      gFrame     = 0;
      gLastTick  = gThisTick;
      gRiseSinkX = gMoveData[4];
      gRiseSinkY = gMoveData[5];
      game_state = GAME_SHOWMARBLES;
      
      // set the marble on the board
      g_board[gRiseSinkX][gRiseSinkY] = pcur + MARBLE_SINKHOLE + 1;

   // there was no place for the marble
   } else {
      GameEndTurn();
   }
}

// get the location of a new marble //////////////////////////////////////
void MarbleNewGetPos(void)
{
   long zero    = 0;
   long fifteen = 15;
   long *i, *j;      // points to the vars containing x & y pos of marble
   long count;       // used to cycle through 'behind the line'
   long chosen;      // the randomly chosen marble
   long index = 0;   // the current number of found marbles
   long list[10][2]; // a list of places to put a free guy

   // figure out how the counters will work
   switch(pcur) {
   case 0: i = &count;   j = &zero;    break; // top (red)
   case 1: i = &fifteen; j = &count;   break; // right (teal)
   case 2: i = &count;   j = &fifteen; break; // bottom (purple)
   case 3: i = &zero;    j = &count;   break; // left (tan)
   }

   // make a list of places to put the new marble
   for(count = 3; count < 13; count++) {
      if(g_board[*i][*j] == MARBLE_SPACE) {
         list[index][0] = *i;
         list[index][1] = *j;
         index++;
      }
   }

   // save the chordinates
   if(index > 0) {
      chosen       = RAND(index);
      gMoveData[4] = (BYTE)list[chosen][0];
      gMoveData[5] = (BYTE)list[chosen][1];
   } else {
      gMoveData[4] = 16;
      gMoveData[5] = 16;
   }
}

// return the x of a marble //////////////////////////////////////////////
long MarbleX(long a)
{
   if(a == 0)       return 68;          // left
   else if(a == 15) return 546;         // right
   else             return 30 * a + 82; // anything else
}

// reurn the y of a marble ///////////////////////////////////////////////
long MarbleY(long b)
{
   if(b == 0)       return 21;          // top
   else if(b == 15) return 327;         // bottom
   else             return 20 * b + 24; // anything else
}


//————————————————————————————————————————————————————————————————————————
// Splash Screen Code
//————————————————————————————————————————————————————————————————————————

// render the splash screen //////////////////////////////////////////////
long GameRenderSplash(long mode)
{
   RECT   r;
   long   i;
   long   y;
   double incr;
   
   incr      = (double)(GetTickCount() - gLastTick) * VEL_SPLASH;
   gLastTick = GetTickCount();

   // update the current frame
   if(mode == 1) gFrame -= incr;
   else gFrame += incr;

   if(mode == 2) {
      if(gFrame > 320) gFrame = 320;
   } else {
      if(gFrame > 480) gFrame = 480;
   }

   if(gFrame < 0) gFrame = 0;

   // clear the surface
   DDClear(ddsBack, mode == 2? CLR_BLACK : CLR_SPLASHBLACK);

   if(gFrame >= 1) {

      // render the game board
      if(mode == 2) {

         // render right side
         RSET(r, 320 - (long)(gFrame), 0, 320, 480);
         DDBltFast(ddsBack, ddsSplash, 0, 0, &r, NOCK);

         // render left side
         RSET(r, 320, 0, (long)(gFrame) + 320, 480);
         DDBltFast(ddsBack, ddsSplash, 640 - (long)(gFrame), 0, &r, NOCK);

      // render the splash screen
      } else {

         // render the current frame
         for(i = 0; i < 16; i++) {

            // setup the rectangle
            r.left  = 40*i;
            r.right = r.left + 40;

            // odd strips
            if(i % 2) {
               y        = 480 - (long)gFrame;
               r.top    = 0;
               r.bottom = (long)gFrame;

            // even strips
            } else {
               y        = 0;
               r.top    = 480 - (long)gFrame;
               r.bottom = 480;
            }

            // render the strip
            DDBltFast(ddsBack, ddsSplash, r.left, y, &r, NOCK);
         }
      }
   }

   // flip the buffers
   DDRVAL(ddsFront->Flip(0, DDFLIP_WAIT));
   
   // return weather the animation is done
   if(mode == 1)      return (gFrame == 0);
   else if(mode == 2) return (gFrame == 320);
   else               return (gFrame == 480);
}


//————————————————————————————————————————————————————————————————————————
// Base Program Functions
//————————————————————————————————————————————————————————————————————————

// Process game menu /////////////////////////////////////////////////////
void ProgMenu(long menu, bool down)
{
   switch(menu) {
   case 0: GameNew();               break;
   case 5: prog_state = PROG_CLOSE; break;
   
   // music
   case 6:
      if(game_flags & GAME_MUSICON) {
         MClose();
         BREM(game_flags, GAME_MUSICON);
      } else {
         MInit();
         MPlayNext();
         BSET(game_flags, GAME_MUSICON);
      }
      break;

   // sound
   case 7:
      break;
   }
}

// Main program loop /////////////////////////////////////////////////////
void ProgLoop(void)
{
   switch(prog_state) {

   // Update the splash screen
   case PROG_SPLASHA:

      // process a frame
      DIGetInput();
      GameRenderSplash(0);

      // see if we should end the splash screen
      if(GetTickCount() - gTimer > WAIT_SPLASH || CUR_CLICK) prog_state = PROG_SPLASHB;
      break;

   // hide the splash screen
   case PROG_SPLASHB:
      if(GameRenderSplash(1)) {

         // start the music
         MPlayNext();

         // change to an all black palette
         memset(gPalette, 0, sizeof(PALETTEENTRY)*256);
         ddpMain->SetEntries(0, 0, 256, gPalette);

         // clear the splash screen & the main buffers
         DDClear(ddsSplash, CLR_BKGND);
         DDClear(ddsBack,   CLR_BLACK);
         DDClear(ddsFront,  CLR_BLACK);

         // render the screen
         LPDDS ddsTemp;
         ddsTemp = ddsBack;
         ddsBack = ddsSplash;
         GameRender();
         GameRenderUI();
         ddsBack = ddsTemp;

         // change to the main palette
         XalLoad(gPalette, PAL_MAIN);
         ddpMain->SetEntries(0, 0, 256, gPalette);

         gFrame     = 1;
         gLastTick  = GetTickCount();
         prog_state = PROG_SPLASHC;
      }
      break;

   // bring in the game screen
   case PROG_SPLASHC:
      if(GameRenderSplash(2)) {

         // release the splash screen
         DXOBJ_DELETE(ddsSplash);

         // change program states
         game_state = GAME_REQUESTNEW;
         prog_state = PROG_PLAYING;
      }
      break;

   // Update game play
   case PROG_PLAYING:
      gThisTick = GetTickCount();
      DIGetInput();

      // update current frame
      GameUpdate();
      GameUpdateUI();

      // render current frame
      DDClear(ddsBack, CLR_BKGND);
      GameRender();
      GameRenderUI();

#ifdef GAME_DEBUG
      DebugDrawFps();
#endif
      DDRVAL(ddsFront->Flip(0, DDFLIP_WAIT));
      break;

   // Program is to be minimized
   case PROG_MINIMIZE:
      ShowWindow(gHWnd, SW_MINIMIZE);
      break;

   // Program is in suspended mode
   case PROG_SUSPENDED:
      break;

   // Program should be shutdown
   case PROG_CLOSE:
      PostMessage(gHWnd, PROG_KILL, 0, 0);
      break;
   }
}

// Initilize application data ////////////////////////////////////////////
long ProgInit(void)
{
#ifdef GAME_DEBUG
   DebugOpen();
#endif


   // make sure there is a resource file to read from
   if(!ResAvalable()) {
      MessageBox(gHWnd, TXT_NORES, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // setup DirectDraw
   ShowCursor(FALSE);
   srand(GetTickCount());
   DDInit();

   // clear the front & back buffers
   DDClear(ddsFront, CLR_SPLASHBLACK);
   DDClear(ddsBack , CLR_SPLASHBLACK);

   // create main palette
   XalLoad(gPalette, PAL_SPLASH);
   if(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256, gPalette, &ddpMain, 0) != DD_OK) {
      DebugSimple("FAILED: ProgInit, CreatePalette");
      return 0;
   }

   // attach new palete to the front surface
   ddsFront->SetPalette(ddpMain);


   /////////////////////////////////////////////////////////////
   // Do Misc. Stuff
   /////////////////////////////////////////////////////////////

   /* Maybe show loading message */

   // initilize input, sound & music   
   DIInit();
   DSInit();
   MInit();


   /////////////////////////////////////////////////////////////
   // Setup DirectDraw surfaces
   /////////////////////////////////////////////////////////////

   // fonts
   FntLoad(&gFntTitle   , FNT_TITLE);
   FntLoad(&gFntNames   , FNT_NAMES);
   FntLoad(&gFntTxt     , FNT_WNDTXT);
   FntLoad(&gFntTxtHil  , FNT_WNDTXTHIL);
   FntLoad(&gFntTxtDim  , FNT_WNDTXTDIM);
   FntLoad(&gFntTxtSmall, FNT_SMALL);

   // create surfaces from xmp images
   DdsCreate(&ddsSplash   , XMP_SPLASH);
   DdsCreate(&ddsItems    , XMP_ITEMS);
   DdsCreate(&ddsBoard    , XMP_BOARD);
   DdsCreate(&ddsExplode  , XMP_EXPLODE);
   DdsCreate(&ddsMenuItems, XMP_MENUS);
   DdsCreate(&ddsDice     , XMP_DICE);
   DdsCreate(&ddsSelect   , XMP_SELECT);

   // blank surfaces
   DdsCreate(&ddsTeams   , 137, 31);
   DdsCreate(&ddsChat    , 336, 89);
   DdsCreate(&ddsMenuBar , 26 , 210);
   DdsCreate(&ddsDiceBar , 104, 35);
   DdsCreate(&ddsNames   , 273, 176);
   DdsCreate(&ddsWnd     , 500, 300);
   DdsCreate(&ddsMsg     , 200, 17);

   /////////////////////////////////////////////////////////////
   // Setup DirectSound buffers
   /////////////////////////////////////////////////////////////

   DsbCreate(&dsbError  , XAV_ERROR);
   DsbCreate(&dsbExplode, XAV_EXPLODE);
   DsbCreate(&dsbFree   , XAV_FREE);
   DsbCreate(&dsbRise   , XAV_RISE);
   DsbCreate(&dsbSelect , XAV_SELECT);
   DsbCreate(&dsbSink   , XAV_SINK);
   DsbCreate(&dsbNoNo   , XAV_NONO);
   DsbCreate(&dsbMHide  , XAV_MENUHIDE);
   DsbCreate(&dsbMShow  , XAV_MENUSHOW);
   DsbCreate(&dsbCWithin, XAV_CWITHIN);


   /////////////////////////////////////////////////////////////
   // Initilize application globals
   /////////////////////////////////////////////////////////////

   // initilize player data
   for(long i = 0; i < 4; i++) {
      wsprintf(pdat[i].name, "Player %d", i+1);
      pdat[i].flags     = PLR_ACTIVE | PLR_ON;
      pdat[i].games_won = 0;
      pdat[i].nmarbles  = 10;
      pdat[i].team      = 0;
   }

   // initilize UI objects
   DDClear(ddsTeams, 0);
   GameInitBarsA();
   GameInitBarsB();
   NameSetup();
   MenuInit();
   ChatInit();
   DiceDraw();

   // push the sound menu button
   if(game_flags & GAME_SOUNDON) MenuPush(7);

   // setup for splash screen
   gFrame    = 1;
   gTimer    = GetTickCount();
   gLastTick = gTimer;
   return 1;
}

// Restore DirectDraw Surfaces ///////////////////////////////////////////
long ProgRestoreSurfaces(void)
{
   // restore the front & back buffer
   if(ddsFront->Restore()           != DD_OK) return 0;
   if(ddsFront->SetPalette(ddpMain) != DD_OK) return 0;

   // restore bitmape holding surfaces
   if(!XmpReload(ddsItems    , XMP_ITEMS))   return 0;
   if(!XmpReload(ddsBoard    , XMP_BOARD))   return 0;
   if(!XmpReload(ddsExplode  , XMP_EXPLODE)) return 0;
   if(!XmpReload(ddsMenuItems, XMP_MENUS))   return 0;
   if(!XmpReload(ddsDice     , XMP_DICE))    return 0;
   if(!XmpReload(ddsSelect   , XMP_SELECT))  return 0;

   // restore fonts
   if(!FntRestore(&gFntTitle   , FNT_TITLE))     return 0;
   if(!FntRestore(&gFntNames   , FNT_NAMES))     return 0;
   if(!FntRestore(&gFntTxt     , FNT_WNDTXT))    return 0;
   if(!FntRestore(&gFntTxtHil  , FNT_WNDTXTHIL)) return 0;
   if(!FntRestore(&gFntTxtDim  , FNT_WNDTXTDIM)) return 0;
   if(!FntRestore(&gFntTxtSmall, FNT_SMALL))     return 0;

   // restore blank surfaces
   if(!DdsRestore(ddsTeams))   return 0;
   if(!DdsRestore(ddsChat))    return 0;
   if(!DdsRestore(ddsMenuBar)) return 0;
   if(!DdsRestore(ddsDiceBar)) return 0;
   if(!DdsRestore(ddsNames))   return 0;
   if(!DdsRestore(ddsWnd))     return 0;
   if(!DdsRestore(ddsMsg))     return 0;

   // re-draw to blank surfaces
   NameDraw();
   ChatDraw();
   DiceDraw();
   TeamDraw();
   MenuDraw();

   // re-draw the message box
   if(game_flags & GAME_MSGBOX) WndRedraw();
   return 1;
}

// Restore DirectSound buffers ///////////////////////////////////////////
long ProgRestoreSound(void)
{
   if(!DsbRestore(dsbError  , XAV_ERROR))    return 0;
   if(!DsbRestore(dsbExplode, XAV_EXPLODE))  return 0;
   if(!DsbRestore(dsbFree   , XAV_FREE))     return 0;
   if(!DsbRestore(dsbRise   , XAV_RISE))     return 0;
   if(!DsbRestore(dsbSelect , XAV_SELECT))   return 0;
   if(!DsbRestore(dsbSink   , XAV_SINK))     return 0;
   if(!DsbRestore(dsbNoNo   , XAV_NONO))     return 0;
   if(!DsbRestore(dsbMHide  , XAV_MENUHIDE)) return 0;
   if(!DsbRestore(dsbMShow  , XAV_MENUSHOW)) return 0;
   if(!DsbRestore(dsbCWithin, XAV_CWITHIN))  return 0;
   return 1;
}

// Release application data //////////////////////////////////////////////
void ProgClose(void)
{
   // exit DP session
   DpQuit();

   // close DirectDraw objects
   if(lpdd) {
      DXOBJ_DELETE(ddpMain);
      DXOBJ_DELETE(ddsFront);

      DXOBJ_DELETE(ddsItems);
      DXOBJ_DELETE(ddsExplode);
      DXOBJ_DELETE(ddsMenuItems);
      DXOBJ_DELETE(ddsDice);
      DXOBJ_DELETE(ddsSelect);

      DXOBJ_DELETE(ddsBoard);
      DXOBJ_DELETE(ddsMenuBar);
      DXOBJ_DELETE(ddsTeams);
      DXOBJ_DELETE(ddsChat);
      DXOBJ_DELETE(ddsNames);
      DXOBJ_DELETE(ddsDiceBar);
      DXOBJ_DELETE(ddsMsg);
      DXOBJ_DELETE(ddsWnd);

      FntDelete(&gFntTitle);
      FntDelete(&gFntNames);
      FntDelete(&gFntTxt);
      FntDelete(&gFntTxtHil);
      FntDelete(&gFntTxtDim);
      FntDelete(&gFntTxtSmall);

      lpdd->Release();
      lpdd = 0;
   }

   // close directsound objects
   if(lpds) {
      lpds->Release();
      lpds = 0;
   }

   // close DirectInput & music
   DIClose();
   MClose();

   // close debug output
#ifdef GAME_DEBUG
   DebugClose();
#endif
}