/************************************************************************/
// game.h 
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

#ifndef GAME_HEADER_INCLUDED
#define GAME_HEADER_INCLUDED

#include "utility.h"
#include "sound.h"
#include "user.h"


//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// misc. application definitions /////////////////////////////////////////
#define SCREEN_W           640
#define SCREEN_H           480
#define TXT_WINCLASS       "ELIMINATION3"
#define TXT_WINTITLE       "Elimination III"
#define RES_FILENAME       "res.dat"

// colors ////////////////////////////////////////////////////////////////
#define CLR_TRANS           110
#define CLR_BKGND           110
#define CLR_BTNPUSHED       220
#define CLR_DTAN            169
#define CLR_LTAN            203
#define CLR_DGREY           204
#define CLR_LGREY           236
#define CLR_WHITE           255
#define CLR_GRAPHBORDER     204
#define CLR_GRAPHFILL       235
#define CLR_NGAMEMARKBORDER 207
#define CLR_BLUE            81
#define CLR_LBLUE           101
#define CLR_BLACK           0
#define CLR_SPLASHBLACK     255
#define CLR_FREEBORDER      88

// program states ////////////////////////////////////////////////////////
#define PROG_SPLASHA        0     // splash screen is being shown
#define PROG_SPLASHB        1     // splash screen is being hidden
#define PROG_SPLASHC        2     // game is being brought in
#define PROG_PLAYING        3     // game is being played
#define PROG_MINIMIZE       4     // minimize button was clicked
#define PROG_SUSPENDED      5     // program is suspended
#define PROG_CLOSE          6     // program should be shutdown

// game states ///////////////////////////////////////////////////////////
#define GAME_IDLE          0      // game is idle
#define GAME_PAUSED        1      // game is paused
#define GAME_REQUESTNEW    2      // a new game is being requested
#define GAME_SHOWMARBLES   3      // show all of the marbles
#define GAME_WAITBARS      4      // waiting for utility bars
#define GAME_WAITNAMEBARS  5      // waiting for the name bars
#define GAME_ROLLDICE      6      // the dice are rolling
#define GAME_GETINPUT      7      // get the players input
#define GAME_BULLETMOVE    8      // the bullet is moving
#define GAME_MARBLEEXPLODE 9      // a marble is exploding
#define GAME_MARBLEMOVE    10     // a marble is moving
#define GAME_MARBLESINK    11     // a marble is sinking into the board
#define GAME_HIDEDICE      12     // the dice are being hidden
#define GAME_NEXTPLAYER    13     // we should change to the next player
#define GAME_ROUNDOVER     14     // the round is over
#define GAME_OVER          15     // the game is over
#define GAME_JOINNET       16     // we are thinking about joinging a network game
#define GAME_STARTNET      17     // we are about to begin a network game
#define GAME_WAITNETINPUT  18     // waiting for a network mvoe
#define GAME_WAITNEWNET    19     // waiting to start a new network game

// game flags ////////////////////////////////////////////////////////////
#define GAME_NETWORKED     0x0001  // game is a network game
#define GAME_SOUNDON       0x0002  // sound is on
#define GAME_MUSICON       0x0004  // music is on
#define GAME_SUNAVALABLE   0x0008  // sound is unavalable
#define GAME_MUNAVALABLE   0x0010  // music is unavalable
#define GAME_DMUNAVALABLE  0x0020  // direct music is unavalable
#define GAME_CDUNAVALABLE  0x0040  // CD music is unavalable
#define GAME_MSGBOX        0x0080  // a message box is visible
#define GAME_SELECTNAME    0x0100  // a players name is being selected

// network game messages /////////////////////////////////////////////////
#define NETMSG_STARTGAME   0       // the game is to be started
#define NETMSG_CHATA       1       // a chat is sent from the window
#define NETMSG_CHATB       2       // a chat is sent from the game
#define NETMSG_MOVEMARBLE  3       // move a marble
#define NETMSG_RELISTPLRS  4       // host says to re-list players
#define NETMSG_READYPLR    5       // a player is changing their ready state
#define NETMSG_CHANGESINK  6       // change the number of sinkholes
#define NETMSG_CHANGEGAMES 7       // change the number of games
#define NETMSG_READYFORNEW 8       // a player is ready for a new game

// board values //////////////////////////////////////////////////////////
#define MARBLE_INVALID     -1      // location is invalid
#define MARBLE_SPACE       0       // location is a space
#define MARBLE_FREE        1       // location has a free marble
#define MARBLE_SINKHOLE    2       // location is a sink hole

// player flags //////////////////////////////////////////////////////////
#define PLR_ON             0x01    // player is on
#define PLR_ACTIVE         0x02    // player is active
#define PLR_SCROLLGRAPH    0x04    // players graph is moving


//————————————————————————————————————————————————————————————————————————
// Structures
//————————————————————————————————————————————————————————————————————————

// player data structure /////////////////////////////////////////////////
struct PLR {
   char  name[16];
   DPID  dpId;
   long  fdir;
   long  team;
   long  frame;
   long  nmarbles;
   long  games_won;
   DWORD ltick;
   UCHAR flags;
};


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// direct draw surfaces //////////////////////////////////////////////////
extern LPDDP ddpMain;
extern LPDDS ddsFront;
extern LPDDS ddsBack;

extern LPDDS ddsItems;
extern LPDDS ddsExplode;
extern LPDDS ddsMenuItems;
extern LPDDS ddsDice;
extern LPDDS ddsSelect;

extern LPDDS ddsBoard;
extern LPDDS ddsMenuBar;
extern LPDDS ddsTeams;
extern LPDDS ddsChat;
extern LPDDS ddsNames;
extern LPDDS ddsDiceBar;
extern LPDDS ddsMsg;
extern LPDDS ddsWnd;

// direct sound buffers //////////////////////////////////////////////////
extern LPDSB dsbError;
extern LPDSB dsbExplode;
extern LPDSB dsbFree;
extern LPDSB dsbRise;
extern LPDSB dsbSelect;
extern LPDSB dsbSink;
extern LPDSB dsbNoNo;
extern LPDSB dsbMHide;
extern LPDSB dsbMShow;
extern LPDSB dsbCWithin;

// game varables /////////////////////////////////////////////////////////
extern DWORD gThisTick;
extern UCHAR prog_state;
extern UCHAR game_nxtst;
extern UCHAR game_state;
extern ULONG game_flags;
extern long  gnGames;
extern long  gnSinkHoles;
extern BYTE  gMoveData[8];
extern BYTE  gNetData[2];
extern long  gMoveDataValid;
extern char  g_board[16][16];
extern long  gThisPlr;

// player data ///////////////////////////////////////////////////////////
extern PLR  pdat[4];
extern long nplrs;
extern long pcur;
extern long gnPlrsReady;

//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// main game loops ///////////////////////////////////////////////////////
void GameUpdate(void);
void GameRender(void);

// move functions ////////////////////////////////////////////////////////
void GameMoveMarble(long, long, long, long);
void GameUnselect(void);
long GameIsMove(long, long);
void GameSelectMoves(void);

// network functions /////////////////////////////////////////////////////
void GameInitNet(void);
void GameSendChat(void);
void GameSendMove(void);
void GameKillNetPlr(DPID);

// misc functions ////////////////////////////////////////////////////////
long GameIsOver(void);
long GameCheckPlayers(void);
void GameEndTurn(void);
void GameUpdateGlow(void);
long GameIsEnemy(long, long);
void GameRoll(void);

// game initilization ////////////////////////////////////////////////////
void GameHideBars(void);
void GameInitNew(void);
void GameNew(void);
void GameInitBoard(void);

// marble functions //////////////////////////////////////////////////////
void MarbleExplode(long, long);
void MarbleSink(long, long);
void MarbleNew(void);
void MarbleNewGetPos(void);
long MarbleX(long);
long MarbleY(long);

// splash screen code ////////////////////////////////////////////////////
long GameRenderSplash(long);

// base program functions ////////////////////////////////////////////////
void ProgMenu(long, bool);
void ProgLoop(void);
long ProgInit(void);
long ProgRestoreSurfaces(void);
long ProgRestoreSound(void);
void ProgClose(void);

#endif // #ifndef GAME_HEADER_INCLUDED