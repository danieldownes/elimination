/************************************************************************/
// utility.h 
//
// Author: Justin Hoffman
// Date:   5/1/00 - 7/18/2000
/************************************************************************/

#ifndef UTILITY_HEADER_INCLUDED
#define UTILITY_HEADER_INCLUDED


//————————————————————————————————————————————————————————————————————————
// Headers
//————————————————————————————————————————————————————————————————————————

// Header related defines  ///////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN          // supress unneeded stuff in windows.h
//#define GAME_DEBUG                   // set the program status to debug
#define DIRECTINPUT_VERSION   0x0300 // make sure we are compiling for DI 3.0

// Main windows headers //////////////////////////////////////////////////
#include <windows.h>
#include <windowsx.h>
#include <dbt.h>

// DirectX and multi-media ///////////////////////////////////////////////
#include <mmsystem.h>
#include <ddraw.h>
#include <dinput.h>
#include <dsound.h>
#include <dmusici.h>
#include <dplobby.h>
#include <dplay.h>


//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// 'Shortcut' definitions ////////////////////////////////////////////////
#define LPDDS              IDirectDrawSurface*
#define LPDDP              IDirectDrawPalette*
#define SRCCK              DDBLTFAST_SRCCOLORKEY
#define NOCK               DDBLTFAST_NOCOLORKEY

// Misc definitions //////////////////////////////////////////////////////
#define PROG_KILL          (WM_USER + 1)
#define PROG_CHECKMUSIC    (WM_USER + 2)
#define DEBUG_FILENAME     "debug.txt"

// DDRect flags //////////////////////////////////////////////////////////
#define DDRECT_NOFILL      0x01
#define DDRECT_NOBORDER    0x02


//————————————————————————————————————————————————————————————————————————
// Globals
//————————————————————————————————————————————————————————————————————————

// Main DirectDraw object ////////////////////////////////////////////////
extern LPDIRECTDRAW lpdd;

// Input globals /////////////////////////////////////////////////////////
extern UCHAR       *key_lfdwn;
extern UCHAR       *key_down;
extern UCHAR        cur_lfdwn;
extern UCHAR        cur_down;
extern long         cur_x;
extern long         cur_y;

// misc varables /////////////////////////////////////////////////////////
extern RECT         R_SCREEN;

// Application globals ///////////////////////////////////////////////////
extern char         gszAppPath[];
extern HWND         gHWnd;
extern HINSTANCE    gHInst;


//————————————————————————————————————————————————————————————————————————
// General Macros
//————————————————————————————————————————————————————————————————————————

// Find the absolute value of a number ///////////////////////////////////
#define ABS(n) ((n)>0? (n):0-(n))

// Generate a random number //////////////////////////////////////////////
#define RAND(n) (rand()%(n))

// Determine if the mouse has been clicked within the last frame /////////
#define CUR_CLICK (cur_down && !cur_lfdwn)

// Determine if a key has been clicked ///////////////////////////////////
#define KEY_CLICK(n) (key_down[n] && !key_lfdwn[n])

// Initilize DirectX structures //////////////////////////////////////////
#define STRUCT_INIT(n) {memset(&n, 0, sizeof(n)); n.dwSize=sizeof(n);}

// Delete a DirectX object ///////////////////////////////////////////////
#define DXOBJ_DELETE(n) if(n) {n->Release(); n = 0;}

// delete an allocated array /////////////////////////////////////////////
#define DELETE_ARRAY(n) if(n) {LessMem(n); n = 0;}

// Process DirectDraw return values //////////////////////////////////////
#define DDRVAL(f) if(f == DDERR_SURFACELOST) ProgRestoreSurfaces()

// Debuging macro ////////////////////////////////////////////////////////
#ifdef GAME_DEBUG
#define DebugSimple(s) DebugWrite(s)
#else
#define DebugSimple(s)
#endif


//————————————————————————————————————————————————————————————————————————
// Byte Flag Macros
//————————————————————————————————————————————————————————————————————————

// Set a byte flag ///////////////////////////////////////////////////////
#define BSET(v, f) v |= f

// Remove a byte flag ////////////////////////////////////////////////////
#define BREM(v, f) v &=~ f

// Invert a byte flag ////////////////////////////////////////////////////
#define BINV(v, f) v ^= f

// Set a byte flag based on a boolean value //////////////////////////////
#define BSETB(v, f, b) {if(b) v |= f; else v &= ~f;}

// Determine if a byte flag is set, return an intiger ////////////////////
#define BISSET(v, f) (v & f)

// Determine if a byte flag is set, return a boolean /////////////////////
#define BISSETB(v, f) ((v & f)? 1:0)


//————————————————————————————————————————————————————————————————————————
// Rectangle Macros
//————————————————————————————————————————————————————————————————————————

// Initilze a rectangle //////////////////////////////////////////////////
#define RSET(x, l, t, r, b) {x.left = l; x.top = t; x.right = r; x.bottom = b;}

// Set a rectangle based on x, y, width, & height ////////////////////////
#define RSETW(r, x, y, w, h) {r.left = x; r.top = y; r.right = r.left+w; r.bottom = r.top+h;}

// set a rect based on x, y, width & height [constant x version] /////////
#define RSETWX(r, x, y, w, h) {r.left = x; r.top = y; r.right = x+w; r.bottom = r.top+h;}

// set a rect based on x, y, width & height [constant y version] /////////
#define RSETWY(r, x, y, w, h) {r.left = x; r.top = y; r.right = r.left+w; r.bottom = y+h;}

// Expand a rectangle ////////////////////////////////////////////////////
#define REXP(r, x) {r.left -= x; r.top -= x; r.bottom += x; r.right += x;}

// Offset a rectangle ////////////////////////////////////////////////////
#define ROFST(r, x, y) {r.left += x; r.right += x; r.top += y; r.bottom += y;}

// Offset a rectangle only by x chords ///////////////////////////////////
#define ROFSTX(r, x) {r.left += x; r.right += x;}

// Offset a rectangle only by y chords ///////////////////////////////////
#define ROFSTY(r, y) {r.top += y; r.bottom += y;}

// See if a point is within a rectangle //////////////////////////////////
#define RISIN(r, x, y) (x >= r.left && x < r.right && y >= r.top && y < r.bottom)


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// DirectDraw setup functions ////////////////////////////////////////////
long DDInit(void);

// Create DirectDraw surfaces ////////////////////////////////////////////
void DdsCreate(LPDDS*, ULONG, ULONG);
void DdsCreate(LPDDS*, UCHAR);
long DdsRestore(LPDDS);

// Effects & Drawing /////////////////////////////////////////////////////
void DDRect(LPDDS, UCHAR, UCHAR, LPRECT, UCHAR);
void DDLine(LPDDS, UCHAR, long, long, long, long);

// DirectDraw Wrappers ///////////////////////////////////////////////////
void DDBltFast(LPDDS, LPDDS, ULONG, ULONG, LPRECT, DWORD);
void DDBltClip(LPDDS, LPDDS, long, long, LPRECT, LPRECT, DWORD);
void DDClear(LPDDS, UCHAR);

#ifdef GAME_DEBUG
void DDGetDC(LPDDS, HDC*);
void DDReleaseDC(LPDDS, HDC);
#endif

void DDLock(LPDDS, UCHAR**, ULONG*);
void DDUnlock(LPDDS);

// XMP functions /////////////////////////////////////////////////////////
void XalLoad(PALETTEENTRY[], UCHAR);
void XmpLoad(UCHAR);
void XmpRead(void);
void XmpCopy(LPDDS, ULONG, ULONG);
long XmpReload(LPDDS, UCHAR);
void XmpDelete(void);

// RES functions /////////////////////////////////////////////////////////
/*long ResAvalable(void);*/
void ResOpen(UCHAR);
void ResSeek(long, UCHAR);
void ResRead(void*, ULONG);
void ResClose(void);

// DirectInput functions /////////////////////////////////////////////////
long DIInit(void);
void DIClose(void);
void DIGetInput(void);

// Windows Interface Functions ///////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

// Debug functions ///////////////////////////////////////////////////////
#ifdef GAME_DEBUG
long  DDRVAL_DEBUG(HRESULT, char *);
void  DebugOpen(void);
void  DebugClose(void);
void  DebugDrawFps(void);
void  DebugWriteBmp(void);
void  DebugWrite(char*,...);
#endif

// memory functions //////////////////////////////////////////////////////
void *MoreMem(ULONG);

#ifdef GAME_DEBUG
void LessMem(void*);
#else
#define LessMem(x) free(x)
#endif


#endif // #ifndef UTILITY_HEADER_INCLUDED