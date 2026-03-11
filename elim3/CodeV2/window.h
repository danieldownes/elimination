/************************************************************************/
// window.h 
//
// Author: Justin Hoffman
// Date:   6/9/00 - 7/18/2000
/************************************************************************/

#include "user.h"

//————————————————————————————————————————————————————————————————————————
// Structures
//————————————————————————————————————————————————————————————————————————

// data structure for storing player info ////////////////////////////////
struct PDATB {
   DPID  dpId;
   bool  active;
   bool  ready;
   char  sName[16];
   UCHAR team;
};


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// generic message box ///////////////////////////////////////////////////
long MsgBox(char*, char*, char*, char*);
long WndMsgBox(WND*, UCHAR, UCHAR);

// start a new game //////////////////////////////////////////////////////
extern UCHAR WndNewGamePlrsOn[4];
extern UCHAR WndNewGamePlrsType[4];
extern UCHAR WndNewGamePlrsTeam[4];
long WndNewGame(WND*, UCHAR, UCHAR);
void WndNewGameDrawPlayerInfo(void);
void WndNewGameUpdate(void);
long WndNewGameValidateData(void);
void WndNewGameTranslateData(WND*);

// connect to a network game /////////////////////////////////////////////
long WndConnect(WND*, UCHAR, UCHAR);
long WndConnectPasswordKeys(TXT*);
void WndConnectDrawConnect(long);
void WndConnectPaste(WND*);
void WndConnectConnect(WND*);

// start network window ///////////////////////////////////////////////////
extern char  WndStartNetChatText[5][80];
extern UCHAR WndStartNetNPlrs;
extern bool  WndStartNetFirstGame;
extern bool  WndStartNetGotNames;
extern bool  WndStartNetShowWinner;
extern PDATB WndStartNetPlrs[4];
long WndStartNet(WND*, UCHAR, UCHAR);
void WndStartNetInit(WND*);
void WndStartNetUpdate(WND*);
void WndStartNetDraw(void);
void WndStartNetDrawChat(void);
void WndStartNetDrawSend(void);
void WndStartNetCopy(void);
void WndStartNetSendChat(void);
long WndStartNetValidateTeams(void);
void WndStartNetDrawPlrs(void);
void WndStartNetAddText(UCHAR, char*);
void WndStartNetReadyPlr(UCHAR, bool);
void WndStartNetChangeTeam(UCHAR, UCHAR);
void WndStartNetReList(void);
void WndStartNetStart(void);

// load a game ///////////////////////////////////////////////////////////
long WndLoad(WND*, UCHAR, UCHAR);
void WndLoadDraw(void);
void WndLoadDrawGames(WND*);
void WndLoadDeleteFile(WND*, long);

// save a game ///////////////////////////////////////////////////////////
long WndSaveAs(WND*, UCHAR, UCHAR);

// help window ///////////////////////////////////////////////////////////
void Help(char);
long WndHelp(WND*, UCHAR, UCHAR);
void WndHelpLoadTopic(char);

// music options window //////////////////////////////////////////////////
long WndMusicOptions(WND*, UCHAR, UCHAR);
void WndMusicOptionsCustomBtn(WND*, UCHAR);
void WndMusicOptionsUpdateList(WND*);
void WndMusicOptionsDrawList(WND*);
void WndMusicOptionsLoadList(void);