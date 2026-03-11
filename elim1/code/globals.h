/**************************************************************/
// globals.h - Global definitions for elim1.cpp
// Written By:		Justin Hoffman
// Date Written:	March 6, 1999
/**************************************************************/


/////////////////////////////////
// Setup
#ifndef gd_globals_header_included
#define gd_globals_header_included

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////
// Global Variables

// Application Globals
HINSTANCE			gg_hInst;					// current instance
HWND				gg_hwndMain;				// main window handle
BOOL				gg_mousedown=FALSE;			// true if mouse is down
BOOL				gg_suspended=FALSE;			// program is not in foreground
BOOL				gg_canpaint=FALSE;			// allows window to paint it self

// Global dcs & bitmaps
HDC	ggh_maina;									// stores screen for re-painting
HDC	ggh_maind;									// main dc (used for non WM_PAINT drawing)
HDC	ggh_paint;									// paint for most objects
HDC	ggh_masks;									// paint for object masks
HDC	ggh_eff;									// paint for animation effects
HDC ggh_select;									// paint for selected chip
HDC ggh_selani;									// animation for selecting a space
HDC ggh_fglow;									// bitmap for storing glowing 'f'
HBITMAP ggm_main;								// stores screen for re-paint
HBITMAP ggm_paint;								// stores paint items
HBITMAP ggm_masks;								// stores item masks
HBITMAP ggm_eff;								// stores animation paint
HBITMAP ggm_select;								// stores selected chip paint
HBITMAP ggm_selani;								// stores space paint
HBITMAP ggm_fglow;

// Global Brushes
HBRUSH ggb_greya		=CreateSolidBrush(gc_greya);
HBRUSH ggb_greyb		=CreateSolidBrush(gc_greyb);
HBRUSH ggb_greyc		=CreateSolidBrush(gc_greyc);
HBRUSH ggb_greyd		=CreateSolidBrush(gc_greyd);
HBRUSH ggb_lightgrey	=CreateSolidBrush(gc_lightgrey);
HBRUSH ggb_lighttan		=CreateSolidBrush(gc_lighttan);
HBRUSH ggb_lightteal	=CreateSolidBrush(gc_lightteal);
HBRUSH ggb_background	=CreateSolidBrush(gc_background);

// Global Pens
HPEN ggp_black			=CreatePen(PS_SOLID, 0, 0x00000000);
HPEN ggp_bkborder		=CreatePen(PS_SOLID, 0, gc_bkborder);
HPEN ggp_darkgrey		=CreatePen(PS_SOLID, 0, gc_darkgrey);
HPEN ggp_darktan		=CreatePen(PS_SOLID, 0, gc_darktan);
HPEN ggp_darkteal		=CreatePen(PS_SOLID, 0, gc_darkteal);

// Global Fonts
HFONT ggf_maina			=CreateFont(10, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						FF_DONTCARE, "MS Sans Serif");
HFONT ggf_mainb			=CreateFont(14, 0, 0, 0, 700, FALSE, FALSE, FALSE, ANSI_CHARSET,
						OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
						FF_DONTCARE, "Courier New");

// Custob Box Globals
char ggcb_text[3][256]={NULL, NULL, NULL};									// text of custom box
UINT ggcb_numdat[2]={0,0};													// icon of custom box

// Game Globals
double					gg_framedevide=gd_fdeva;							// change animation speed
int						gg_aselnum=0;										// current selection frame
int						gg_seltype=0;										// type of chip selected
long					gg_notturn=0;										// number of times other chip clicked
int						gg_player=0;										// current player
int						gg_roll[2];											// currently rolled dice
int						gg_preroll[2];										// die roll to see who goes first
int						gg_board[15][10];									// contents of the board
int						gg_docChips[2][10];									// contents behind the lines
int						gg_selected[2][9];									// selected chip and moves
int						gg_freeguys[2][20];									// location of free guys
int						gg_pselnum=0;										// current 'f' frame
int						gg_numfreeguys;										// number of free guys
int						gg_chipsremain[2];									// number of remaining chips
long					gg_score[2];										// scores of the players
int						gg_holes[2][50];									// location of sink holes
int						gg_numholes=15;										// number of sink holes
int						gg_pushspace[5];									// data for pushing spaces
POINT					gg_pushpoint;										// location of pushed space
long					gg_selchord[2]={0,0};								// chordinates of selected chip
char					gg_names[2][15]={"Player 1","Player 2"};			// player names
BOOL					gg_options[5]={TRUE,TRUE,TRUE,TRUE,FALSE};			// game options
int						gg_midopt[4]={1,0,0};								// midi data
int						gg_midlist[gd_nummidi]={0};							// random list						
char					gg_path[256];										// pathname of program

#endif // #ifndef gd_globals_header_included