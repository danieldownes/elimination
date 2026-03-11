/************************************************************************/
// main.cpp - Test DirectPlay network programming
//
// Author: Justin Hoffman
// Date:   3/26/00 - 
/************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "game.h"
#include "network.h"
#include "window.h"


/*——————————————————————————————————————————————————————————————————————*/
// Global Varables
/*——————————————————————————————————————————————————————————————————————*/

// Applicatoin GUID //////////////////////////////////////////////////////
static GUID ELIM_GUID = {0xc6334fc0, 0x3b80, 0x4fed, {0x89, 0xf1, 0xa4, 0xde, 0xfe, 0xb6, 0xdb, 0x20}};

// direct play objects ///////////////////////////////////////////////////
static LPDIRECTPLAY4A     dpMain       = 0;
static LPDIRECTPLAYLOBBY3 dpLobby      = 0;
static HANDLE             dpEvent      = 0;
static HANDLE             dpThread     = 0;
static bool               dpConnected  = 0;
long                      dpCurEnumPlr = 0;
bool                      dpHost       = 0;
DPID                      dpThisPlrId  = 0;
char                      dpThisPlrIp[32]   = "0.0.0.0";
char                      dpSessionName[32] = "";
char                      dpThisPlrName[32] = "";


//————————————————————————————————————————————————————————————————————————
// Game Interface Functions
//————————————————————————————————————————————————————————————————————————

// master connection function ////////////////////////////////////////////
bool DpConnect(char *hostIp)
{
   DWORD threadId;

   // clean up direct old play stuff
   DpCleanUp();

   // get DirectPlayLobby interface
   if(CoCreateInstance(CLSID_DirectPlayLobby, NULL, CLSCTX_INPROC_SERVER,
      IID_IDirectPlayLobby3A, (VOID**)&dpLobby) != DP_OK) return 0;
   
   // create DirectPlay4 interface
   if(CoCreateInstance(CLSID_DirectPlay, 0, CLSCTX_INPROC_SERVER,
      IID_IDirectPlay4A, (VOID**)&dpMain) != DP_OK) return 0;

   // create event for signaling a message was sent
   if(!(dpEvent = CreateEvent(0, 0, 0, 0))) return 0;

   // create thread for checking for messages
   if(!(dpThread = CreateThread(0, 10240, DpThread, 0, 0, &threadId))) return 0;

   // initilize connection
   if(!DpConnectInit(hostIp)) return 0;

   // host or join a game
   if(dpHost) { if(!DpHost()) return 0; } 
   else       { if(!DpJoin()) return 0; }

   // create a new player in the session
   if(!DpCreatePlayer()) return 0;

   // list all players currently in the session
   if(!DpListPlayers()) return 0;

   // clean up direct play lobby interface
   DXOBJ_DELETE(dpLobby);
   return 1;
}

// lock the DirectPlay session ///////////////////////////////////////////
bool DpLock(bool lock)
{
   DPSESSIONDESC2 sdesc;

   // setup the session description
   STRUCT_INIT(sdesc);
   sdesc.dwFlags          = DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
   sdesc.lpszSessionNameA = dpSessionName;
   sdesc.dwMaxPlayers     = 4;

   // add DISABLEJOIN flags if necessary
   if(lock) sdesc.dwFlags |= DPSESSION_JOINDISABLED;

   // set the session description
   return (dpMain->SetSessionDesc(&sdesc, 0) == DP_OK);
}

// send data between players /////////////////////////////////////////////
bool DpSend(BYTE *pdata, DWORD size)
{
   if(dpMain) {

      // send the message to all users
      switch(dpMain->Send(dpThisPlrId, DPID_ALLPLAYERS, DPSEND_GUARANTEED, pdata, size)) {
      case DPERR_BUSY:           DebugWrite("FAILED,dpMain->Send(),DPERR_BUSY");           break;
      case DPERR_CONNECTIONLOST: DebugWrite("FAILED,dpMain->Send(),DPERR_CONNECTIONLOST"); break;
      case DPERR_INVALIDPARAMS:  DebugWrite("FAILED,dpMain->Send(),DPERR_INVALIDPARAMS");  break;
      case DPERR_INVALIDPLAYER:  DebugWrite("FAILED,dpMain->Send(),DPERR_INVALIDPLAYER");  break;
      case DPERR_NOTLOGGEDIN:    DebugWrite("FAILED,dpMain->Send(),DPERR_NOTLOGGEDIN");    break;
      case DPERR_SENDTOOBIG:     DebugWrite("FAILED,dpMain->Send(),DPERR_SENDTOOBIG");     break;
      }
   }

   return 1;
}

// exit the network game /////////////////////////////////////////////////
bool DpQuit(void)
{
   // remove local player from session & clean up direct play objects
   if(dpMain) dpMain->DestroyPlayer(dpThisPlrId);
   DpCleanUp();
   return 1;
}

// check for messages from other players /////////////////////////////////
void DpCheckMsg(void)
{
   DPID    from     = 0;
   DPID    to       = 0;
   LPVOID  pBuff    = 0;
   DWORD   buffSize = 0;
   HRESULT hr;
    
   // figure out how much space is needed to store the incomming data
   hr = dpMain->Receive(&from, &to, DPRECEIVE_PEEK, 0, &buffSize);

   // make sure there were even any messages to worry about
   if(hr == DPERR_NOMESSAGES) return;
   
   // error check, the buffer is supposed to
   // be too small for this particular call
   if(hr != DPERR_BUFFERTOOSMALL) return;

   // allocate space for the data
   pBuff = new BYTE[buffSize];

   // get the data
   if((hr = dpMain->Receive(&from, &to, 0, pBuff, &buffSize)) != DP_OK) {
      delete [] pBuff;
      return;
   }

   // process system messages
   if(from == DPID_SYSMSG) DpSytmMsg((DPMSG_GENERIC*)pBuff, buffSize, from, to);
   else                    DpGameMsg((BYTE*)pBuff, buffSize, from, to);

   // release the buffer
   delete [] pBuff;
}


//————————————————————————————————————————————————————————————————————————
// Connection Functions
//————————————————————————————————————————————————————————————————————————

// clean up objects used for direct play /////////////////////////////////
static void DpCleanUp(void)
{
   // clean up old direct play interface
   if(dpMain) { dpMain->Close(); dpMain->Release(); dpMain = 0; }

   // clean up old lobby interface
   DXOBJ_DELETE(dpLobby);

   // clean up thread & event
   TerminateThread(dpThread, 0);
   CloseHandle(dpEvent);
   CloseHandle(dpThread);
}

// initilize direct play address & connection ////////////////////////////
static bool DpConnectInit(char* hostIp)
{
   DPCOMPOUNDADDRESSELEMENT elements[2];
   LPVOID                   pAddress;
   DWORD                    addressSize;

   // copy standard name into session name
   strcpy(dpSessionName, ELIM_SESSION);

   // set TCP/IP service provider
   elements[0].guidDataType = DPAID_ServiceProvider;
   elements[0].dwDataSize   = sizeof(GUID);
   elements[0].lpData       = (VOID*)&DPSPGUID_TCPIP;

   // IP address string
   elements[1].guidDataType = DPAID_INet;
   elements[1].dwDataSize   = lstrlen(hostIp) + 1;
   elements[1].lpData       = hostIp;

   // see how much space is needed for address storage & allocate it
   if(dpLobby->CreateCompoundAddress(elements, 2, 0, &addressSize) != DPERR_BUFFERTOOSMALL) return 0;
   pAddress = new BYTE[addressSize]; 

   // create the address
   if(dpLobby->CreateCompoundAddress(elements, 2, pAddress, &addressSize) != DP_OK) {
      delete [] pAddress;
      return 0;
   }

   // initialize the connection using the address
   if(dpMain->InitializeConnection(pAddress, 0) != DP_OK) {
      delete [] pAddress;
      return 0;
   }

   return 1;
}

// host a game ///////////////////////////////////////////////////////////
static bool DpHost(void)
{
   DPSESSIONDESC2 sdesc;

   // setup session description
   STRUCT_INIT(sdesc);
   sdesc.dwFlags          = DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
   sdesc.lpszSessionNameA = dpSessionName;
   sdesc.guidApplication  = ELIM_GUID;
   sdesc.dwMaxPlayers     = 4;

   // create a new session
   if(dpMain->Open(&sdesc, DPOPEN_CREATE) != DP_OK) return 0;

   // set connected to true
   dpConnected = 1;
   return 1;
}

// join a game ///////////////////////////////////////////////////////////
static bool DpJoin(void)
{
   HRESULT        hr;
   DPSESSIONDESC2 sdesc;

   // enumerate sessions
   STRUCT_INIT(sdesc);
   sdesc.guidApplication = ELIM_GUID;

   while((hr = dpMain->EnumSessions(&sdesc, 0, DpEnumSessions, 0,
      DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_RETURNSTATUS)) == DPERR_CONNECTING) ;

   return hr == DP_OK;
}

// create a player in the direct play session ////////////////////////////
static bool DpCreatePlayer(void)
{
   DPNAME dpname;
   LPVOID pAddress;
   DWORD  addressSize;

   STRUCT_INIT(dpname);
   dpname.lpszShortNameA = dpThisPlrName;

   // create a new player in the session then determine how much
   // memory is needed to get the players address
   if(dpMain->CreatePlayer(&dpThisPlrId, &dpname, dpEvent, 0, 0, 0) != DP_OK) return 0;
   if(dpMain->GetPlayerAddress(dpThisPlrId, 0, &addressSize) != DPERR_BUFFERTOOSMALL) return 0;

   // allocate space for address
   pAddress = new BYTE[addressSize];

   // get actual address data, then enumerate address data
   if(dpMain->GetPlayerAddress(dpThisPlrId, pAddress, &addressSize) != DP_OK) return 0;
   if(dpLobby->EnumAddress(DpEnumAddress, pAddress, addressSize, 0) != DP_OK) return 0;
   return 1;
}

// list all of the players currently in the session //////////////////////
bool DpListPlayers(void)
{
   int i;
  
   // set current enumerated player to zero
   dpCurEnumPlr = 0;

   // clear the names from name structure
   for(i = 0; i < 4; i++) {
      WndStartNetPlrs[i].ready  = 0;
      WndStartNetPlrs[i].active = 0;
   }

   // enumerate the players
   if(dpMain->EnumPlayers(0, DpEnumPlayers, 0, DPENUMPLAYERS_ALL) != DP_OK) return 0;
   return 1;
}


/*——————————————————————————————————————————————————————————————————————*/
// Message Management functions
/*——————————————————————————————————————————————————————————————————————*/

// process game messages /////////////////////////////////////////////////
static void DpGameMsg(BYTE* pMsg, DWORD msgSize, DPID from, DPID to)
{
   long p;
   long i;

   // determine which players sent the message
   for(i = 0; i < 4; i++) if(from == pdat[i].dpId) p = i;

   // figure out what message was sent
   switch(pMsg[0]) {

   // start the game
   case NETMSG_STARTGAME:
      memcpy(g_board, pMsg + 1, sizeof(g_board));
      gMoveData[6] = pMsg[sizeof(g_board) + 2];
      gMoveData[7] = pMsg[sizeof(g_board) + 3];
      pcur         = pMsg[sizeof(g_board) + 4];
      gnGames      = pMsg[sizeof(g_board) + 5];

      if(game_state == GAME_WAITNEWNET) {
         GameInitNew();
      } else {
         WndStartNetStart();
         WndClose(1);
      }
      break;

   // send chat into window
   case NETMSG_CHATA:
      WndStartNetAddText((UCHAR)p, (char*)(pMsg + 1));
      break;

   // send chat into game
   case NETMSG_CHATB:
      ChatAddText((UCHAR)p, (char*)(pMsg + 1));
      break;

   // move a marble
   case NETMSG_MOVEMARBLE:
      gMoveDataValid = 1;
      memcpy(gMoveData, pMsg + 1, sizeof(gMoveData));
      break;

   // re-list the players
   case NETMSG_RELISTPLRS:
      memcpy(WndStartNetPlrs, pMsg + 1, sizeof(WndStartNetPlrs));
      WndStartNetNPlrs = 0;

      for(i = 0; i < 4; i++) {
         if(WndStartNetPlrs[i].active) WndStartNetNPlrs++;
         if(WndStartNetPlrs[i].dpId == dpThisPlrId) gThisPlr = i;
      }

      // re-draw players & remove check from ready button
      WndStartNetDrawPlrs();
      BREM(gWndA.pbtns[2].flags, BTN_TDOWN);
      BtnDraw(&gWndA.pbtns[2]);
      break;

   // change the ready state of a player
   case NETMSG_READYPLR:
      WndStartNetReadyPlr((UCHAR)p, pMsg[1]? 1 : 0);
      break;

   // change the number of sinkholes in a game
   case NETMSG_CHANGESINK:
      if(game_flags & GAME_MSGBOX) {
         gWndA.pnums[0].value = pMsg[1];
         NumDraw(&gWndA.pnums[0]);
      }
      break;

   // change the number of rounds in a game
   case NETMSG_CHANGEGAMES:
      if(game_flags & GAME_MSGBOX) {
         gWndA.pnums[1].value = pMsg[1];
         NumDraw(&gWndA.pnums[1]);
      }
      break;

   // a player is signaling they are ready for the next game
   case NETMSG_READYFORNEW:
      if(dpHost) gnPlrsReady++;
      break;
   }
}

// process system messages ///////////////////////////////////////////////
static void DpSytmMsg(DPMSG_GENERIC *pMsg, DWORD msgSize, DPID from, DPID to)
{
   switch(pMsg->dwType) {

   // this message is sent to a player who will now dawn hosthood
   case DPSYS_HOST:

      // set this player as host
      dpHost = 1;

      // if the game window is still present, re-initilze it
      if(game_flags & GAME_MSGBOX) {
         gWndA.loop(&gWndA, WND_INIT, 0);
         if(prog_state != PROG_SUSPENDED) WndDraw(&gWndA);
      }
      break;

   // this message is sent to all players when the session gets messed up
   case DPSYS_SESSIONLOST:
      break;

   // a player has been destroyed
   // no break statement for fall through
   case DPSYS_DESTROYPLAYERORGROUP:
      if(game_state != GAME_IDLE) {
         DPMSG_DESTROYPLAYERORGROUP *info = (DPMSG_DESTROYPLAYERORGROUP*)pMsg;
         GameKillNetPlr(info->dpId);
      }

   // a player has been created
   case DPSYS_CREATEPLAYERORGROUP:
      if((game_flags & GAME_MSGBOX) && dpHost) WndStartNetReList();
      break;
   }
}

// thread for detecting if messages are present //////////////////////////
static DWORD WINAPI DpThread(void*)
{
   while(1) {
      if(WaitForSingleObject(dpEvent, INFINITE) != WAIT_OBJECT_0) return 0;
      PostMessage(gHWnd, PROG_CHECKDPMSG, 0, 0);
   }

   return 1;
}

//————————————————————————————————————————————————————————————————————————
// Enumeration Callbacks
//————————————————————————————————————————————————————————————————————————

// enumerate direct play sessions on remote computer /////////////////////
static BOOL FAR PASCAL DpEnumSessions(const DPSESSIONDESC2 *psdesc,
   DWORD *pdwTimeOut, DWORD dwFlags, LPVOID pContext)
{
   DPSESSIONDESC2 sdesc;

   // if we are already connected, dont bother checking
   if(dpConnected) return 0;

   // make sure enumeration has not timed out
   if(dwFlags & DPESC_TIMEDOUT) {
      return 0;
   }

   // make sure session has correct name
   if(strcmp(psdesc->lpszSessionNameA, dpSessionName) != 0) return 1;

   // setup session description
   STRUCT_INIT(sdesc);
   sdesc.guidApplication = ELIM_GUID;
   sdesc.guidInstance    = psdesc->guidInstance;

   // open session
   if(dpMain->Open(&sdesc, DPOPEN_JOIN) != DP_OK) {
      return 0;
   }

   dpConnected = 1;
   return 1;
}

// enumerate address of player ///////////////////////////////////////////
static BOOL FAR PASCAL DpEnumAddress(REFGUID guidDataType,
   DWORD dataSize, LPCVOID pData, LPVOID pContext)
{
   // if we are receiving the DPAID_INet type, it is
   // the local players IP, save it to a string varable
   if(IsEqualGUID(DPAID_INet, guidDataType)) {
      strcpy(dpThisPlrIp, "");
      memcpy(dpThisPlrIp, pData, dataSize);
   }

   return 1;
}

// enumerate players in a session ////////////////////////////////////////
static BOOL FAR PASCAL DpEnumPlayers(DPID dpId, DWORD playerType,
   LPCDPNAME pName, DWORD flags, LPVOID pContext)
{
   long x;

   switch(dpCurEnumPlr) {
   case 0: x = 0; break;
   case 1: x = 2; break;
   case 2: x = 1; break;
   case 3: x = 4; break;
   }

   // get player id & name
   WndStartNetPlrs[x].active = 1;
   WndStartNetPlrs[x].ready  = 0;
   WndStartNetPlrs[x].dpId   = dpId;
   pdat[x].dpId              = dpId;
   strcpy(WndStartNetPlrs[x].sName, pName->lpszShortNameA);

   // reset host's id if needed
   if(dpId == dpThisPlrId) {
      gThisPlr                 = x;
      WndStartNetPlrs[x].ready = 1;
   }

   // increment current enumerated player
   dpCurEnumPlr++;
   return 1;
}