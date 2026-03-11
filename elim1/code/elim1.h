/**************************************************************/
// elim1.h - elim1.cpp Definitions
// Written By:		Justin Hoffman
// Date Written:	Feb 14, 1999 - April 7, 1999
/**************************************************************/


/////////////////////////////////
// Setup
#ifndef gd_elim1_header_included
#define gd_elim1_header_included

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "class.h"


/////////////////////////////////
// Definitions

// Miscellaneous
#define gd_wintitle					"Elimination"
#define gd_winclass					"ELIMWINCLASS"
#define gd_midtitle					"ElimMusic"
#define gd_leftcenter				(DT_SINGLELINE | DT_VCENTER | DT_LEFT)
#define gd_centercenter				(DT_SINGLELINE | DT_VCENTER | DT_CENTER)
#define gd_rightcenter				(DT_SINGLELINE | DT_VCENTER | DT_RIGHT)
#define gd_nummidi					8
#define gd_fdeva					2
#define gd_fdevb					3
#define gd_btnokay					1
#define gd_btncancel				2

// Score Values
#define gdss_kill					100
#define gdss_new					50

// Colors
#define gc_greya					0x00E3E3E3
#define gc_greyb					0x00C7C7C7
#define gc_greyc					0x00ABABAB
#define gc_greyd					0x00C0C0C0
#define gc_lightgrey				0x00D7D7D7
#define gc_darkgrey					0x00808080
#define gc_lighttan					0x0087C7CB
#define gc_darktan					0x002B5F7B
#define gc_lightteal				0x00A7BF1F
#define gc_darkteal					0x00434300
#define gc_background				0x00E7DB8B
#define gc_bkborder					0x00B77B13


/////////////////////////////////
// Function Prototypes
ATOM				MyRegisterClass(HINSTANCE hInstance);	// register window class
BOOL				InitInstance(HINSTANCE, int);			// initialize app instance and create main window

LRESULT CALLBACK	aboutProc(HWND, UINT, WPARAM, LPARAM);	// about dialog box
LRESULT CALLBACK	rollWinProc(HWND, UINT, WPARAM, LPARAM);// roll dice to see who goes first
LRESULT CALLBACK	namesProc(HWND, UINT, WPARAM, LPARAM);	// get user names
LRESULT CALLBACK	optionsProc(HWND, UINT, WPARAM, LPARAM);// change game options
LRESULT CALLBACK	htpProc(HWND, UINT, WPARAM, LPARAM);	// display game rules

LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);	// main window callback
VOID CALLBACK		TimerProc(HWND, UINT, UINT, DWORD);		// timer callback

BOOL				makeBitmaps(void);						// initilize main bitmaps
BOOL				checkMenus(void);						// check menuitems that need it
BOOL				initBoard(void);						// initilize game varaibles

BOOL				pBoard(void);							// paints window contents into global dc
BOOL				pRect(HDC*, HPEN*, HBRUSH*, long, long,	// draw a colored rectangle
						long, long);
BOOL				pStart(int p);							// draw the starting chips
BOOL				pSelect(int, long, long);				// select a chip
BOOL				pUnselect(void);						// unselects chip
BOOL				pMoves(BOOL select);					// paint the moves
BOOL				pCreateSelSpace(HDC* hdc, BOOL select,  // paint offscreen bitmap
						long x, long y);	
BOOL				pCreateSel(int p, long x, long y);		// create offscreen bitmap for selection
BOOL				pIcon(HDC* hdc, int p);					// paint icon in name box
BOOL				pSpace(int t, int a, int b);			// paint a space
BOOL				pChip(int t, int a, int b);				// paint a chip
BOOL				pAddFreeGuy(int a, int b);				// paint the 'f'
BOOL				pScore(HDC* hdc, BOOL grey, long x,		// paint the score
						long y, int p);		

BOOL				aSelect(void);							// animate seleted chip
BOOL				aMoves(HDC* hdca, BOOL isP[]);			// animate selecting moves
BOOL				aPushSpace(void);						// animate pushing a space
BOOL				aCurve(LPgs_curve);						// animate a curved jump
BOOL				aExplode(int a, int b);					// blow up a chip
BOOL				aSinkChip(int a, int b, BOOL down);		// sink a chip into the board
BOOL				aName(int p, BOOL scroll);				// scroll names
BOOL				aGraph(int p, BOOL scroll, int dir);	// scroll a graph
BOOL				aDie(BOOL up, int p, int die, int num);	// animate dice
BOOL				aScrollScreen(void);					// scroll the screen
BOOL				aFreeGuys();							// animate free guys

BOOL				mseDown(long x, long y);				// mouse is pushed
BOOL				mseMove(long x, long y);				// mouse is moved
BOOL				mseUp(long x, long y);					// mouse is released

BOOL				rollDice(void);							// roll the dice
BOOL				nextPlayer(void);						// change players
BOOL				gameOver(int p);						// see if the game is over

BOOL				cSelect(int t, int a, int b);			// select a chip
BOOL				cUnselect(void);						// unselect the chip

BOOL				sClickSpace(int a, int b);				// a space was clicked
BOOL				sIsMove(int a, int b);					// is space a move?
BOOL				sWithinClickedSpace(long x, long y);	// is the mouse within the space?

BOOL				vMoveChip(int t, int sa, int sb, int da, int db);  // move a chip
BOOL				vShootChip(int t, int sa, int sb, int da, int db); // if chips need to be removed do so
BOOL				vSinkChip(int a, int b);						   // test if chip needs sunk and so so
BOOL				vUpdateDice(int t, int sa, int sb, int da, int db);// update dice
BOOL				vAddFreeGuy(int a, int b);				// add another free guy
BOOL				vTakeFreeGuy(int a, int b);				// a player landed on a free guy

BOOL				iCanMove(void);							// report if no moves
BOOL				iSearchMoves(void);						// test for any moves
BOOL				iMoves(int t, int a, int b);			// put possible moves into gg_selected
int					iBoardItem(int a, int b);				// return board item
BOOL				iIsFreeGuy(int a, int b);				// test if a space is a free guy

long				chdDocX(int a);							// return x chord of dock chips
long				chdBoardX(int a);						// return x chord of board chips
long				chdBoardY(int b);						// return y chord of board chips
long				chdFreePY(void);						// return y chord of the 'f'

BOOL				changeOption(UINT menuItem, int arritem);	// change a game option
BOOL				changeAniSpeed(UINT meniItem);				// change speed of animation
BOOL				newGame(void);								// start a new game

int					customBox(HWND, UINT, UINT, char*, char*, char*);// display the custom message box
BOOL				reportError(int errorNum=0, BOOL win=TRUE);	// display an error message
long				randNum(long n);						// return a random number between 0 and n-1
BOOL				sndPlay(DWORD id);						// play a sound
BOOL				waitTicks(DWORD ticks);					// wait for specified milliseconds
BOOL				wantToQuit(HWND hWnd);					// exit menu or close button clicked
BOOL				destBrushes(void);						// destroy global brushes

BOOL				mMakeRandList(void);					// create random song list
BOOL				mNextSong(void);						// play the next song
BOOL				mPlayMidi(void);						// play specified mid file
BOOL				mPauseMidi(void);						// pause music
BOOL				mResumeMidi(void);						// resume music
BOOL				mCloseMidi(void);						// close all midi files
int					mCurentSong(void);						// get number of current song

BOOL				openSettings(void);						// open settings
BOOL				writeSettings(void);					// save settings

#endif //#ifndef gd_elim1_header_included