/************************************************************************/
// game.cpp - Main game play code stuff
//
// Author: Justin Hoffman
// Date:   6/4/00 - 7/18/2000
/************************************************************************/

// headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "user.h"
#include "game.h"
#include "file.h"
#include "window.h"
#include "resdef.h"
#include "network.h"
#include "computer.h"


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// game options //////////////////////////////////////////////////////////
bool gbSinkPlayed      = 1;
bool gbVisible         = 1;
long gbPaused          = 0;
bool gbNetwork         = 0;
bool gbSoundOn         = 1;
bool gbMusicOn         = 1;
bool gbSoundAval       = 1;
bool gbMusicAval       = 1;
bool gbMciAval         = 1;
bool gbDirectMusicAval = 1;
bool gbCdAudioAval     = 1;
bool gbMsgBox          = 0;
bool gbSelectName      = 0;

// direct draw objects ///////////////////////////////////////////////////
LPDDP ddpMain     = 0;
LPDDS ddsFront    = 0;
LPDDS ddsBack     = 0;

LPDDS ddsLoading  = 0;
LPDDS ddsItems    = 0;
LPDDS ddsBoard    = 0;
LPDDS ddsMarbles  = 0;
LPDDS ddsHoles    = 0;
LPDDS ddsExplode  = 0;
LPDDS ddsSelect   = 0;
LPDDS ddsDice     = 0;
LPDDS ddsSMenu    = 0;
LPDDS ddsMenus    = 0;
LPDDS ddsPlrType  = 0;

LPDDS ddsWnd      = 0;
LPDDS ddsMsg      = 0;
LPDDS ddsMenuBar  = 0;
LPDDS ddsNames    = 0;
LPDDS ddsTeams    = 0;
LPDDS ddsChat     = 0;
LPDDS ddsDiceBar  = 0;

// DirectSound buffers ///////////////////////////////////////////////////
LPDSB dsbCurWithin = 0;
LPDSB dsbDefault   = 0;
LPDSB dsbExplode   = 0;
LPDSB dsbFree      = 0;
LPDSB dsbCant      = 0;
LPDSB dsbSelect    = 0;
LPDSB dsbSink      = 0;
LPDSB dsbRise      = 0;

// fonts /////////////////////////////////////////////////////////////////
FNT gFntTitle;
FNT gFntNames;
FNT gFntSmall;
FNT gFntWndTxt;
FNT gFntWndTxtHil;
FNT gFntWndTxtDim;

// game varables /////////////////////////////////////////////////////////
PALETTEENTRY gPalette[256];
char   gsWinText[128];
DWORD  gThisTick         = 0;
UCHAR  prog_state        = PROG_SPLASHA;
UCHAR  game_state        = GAME_IDLE;
UCHAR  game_nxtst        = GAME_IDLE;
UCHAR  game_psest        = GAME_IDLE;

double gFrame            = 0;
double gFrameVel         = 0;
DWORD  gLastTick         = 0;
DWORD  gTimerStart       = 0;

double gGlowFrame        = 0;
DWORD  gGlowLTick        = 0;

UCHAR  gWinner           = -1;
UCHAR  gnGames           = 1;
UCHAR  gnSinkHoles       = 25;
UCHAR  gnTeamPlrs[2]     = {0,0};
UCHAR  gnTeamPlrsLeft[2] = {0,0};
UCHAR  gTeams[2][3]      = {{0,0,0},{0,0,0}};
bool   gMoveDataValid    = 0;
long   gnMoveDataCashe   = 0;
BYTE   gMoveDataCashe[6][8];
BYTE   gMoveData[8];
BYTE   gNetData[2];
char   gBoard[16][16];

// player data ///////////////////////////////////////////////////////////
PDAT  pdat[4];
UCHAR gThisPlr    = 0;
UCHAR nplrs       = 4;
UCHAR pcur        = 0;
UCHAR gnPlrsReady = 0;

// input data ////////////////////////////////////////////////////////////
double gSelectFrame = 0;
DWORD  gSelectLTick = 0;
char   gCurWithinX  = -1;
char   gCurWithinY  = -1;
char   gSelectedX   = -1;
char   gSelectedY   = -1;       
char   gAnimateX    = -1;   // position of animating marble (sinking or exploding)
char   gAnimateY    = -1;
char   gMovesX[8]   = {-1,-1,-1,-1,-1,-1,-1,-1};
char   gMovesY[8]   = {-1,-1,-1,-1,-1,-1,-1,-1};

// move data /////////////////////////////////////////////////////////////
char   gMoveDstVal = -1;

// flying object data ////////////////////////////////////////////////////
double gObjFrame  = 0;
double gObjWScale = 0;
double gObjHScale = 0;
DWORD  gObjLTick  = 0;
char   gObjValue  = 0;
long   gObjX      = 0;
long   gObjY      = 0;
char   gObjSrcX   = -1;
char   gObjSrcY   = -1;
char   gObjDstX   = -1;
char   gObjDstY   = -1;


//————————————————————————————————————————————————————————————————————————
// Main Game Loops
//————————————————————————————————————————————————————————————————————————

// master game updating loop /////////////////////////////////////////////
void GameUpdate(void)
{
   // update the frame varable
   if(!gbPaused) gFrame += ((double)(gThisTick - gLastTick)) * gFrameVel;
   gLastTick = gThisTick;

   // update the glowing "f's"
   GameUpdateGlow();

   switch(game_state) {

   // a new game has been requested
   case GAME_REQUESTNEW:
      GameRequestNew();
      return;

   // we are supposed to wait for the other
   // players before starting a new network game
   case GAME_WAITNEWNET:
      if(dpHost && gnPlrsReady == nplrs - 1) {
         GameSendNewGame();
         GameInitNewGame();
         gnPlrsReady = 0;
      }
      return;

   // bring up the connect message box
   case GAME_CONNECT:
      GameConnect();
      return;

   // bring up the start network game window
   case GAME_STARTNET:
      GameStartNet();
      return;

   // if the marbles are done appearing,
   // start the utility bars moving
   case GAME_ALLMARBLESSHOW:
      if(gFrame > 20) GameEndShowMarbles();
      return;

   // waiting for the utility bars to quit moving
   case GAME_WAITUTILBARS:
      if(gThisTick - gTimerStart > WAIT_UTILBARS) {
         BarShow(&gBarNames[0], gBarNames[0].xmax > 320, 0);
         game_state = GAME_WAITNAMEBARS;
      }
      return;

   // the dice are being rolled
   case GAME_ROLLDICE:
      if     (gBarDice[0].state == BAR_FINISHED) BarShow(&gBarDice[1], 0, 1);
      else if(gBarDice[1].state == BAR_FINISHED) GameSetMove();
      return;

   // have to aquire input from the user
   case GAME_GETINPUT:
      GameGetInput();
      if(CUR_CLICK) GameAttemptMove();
      return;

   // waiting for a network player to make their move
   case GAME_WAITNETINPUT:
      if(gMoveDataValid) {

#ifdef GAME_DEBUG
         DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d", pcur,
            gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
            gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
#endif

         // move the marble
         GameMoveMarble(gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3]);
         gMoveDataValid = 0;
      }
      return;

   // a bullet is flying through the air
   case GAME_BULLETMOVE:
      if(GameUpdateArc(10, 8)) {
         NameReGraph(gBoard[gObjDstX][gObjDstY], -1);
         DsbPlay(dsbExplode, MarbleX(gObjDstX));
         MarbleExplode(gObjDstX, gObjDstY);
      }
      return;

   // if the animation has completed, set the
   // space to a free guy & try to move again
   case GAME_MARBLEEXPLODE:
      if(gFrame > 11) {
         gBoard[gObjDstX][gObjDstY] = MARBLE_FREE;
         GameMoveMarble(gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3]);
      }
      return;

   // super marble strikes again
   case GAME_MARBLEMOVE:
      if(GameUpdateArc(2, 5)) GameEndMarbleMove();
      return;

   // a new marble is appearing
   case GAME_MARBLESHOW:
      if(gFrame > 20) {
         DsbPlay(dsbRise, MarbleX(gAnimateX));
         GameEndTurn();
      }
      return;

   // if the sinkhole animation is complete, remove
   // the marble from the board and end the turn
   case GAME_MARBLEHIDE:
      
      // play the sink sound
      if(gFrame < 6 && !gbSinkPlayed) {
         DsbPlay(dsbSink, MarbleX(gAnimateX));
         gbSinkPlayed = 1;
      }

      // the animation has finished
      if(gFrame < 0) {
         gBoard[gAnimateX][gAnimateY] = MARBLE_SPACE;
         GameEndTurn();
      }
      return;

   // if the animation has completed move to the
   // next game state & clear all the marbles
   case GAME_ALLMARBLESHIDE:    
      if(gFrame < 0) {
         game_state = game_nxtst;
         memset(gBoard, MARBLE_SPACE, sizeof(gBoard));
      }
      return;

   // the dice are going away, they are too good for us
   case GAME_HIDEDICE:
      if(gBarDice[0].state == BAR_FINISHED) BarShow(&gBarDice[1], 0, 0);
      if(gBarDice[1].state == BAR_FINISHED) game_state = GAME_NEXTPLAYER;
      return;

   // gotta switch to another player now
   case GAME_NEXTPLAYER:
      GameNextPlayer();
      return;

   // the round is finally over
   case GAME_ROUNDOVER:
      GameRoundOver();
      return;

   // over already?!? but I was having much fun!
   case GAME_OVER:
      GameOver();
      return;
   }
}

// master game rendering loop ////////////////////////////////////////////
void GameRender(void)
{
   RECT  r;
   long  i, j;
   long  x, y;
   UCHAR state;

   // render the board
   RSET(r, 0, 0, 517, 344);
   DDBltFast(ddsBack, ddsBoard, 61, 17, &r, NOCK);

   // render the moves
   GameDrawMoves();

   // get the "rendering" state
   state = game_state == GAME_PAUSED? game_psest : game_state;

   // render the marbles
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) {

         x = MarbleX(i);
         y = MarbleY(j);

         // the space is a marble
         if(0 <= gBoard[i][j] && gBoard[i][j] < 4) {

            // determine if the marble is rising/sinking
            if(state == GAME_ALLMARBLESHIDE || state == GAME_ALLMARBLESSHOW ||
              (gAnimateX == i && gAnimateY == j && (state == GAME_MARBLESHOW || state == GAME_MARBLEHIDE))) {

               GameDrawMarbleSink(gBoard[i][j], x, y);
  
            // determine if the marble is exploding
            } else if(state == GAME_MARBLEEXPLODE && gAnimateX == i && gAnimateY == j) {

               GameDrawMarbleExplode(gBoard[i][j], x, y);

            // render a normal marble
            } else {

               // see if the marble is located on the top row
               if(((i < 3 || i > 12) && j == 3) || j == 0)
                  DDLine(ddsBack, CLR_BKGND, x + 5, y - 4, x + 20, y - 4);

               // render the marble
               RSETWX(r, 10, gBoard[i][j]*16, 25, 16);
               DDBltFast(ddsBack, ddsItems, x, y - 5, &r, SRCCK);
            }

         // the space is a free marble
         } else if(gBoard[i][j] == MARBLE_FREE) {
            RSET(r, 51, 40, 62, 53);
            DDBltFast(ddsBack, ddsItems, x + 8, y - 5, &r, SRCCK);
         }
      }
   }

   // render the mouse overs & selection ring
   GameDrawSelections();

   // draw super marble
   if(state == GAME_MARBLEMOVE) GameDrawMarbleJump();

   // render the bullet if there is one
   if(state == GAME_BULLETMOVE) {
      RSETWX(r, 65, 5*pcur + 21, 5, 5);
      DDBltClip(ddsBack, ddsItems, gObjX, gObjY, &R_SCREEN, &r, SRCCK);
   }
}


//————————————————————————————————————————————————————————————————————————
// Delegated Game Updating Functions
//————————————————————————————————————————————————————————————————————————

// a new game has been "requested" ///////////////////////////////////////
void GameRequestNew(void)
{
   // hide the bars & set the games state to avoid recursion
   GameHideBars();
   game_state = GAME_IDLE;

   // display the new game window
   switch(Wnd(WNDID_NEWGAME, WndNewGame)) {
   case 0: game_state = GAME_CONNECT; break; // go to the connect window
   case 1: ProgMenuLoad();            break; // go to the load game window
   case 2: prog_state = PROG_CLOSE;   break; // exit button was used

   // init an offline game
   case 3:
      GameInitNewGame();
      FILE_LOADED[0] = TXT_NULLCHAR;
      break;

   // show the help on this window
   case 4:
      Help(4);
      game_state = GAME_REQUESTNEW;
      break;
   }
}

// show the connection window ////////////////////////////////////////////
void GameConnect(void)
{
   // set the state to idle to avoid recursion
   game_state = GAME_IDLE;

   // show the window
   switch(Wnd(WNDID_CONNECT, WndConnect)) {
   case 0: game_state = GAME_REQUESTNEW; break; // connect button used
   case 1: game_state = GAME_STARTNET;   break; // request new button used
   case 2:                                      // help button used
      Help(5);
      game_state = GAME_CONNECT;
      break;
   }
}

// the start network game window needs to be shown ///////////////////////
void GameStartNet(void)
{
   game_state = GAME_STARTNETIDLE;

   // display the network game window
   if(Wnd(WNDID_STARTNET, WndStartNet)) GameInitNewGame();  // initilize a new game
   else                                 game_state = GAME_CONNECT;  // cancel button
}

// a marble is done appearing ////////////////////////////////////////////
void GameEndShowMarbles(void)
{
   // this will be undone if any bars are going to move
   gTimerStart = 0;

   // show the team bars
   if(gnTeamPlrs[0] > 0) {
      BarShow(&gBarTeams, 0, 0);
      gTimerStart = gThisTick;
   }

   // show the chat & roll bars
   if(gbNetwork) {
      BarShow(&gBarChat , 0, 1);
      BarShow(&gBarRoll , 1, 0);
      gTimerStart = gThisTick;
   }

   // set the game state
   game_state = GAME_WAITUTILBARS;
}

// set the dice rectangles & values //////////////////////////////////////
void GameSetDice(void)
{
   long x = 26*pcur;

   // setup the dice
   gBarDice[0].extra = gMoveData[6];
   gBarDice[1].extra = gMoveData[7];
   RSETW(gBarDice[0].rsrc, x, 26*gBarDice[0].extra, 26, 27);
   RSETW(gBarDice[1].rsrc, x, 26*gBarDice[1].extra, 26, 27);
}

// roll the dice /////////////////////////////////////////////////////////
void GameRoll(void)
{
   // start the first die moving
   GameSetDice();
   BarShow(&gBarDice[0], 0, 1);
   game_state = GAME_ROLLDICE;
}

// we are watching the player, his every move... /////////////////////////
void GameGetInput(void)
{
   // update the selection ring frame
   gSelectFrame += ((double)(gThisTick - gSelectLTick) * VEL_SELECTRING);
   gSelectLTick = gThisTick;
   if(gSelectFrame >= 24) gSelectFrame = 0;

   // make sure the moves frame doesnt excede 8
   if(gFrame > 8) gFrame = 8;

   // update the current "within" marble x
   if     (cur_x < 571 && cur_x > 545) gCurWithinX = 15;
   else if(cur_x > 106 && cur_x < 527) gCurWithinX = (char)((cur_x - 78) / 30);
   else if(cur_x > 68  && cur_x < 93 ) gCurWithinX = 0;
   else                                gCurWithinX = -1;

   // update the current "within" marble y
   if(cur_y < 340 && cur_y > 16) {
      if     (cur_y > 322) gCurWithinY = 15;
      else if(cur_y > 36)  gCurWithinY = (char)((cur_y - 16) / 20);
      else                 gCurWithinY = 0;
   } else gCurWithinY = -1;
}

// the player thinks that clicking is really gonna do something //////////
void GameAttemptMove(void)
{
   // see if a marble should be moved
   if(GameIsMove(gCurWithinX, gCurWithinY) >= 0) {

      // move the marble
      GameMoveMarbleEx(gSelectedX, gSelectedY, gCurWithinX, gCurWithinY);

      // send data via network
      if(gbNetwork) GameSendMove();

#ifdef GAME_DEBUG
      DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d", pcur,
         gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
         gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
#endif

      // unselect the moves and marble
      GameUnselect();

   // see if a marble should be selected
   } else if(gCurWithinX != -1 && gCurWithinY != -1) {

      // make sure its the proper player
      if(gBoard[gCurWithinX][gCurWithinY] == pcur) {
         if(gCurWithinX == gSelectedX && gCurWithinY == gSelectedY) {
            GameUnselect();
         } else {

            // play the select sound
            DsbPlay(dsbSelect, MarbleX(gCurWithinX));

            // set the selected marble
            gSelectedX = gCurWithinX;
            gSelectedY = gCurWithinY;

            // get the possible moves
            GameSelectMoves();
         }

      // de-select the marble
      } else if(gBoard[gCurWithinX][gCurWithinY] == MARBLE_INVALID) {
         GameUnselect();

      // player clicked on another players marble
      } else if(gBoard[gCurWithinX][gCurWithinY] < 4) {
         DsbPlay(dsbCant, SOUND_CENTER);
      }

   // unselect everything
   } else GameUnselect();
}

// its a bird, its a plane, no, its the super marble! ////////////////////
long GameUpdateArc(long xOfst, long yOfst)
{
   // update the frame number
   gObjFrame += gThisTick - gObjLTick;
   gObjLTick = gThisTick;

   // make sure we dont go over 500 frames
   if(gObjFrame > 500) gObjFrame = 500;

   // update the marble x & y chords
   gObjX = (long)(gObjWScale * gObjFrame + MarbleX(gObjSrcX) + xOfst);
   gObjY = (long)((.0012*gObjFrame*gObjFrame)-(0.6*gObjFrame)+(double)(gObjHScale*gObjFrame))+MarbleY(gObjSrcY) - yOfst;
 
   // return weather the animation is complete
   return (gObjFrame >= 500);
}

// the marble has landed /////////////////////////////////////////////////
void GameEndMarbleMove(void)
{
   // ajust the dice
   if(abs(gObjDstX - gObjSrcX) + abs(gObjDstY - gObjSrcY) - 1 == gBarDice[0].extra) {
      gBarDice[0].extra = 25;
   } else {
      gBarDice[1].extra = 25;
   }

   // set the board item
   gBoard[gObjDstX][gObjDstY] = gObjValue;

   // the destination was a sinkhole
   if(gMoveDstVal == MARBLE_SINKHOLE) {
      NameReGraph(pcur, -1);
      MarbleSink(gObjDstX, gObjDstY);

   // the destination was a free marble
   } else if(gMoveDstVal == MARBLE_FREE) {
      MarbleCreateFree();

   // go to the next turn
   } else GameEndTurn();
}

// change to the next player /////////////////////////////////////////////
void GameNextPlayer(void)
{
   // remove the selection around the current player
   NameDrawSelect(pcur, 2);

   // go to the next player
   GameChangePlayerId();

   // start the next turn
   GameRoll();
   NameSelect();
}

// change the current player ID //////////////////////////////////////////
void GameChangePlayerId(void)
{
   do { 
      if(++pcur > 3) pcur = 0;
   } while((pdat[pcur].flags & PLR_ACTIVE) == 0);
}

// process round over stuff, maybe start a new game or something /////////
void GameRoundOver(void)
{
   // start a new network round
   if(gbNetwork) {

      // prepair for the next game
      ChatAddText(4, TXT_ROUNDOVER);
      game_state = GAME_WAITNEWNET;

      // send the host message that we are ready
      if(!dpHost) {
         game_nxtst  = 0;
         gNetData[0] = NETMSG_READYFORNEW;
         DpSend(&gNetData[0], sizeof(gNetData[0]));
      }

   // start a new standard round
   } else {
      GameHideBars();
      game_state = GAME_IDLE;
      MsgBox(gsWinText, TXT_GENTITLE, TXT_OKAY, 0);
      GameInitNewGame();
   }
}

// process game over stuff, it should do something interesting, but, no //
void GameOver(void)
{
   GameHideBars();
   game_state = GAME_IDLE;

   // end a network game
   if(gbNetwork) {

      // setup game end stuff
      WndStartNetShowWinner = 1;
      WndStartNetGotNames   = dpHost;
      game_state            = GAME_STARTNET;

      // clear the player data
      memset(WndStartNetPlrs, 0, sizeof(WndStartNetPlrs));

   // end a standard game
   } else {
      if(MsgBox(gsWinText, TXT_GENTITLE, TXT_NEW, TXT_EXIT) == 0) game_state = GAME_REQUESTNEW;
      else prog_state = PROG_CLOSE;
   }
}

// didn't this have somethign to do with the northern lights? ////////////
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


//————————————————————————————————————————————————————————————————————————
// Delegated Game Rendering Functions
//————————————————————————————————————————————————————————————————————————

// draw possible moves ///////////////////////////////////////////////////
void GameDrawMoves(void)
{
   RECT r;
   long i;

   // "select" the moves
   for(i = 0; i < 8; i++) {
      if(gMovesX[i] != -1) {
         RSETWX(r, 0, 11*(long)(gFrame), 25, 11);
         DDBltFast(ddsBack, ddsHoles, MarbleX(gMovesX[i]), MarbleY(gMovesY[i]), &r, NOCK);
      }
   }

   // the space is a possible move
   if(GameIsMove(gCurWithinX, gCurWithinY) >= 0) {
      RSET(r, 0, 99, 25, 110);
      DDBltFast(ddsBack, ddsHoles, MarbleX(gCurWithinX), MarbleY(gCurWithinY), &r, SRCCK);
   }
}

// draw a sinking marble /////////////////////////////////////////////////
void GameDrawMarbleSink(char p, long x, long y)
{
   RECT ra, rb;

   // we are rendering the hole moving
   if(gFrame <= 5) {
      RSETWX(ra, 0, 11*(long)(gFrame) + 110, 25, 11);
      DDBltFast(ddsBack, ddsHoles, x, y, &ra, NOCK);

   // we are rendring the sinking marble
   } else {

      // render the back
      RSET(ra, 0, 154, 25, 165);
      DDBltFast(ddsBack, ddsHoles, x, y, &ra, NOCK);

      // render the marble
      RSETWY(ra, p*21, 0, 21, 21);
      RSET(rb, 0, 0, 640, y + 11);
      DDBltClip(ddsBack, ddsMarbles, x + 2, (long)(y - gFrame + 15), &rb, &ra, SRCCK);

      // render the front
      RSET(ra, 10, 65, 35, 70);
      DDBltFast(ddsBack, ddsItems, x, y + 6, &ra, SRCCK);
   }
}

// draw an exploding marble //////////////////////////////////////////////
void GameDrawMarbleExplode(char p, long x, long y)
{
   RECT r;

   // render the marble
   if(gFrame < 6) {
      RSETWX(r, 10, p*16, 25, 16);
      DDBltFast(ddsBack, ddsItems, x, y - 5, &r, SRCCK);

   // render a free guy
   } else {
      RSET(r, 51, 40, 62, 53);
      DDBltFast(ddsBack, ddsItems, x + 8, y - 5, &r, SRCCK);
   }

   // render the explosion
   RSETW(r, 29*p, 24*(long)(gFrame), 29, 24);
   DDBltFast(ddsBack, ddsExplode, x - 2, y - 11, &r, SRCCK);
}

// draw the marble selections ////////////////////////////////////////////
void GameDrawSelections(void)
{
   RECT r;

   // render the "mouse within marble" stuff
   if(gCurWithinX != -1 && gCurWithinY != -1 &&
      (gCurWithinX != gSelectedX || gCurWithinY != gSelectedY)) {

      // the space is this players marble
      if(gBoard[gCurWithinX][gCurWithinY] == pcur) {
         RSET(r, 72, 0, 101, 20);
         DDBltFast(ddsBack, ddsItems, MarbleX(gCurWithinX) - 2, MarbleY(gCurWithinY) - 7, &r, SRCCK);
      }
   }

   // render the selection ring
   if(gSelectedX != -1 && gSelectedY != -1) {
      RSETWX(r, 0, 18*(long)(gSelectFrame), 27, 18);
      DDBltFast(ddsBack, ddsSelect, MarbleX(gSelectedX) - 1, MarbleY(gSelectedY) - 7, &r, SRCCK);
   }
}

// draw super marble, hopefully we can fit his ego ///////////////////////
void GameDrawMarbleJump(void)
{
   RECT ra, rb;

   // calculate source & clipping rectangles
   RSETWY(ra, 21*gObjValue, 0, 21, 21);
   RSET(rb, 0, 0, 640, 480);

   // edit the clipping rect if needed
   if(gObjFrame < 25)  rb.bottom = MarbleY(gObjSrcY) + 11;
   if(gObjFrame > 475) rb.bottom = MarbleY(gObjDstY) + 11;

   // render the marble
   DDBltClip(ddsBack, ddsMarbles, gObjX, gObjY, &rb, &ra, SRCCK);

   // acount for closeness to the starting/ending points
   RSET(ra, 10, 65, 35, 70);
   if(gObjFrame < 25) {
      DDBltFast(ddsBack, ddsItems, MarbleX(gObjSrcX), MarbleY(gObjSrcY) + 6, &ra, SRCCK);
   } else if(gObjFrame > 475) {
      DDBltFast(ddsBack, ddsItems, MarbleX(gObjDstX), MarbleY(gObjDstY) + 6, &ra, SRCCK);
   }
}


//————————————————————————————————————————————————————————————————————————
// Move Functions
//————————————————————————————————————————————————————————————————————————

// see if a player can make a move ///////////////////////////////////////
long GameCanMove(void)
{
   // local variables
   int i, j, k, l, x, y;
   int dice[2];

   // directions to check
   const int dirX[] = {0, 1, 0, -1};
   const int dirY[] = {-1, 0, 1, 0};
   
   // get dice values
   dice[0] = gMoveData[6] + 1;
   dice[1] = gMoveData[7] + 1;

   // look at the whole board
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) {

         // make sure the marble is this players
         if(gBoard[i][j] != pcur) continue;

         // check all of the directions for a valid move
         for(k = 0; k < 4; k++) {

            // cycle through both dice
            for(l = 0; l < 2; l++) {
               x = dirX[k]*dice[l] + i;
               y = dirY[k]*dice[l] + j;

               // make sure value is between 1 and 14
               if(0 < x && x < 15 && 0 < y && y < 15) {

                  // see if it is a possible move
                  if(gBoard[x][y] > 3) return 1;
               }
            }
         }
      }
   }

   // we didn't find any moves
   return 0;
}

// prepair for a move ////////////////////////////////////////////////////
void GameMoveMarbleEx(UCHAR ax, UCHAR ay, UCHAR bx, UCHAR by)
{
   // if we are going to move on a free space, get the position of the new marble
   if(gBoard[bx][by] == MARBLE_FREE) MarbleGetFreePos();

   // save data in move data
   gMoveData[0] = ax;
   gMoveData[1] = ay;
   gMoveData[2] = bx;
   gMoveData[3] = by;

   // get the new dice
   gMoveData[6] = RAND(6);
   gMoveData[7] = RAND(6);

   // move the marble
   GameMoveMarble(ax, ay, bx, by);
}

// move a marble somewhere else, we dont want it /////////////////////////
void GameMoveMarble(UCHAR ax, UCHAR ay, UCHAR bx, UCHAR by)
{
   // save the move destination value
   gMoveDstVal = gBoard[bx][by];

   // setup data common to bullets & the moving marble
   gObjValue = gBoard[ax][ay];
   gObjX     = MarbleX(ax);
   gObjY     = MarbleY(ay);
   gObjLTick = gThisTick;
   gObjSrcX  = ax;
   gObjSrcY  = ay;
   gObjFrame = 0;

   // see if there are any enemy marbles in the way
   if(GameCheckForHit(ax, ay, bx, by, &gObjDstX, &gObjDstY)) {

      // set data for bullet
      gObjWScale  = (double)(MarbleX(gObjDstX) - gObjX) / 500.00;
      gObjHScale  = (double)(MarbleY(gObjDstY) - gObjY) / 500.00;
      gObjX      += 10;
      gObjY      -= 8;

      // change the game state
      game_state = GAME_BULLETMOVE;

   } else {

      // setup the marble to move
      gObjWScale  = (double)(MarbleX(bx) - gObjX) / 500.00;
      gObjHScale  = (double)(MarbleY(by) - gObjY) / 500.00;
      gObjX      += 2;
      gObjY      -= 5;
      gObjDstX    = bx;
      gObjDstY    = by;

      // remove the marble from the board & change the game state
      gBoard[ax][ay] = MARBLE_SPACE;
      game_state = GAME_MARBLEMOVE;
   }
}

// move a marble from move cashe /////////////////////////////////////////
void GameMoveMarbleCashe(void)
{
   // de-increment cashe value
   gnMoveDataCashe--;

   // copy move from cache
   memcpy(gMoveData, gMoveDataCashe[0], sizeof(gMoveData));

   // move the cache items down
   memcpy(gMoveDataCashe[0], gMoveDataCashe[1], sizeof(gMoveDataCashe[0])*5);

#ifdef GAME_DEBUG
   DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d,(cashed move)", pcur,
      gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
      gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
#endif

   // move the marble
   GameMoveMarble(gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3]);
}

// process moves in minimized mode, avoiding Elim3a problems /////////////
void GameMoveMarbleMinimized(UCHAR ax, UCHAR ay, UCHAR bx, UCHAR by)
{
   UCHAR DstVal;
   UCHAR SrcVal;
   char  HitX;
   char  HitY;

#ifdef GAME_DEBUG
   DebugWrite("%d,%d,%d,%d,%d,%d,%d,%d,%d,(minimized)", pcur,
      gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3],
      gMoveData[4], gMoveData[5], gMoveData[6], gMoveData[7]);
#endif

   // save the move destination value
   DstVal = gBoard[bx][by];
   SrcVal = gBoard[ax][ay];

   // see if there are any enemy marbles in the way
   while(GameCheckForHit(ax, ay, bx, by, &HitX, &HitY)) {
      pdat[ gBoard[HitX][HitY] ].nMarbles--;
      gBoard[HitX][HitY] = MARBLE_FREE;
   }

   // move the marble
   gBoard[ax][ay] = MARBLE_SPACE;
   gBoard[bx][by] = SrcVal;

   // create a new marble if needed
   if(DstVal == MARBLE_FREE && gMoveData[4] != 16) {
      gBoard[ gMoveData[4] ][ gMoveData[5] ] = pcur;
      pdat[pcur].nMarbles++;

   // see if the destination was a sinkhole
   } else if(DstVal == MARBLE_SINKHOLE) {
      gBoard[bx][by] = MARBLE_SPACE;
      pdat[pcur].nMarbles--;
   }

   // ajust the dice
   if(abs(bx - ax) + abs(by - ay) - 1 == gBarDice[0].extra) {
      gBarDice[0].extra = 25;
   } else {
      gBarDice[1].extra = 25;
   }

   // check to see if anyone was knocked out
   if(GameCheckPlayers()) {

      // quit now if the game is over
      if(GameIsOver()) return;

      // see if the current player was knocked out
      if((pdat[pcur].flags & PLR_ACTIVE) == 0) {
         GameChangePlrMinimized();
         return;
      }
   }

   // see if we need to change players
   if(gBarDice[0].extra == 25 && gBarDice[1].extra == 25)
      GameChangePlrMinimized();

   // we should wait for another network player to 
   // make a move, or let the current user make a move
   game_state = gThisPlr == pcur? GAME_GETINPUT : GAME_WAITNETINPUT;
}

// change players in minimized mode //////////////////////////////////////
void GameChangePlrMinimized(void)
{
   // go to the next player
   GameChangePlayerId();

   // setup & show the dice
   GameSetDice();
   BarSetEnd(&gBarDice[0], 0, 1);
   BarSetEnd(&gBarDice[1], 0, 1);
}

// see who was shot //////////////////////////////////////////////////////
long GameCheckForHit(UCHAR ax, UCHAR ay, UCHAR bx, UCHAR by, char *x, char *y)
{
   UCHAR a, b;
   long  dist;
   long  dirx;
   long  diry;
   long  i;

   // see if there are any enemy marbles in the way
   dist = abs(ax - bx) + abs(ay - by);
   dirx = ax > bx? -1 : ax == bx? 0 : 1;
   diry = ay > by? -1 : ay == by? 0 : 1;

   for(i = 1; i < dist; i++) {

      a = dirx*i + ax;
      b = diry*i + ay;

      if(gBoard[a][b] < 4 && MarbleIsEnemy(pcur, gBoard[a][b])) {
         *x = a;
         *y = b;
         return 1;
      }
   }

   return 0;
}

// select the players current moves //////////////////////////////////////
void GameSelectMoves(void)
{
   long i, j;
   char dirX[] = {0, 1, 0, -1};
   char dirY[] = {-1, 0, 1, 0};

   gFrame    = 0;
   gFrameVel = VEL_MOVE;
   gLastTick = gThisTick;

   // clear the moves
   if(gSelectedX == -1 || gSelectedY == -1) {

      for(i = 0; i < 8; i++) {
         gMovesX[i] = -1;
         gMovesY[i] = -1;
      }

   // select the moves
   } else {

      // get the moves
      for(i = j = 0; i < 4; i++) {
         gMovesX[j] = gSelectedX + dirX[i]*(gBarDice[0].extra+1);
         gMovesY[j] = gSelectedY + dirY[i]*(gBarDice[0].extra+1);
         j++;

         gMovesX[j] = gSelectedX + dirX[i]*(gBarDice[1].extra+1);
         gMovesY[j] = gSelectedY + dirY[i]*(gBarDice[1].extra+1);
         j++;
      }

      // validate the moves
      for(i = 0; i < 8; i++) {

         // if <the move is not even on the board>
         if(gMovesX[i] > 14 || gMovesX[i] < 1 || gMovesY[i] > 14 || gMovesY[i] < 1) {
            gMovesX[i] = -1;
            gMovesY[i] = -1;

         // if <the destination on the board is invalid>
         } else if(gBoard[ gMovesX[i] ][ gMovesY[i] ] < 4 ||
            gBoard[ gMovesX[i] ][ gMovesY[i] ] == MARBLE_INVALID) {
 
            gMovesX[i] = -1;
            gMovesY[i] = -1;
         }
      }
   }
}

// test to see if a marble is a move /////////////////////////////////////
long GameIsMove(char a, char b)
{
   long i;

   // count it out if the location is invalid
   if(a == -1 || b == -1) return -1;
   for(i = 0; i < 8; i++) if(gMovesX[i] == a && gMovesY[i] == b) return i;
   return -1;
}

// unselect the players moves and marble /////////////////////////////////
void GameUnselect(void)
{
   gSelectedX  = -1;
   gSelectedY  = -1;
   gCurWithinX = -1;
   gCurWithinY = -1;
   GameSelectMoves();
}


//————————————————————————————————————————————————————————————————————————
// Network Functions
//————————————————————————————————————————————————————————————————————————

// process game network messages /////////////////////////////////////////
void GameNetMsg(BYTE *pMsg, DPID from)
{
   UCHAR p, i;
   BYTE *pData;

   // cashe location of acutal data
   pData = pMsg + 1;

   // determine which player sent the message
   for(i = p = 0; i < 4; i++) if(from == pdat[i].dpId) p = i;

#ifdef GAME_DEBUG
   DebugWrite("%d, %d, %d, %d, %d, %d, %d",
      pdat[0].dpId, pdat[1].dpId, pdat[2].dpId,
      pdat[3].dpId, from, p, pMsg[0]);
#endif

   // figure out what message was sent
   switch(pMsg[0]) {

   // start the game
   case NETMSG_STARTGAME:

      // copy the new game data
      memcpy(gBoard, pData, sizeof(gBoard));
      gMoveData[6] = pMsg[sizeof(gBoard) + 2];
      gMoveData[7] = pMsg[sizeof(gBoard) + 3];
      pcur         = pMsg[sizeof(gBoard) + 4];
      gnGames      = pMsg[sizeof(gBoard) + 5];

      // initlize the new game
      if(game_state == GAME_WAITNEWNET) {
         GameInitNewGame();
      } else {
         WndStartNetStart();
         WndClose(1);
      }
      return;

   // send chat into window
   case NETMSG_CHATA:
      WndStartNetAddText(p, (char*)pData);
      return;

   // send chat into game
   case NETMSG_CHATB:
      ChatAddText(p, (char*)pData);
      return;

   // move a marble
   case NETMSG_MOVEMARBLE:

      // determine weather move needs to be cashed
      bool cache;
      cache = gbVisible? (gMoveDataValid || gnMoveDataCashe > 0) : (game_state != GAME_WAITNETINPUT);

      // copy the move data
      memcpy(cache? gMoveDataCashe[gnMoveDataCashe] : gMoveData, pData, 8);

      // setup the move
      if(cache)          gnMoveDataCashe++;
      else if(gbVisible) gMoveDataValid = 1;
      else GameMoveMarbleMinimized(gMoveData[0], gMoveData[1], gMoveData[2], gMoveData[3]);
      return;

   // re-list the players
   case NETMSG_RELISTPLRS:

      // copy data received
      memcpy(WndStartNetPlrs, pData, sizeof(WndStartNetPlrs));

      // copy the number of sinkholes & games
      gnSinkHoles = pMsg[ sizeof(WndStartNetPlrs) + 2 ];
      gnGames     = pMsg[ sizeof(WndStartNetPlrs) + 3 ];

      // get the number of active players
      for(i = WndStartNetNPlrs = 0; i < 4; i++) {
         pdat[i].dpId = WndStartNetPlrs[i].dpId;
         if(WndStartNetPlrs[i].active) WndStartNetNPlrs++;
         if(WndStartNetPlrs[i].dpId == dpThisPlrId) gThisPlr = i;
      }

      // state we have gotten a player list
      WndStartNetGotNames = 1;

      // make sure the message box is even showing yet
      if(gbMsgBox) {

         // set the number control values
         NumSetValue(&gWndA.pnums[0], pMsg[ sizeof(WndStartNetPlrs) + 2 ]);
         NumSetValue(&gWndA.pnums[1], pMsg[ sizeof(WndStartNetPlrs) + 3 ]);

         // remove check from ready button
         BREM(gWndA.pbtns[2].flags, BTN_TDOWN);

         // re-draw players & ready button
         if(gbVisible) {
            WndStartNetDrawPlrs();
            BtnDraw(&gWndA.pbtns[2]);
         }
      }

      return;

   // change the ready state of a player
   case NETMSG_READYPLR:
      WndStartNetReadyPlr(p, pMsg[1]? 1 : 0);
      return;

   // change the number of sinkholes in a game
   case NETMSG_CHANGESINK:
      if(gbMsgBox) NumSetValue(&gWndA.pnums[0], pMsg[1]);
      return;

   // change the number of rounds in a game
   case NETMSG_CHANGEGAMES:
      if(gbMsgBox) NumSetValue(&gWndA.pnums[1], pMsg[1]);
      return;

   // a player is signaling they are ready for the next game
   case NETMSG_READYFORNEW:
      if(dpHost) gnPlrsReady++;
      return;

   // a player has changed their team
   case NETMSG_CHANGETEAM:
      WndStartNetChangeTeam(p, pMsg[1]);
      return;
   }
}

// send a new netowrk game (host only) ///////////////////////////////////
void GameSendNewGame(void)
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
   data    = (BYTE*)MoreMem(sizeof(gBoard) + 6);
   data[0] = NETMSG_STARTGAME;
   
   // copy the board
   memcpy(data + 1, gBoard, sizeof(gBoard));

   // copy in misc. data
   data[sizeof(gBoard) + 2] = gMoveData[6];
   data[sizeof(gBoard) + 3] = gMoveData[7];
   data[sizeof(gBoard) + 4] = (BYTE)pcur;
   data[sizeof(gBoard) + 5] = (BYTE)gnGames;

   // send the data
   DpSend(data, sizeof(gBoard) + 6);
   LessMem(data);
}

// send a move ///////////////////////////////////////////////////////////
void GameSendMove(void)
{
   BYTE *data;

   // create the message
   data    = (BYTE*)MoreMem(sizeof(gMoveData) + 1);
   data[0] = NETMSG_MOVEMARBLE;

   // copy the move data
   memcpy(data + 1, gMoveData, sizeof(gMoveData));

   // send & release the message
   DpSend(data, sizeof(gMoveData) + 1);
   LessMem(data);
}

// send the current chat text ////////////////////////////////////////////
void GameSendChat(void)
{
   BYTE *data;

   // create the message
   data    = (BYTE*)MoreMem(gsChatSendLen + 2);
   data[0] = NETMSG_CHATB;

   // copy the string & add a null terminator to string
   memcpy(data + 1, gsChatSend, gsChatSendLen);
   data[gsChatSendLen + 1] = TXT_NULLCHAR;

   // send & release data
   DpSend(data, gsChatSendLen + 2);
   LessMem(data);
}

// a network player has quit, the dirty rat //////////////////////////////
void GameNetPlrQuit(DPID dpId)
{
   long i, p = -1;
   char sText[64];

   // determine which player quit
   for(i = 0; i < 4; i++) if(pdat[i].dpId == dpId) p = i;

   // if we are only in pre-game chat, just make exit message
   if(game_state == GAME_STARTNETIDLE) {
      wsprintf(sText, TXT_PLRDISCONNECT, WndStartNetPlrs[p].sName);
      WndStartNetAddText(4, sText);
      return;
   }

   // make sure we even found a player
   if(p == -1 || (pdat[p].flags & PLR_ON) == 0) return;

   // remove the player from their team
   gnTeamPlrs[ pdat[p].team ]--;
   gnTeamPlrsLeft[ pdat[p].team ]--;

   // disable the player
   BREM(pdat[p].flags, PLR_ON);
   BREM(pdat[p].flags, PLR_ACTIVE);
   nplrs--;

   // re-setup the names
   NameSetup();
   for(i = 0; i < nplrs; i++) BarSetEnd(&gBarNames[i], 1, 0);

   // if there are no more players, the game is over
   if(nplrs < 2) {
      gWinner    = gThisPlr;
      game_state = GAME_OVER;

   // the last player of the team is gone
   } else if(gnTeamPlrsLeft[ pdat[p].team ] == 0) {
      gWinner    = (UCHAR)(1 - pdat[p].team);
      game_state = GAME_OVER;

   // if it was the players turn, go to the next turn
   } else if(p == pcur) {
      if(gbVisible) {
         game_state = GAME_HIDEDICE;
         BarShow(&gBarDice[0], 0, 0);
      } else GameChangePlrMinimized();
   }

   // create the chat message
   wsprintf(sText, TXT_PLRDISCONNECT, pdat[p].sName);

   // if the game is over, add the text to the pre game chat
   if(game_state == GAME_OVER) WndStartNetAddText(4, sText);
   else                        ChatAddText(4, sText);
}


//————————————————————————————————————————————————————————————————————————
// "Wrap Up" Functions
//————————————————————————————————————————————————————————————————————————

// hmm, finally done moving a marble /////////////////////////////////////
void GameEndTurn(void)
{
   if(GameCheckPlayers()) {

      // quit now if the game is over
      if(GameIsOver()) return;

      // see if the current player was knocked out
      if((pdat[pcur].flags & PLR_ACTIVE) == 0) {
         game_state = GAME_HIDEDICE;
         BarShow(&gBarDice[0], 0, 0);
         return;
      }
   }

   // see if we need to change players, hide the dice
   if(gBarDice[0].extra == 25 && gBarDice[1].extra == 25) {
      game_state = GAME_HIDEDICE;
      BarShow(&gBarDice[0], 0, 0);

   // get input for second move
   } else GameSetMove();
}

// test to see if someone finally won something //////////////////////////
long GameIsOver(void)
{
   long i, j, k;
   bool gmeOver = 0;  // is the game over?
   bool rndOver = 0;  // is the round over?
   long nout    = 0;  // number of out players
   char left    = -1; // ID of player that remains

   // see how many people are out
   for(i = 0; i < 4; i++) {
      if(pdat[i].flags & PLR_ON) {
         if(pdat[i].flags & PLR_ACTIVE) left = (char)i;
         else nout++;
      }
   }

   // check to see if everyone is gone
   if(nout == nplrs) {
      gWinner = -1;
      rndOver = 1;
      gmeOver = gnGames > 1;

   // check teams
   } else if(gnTeamPlrs[0] > 0) {

      for(i = 0; i < 2; i++) {
         if(gnTeamPlrsLeft[i] <= 0) {

            // get ID of opposite team
            k = 1 - i;

            // ajust their games won varables
            for(j = 0; j < gnTeamPlrs[k]; j++) pdat[ gTeams[k][j] ].nGamesWon++;

            gWinner = (UCHAR)k;
            rndOver = 1;
            gmeOver = pdat[ gTeams[k][0] ].nGamesWon >= gnGames;
         }
      }

   // see if a single person has won the game
   } else if(nout + 1 == nplrs) {
      pdat[left].nGamesWon++;
      gWinner = left;
      rndOver = 1;
      gmeOver = pdat[left].nGamesWon >= gnGames;
   }

   // if the round is over, close out the game
   if(rndOver) {

      // hide the bars now if its a network game
      if(gbNetwork) {
         BarShow(&gBarNames[0], 0, 0);
         BarShow(&gBarNames[1], 0, 0);
         BarShow(&gBarNames[2], 0, 0);
         BarShow(&gBarNames[3], 0, 0);
         BarShow(&gBarDice[0], 0, 0);
         BarShow(&gBarDice[1], 0, 0);
      }

      // set the game to clear for the next round
      // NOTE: MUST call GameClearForNew before setting
      // game_nxtst, because GameClearForNew would un-do it.
      GameClearForNew();
      GameGetWinText();
      game_nxtst = gmeOver? GAME_OVER : GAME_ROUNDOVER;
      NameDraw();
   }

   return rndOver;
}

// check to see who got knocked out //////////////////////////////////////
long GameCheckPlayers(void)
{
   long i, j, r;

   for(i = j = r = 0; i < 4; i++) {

      // make sure a player is even in the game
      if(pdat[i].flags & PLR_ACTIVE) {

         // the players is out, hide their name
         if(pdat[i].nMarbles <= 0) {
            BREM(pdat[i].flags, PLR_ACTIVE);
            gnTeamPlrsLeft[ pdat[i].team ]--;
            BarShow(&gBarNames[j], gBarNames[j].xmin < 320, 0);
            r = 1;
         }
      }

      // increment players counted
      if(pdat[i].flags & PLR_ON) j++; 
   }

   // return weather any players were knocked out
   return r;
}

// get the text for who won the game /////////////////////////////////////
void GameGetWinText(void)
{
   long nWon;

   // make string with teams in mind
   if(gnTeamPlrs[0] > 0) {

      switch(gnTeamPlrs[gWinner]) {

      // only one player has won
      case 1:
         wsprintf(gsWinText, TXT_1PLRSWIN, pdat[ gTeams[gWinner][0] ].sName);
         break;
      
      // two players have won
      case 2:
         wsprintf(gsWinText, TXT_2PLRSWIN,
            pdat[ gTeams[gWinner][0] ].sName,
            pdat[ gTeams[gWinner][1] ].sName);
         break;

      // three players have won
      case 3:
         wsprintf(gsWinText, TXT_3PLRSWIN,
            pdat[ gTeams[gWinner][0] ].sName,
            pdat[ gTeams[gWinner][1] ].sName,
            pdat[ gTeams[gWinner][2] ].sName);
         break;
      }

      // get the number of games this team has won
      nWon = pdat[gTeams[gWinner][0] ].nGamesWon;

   // only a single player has won
   } else {
      wsprintf(gsWinText, TXT_1PLRSWIN, pdat[gWinner].sName);
      nWon = pdat[gWinner].nGamesWon;
   }

   // if it is a network game, we are done
   if(gbNetwork) return;

   // decide weather to add "round" or "game" to the end of the string
   strcat(gsWinText, gnGames == nWon? TXT_WINGAME : TXT_WINROUND);
}


//————————————————————————————————————————————————————————————————————————
// Game Initilization
//————————————————————————————————————————————————————————————————————————

// clear stuff up to prepair for a new game //////////////////////////////
void GameClearForNew(void)
{
   gAnimateX  = -1;
   gAnimateY  = -1;
   gFrame     = 20;
   gFrameVel  = VEL_HIDEMARBLE;
   gLastTick  = gThisTick;
   game_state = GAME_ALLMARBLESHIDE;
   game_nxtst = GAME_REQUESTNEW;
}

// hide the utility bars /////////////////////////////////////////////////
void GameHideBars(void)
{
   BarShow(&gBarTeams, 0, 1);
   BarShow(&gBarChat , 0, 0);
   BarShow(&gBarRoll , 0, 0);
   BarShow(&gBarDice[0], 0, 0);
   BarShow(&gBarDice[1], 0, 0);
   BarShow(&gBarNames[0], gBarNames[0].x < 320, 0);
   BarShow(&gBarNames[1], gBarNames[1].x < 320, 0);
   BarShow(&gBarNames[2], gBarNames[2].x < 320, 0);
   BarShow(&gBarNames[3], gBarNames[3].x < 320, 0);
}

// initilzie a new game //////////////////////////////////////////////////
void GameInitNewGame(void)
{
   long i;

   // set number of players remaining on a team
   gnTeamPlrsLeft[0] = gnTeamPlrs[0];
   gnTeamPlrsLeft[1] = gnTeamPlrs[1];

   // initilize player data
   for(i = 0; i < 4; i++) {
      pdat[i].nMarbles = 10;
      if(pdat[i].flags & PLR_ON) pdat[i].flags |=  PLR_ACTIVE;
      else                       pdat[i].flags &= ~PLR_ACTIVE;
   }

   // make sure the game is not networked
   if(!gbNetwork) {

      // re-initilize stuff
      GameInitBoard();

      // choose a random player
      do { pcur = RAND(4); } while((pdat[pcur].flags & PLR_ACTIVE) == 0);

      // give the first set of dice a value
      gMoveData[6] = RAND(6);
      gMoveData[7] = RAND(6);
   }

   // initilize game stuff
   if(gbVisible && gnTeamPlrs[0] > 0) BarsDrawTeam();
   BarsInitB();
   MenuInit();
   NameSetup();

   // edit menu items
   if(gbSoundOn) MenuPush(8);
   if(gbNetwork && gbMusicOn) MenuPush(7);
   MenuEnable(7, gbMusicAval);
   MenuEnable(8, gbSoundAval);

   // initilize varables
   memset(gMovesX, -1, sizeof(gMovesX));
   memset(gMovesY, -1, sizeof(gMovesY));
   gSelectedX = -1;
   gSelectedY = -1;
   gAnimateX  = -1;
   gAnimateY  = -1;
   gFrame     = 0;

   if(gbVisible) {

      gFrameVel  = VEL_SHOWMARBLE;
      gLastTick  = gThisTick;
      game_state = GAME_ALLMARBLESSHOW;

   } else {

      // show the utility bars
      GameEndShowMarbles();

      // show the name bars
      for(i = 0; i < nplrs; i++) {
         BarSetEnd(&gBarNames[i], gBarNames[i].xmax > 320, 0);
      }

      // setup the dice
      i = 26*pcur;
      gBarDice[0].extra = gMoveData[6];
      gBarDice[1].extra = gMoveData[7];
      RSETW(gBarDice[0].rsrc, i, 26*gBarDice[0].extra, 26, 27);
      RSETW(gBarDice[1].rsrc, i, 26*gBarDice[1].extra, 26, 27);

      // show the dice
      BarSetEnd(&gBarDice[0], 0, 1);
      BarSetEnd(&gBarDice[1], 0, 1);

      // setup for first move
      GameSetMove();
   }
}

// setup for the first move //////////////////////////////////////////////
void GameSetMove(void)
{
   char sText[64];

   // make sure the player can move
   if(!GameCanMove()) {

      // display message that player couldnt move
      wsprintf(sText, TXT_HASNOMOVES, pdat[pcur].sName);

      if(gbNetwork) {
         ChatAddText(4, sText);
      } else {
         game_state = GAME_IDLE;
         MsgBox(sText, TXT_GENTITLE, TXT_OKAY, 0);
      }

      // switch players
      if(gbVisible) {
         game_state = GAME_HIDEDICE;
         BarShow(&gBarDice[0], 0, 0);
      } else GameChangePlrMinimized();

      return;
   }

   // get input from network players
   if(gbNetwork && gThisPlr != pcur) {

      if(gnMoveDataCashe > 0) GameMoveMarbleCashe();  // we had another move cashed
      else game_state = GAME_WAITNETINPUT;  // we can get input from network player

   // get input from computer players
   } else if(pdat[pcur].type > 0) {
      PcExecuteMove();

   // get input from human players
   } else {
      game_state      = GAME_GETINPUT;
      gnMoveDataCashe = 0; // just in case
   }
}

// initilize the game board //////////////////////////////////////////////
void GameInitBoard(void)
{
   long i, j;
   long a, b;
   
   // clear the board
   memset(gBoard, MARBLE_SPACE, sizeof(gBoard));

   // set the invalid corner regions
   for(i = 0; i < 3; i++) {
      for(j = 0; j < 3; j++) {

         // cash values
         a = i + 13;
         b = j + 13;

         // set an item in each corner to invalid
         gBoard[i][j] = MARBLE_INVALID;
         gBoard[i][b] = MARBLE_INVALID;
         gBoard[a][j] = MARBLE_INVALID;
         gBoard[a][b] = MARBLE_INVALID;
      }
   }

   // set the player's marbles up
   for(i = 3; i < 13; i++) {
      if(pdat[0].flags & PLR_ON) gBoard[i][0]  = 0; // red
      if(pdat[1].flags & PLR_ON) gBoard[15][i] = 1; // teal
      if(pdat[2].flags & PLR_ON) gBoard[i][15] = 2; // purple
      if(pdat[3].flags & PLR_ON) gBoard[0][i]  = 3; // tan
   }

   // setup random sink holes
   for(i = 0; i < gnSinkHoles; i++) {

      // find an empty location
      do {
         a = RAND(14) + 1;
         b = RAND(14) + 1;
      } while(gBoard[a][b] != MARBLE_SPACE);

      // set the location to be a sinkhole
      gBoard[a][b] = MARBLE_SINKHOLE;
   }
}


//————————————————————————————————————————————————————————————————————————
// Marble Functions
//————————————————————————————————————————————————————————————————————————

// oops, we blew up a marble /////////////////////////////////////////////
void MarbleExplode(char x, char y)
{
   gFrame     = 0;
   gFrameVel  = VEL_EXPLODE;
   gAnimateX  = x;
   gAnimateY  = y;
   gLastTick  = gThisTick;
   game_state = GAME_MARBLEEXPLODE;
}

// haha, those sink holes can be a real pain /////////////////////////////
void MarbleSink(char x, char y)
{
   gbSinkPlayed = 0;
   gFrame       = 20;
   gFrameVel    = VEL_HIDEMARBLE;
   gAnimateX    = x;
   gAnimateY    = y;
   gLastTick    = gThisTick;
   game_state   = GAME_MARBLEHIDE;
}

// get the location where the new free marble is gonna go ////////////////
void MarbleGetFreePos(void)
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
      if(gBoard[*i][*j] == MARBLE_SPACE || (*i == gSelectedX && *j == gSelectedY)) {
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

// create that new marble ////////////////////////////////////////////////
void MarbleCreateFree(void)
{
   // see if there was a place for the marble to go
   if(gMoveData[4] < 16) {
      
      // play the free marble sound
      DsbPlay(dsbFree, MarbleX(gMoveData[4]));

      // update the graph
      NameReGraph(pcur, 1);

      // create the new marble
      gFrame     = 0;
      gFrameVel  = VEL_SHOWMARBLE;
      gAnimateX  = gMoveData[4];
      gAnimateY  = gMoveData[5];
      gLastTick  = gThisTick;
      game_state = GAME_MARBLESHOW;

      // set the marble on the board
      gBoard[ gMoveData[4] ][ gMoveData[5] ] = pcur;

   // there was no place for the marble
   } else {
      DsbPlay(dsbCant, SOUND_CENTER);
      GameEndTurn();
   }
}

// figure out weather the marble looks friendly or not ///////////////////
long MarbleIsEnemy(char a, char b)
{
   if(gnTeamPlrs[0] > 0) return (pdat[a].team != pdat[b].team); 
   else                  return a != b;
}

// return the x of a marble //////////////////////////////////////////////
long MarbleX(long a)
{
   if(a == 0)       return 68;          // left
   else if(a == 15) return 546;         // right
   else             return 30 * a + 82; // anything else
}

// return the y of a marble //////////////////////////////////////////////
long MarbleY(long b)
{
   if(b == 0)       return 21;          // top
   else if(b == 15) return 327;         // bottom
   else             return 20 * b + 24; // anything else
}

//————————————————————————————————————————————————————————————————————————
// Pause Functions
//————————————————————————————————————————————————————————————————————————

// pause the game ////////////////////////////////////////////////////////
void GamePause(void)
{
   if(++gbPaused < 2) {
      game_psest = game_state;
      game_state = GAME_PAUSED;
   }
}

// resume the game ///////////////////////////////////////////////////////
void GameResume(void)
{
   gbPaused--;

   if(gbPaused == 0) {
      gObjLTick  = gThisTick;
      gLastTick  = gThisTick;
      game_state = game_psest;
      game_psest = GAME_IDLE;
   }
}


//————————————————————————————————————————————————————————————————————————
// Splash Screen Code
//————————————————————————————————————————————————————————————————————————

// render the really snazy intro /////////////////////////////////////////
long SplashRender(long mode)
{
   PALETTEENTRY pal[256];
   RECT         rect;
   double       value;
   long         i;

   // get the frame change
   value = ((double)(gThisTick - gLastTick)) * VEL_SPLASH;

   //  update the frame
   if(mode == 1) gFrame -= value;
   else          gFrame += value;

   // save last tick
   gLastTick = gThisTick;

   // make sure the frame doesnt dip below zero, or above 256
   if(gFrame < 0)   gFrame = 0;
   if(gFrame > 255) gFrame = 255;

   // get the current palette
   ddpMain->GetEntries(0, 0, 256, pal);

   // update the palette
   for(i = 0; i < 256; i++) {

      pal[i].peRed   = (UCHAR)gFrame;
      pal[i].peGreen = (UCHAR)gFrame;
      pal[i].peBlue  = (UCHAR)gFrame;

      if(pal[i].peRed > gPalette[i].peRed)     pal[i].peRed = gPalette[i].peRed;
      if(pal[i].peGreen > gPalette[i].peGreen) pal[i].peGreen = gPalette[i].peGreen;
      if(pal[i].peBlue > gPalette[i].peBlue)   pal[i].peBlue = gPalette[i].peBlue;
   }

   // re-set the palette
   ddpMain->SetEntries(0, 0, 256, pal);

   // put image on back buffer
   RSET(rect, 0, 0, 640, 480);
   DDBltFast(ddsBack, ddsWnd, 0, 0, &rect, NOCK);

   // flip the font & back buffers
   DDRVAL(ddsFront->Flip(0, DDFLIP_WAIT));
   return mode == 1? gFrame <= 0 : gFrame >= 255;
}


//————————————————————————————————————————————————————————————————————————
// Core Functions
//————————————————————————————————————————————————————————————————————————

// show the load game window /////////////////////////////////////////////
void ProgMenuLoad(void)
{
   long rval;

   // display the load game window
   GamePause();
   rval = Wnd(WNDID_LOAD, WndLoad);
   GameResume();

   // see if we should load a game
   if(rval >= 0) {

      // load the game
      IoGame(FILE_LOADED, 1);

      // setup team bar
      BarsDrawTeam();
      if(gnTeamPlrs[0] > 0) BarSetEnd(&gBarTeams, 0, 0);

   // cancel button was used
   } else if(game_state == GAME_IDLE) {
      game_state = GAME_REQUESTNEW;
   }
}

// save as menu wrapper //////////////////////////////////////////////////
void ProgMenuSaveAs(void)
{
   GamePause();

   while(1) {

      // save button used
      if(Wnd(WNDID_SAVE, WndSaveAs)) {

         // see if a name was typed
         if(lstrlen(FILE_LOADED) <= 9) {
            MsgBox(TXT_ENTERFILENAME, TXT_GENTITLE, TXT_OKAY, 0);

         // see if the file alredy exists
         } else if(IoFileExists(FILE_LOADED)) {

            // confirm player wants to overwrite
            if(MsgBox(TXT_OVERWRITE, TXT_GENTITLE, TXT_CANCEL, TXT_OKAY)) {
               GameResume();
               IoGame(FILE_LOADED, 0);
               return;
            }

         // no other errors to entertain
         } else {
            GameResume();
            IoGame(FILE_LOADED, 0);
            return;
         }

      // cancel button used
      } else {
         GameResume();
         return;
      }
   }
}

// music menu wrapper ////////////////////////////////////////////////////
void ProgMenuMusic(bool down)
{
   // only turn the music on or off
   if(gbNetwork) {

      gbMusicOn = down;

      if(down) {
         MusicInit();
         MusicPlayNext();
      } else MusicClose();

   // bring up the music options window
   } else {
      GamePause();
      Wnd(WNDID_MUSIC, WndMusicOptions);
      GameResume();
   }
}

// the main menu thingie /////////////////////////////////////////////////
void ProgMenu(long menuId, bool down)
{
   switch(menuId) {
   case 0: GameClearForNew();          return; // new game
   case 1: ProgMenuLoad();             return; // open game
   case 3: ProgMenuSaveAs();           return; // save as
   case 4: Help(0);                    return; // help menu
   case 5: prog_state = PROG_MINIMIZE; return; // minimize
   case 7: ProgMenuMusic(down);        return; // music options
   case 8: gbSoundOn  = down;          return; // sound on/off

   // exit menu
   case 6:

      // confirm exit command for network mode
      if(gbNetwork) if(MsgBox(TXT_NETEXIT, TXT_GENTITLE, TXT_OKAY, TXT_CANCEL)) return;

      // set the game to exit
      prog_state = PROG_CLOSE;
      return;

   // save game
   case 2:
      if(lstrlen(FILE_LOADED) <= 9) ProgMenuSaveAs();
      else                          IoGame(FILE_LOADED, 0);
      return;
   
   }
}

// the bottom of nearly everything ///////////////////////////////////////
void ProgLoop(void)
{
   // get current tick
   gThisTick = GetTickCount();

   switch(prog_state) {

   // the splash screen is being shown
   case PROG_SPLASHA:

      long i;
      bool keyUsed;
      keyUsed = 0;

      // get input & render the splash screen
      DIGetInput();
      SplashRender(0);

      // see if any of the keyboard keys were pressed
      for(i = 0; i < 256; i++) keyUsed = keyUsed || key_down[i];

      // see if we should end the splash screen
      if(gThisTick - gTimerStart > WAIT_SPLASH || CUR_CLICK || keyUsed)
         prog_state = PROG_SPLASHB;

      return;

   // the splash screen is being hidden
   case PROG_SPLASHB:
      if(SplashRender(1)) {

         LPDDS temp;

         // load the game palette & clear window surface
         XalLoad(gPalette, PAL_MAIN);
         DDClear(ddsWnd, CLR_BKGND);

         // render the game to the window surface
         temp = ddsBack;
         ddsBack = ddsWnd;
         GameRender();
         GameRenderUi();
         ddsBack = temp;

         // set the new game state
         prog_state = PROG_SPLASHC;
      }
      return;

   // the game is being shown
   case PROG_SPLASHC:
      if(SplashRender(2)) {
         ddpMain->SetEntries(0, 0, 256, gPalette);
         prog_state = PROG_PLAYING;
         game_state = GAME_REQUESTNEW;
         MusicPlayNext();
      }
      return;
   
   // the cool stuff
   case PROG_PLAYING:

      DIGetInput();

      // update current frame
      GameUpdate();

      if(!gbVisible) return;

      GameUpdateCursor();
      GameUpdateBars();
      GameUpdateMenu();
      if(gbNetwork && !gbMsgBox) GameUpdateChat();
      GameUpdateName();
      if(gbMsgBox) WndUpdate();
      GameUpdatePopUp();

      // render current frame
      DDClear(ddsBack, CLR_BKGND);
      GameRender();
      GameRenderUi();

#ifdef GAME_DEBUG
      DebugDrawFps();
#endif

      // flip the font & back buffers
      DDRVAL(ddsFront->Flip(0, DDFLIP_WAIT));
      return;

   // the game is to be minimized
   case PROG_MINIMIZE:
      ShowWindow(gHWnd, SW_MINIMIZE);
      return;

   // the game is to be closed
   case PROG_CLOSE:
      PostMessage(gHWnd, PROG_KILL, 0, 0);
      return;
   }
}

// initilize game data ///////////////////////////////////////////////////
long ProgInit(void)
{
#ifdef GAME_DEBUG
   DebugOpen();
#endif 

   srand(GetTickCount());
   ShowCursor(0);

   //-------------------------------------------------
   // Component Initilization
   //-------------------------------------------------

   // make sure there is a resource file to read from
   if(!IoFileExists(RES_FILENAME)) {
      MessageBox(gHWnd, TXT_NORES, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // Initilize COM
   if(FAILED(CoInitialize(0))) {
      MessageBox(gHWnd, TXT_NOCOM, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // attempt to initilize DirectInput
   if(!DIInit()) {
      MessageBox(gHWnd, TXT_NODIRECTINPUT, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // attempt to initilize DirectDraw
   if(!DDInit()) {
      MessageBox(gHWnd, TXT_NODIRECTDRAW, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // clear the palette
   memset(gPalette, 0, sizeof(gPalette));

   // make the last entry white for loading bitmap
   gPalette[255].peBlue = gPalette[255].peGreen = gPalette[255].peRed   = 255;

   // create the palette
   if(lpdd->CreatePalette(DDPCAPS_8BIT | DDPCAPS_INITIALIZE | DDPCAPS_ALLOW256, gPalette, &ddpMain, 0) != DD_OK) {
      MessageBox(gHWnd, TXT_NOPALETTE, TXT_GENTITLE, MB_OK);
      return 0;
   }

   // attach new palete to the front surface
   ddsFront->SetPalette(ddpMain);

   // load the splash palette
   XalLoad(gPalette, PAL_SPLASH);

   // load the loading image
   DdsCreate(&ddsLoading, XMP_LOADING);

   // initilize player data (must be done *before*
   // IoSettingsCreateDefault ever has a chanced to be called)
   for(long i = 0; i < 4; i++) {
      wsprintf(pdat[i].sName, "Player %d", i+1);
      pdat[i].flags     = PLR_ACTIVE | PLR_ON;
      pdat[i].nGamesWon = 0;
      pdat[i].nMarbles  = 10;
      pdat[i].team      = 0;
   }

   // load sound activation
   IoSettingsSoundOn(1);

   // initilize sound & music
   DsInit();
   MusicInit();


   //-------------------------------------------------
   // Setup DirectDraw Surfaces
   //-------------------------------------------------

   // setup fonts
   FntLoad(&gFntTitle    , FNT_TITLE);
   FntLoad(&gFntNames    , FNT_NAMES);
   FntLoad(&gFntSmall    , FNT_SMALL);
   FntLoad(&gFntWndTxt   , FNT_WNDTXT);
   FntLoad(&gFntWndTxtHil, FNT_WNDTXTHIL);
   FntLoad(&gFntWndTxtDim, FNT_WNDTXTDIM);

   // setup bitmap holding surfaces
   DdsCreate(&ddsItems   , XMP_ITEMS);
   DdsCreate(&ddsBoard   , XMP_BOARD);
   DdsCreate(&ddsMarbles , XMP_MARBLES);
   DdsCreate(&ddsHoles   , XMP_HOLES);
   DdsCreate(&ddsExplode , XMP_EXPLODE);
   DdsCreate(&ddsSelect  , XMP_SELECT);
   DdsCreate(&ddsDice    , XMP_DICE);
   DdsCreate(&ddsSMenu   , XMP_SMENU);
   DdsCreate(&ddsMenus   , XMP_MENUS);
   DdsCreate(&ddsPlrType , XMP_PLAYERTYPE);

   // setup blank surfaces
   DdsCreate(&ddsMsg     , 300, 17);
   DdsCreate(&ddsMenuBar , 26 , 210);
   DdsCreate(&ddsNames   , 273, 176);
   DdsCreate(&ddsTeams   , 137, 31);
   DdsCreate(&ddsChat    , 336, 89);
   DdsCreate(&ddsDiceBar , 104, 35);
  
   // load the splash screen onto the wnd surface
   DdsCreate(&ddsWnd, XMP_SPLASH);


   //-------------------------------------------------
   // Setup DirectSound Buffers
   //-------------------------------------------------

   DsbCreate(&dsbCurWithin, XAV_CURWITHIN);
   DsbCreate(&dsbDefault  , XAV_DEFAULT);
   DsbCreate(&dsbExplode  , XAV_EXPLODE);
   DsbCreate(&dsbFree     , XAV_FREEMARBLE);
   DsbCreate(&dsbCant     , XAV_CANT);
   DsbCreate(&dsbSelect   , XAV_SELECT);
   DsbCreate(&dsbSink     , XAV_SINK);
   DsbCreate(&dsbRise     , XAV_RISE);


   //-------------------------------------------------
   // Initilize Application Globals
   //-------------------------------------------------


   // clear the board varable
   memset(gBoard, MARBLE_SPACE, sizeof(gBoard));

   // initilize UI objects
   BarsInitA();
   BarsInitB();
   NameSetup();
   MenuInit();
   ChatInit();
   BarsDrawDice();

   // setup menu items
   if(gbSoundOn) MenuPush(8);
   MenuEnable(7, gbMusicAval);
   MenuEnable(8, gbSoundAval);

   // setup for splash screen
   gFrame      = 0;
   gTimerStart = GetTickCount();
   gLastTick   = gTimerStart;

   /*
   {
#include <stdio.h>
#include <stdlib.h>

      FILE         *file;
      MEMORYSTATUS  memstat;
      memset(&memstat, 0, sizeof(memstat));
      memstat.dwLength = sizeof(MEMORYSTATUS);
      GlobalMemoryStatus(&memstat);
      file = fopen("c:\\windows\\desktop\\out.txt", "w");
      fprintf(file, "Free Memory: %d", memstat.dwAvailPhys);
      fclose(file);
   }
   */

   return 1;
}

// restore all of the DirectDraw surfaces (fonts included :) /////////////
long ProgRestoreSurfaces(void)
{
   // restore the main surface
   if(!DdsRestore(ddsFront)) return 0;

   // re-attach the main palette
   ddsFront->SetPalette(ddpMain);

   // restore fonts
   FntRestore(&gFntTitle    , FNT_TITLE);
   FntRestore(&gFntNames    , FNT_NAMES);
   FntRestore(&gFntSmall    , FNT_SMALL);
   FntRestore(&gFntWndTxt   , FNT_WNDTXT);
   FntRestore(&gFntWndTxtHil, FNT_WNDTXTHIL);
   FntRestore(&gFntWndTxtDim, FNT_WNDTXTDIM);

   // restore bitmap holding surfaces
   XmpReload(ddsLoading , XMP_LOADING);
   XmpReload(ddsItems   , XMP_ITEMS);
   XmpReload(ddsBoard   , XMP_BOARD);
   XmpReload(ddsMarbles , XMP_MARBLES);
   XmpReload(ddsHoles   , XMP_HOLES);
   XmpReload(ddsExplode , XMP_EXPLODE);
   XmpReload(ddsSelect  , XMP_SELECT);
   XmpReload(ddsDice    , XMP_DICE);
   XmpReload(ddsSMenu   , XMP_SMENU);
   XmpReload(ddsMenus   , XMP_MENUS);
   XmpReload(ddsPlrType , XMP_PLAYERTYPE);

   // restore blank surfaces
   DdsRestore(ddsMsg);
   DdsRestore(ddsMenuBar);
   DdsRestore(ddsNames);
   DdsRestore(ddsTeams);
   DdsRestore(ddsChat);
   DdsRestore(ddsDiceBar);
   DdsRestore(ddsWnd);

   // re-draw surfaces
   MenuDraw();
   NameDraw();
   ChatDraw();
   BarsDrawDice();
   BarsDrawTeam();
   if(gbMsgBox) WndReDraw();
   NameDrawSelect(pcur, 1);
   return 1;
}

// restore all the DirectSound buffes ////////////////////////////////////
long ProgRestoreSound(void)
{
   DsbRestore(dsbCurWithin, XAV_CURWITHIN);
   DsbRestore(dsbDefault  , XAV_DEFAULT);
   DsbRestore(dsbExplode  , XAV_EXPLODE);
   DsbRestore(dsbFree     , XAV_FREEMARBLE);
   DsbRestore(dsbCant     , XAV_CANT);
   DsbRestore(dsbSelect   , XAV_SELECT);
   DsbRestore(dsbSink     , XAV_SINK);
   DsbRestore(dsbRise     , XAV_RISE);
   return 1;
}

// close the program stuff ///////////////////////////////////////////////
void ProgClose(void)
{
   // save sound activation
   IoSettingsSoundOn(0);

   // close DirectPlay
   DpQuit();

   // close DirectDraw
   if(lpdd) {
      DXOBJ_DELETE(ddpMain);
      DXOBJ_DELETE(ddsFront);

      DXOBJ_DELETE(ddsLoading);
      DXOBJ_DELETE(ddsItems);
      DXOBJ_DELETE(ddsBoard);
      DXOBJ_DELETE(ddsMarbles);
      DXOBJ_DELETE(ddsHoles);
      DXOBJ_DELETE(ddsExplode);
      DXOBJ_DELETE(ddsSelect);
      DXOBJ_DELETE(ddsDice);
      DXOBJ_DELETE(ddsSMenu);
      DXOBJ_DELETE(ddsMenus);
      DXOBJ_DELETE(ddsPlrType);

      DXOBJ_DELETE(ddsWnd);
      DXOBJ_DELETE(ddsMsg);
      DXOBJ_DELETE(ddsMenuBar);
      DXOBJ_DELETE(ddsNames);
      DXOBJ_DELETE(ddsTeams);
      DXOBJ_DELETE(ddsChat);
      DXOBJ_DELETE(ddsDiceBar);

      FntDelete(&gFntTitle);
      FntDelete(&gFntNames);
      FntDelete(&gFntSmall);
      FntDelete(&gFntWndTxt);
      FntDelete(&gFntWndTxtHil);
      FntDelete(&gFntWndTxtDim);

      lpdd->Release();
      lpdd = 0;
   }

   // close DirectSound
   if(lpds) {
      DXOBJ_DELETE(dsbCurWithin);
      DXOBJ_DELETE(dsbDefault);
      DXOBJ_DELETE(dsbExplode);
      DXOBJ_DELETE(dsbFree);
      DXOBJ_DELETE(dsbCant);
      DXOBJ_DELETE(dsbSelect);
      DXOBJ_DELETE(dsbSink);
      DXOBJ_DELETE(dsbRise);

      lpds->Release();
      lpds = 0;
   }

   // close music & DirectInput 
   MusicClose();
   DIClose();

   // Release COM
   CoUninitialize();

   ShowCursor(1);

#ifdef GAME_DEBUG
   DebugClose();
#endif
}