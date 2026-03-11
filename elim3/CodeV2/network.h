/************************************************************************/
// network.h - Header for network.cpp
//
// Author: Justin Hoffman
// Date:   3/28/00 - 7/18/2000
/************************************************************************/

#ifndef NETWORK_HEADER_INCLUDED
#define NETWORK_HEADER_INCLUDED


//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// application messages //////////////////////////////////////////////////
#define PROG_CHECKDPMSG   (WM_USER + 100)

// standard session name /////////////////////////////////////////////////
#define ELIM_SESSION      "ELIM3"

// game messages /////////////////////////////////////////////////////////
#define GAMEMSG_WAVE       0
#define GAMEMSG_WAVEALL    1
#define GAMEMSG_BEGINTEST  2
#define GAMEMSG_TEST       3
#define GAMEMSG_ENDTEST    4


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// misc varables /////////////////////////////////////////////////////////
extern bool  dpHost;
extern bool  dpUseLan;
extern DPID  dpThisPlrId;
extern long  dpCurEnumPlr;
extern DWORD dpError;
extern char  dpThisPlrIp[32];
extern char  dpSessionName[16];
extern char  dpThisPlrName[16];
extern char  dpPassword[16];


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// game interface functions //////////////////////////////////////////////
bool DpConnect(char*);
bool DpLock(bool lock);
bool DpSend(BYTE*, DWORD);
bool DpQuit(void);
void DpCheckMsg(void);

// connection functions //////////////////////////////////////////////////
void DpCleanUp(void);
bool DpConnectInit(char*);
bool DpHost(void);
bool DpJoin(void);
bool DpCreatePlayer(void);
bool DpListPlayers(void);

// message management functions //////////////////////////////////////////
void DpSytmMsg(DPMSG_GENERIC*, DWORD, DPID, DPID);
DWORD WINAPI DpThread(void*);

// enumeration callbacks /////////////////////////////////////////////////
BOOL FAR PASCAL DpEnumSessions(const DPSESSIONDESC2*, DWORD*, DWORD, VOID*);
BOOL FAR PASCAL DpEnumAddress(REFGUID, DWORD, LPCVOID, LPVOID);
BOOL FAR PASCAL DpEnumPlayers(DPID, DWORD, LPCDPNAME, DWORD, LPVOID);


#endif // #ifndef NETWORK_HEADER_INCLUDED