/*******************************************************************************/
// main.h - Misc. definitions, and master includes
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 -
/*******************************************************************************/

#ifndef MAIN_HEADER_INCLUDED
#define MAIN_HEADER_INCLUDED

//#define ELIM_DEBUG /* comment this when i am done with the application */

/*-----------------------------------------------------------------------------*/
// Headers
/*-----------------------------------------------------------------------------*/

// standard windows headers //////////////////////////////////////////////
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

// Multi-Media and DirectX headers ///////////////////////////////////////
#include <mmsystem.h>
#include <ddraw.h>
#include <dsound.h>
#include <dmusici.h>

// File I/O headers //////////////////////////////////////////////////////
//#include <commdlg.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys\stat.h>
#include <io.h>


/*-----------------------------------------------------------------------------*/
// Game Definitions
/*-----------------------------------------------------------------------------*/

// Type definitions //////////////////////////////////////////////////////
typedef LPDIRECTDRAWSURFACE LPDDS;  // Shorten things up
typedef LPDIRECTSOUNDBUFFER LPDSO;  //         "

// Game Settings Flags ///////////////////////////////////////////////////
#define SET_REGISTERED        0x00001 // game is registered
#define SET_SOUND_ON          0x00002 // sound is on
#define SET_MUSIC_ON          0x00004 // music is on
#define SET_SHOW_MOVES        0x00008 // moves should be shone
#define SET_PAUSED            0x00010 // the game is paused by a window
#define SET_APP_PAUSED        0x00020 // the application is suspended
#define SET_WINDOW            0x00040 // a window is present
#define SET_FIRST_GAME        0x00080 // this is the first game since open

#define SET_ROLL_TURN         0x00100 // we are rolling dice for first player
#define SET_ROLL_DICE         0x00200 // we are rolling the normal dice
#define SET_GAME_OVER         0x00400 // the game is over, used to stop the pc
#define SET_CHECK_TIME        0x00800 // we should check the time
#define SET_MARBLE_JUMPING    0x01000 // a marble is jumping
#define SET_LOADING           0x02000 // a game has been loaded

// default game settings
#ifdef ELIM_DEBUG
#define SET_DEFAULT           (SET_REGISTERED | SET_SOUND_ON | SET_MUSIC_ON |\
                              SET_SHOW_MOVES | SET_FIRST_GAME)
#else
#define SET_DEFAULT           (SET_REGISTERED | SET_SOUND_ON | SET_MUSIC_ON | SET_SHOW_MOVES |\
                               SET_FIRST_GAME)
#endif

// 'Moving Box' Flags and statuses ///////////////////////////////////////
#define BOX_INACTIVE          1
#define BOX_IDLE              2
#define BOX_MOVING            3
#define BOX_FINISHED_MOVING   4

#define BOX_VERTICAL          0x00001 // does the box move vertically?
#define BOX_MOVING_MIN        0x00002 // the box is moving twards the min value, if not set assume max

#define DIE_LAST              0x00004 // used to signify the dice is the last one
#define DIE_REROLLING         0x00008 // use to say the die is being hidden
#define DIE_DONT_ROLL         0x00010 // used after validating dice to say it wasn't in the tie
#define DIE_HIDING            0x00020 // used if the die is being hidden

#define MENUBAR_NOTEXT        0x00004 // set if the message text is blank

#define PINFO_SCROLLING       0x00004 // set if the player graph is moving
#define PINFO_FLASHING        0x00008 // set if the pInfo is being 'selected'
#define PINFO_ON_LEFT         0x00010 // set if pInfo is on the left side
#define PINFO_SELECT          0x00020 // we have to select this box


// Object velocities /////////////////////////////////////////////////////
#define VEL_DIEA              0.1  // dice for rolling turns
#define VEL_DIEB              0.25 // main dice
#define VEL_MENU              0.1  // menubar
#define VEL_TEAMS             0.1  // teams box
#define VEL_REGTIME           0.1  // registration time box
#define VEL_PINFO             1    // player info box


// Animation wait times //////////////////////////////////////////////////
#define WAIT_MARBLE_ASCENDING 25
#define WAIT_MARBLE_DECENDING 25
#define WAIT_MOVE_SELECT      50
#define WAIT_GLOW             20
#define WAIT_EXPLODE          50


// selected moves statuses ///////////////////////////////////////////////
#define MOVE_INACTIVE         0
#define MOVE_SELECTING        1
#define MOVE_IDLE             2
#define MOVE_CLICKING         3


// marble statuses ///////////////////////////////////////////////////////
#define MARBLE_ASCENDING      0
#define MARBLE_DECENDING      1
#define MARBLE_EXPLODING      2
#define MARBLE_INIT_JUMP      3
#define MARBLE_JUMPING        4
#define MARBLE_IDLE           5


// marble types //////////////////////////////////////////////////////////
#define MARBLE_INVALID        101
#define MARBLE_SPACE          102
#define MARBLE_SINKHOLE       103
#define MARBLE_FREE           104


// Scores ////////////////////////////////////////////////////////////////
#define SCORE_HIT             50
#define SCORE_FREE            100


// button flags //////////////////////////////////////////////////////////
#define BUTTON_ACTIVE         0x00001 // is active
#define BUTTON_ENABLED        0x00002 // is enabled
#define BUTTON_BEING_PUSHED   0x00004 // should check mouse movement
#define BUTTON_DOWN           0x00008 // is down
#define BUTTON_TOGGLE         0x00010 // is a toggle button
#define BUTTON_TOGGLE_DOWN    0x00020 // is in a down state


// Player flags //////////////////////////////////////////////////////////
#define PLAYER_ACTIVE         0x01
#define PLAYER_IS_PC          0x02
#define PLAYER_OUT            0x04


// default flags
#define PLAYER_DEF_1          0
#define PLAYER_DEF_2          PLAYER_ACTIVE
#define PLAYER_DEF_3          0
#define PLAYER_DEF_4          PLAYER_ACTIVE


// Misc. definitions /////////////////////////////////////////////////////
#define TXT_WIN_CLASS         "ELIM2WINCLASS"
#define TXT_WIN_TITLE         "Elimination 2.0"
#define TXT_MUSIC_NAME        "music\\music%d.mid"
#define SCREEN_W              640
#define SCREEN_H              480
#define SCREEN_BPP            8
#define D_NUM_MENUS           9
#define D_INVALID             -1
#define D_NUMSONGS            10
#define D_SOUNDSPEED          22050
#define D_CENTERCHANNEL       SCREEN_W / 2
#define D_UNREG_TIME_ALLOWED  600000

#define WRITE                 0
#define READ                  1

#define TXT_FMT_CC            (DT_SINGLELINE | DT_VCENTER | DT_CENTER)
#define TXT_FMT_CL            (DT_SINGLELINE | DT_VCENTER | DT_LEFT)
#define TXT_FMT_CR            (DT_SINGLELINE | DT_VCENTER | DT_RIGHT)


// Filename definitions //////////////////////////////////////////////////
#define FILE_REGISTERED       "elim2.elm"
#define FILE_SETTINGS         "data\\settings.elm"
#define FILE_SAVED            "data\\saved.elm"
#define FILE_RESDATA          "res.elm"


// id's used to see if they truly are the file
#define FILE_IS_REGISTERED    0x091960
#define FILE_IS_SETTINGS      0x070555
#define FILE_IS_SAVED         0x071154


// used to locate saved games (the second int is because teams is 6 of them,
// the third is for the dice values, the fourth is for the marble locations)
//#define SAVED_SIZE            (sizeof(char) * 160 + sizeof(BOOL) + sizeof(int) * 270 + \
//                               sizeof(int) * 6 + sizeof(int) * 2 + sizeof(DWORD) + \
//                               sizeof(int) * 5 + sizeof(SPRITE))

#define SAVED_SIZE            (sizeof(char) * 160 + sizeof(int) * 284 + \
                               sizeof(DWORD) + sizeof(SPRITE) + sizeof(long))   
      

// Message Box Texts /////////////////////////////////////////////////////
#define TXT_GENTITLE          "Elimination"
#define TXT_REGTITLE          "Demo Version"
#define TXT_HELP              "How To Play"
#define TXT_ABOUT_ELIM        "About Elimination"
#define TXT_OK                "OK"
#define TXT_CANCEL            "Cancel"
#define TXT_LOADING           "Loading..."
#define TXT_DATA_FOLDER       "data"
#define TXT_NORESOURCE        "Error: Failed to locate the resource file.  Please re-install Eliminatoin II."
#define TXT_DDINIT_FAILED     "Error: Failed to initilize DirectDraw."
#define TXT_NEED_TWO          "There must be at least two players in the game."
#define TXT_NEED_THREE        "There must be at least three players to have a team game."
#define TXT_TWO_ON_TEAM       "There can only be up to two players on a team."
#define TXT_ALL_ON_TEAM       "All players must belong to a team in team games."
#define TXT_DICE_TIE          "Tie, rolling again..."
#define TXT_DICE_WINNER       "%s has won the roll."
#define TXT_RED               "red"
#define TXT_TEAL              "teal"
#define TXT_PURPLE            "purple"
#define TXT_TAN               "tan"
#define TXT_NOT_TURN          "It is %s's turn, please click on a %s marble."
#define TXT_NO_NEW_MARBLE     "Sorry %s, there is no place for the new marble to go"
#define TXT_NO_MOVES          "Sorry %s, you currently have no possible moves."
#define TXT_NO_PC_MOVES       "The computer player %s has no possible moves."
#define TXT_PLAYER_OUT        "%s was out of the game before it was last saved "\
                              "with a score of %d."
#define TXT_PC_PLAYER_OUT     "The computer player %s was out of the game before "\
                              "it was last saved with a score of %d."
#define TXT_GO_ALL_LOSE       "All of the players are out, it is a draw.  Do you want to play again?"
#define TXT_GO_ONE_WIN        "%s has won the game.  Do you want to play again?"
#define TXT_GO_TWO_WIN        "%s and %s have won the game. Do you want to play again?"
#define TXT_ERROR             "Error"
#define TXT_ERR_READ          "An error occured while trying to read the nessary file."
#define TXT_ERR_WRITE         "An error occured while trying to write to a file. "\
                              "The disk may be locked."
#define TXT_ACT_UNREGISTERED  "Activating and deactivating players is a feature in the "\
                              "full version of this game."
#define TXT_GENERIC_UNREG     "This feature is avalable in the full version.  "\
                              "See the help file on information about obtaining "\
                              "the full version of this game."
#define TXT_UNREG_TIME        "Demo - %d:%s"
#define TXT_UNREG_TIME_PAUSE  "Paused"
#define TXT_TIME_UP           "You have been playing the demo version of "\
                              "Elimination 2.  You can get the full version from www.xgames3d.com to remove the time limit, "\
                              "be able to save up to ten games, and to be able "\
                              "to have more that two players."
#define TXT_UNREGISTERED      "Sorry, this feature is only avalable in the full version."


// Colors ////////////////////////////////////////////////////////////////
#define CLR_TRANSPARENT       76
#define CLR_BACKGROUND        96
#define CLR_FREEGUYS          88 // pal etry for the 'f'
#define CLR_PINFO             75 // pal etry for selected pinfo
#define CLR_WHITE             254
#define CLR_BLACK             0
#define CLR_RED               117
#define CLR_WINDOWFILL        194
#define CLR_WINDOWBORDER      164
#define CLR_BORDER            30
#define CLR_MENU_TOP          250
#define CLR_MENU_BOTTOM       164
#define CLR_MENU_FILLDOWN     229

#define CLR_L_GREY            245
#define CLR_D_GREY            198
#define CLR_L_RED             217
#define CLR_D_RED             39
#define CLR_L_TEAL            211
#define CLR_D_TEAL            80
#define CLR_L_PURPLE          240
#define CLR_D_PURPLE          136
#define CLR_L_TAN             242
#define CLR_D_TAN             172


// RGB colors ////////////////////////////////////////////////////////////
#define RGB_WHITE             RGB(254, 254, 254)
#define RGB_GREY              RGB(125, 125, 125)
#define RGB_BLACK             RGB(0, 0, 0)
#define RGB_RED               RGB(255, 0, 0)
#define RGB_WINDOWBORDER      RGB(187, 148, 11)
#define RGB_MENUBAR_TEXT      RGB(155, 125, 9)


// Game States ///////////////////////////////////////////////////////////
#define STATE_INTRO           0
#define STATE_INITGAME        1
#define STATE_PLAYING         2
#define STATE_SHUTDOWN        3


// Game Messages /////////////////////////////////////////////////////////
#define MSG_DRAW              99
#define MSG_BUTTON1           100
#define MSG_BUTTON2           101
#define MSG_BUTTON3           102
#define MSG_TEXT_BOX          103


// Game data files ///////////////////////////////////////////////////////
#define XAL_INTRO                       0
#define XAL_ITEMS                       1

#define WAV_ERROR                       2
#define WAV_EXPLODE                     3
#define WAV_FREE                        4
#define WAV_MENU                        5
#define WAV_MENUB                       6
#define WAV_QUESTION                    7
#define WAV_RISE                        8
#define WAV_SELECT                      9
#define WAV_SINK                        10
#define WAV_UNALLOWED                   11

#define XMP_ABOUT                       12
#define XMP_BOARD                       13
#define XMP_DICE                        14
#define XMP_EXPLODE                     15
#define XMP_INTRO                       16
#define XMP_ITEMS                       17
#define XMP_MENU                        18
#define XMP_NAMES                       19
#define XMP_SELECT                      20


/*-----------------------------------------------------------------------------*/
// Structure Definitions
/*-----------------------------------------------------------------------------*/

// Structure for describing a team ///////////////////////////////////////
typedef struct sTeam
{
   int nTeamA, nTeamB; // number of players in each team
   int teamA[2];       // players in teamA
   int teamB[2];       // players in teamB
} TEAM;


// Structure for describing a 'moving box' ///////////////////////////////
typedef struct sBox
{
   long   constant;   // the constant chord (i.e. x for vertical and y for horisontal)
   long   variable;   // the current variable chord (i.e. y for vertical and x for horisontal)
   long   max, min;   // the variable maximum and minimum
   double velocity;   // the boxes velocity in pixels/millisecond

   int value;       // used for dice to identify what was rolled
   int owner;       // what player owns this object?
   int position;    // when is it that players turn?

   DWORD status;    // status of the object
   DWORD flags;     // object specific flags that are set
   DWORD bltFlags;  // flags to give to bltFast
 
   DWORD lastTick;  // last frame executed

   LPDIRECTDRAWSURFACE dds; // source DirectDraw surface
   RECT rectSource; // source rectangle for this object
   RECT rectClip;   // clipping rectangle for this objec0t

   sBox() {status = BOX_INACTIVE; flags = 0;}
} BOX;


// Used for the player info boxes ////////////////////////////////////////
typedef struct sPInfo
{
   BOX box;       // holds the 'moving box' data used

   int velocity;  // velocity the graph is moving
   long width;    // the current x position
   long widthMax; // the maximum x position

   int cFFrameA;  // current flash frame
   int cFFrameB;  // number of times it has flashed
} PINFO;


/*-----------------------------------------------------------------------------*/
// Macros
/*-----------------------------------------------------------------------------*/

// Random - return a number between 0 and n-1 ////////////////////////////
#define RAND(a) rand() % a

// set a flag based on a boolean value ///////////////////////////////////
#define SET_FLAG(value, flag, set) if(set) value |= flag; else value &= ~flag

// see if a flag is set //////////////////////////////////////////////////
#define FLAG_SET(value, flag) ((value & flag)? 1:0)


/*-----------------------------------------------------------------------------*/
// Globals in main.cpp
/*-----------------------------------------------------------------------------*/

// Game Globals //////////////////////////////////////////////////////////
extern int   g_nPlayers;
extern int   g_nSinkholes;
extern int   g_cPlayer;
extern int   g_cColor;
extern int   g_marbles[4];
extern long  g_score[4];
extern char  g_name[4][32];
extern int   g_pFlags[4];
extern TEAM  g_teams;
extern DWORD g_startTime;

// Game Objects //////////////////////////////////////////////////////////
extern BOX   g_diceA[4];
extern BOX   g_diceB[2];


// General Globals ///////////////////////////////////////////////////////
extern RECT      R_SCREEN;
extern HINSTANCE g_hInst;
extern HWND      g_hWnd;
extern char      g_appPath[];
extern DWORD     g_settings;
extern DWORD     g_pauseTime;
extern int       g_gameState;


// GDI Globals ///////////////////////////////////////////////////////////
extern HFONT g_fontSmall;
extern HFONT g_fontBig;
extern HFONT g_fontHelp;


// Palette Globals ///////////////////////////////////////////////////////
extern LPDIRECTDRAWPALETTE ddpMain;
extern PALETTEENTRY        g_palIntro[];
extern PALETTEENTRY        g_palMain[];


// DirectDraw Globals ////////////////////////////////////////////////////
extern LPDDS ddsFront;
extern LPDDS ddsBack;
extern LPDDS ddsBoard;
extern LPDDS ddsDice;
extern LPDDS ddsExplode;
extern LPDDS ddsItems;
extern LPDDS ddsMenu;
extern LPDDS ddsSelect;

extern LPDDS ddsWindow;
extern LPDDS ddsMenubar;
extern LPDDS ddsTeams;
extern LPDDS ddsRegTime;
extern LPDDS ddsPInfo;

// DirectSound Globals ///////////////////////////////////////////////////
extern LPDSO dsoMenu;
extern LPDSO dsoMenuB;
extern LPDSO dsoQuestion;
extern LPDSO dsoUnallowed;
extern LPDSO dsoSink;
extern LPDSO dsoMarbleUp;
extern LPDSO dsoSelect;
extern LPDSO dsoExplode;
extern LPDSO dsoFree;
extern LPDSO dsoError;


/*-----------------------------------------------------------------------------*/
// Function Prototypes for main.cpp
/*-----------------------------------------------------------------------------*/

// Message handeling /////////////////////////////////////////////////////
LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

// Game setup and processing /////////////////////////////////////////////
void ElimMain(void);
void ElimMenu(UINT, BOOL);
void ElimInit(void);
void ElimClose(void);

// Main Frame Rendering //////////////////////////////////////////////////
void RenderFrame(void);

// Misc game functions ///////////////////////////////////////////////////
void NewGame(void);
void SetupLoadedGame(void);
void CloseGame(void);
void NextPlayer(void);
int  ColorToPlayer(int);
int  PlayerToColor(int);

// Generic Box Functions /////////////////////////////////////////////////
void Box_Move(BOX*, BOOL);
void Box_Render(BOX*);

// Roll dice for the first turn //////////////////////////////////////////
void Dice_SetUpA(void);
void Dice_ReSetA(void);
void Dice_Validate(void);
void Dice_UpdateA(void);
void Dicc_UpdateB(void);

// Roll the main dice ////////////////////////////////////////////////////
void Dice_InitB(void);
void Dice_Show(void);
void Dice_ShowB(void);
void Dice_Roll(void);
void Dice_Hide(void);
void Dice_UpdateB(void);

// Menubar Functions /////////////////////////////////////////////////////
void Menu_Init(void);
void Menu_Disable(int);
void Menu_Draw(void);
void Menu_DrawMenu(int);
void Menu_MouseDown(long, long);
void Menu_MouseMove(long, long);
void Menu_MouseUp(long, long);
int  Menu_MouseToBtn(long, long);

// Teams functions ///////////////////////////////////////////////////////
void Teams_Init(void);
void Teams_Draw(void);

// Registration time functions ///////////////////////////////////////////
void RegTime_Init(void);
void RegTime_Update(void);
void RegTime_Draw(void);

// Player Info boxes /////////////////////////////////////////////////////
void PInfo_Select(int);
void PInfo_UnSelect(int);
void PInfo_UpdateScore(int, int);
void PInfo_UpdateGraph(int, int);
void PInfo_KillPlayer(int);
void PInfo_Init(void);
void PInfo_Close(void);
void PInfo_Render(void);
void PInfo_Draw(void);


/*-----------------------------------------------------------------------------*/
// Inline rectangle functions
/*-----------------------------------------------------------------------------*/

// Set a rectangle based on chords ///////////////////////////////////////
inline void Rect_SetPoint(RECT *r, long xa, long ya, long xb, long yb)
{
   r->left   = xa;
   r->top    = ya;
   r->right  = xb;
   r->bottom = yb;
}


// Set a rectangle based on it's width and height ////////////////////////
inline void Rect_SetDimen(RECT *r, long x, long y, long width, long height)
{
   r->left   = x;
   r->top    = y;
   r->right  = x + width;
   r->bottom = y + height;
}


// Expand a rectangle ////////////////////////////////////////////////////
inline void Rect_Expand(RECT *r, int a)
{
   r->left   -= a;
   r->top    -= a;
   r->right  += a;
   r->bottom += a;
}


// Offset a rectangle ////////////////////////////////////////////////////
inline void Rect_Offset(RECT *r, long ox, long oy)
{
   r->left   += ox;
   r->top    += oy;
   r->right  += ox;
   r->bottom += oy;
}


// Return weather a point is within a rectangle //////////////////////////
inline BOOL Rect_Within(RECT *r, long x, long y)
{
   return (r->left < x && x < r->right && r->top < y && y < r->bottom);
}


// Check to see if the game is idle //////////////////////////////////////
inline BOOL GameIsBusy(void)
{
   return ((g_settings & SET_MARBLE_JUMPING) || (g_settings & SET_ROLL_DICE));
}


#endif // ifndef MAIN_HEADER_INCLUDED