/************************************************************************/
// game.h 
//
// Author: Justin Hoffman
// Date:   6/4/00 - 7/18/2000
/************************************************************************/

#ifndef GAME_HEADER_INCLUDED
#define GAME_HEADER_INCLUDED

#include "utility.h"
#include "sound.h"
#include "user.h"


//————————————————————————————————————————————————————————————————————————
// Definitins
//————————————————————————————————————————————————————————————————————————

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
#define NETMSG_CHANGETEAM  9       // a player has changed their team

// board values //////////////////////////////////////////////////////////
#define MARBLE_INVALID     -1      // location is invalid
#define MARBLE_SPACE       4       // location is a space
#define MARBLE_FREE        5       // location has a free marble
#define MARBLE_SINKHOLE    6       // location is a sink hole

// player flags //////////////////////////////////////////////////////////
#define PLR_ON             0x01    // player is on
#define PLR_ACTIVE         0x02    // player is active
#define PLR_SCROLLGRAPH    0x04    // players graph is moving

// game states ///////////////////////////////////////////////////////////
#define GAME_IDLE             0     // we dont seem to be doing alot right now
#define GAME_REQUESTNEW       1     // *please* bring the new game dialog up
#define GAME_WAITNEWNET       2     // waiting for a new network round to start
#define GAME_CONNECT          3     // bring up the connection window
#define GAME_STARTNET         4     // bring up the last network game window
#define GAME_STARTNETIDLE     5     // we are sitting in the last network game window
#define GAME_ALLMARBLESSHOW   6     // all marble are being shown
#define GAME_WAITUTILBARS     7     // the utility bar(s) are being shown
#define GAME_WAITNAMEBARS     8     // the name bars are being shown
#define GAME_ROLLDICE         9     // we are rolling the dice, hope its something good
#define GAME_GETINPUT         10    // i suppose we should wait for user input
#define GAME_WAITNETINPUT     11    // waiting for network buddies to finnaly make a move
#define GAME_BULLETMOVE       12    // ut oh, someone's getting shot at
#define GAME_MARBLEEXPLODE    13    // bigger ut ot, someone exploded
#define GAME_MARBLEMOVE       14    // its super marble!
#define GAME_MARBLESHOW       15    // a new marble is rising
#define GAME_MARBLEHIDE       16    // dang it, we found us a sinkhole
#define GAME_ALLMARBLESHIDE   17    // all the marbles are sinking into the baord
#define GAME_HIDEDICE         18    // put those dice away, you could hurt someone
#define GAME_NEXTPLAYER       19    // there are players in this game?
#define GAME_ROUNDOVER        20    // someone won the battle, but not the war
#define GAME_OVER             21    // over already??? I was having so much fun
#define GAME_PAUSED           22    // game is paused

// program states ////////////////////////////////////////////////////////
#define PROG_SPLASHA          0     // splash screen is being shown
#define PROG_SPLASHB          1     // splash screen is being hidden
#define PROG_SPLASHC          2     // game is being brought in
#define PROG_PLAYING          3     // game is being played
#define PROG_MINIMIZE         4     // minimize button was clicked
#define PROG_SUSPENDED        5     // program is suspended
#define PROG_CLOSE            6     // program should be shutdown

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
#define CLR_MENUBORDER      69

// misc. application definitions /////////////////////////////////////////
#define SCREEN_W           640
#define SCREEN_H           480
#define TXT_WINCLASS       "ELIMINATION3"
#define TXT_WINTITLE       "Elimination III"
#define RES_FILENAME       "res.dat"


//————————————————————————————————————————————————————————————————————————
// Structures
//————————————————————————————————————————————————————————————————————————

// player data structure /////////////////////////////////////////////////
struct PDAT {
   char  sName[16];
   DPID  dpId;
   long  frame_a;
   long  frame_b;
   long  frameDir;
   long  nMarbles;
   long  nGamesWon;
   long  team;
   UCHAR type;
   DWORD ltick_a;
   DWORD ltick_b;
   UCHAR flags;
};


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// game options //////////////////////////////////////////////////////////
extern bool gbVisible;
extern long gbPaused;
extern bool gbNetwork;
extern bool gbSoundOn;
extern bool gbMusicOn;
extern bool gbSoundAval;
extern bool gbMusicAval;
extern bool gbMciAval;
extern bool gbDirectMusicAval;
extern bool gbCdAudioAval;
extern bool gbMsgBox;
extern bool gbSelectName;

// direct draw surfaces //////////////////////////////////////////////////
extern LPDDP ddpMain;
extern LPDDS ddsFront;
extern LPDDS ddsBack;

extern LPDDS ddsLoading;
extern LPDDS ddsItems;
extern LPDDS ddsBoard;
extern LPDDS ddsMarbles;
extern LPDDS ddsHoles;
extern LPDDS ddsExplode;
extern LPDDS ddsSelect;
extern LPDDS ddsDice;
extern LPDDS ddsSMenu;
extern LPDDS ddsMenus;
extern LPDDS ddsPlrType;

extern LPDDS ddsWnd;
extern LPDDS ddsMsg;
extern LPDDS ddsMenuBar;
extern LPDDS ddsNames;
extern LPDDS ddsTeams;
extern LPDDS ddsChat;
extern LPDDS ddsDiceBar;

// DirectSound buffers ///////////////////////////////////////////////////
extern LPDSB dsbCurWithin;
extern LPDSB dsbDefault;
extern LPDSB dsbExplode;
extern LPDSB dsbFree;
extern LPDSB dsbCant;
extern LPDSB dsbSelect;
extern LPDSB dsbSink;
extern LPDSB dsbRise;

// fonts /////////////////////////////////////////////////////////////////
extern FNT gFntTitle;
extern FNT gFntNames;
extern FNT gFntSmall;
extern FNT gFntWndTxt;
extern FNT gFntWndTxtHil;
extern FNT gFntWndTxtDim;

// game varables /////////////////////////////////////////////////////////
extern char  gsWinText[128];
extern DWORD gThisTick;
extern UCHAR prog_state;
extern UCHAR game_state;
extern UCHAR game_nxtst;
extern UCHAR game_psest;

extern double gFrame;
extern double gFrameVel;
extern DWORD  gTimerStart;

extern UCHAR gWinner;
extern UCHAR gnGames;
extern UCHAR gnSinkHoles;
extern UCHAR gnTeamPlrs[2];
extern UCHAR gnTeamPlrsLeft[2];
extern UCHAR gTeams[2][3];
extern bool  gMoveDataValid;
extern BYTE  gMoveData[8];
extern BYTE  gNetData[2];
extern char  gBoard[16][16];

// player data ///////////////////////////////////////////////////////////
extern PDAT  pdat[4];
extern UCHAR gThisPlr;
extern UCHAR nplrs;
extern UCHAR pcur;
extern UCHAR gnPlrsReady;

// animating object data /////////////////////////////////////////////////
extern char   gAnimateX;
extern char   gAnimateY;
extern char   gMoveDstVal;

// flying object data ////////////////////////////////////////////////////
extern double gObjFrame;
extern double gObjWScale;
extern double gObjHScale;
extern DWORD  gObjLTick;
extern char   gObjValue;
extern long   gObjX;
extern long   gObjY;
extern char   gObjSrcX;
extern char   gObjSrcY;
extern char   gObjDstX;
extern char   gObjDstY;


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// main game loops ///////////////////////////////////////////////////////
void GameUpdate(void);
void GameRender(void);

// delegated game updating functions /////////////////////////////////////
void GameRequestNew(void);
void GameConnect(void);
void GameStartNet(void);
void GameEndShowMarbles(void);
void GameSetDice(void);
void GameRoll(void);
void GameGetInput(void);
void GameAttemptMove(void);
long GameUpdateArc(long, long);
void GameEndMarbleMove(void);
void GameNextPlayer(void);
void GameChangePlayerId(void);
void GameRoundOver(void);
void GameOver(void);
void GameUpdateGlow(void);

// delegated game rendering functions ////////////////////////////////////
void GameDrawMoves(void);
void GameDrawMarbleSink(char, long, long);
void GameDrawMarbleExplode(char, long, long);
void GameDrawSelections(void);
void GameDrawMarbleJump(void);

// move functions ////////////////////////////////////////////////////////
long GameCanMove(void);
void GameMoveMarbleEx(UCHAR, UCHAR, UCHAR, UCHAR);
void GameMoveMarble(UCHAR, UCHAR, UCHAR, UCHAR);
void GameMoveMarbleCashe(void);
void GameMoveMarbleMinimized(UCHAR, UCHAR, UCHAR, UCHAR);
void GameChangePlrMinimized(void);
long GameCheckForHit(UCHAR, UCHAR, UCHAR, UCHAR, char*, char*);
void GameSelectMoves(void);
long GameIsMove(char, char);
void GameUnselect(void);

// network functions /////////////////////////////////////////////////////
void GameNetMsg(BYTE* pMsg, DPID from);
void GameSendNewGame(void);
void GameSendMove(void);
void GameSendChat(void);
void GameNetPlrQuit(DPID);

// "wrap up" functions ///////////////////////////////////////////////////
void GameEndTurn(void);
long GameIsOver(void);
long GameCheckPlayers(void);
void GameGetWinText(void);

// game initilization ////////////////////////////////////////////////////
void GameClearForNew(void);
void GameHideBars(void);
void GameInitNewGame(void);
void GameInitBoard(void);
void GameSetMove(void);

// marble functions //////////////////////////////////////////////////////
void MarbleExplode(char, char);
void MarbleSink(char, char);
void MarbleGetFreePos(void);
void MarbleCreateFree(void);
long MarbleIsEnemy(char, char);
long MarbleX(long);
long MarbleY(long);

// pause functions ///////////////////////////////////////////////////////
void GamePause(void);
void GameResume(void);

// splash screen code ////////////////////////////////////////////////////
long SplashRender(long);

// "core root" functions /////////////////////////////////////////////////
void ProgMenuLoad(void);
void ProgMenuSaveAs(void);
void ProgMenuMusic(bool);
void ProgMenu(long, bool);
void ProgLoop(void);
long ProgInit(void);
long ProgRestoreSurfaces(void);
long ProgRestoreSound(void);
void ProgClose(void);

#endif // #ifndef GAME_HEADER_INCLUDED