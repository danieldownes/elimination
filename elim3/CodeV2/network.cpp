/************************************************************************/
// main.cpp - DirectPlay network programming
//
// Author: Justin Hoffman
// Date:   3/26/00 - 7/18/2000
/************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "game.h"
#include "network.h"
#include "window.h"


/*——————————————————————————————————————————————————————————————————————*/
// Global Varables
/*——————————————————————————————————————————————————————————————————————*/

// Applicatoin GUID {5FF339A3-58FE-11d4-986F-8A990E78F40F} ///////////////
static GUID ELIM_GUID = {0x5ff339a3, 0x58fe, 0x11d4, {0x98, 0x6f, 0x8a, 0x99, 0xe, 0x78, 0xf4, 0xf}};


// direct play objects ///////////////////////////////////////////////////
static LPDIRECTPLAY4A     dpMain       = 0;
static LPDIRECTPLAYLOBBY3 dpLobby      = 0;
static HANDLE             dpThread     = 0;
static HANDLE             dpEvents[2]  = {0, 0};
static bool               dpConnected  = 0;
long                      dpCurEnumPlr = 0;
bool                      dpHost       = 0;
bool                      dpUseLan     = 0;
DPID                      dpThisPlrId  = 0;
DWORD                     dpError      = 0;
char                      dpThisPlrIp[32]   = "0.0.0.0";
char                      dpSessionName[16] = "";
char                      dpThisPlrName[16] = "";
char                      dpPassword[16]    = "";


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
   if(!(dpEvents[0] = CreateEvent(0, 0, 0, 0))) return 0;

   // create a kill event
   if(!(dpEvents[1] = CreateEvent(0, 0, 0, 0))) return 0;

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
#ifdef GAME_DEBUG
      switch(dpMain->Send(dpThisPlrId, DPID_ALLPLAYERS, DPSEND_GUARANTEED, pdata, size)) {
      case DPERR_BUSY:           DebugWrite("FAILED,dpMain->Send(),DPERR_BUSY");           break;
      case DPERR_CONNECTIONLOST: DebugWrite("FAILED,dpMain->Send(),DPERR_CONNECTIONLOST"); break;
      case DPERR_INVALIDPARAMS:  DebugWrite("FAILED,dpMain->Send(),DPERR_INVALIDPARAMS");  break;
      case DPERR_INVALIDPLAYER:  DebugWrite("FAILED,dpMain->Send(),DPERR_INVALIDPLAYER");  break;
      case DPERR_NOTLOGGEDIN:    DebugWrite("FAILED,dpMain->Send(),DPERR_NOTLOGGEDIN");    break;
      case DPERR_SENDTOOBIG:     DebugWrite("FAILED,dpMain->Send(),DPERR_SENDTOOBIG");     break;
      }
#else
      dpMain->Send(dpThisPlrId, DPID_ALLPLAYERS, DPSEND_GUARANTEED, pdata, size);
#endif
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
   pBuff = MoreMem(buffSize);

   // get the data
   if((hr = dpMain->Receive(&from, &to, 0, pBuff, &buffSize)) != DP_OK) {
      LessMem(pBuff);
      return;
   }

   // process system messages
   if(from == DPID_SYSMSG) DpSytmMsg((DPMSG_GENERIC*)pBuff, buffSize, from, to);
   else                    GameNetMsg((BYTE*)pBuff, from);

   // release the buffer
   LessMem(pBuff);
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

   // close the thread & wait for it to quit
   SetEvent(dpEvents[1]);
   WaitForSingleObject(dpThread, INFINITE);
   CloseHandle(dpThread);

   // close the handles
   CloseHandle(dpEvents[0]);
   CloseHandle(dpEvents[1]);
}

// initilize direct play address & connection ////////////////////////////
static bool DpConnectInit(char* hostIp)
{
   DPCOMPOUNDADDRESSELEMENT elements[2];
   LPVOID                   pAddress;
   DWORD                    addressSize;

   // set TCP/IP service provider
   elements[0].guidDataType = DPAID_ServiceProvider;
   elements[0].dwDataSize   = sizeof(GUID);
   elements[0].lpData       = (VOID*)&DPSPGUID_TCPIP;

   // IP address string
   elements[1].guidDataType = DPAID_INet;

   if(dpUseLan) {

      // setup address elements
      elements[1].dwDataSize   = 1;
      elements[1].lpData       = "";

      // copy custom session name
      lstrcpy(dpSessionName, hostIp);

   } else {

      // setup address elements
      elements[1].dwDataSize = lstrlen(hostIp) + 1;
      elements[1].lpData     = hostIp;
 
      // copy standard name into session name
      lstrcpy(dpSessionName, ELIM_SESSION);
   }

   // see how much space is needed for address storage & allocate it
   if(dpLobby->CreateCompoundAddress(elements, 2, 0, &addressSize) != DPERR_BUFFERTOOSMALL) return 0;
   pAddress = MoreMem(addressSize);

   // create the address
   if(dpLobby->CreateCompoundAddress(elements, 2, pAddress, &addressSize) != DP_OK) {
      LessMem(pAddress);
      return 0;
   }

   // initialize the connection using the address
   if(dpMain->InitializeConnection(pAddress, 0) != DP_OK) {
      LessMem(pAddress);
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
   sdesc.dwFlags          = DPSESSION_OPTIMIZELATENCY | DPSESSION_MIGRATEHOST | DPSESSION_KEEPALIVE;
   sdesc.lpszSessionNameA = dpSessionName;
   sdesc.guidApplication  = ELIM_GUID;
   sdesc.dwMaxPlayers     = 4;

   // set the password if user is using one
   if(dpPassword[0] != TXT_NULLCHAR) sdesc.lpszPasswordA = dpPassword;

   // create a new session
   if((dpError = dpMain->Open(&sdesc, DPOPEN_CREATE)) != DP_OK) return 0;
   return 1;
}

// join a game ///////////////////////////////////////////////////////////
static bool DpJoin(void)
{
   DPSESSIONDESC2 sdesc;

   // enumerate sessions
   STRUCT_INIT(sdesc);
   sdesc.guidApplication = ELIM_GUID;

   // set connected to false
   dpConnected = 0;

   // enumerate sessions
   while((dpError = dpMain->EnumSessions(&sdesc, 0, DpEnumSessions, 0,
      DPENUMSESSIONS_AVAILABLE | DPENUMSESSIONS_PASSWORDREQUIRED |
      DPENUMSESSIONS_RETURNSTATUS)) == DPERR_CONNECTING) ;

#ifdef GAME_DEBUG
   switch(dpError) {
   case DPERR_CONNECTING:     DebugWrite("EnumSessions: DPERR_CONNECTING");     break;
   case DPERR_CONNECTIONLOST: DebugWrite("EnumSessions: DPERR_CONNECTIONLOST"); break;
   case DPERR_EXCEPTION:      DebugWrite("EnumSessions: DPERR_EXCEPTION");      break;
   case DPERR_GENERIC:        DebugWrite("EnumSessions: DPERR_GENERIC");        break;
   case DPERR_INVALIDOBJECT:  DebugWrite("EnumSessions: DPERR_INVALIDOBJECT");  break;
   case DPERR_INVALIDPARAMS:  DebugWrite("EnumSessions: DPERR_INVALIDPARAMS");  break;
   case DPERR_NOCONNECTION:   DebugWrite("EnumSessions: DPERR_NOCONNECTION");   break;
   case DPERR_UNINITIALIZED:  DebugWrite("EnumSessions: DPERR_UNINITIALIZED");  break;
   case DPERR_USERCANCEL:     DebugWrite("EnumSessions: DPERR_USERCANCEL");     break;
   }
#endif

   return dpError == DP_OK;
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
   if(dpMain->CreatePlayer(&dpThisPlrId, &dpname, dpEvents[0], 0, 0, 0) != DP_OK) return 0;
   if(dpMain->GetPlayerAddress(dpThisPlrId, 0, &addressSize) != DPERR_BUFFERTOOSMALL) return 0;

   // allocate space for address
   pAddress = MoreMem(addressSize);

   // get actual address data, then enumerate address data
   if(dpMain->GetPlayerAddress(dpThisPlrId, pAddress, &addressSize) != DP_OK) return 0;
   if(dpLobby->EnumAddress(DpEnumAddress, pAddress, addressSize, 0) != DP_OK) return 0;
   return 1;
}

// list all of the players currently in the session //////////////////////
bool DpListPlayers(void)
{
   // set current enumerated player to zero
   dpCurEnumPlr = 0;

   // clear the players data
   memset(WndStartNetPlrs, 0, sizeof(WndStartNetPlrs));
   pdat[0].dpId = 0;
   pdat[1].dpId = 0;
   pdat[2].dpId = 0;
   pdat[3].dpId = 0;

   // enumerate the players
   if(dpMain->EnumPlayers(0, DpEnumPlayers, 0, DPENUMPLAYERS_ALL) != DP_OK) return 0;
   return 1;
}


//————————————————————————————————————————————————————————————————————————
// Message Management functions
//————————————————————————————————————————————————————————————————————————

// process system messages ///////////////////////////////////////////////
static void DpSytmMsg(DPMSG_GENERIC *pMsg, DWORD msgSize, DPID from, DPID to)
{
   switch(pMsg->dwType) {

   // this message is sent to a player who will now dawn hosthood
   case DPSYS_HOST:

      // set this player as host
      dpHost = 1;

      // if the game window is still present, re-initilze it
      if(gbMsgBox) {
         gWndA.loop(&gWndA, WND_INIT, 0);
         if(gbVisible) WndDraw(&gWndA);
      }
      break;

   // this message is sent to all players
   // when the session gets dropped
#ifdef GAME_DEBUG
   case DPSYS_SESSIONLOST:
      DebugWrite("DPSYS_SESSIONLOST received");
      break;
#endif

   // a player has been destroyed
   // no break statement for fall through
   case DPSYS_DESTROYPLAYERORGROUP:
      DPMSG_DESTROYPLAYERORGROUP *info;
      info = (DPMSG_DESTROYPLAYERORGROUP*)pMsg;
      GameNetPlrQuit(info->dpId);

   // a player has been created
   case DPSYS_CREATEPLAYERORGROUP:
      if(gbMsgBox && dpHost) WndStartNetReList();
      break;
   }
}

// thread for detecting if messages are present //////////////////////////
static DWORD WINAPI DpThread(void*)
{   
   // keep the thread going until the kill switch is used
   while(WaitForMultipleObjects(2, dpEvents, 0, INFINITE) == WAIT_OBJECT_0)
      PostMessage(gHWnd, PROG_CHECKDPMSG, 0, 0);

   // exit the thread
   ExitThread(0);
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
   // then make sure enumeration has not timed out
   if(dpConnected || dwFlags & DPESC_TIMEDOUT) {
      dpError = DPERR_TIMEOUT;
      return 0;
   }

   // make sure session has correct name
   if(lstrcmp(psdesc->lpszSessionNameA, dpSessionName) != 0) return 1;

   // setup session description
   STRUCT_INIT(sdesc);
   sdesc.guidApplication = ELIM_GUID;
   sdesc.guidInstance    = psdesc->guidInstance;

   // set the password if user is using one
   if(dpPassword[0] != TXT_NULLCHAR) sdesc.lpszPasswordA = dpPassword;

   // open session
   while((dpError = dpMain->Open(&sdesc, DPOPEN_JOIN | DPOPEN_RETURNSTATUS)) == DPERR_CONNECTING) ;

   // return weather we were really connected
   dpConnected = dpError == DP_OK;
   return dpConnected;
}

// enumerate address of player ///////////////////////////////////////////
static BOOL FAR PASCAL DpEnumAddress(REFGUID guidDataType,
   DWORD dataSize, LPCVOID pData, LPVOID pContext)
{
   // if we are receiving the DPAID_INet type, it is
   // the local players IP, save it to a string varable
   if(IsEqualGUID(DPAID_INet, guidDataType)) {
      dpThisPlrIp[0] = TXT_NULLCHAR;
      memcpy(dpThisPlrIp, pData, dataSize);
   }

   return 1;
}

// enumerate players in a session ////////////////////////////////////////
static BOOL FAR PASCAL DpEnumPlayers(DPID dpId, DWORD playerType,
   LPCDPNAME pName, DWORD flags, LPVOID pContext)
{
   UCHAR x;

   switch(dpCurEnumPlr) {
   case 0: x = 0; break;
   case 1: x = 2; break;
   case 2: x = 1; break;
   case 3: x = 3; break;
   }

   // get player id & name
   WndStartNetPlrs[x].active = 1;
   WndStartNetPlrs[x].ready  = 0;
   WndStartNetPlrs[x].dpId   = dpId;
   pdat[x].dpId              = dpId;
   lstrcpy(WndStartNetPlrs[x].sName, pName->lpszShortNameA);

   // reset host's id if needed
   if(dpId == dpThisPlrId) {
      gThisPlr                 = x;
      WndStartNetPlrs[x].ready = 1;
   }

   // increment current enumerated player
   dpCurEnumPlr++;
   return 1;
}