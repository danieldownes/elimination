/************************************************************************/
// file.h 
//
// Author: Justin Hoffman
// Date:   6/29/2000 - 7/18/2000
/************************************************************************/

//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// file offsets //////////////////////////////////////////////////////////
#define FILE_OFFSET_NETINFO      78
#define FILE_OFFSET_SOUNDON      111
#define FILE_OFFSET_SONGINFO     114

// file names ////////////////////////////////////////////////////////////
extern char *FILE_SETTINGS;
extern char *FILE_SAVEDGAME;
extern char  FILE_LOADED[];


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// main settings file functions //////////////////////////////////////////
void IoSettingsCreateDefault(void);
void IoSettingsNewGameInfo(bool);
void IoSettingsNetInfo(bool);
void IoSettingsSoundOn(bool);
void IoSettingsSongInfo(bool);

// saved game I/O ////////////////////////////////////////////////////////
char *IoGameGetInfo(long*);
void  IoGameGetFileInfo(long, WIN32_FIND_DATA*, char*);
void  IoGame(char*, bool);

// I/O function wrappers /////////////////////////////////////////////////
bool IoFileExists(char*);
int  IoOpenFile(bool, LPSTR);
void IoReadWrite(void*, UINT);
void IoClose(void);