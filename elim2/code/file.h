/*******************************************************************************/
// file.h - Function Prototypes For file.cpp 
//
// Written By : Justin Hoffman
// Date       : July 15, 1999 -
/*******************************************************************************/

#ifndef FILE_HEADER_INCLUDED
#define FILE_HEADER_INCLUDED

#include "window.h"

/*-----------------------------------------------------------------------------*/
// Function Prototypes
/*-----------------------------------------------------------------------------*/

// Settings I/O //////////////////////////////////////////////////////////
void IO_Settings(int);
void IO_Registered(int);

// Saved Games I/O ///////////////////////////////////////////////////////
void IO_NewGameFile(void);
BOOL IO_GameNames(char[10][32], int);
BOOL IO_LoadGameStats(EDAT_LOADGAME*);
BOOL IO_Game(DWORD, int);

// Main file I/O functions ///////////////////////////////////////////////
BOOL IO_OpenFileWrite(LPSTR, HANDLE*, DWORD, DWORD);
BOOL IO_OpenFileRead(LPSTR, HANDLE*, DWORD, DWORD);
void IO_ReadWrite(HANDLE*, LPVOID, DWORD, int);


#ifdef ELIM_DEBUG

// Debug I/O functions ///////////////////////////////////////////////////
void IO_DebugOpen(void);
void IO_DebugWrite(char* event, DWORD ptr);
void IO_DebugClose(void);

#endif

#endif // ifndef FILE_HEADER_INVCLUDED