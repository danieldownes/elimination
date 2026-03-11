/************************************************************************/
// window.h 
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

#ifndef WINDOW_HEADER_INCLUDED
#define WINDOW_HEADER_INCLUDED

#include "user.h"

//————————————————————————————————————————————————————————————————————————
// Data Structure for storing player info
//————————————————————————————————————————————————————————————————————————

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

// new game window ///////////////////////////////////////////////////////
long WndNewGame(WND*, UCHAR, UCHAR);

// message box window ////////////////////////////////////////////////////
long MsgBox(char*, char*, char*, char*);
long WndMsgBox(WND*, UCHAR, UCHAR);

// join network window ///////////////////////////////////////////////////
long WndJoinNet(WND*, UCHAR, UCHAR);
void WndJoinNetDrawConnect(long);
void WndJoinNetPaste(WND*);

// start network window ///////////////////////////////////////////////////
extern long  WndStartNetNPlrs;
extern long  WndStartNetNewGame;
extern PDATB WndStartNetPlrs[4];
long WndStartNet(WND*, UCHAR, UCHAR);
void WndStartNetDraw(WND *wnd);
void WndStartNetDrawChat(void);
void WndStartNetDrawSend(void);
void WndStartNetCopy(void);
void WndStartNetSendChat(void);
void WndStartNetDrawPlrs(void);
void WndStartNetAddText(UCHAR, char*);
void WndStartNetReadyPlr(UCHAR, bool);
void WndStartNetReList(void);
void WndStartNetStart(void);


#endif // #ifndef WINDOW_HEADER_INCLUDED