/************************************************************************/
// user.h 
//
// Author: Justin Hoffman
// Date:   5/1/00 - 
/************************************************************************/

#ifndef USER_HEADER_INCLUDED
#define USER_HEADER_INCLUDED

#include "utility.h"


//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

// game texts ////////////////////////////////////////////////////////////
#define TXT_PLRDISCONNECT  "%s has been disconnected"
#define TXT_NORES          "Error: Resource file could not be found"
#define TXT_GENTITLE       "Elimination III"
#define TXT_OKAY           "Okay"
#define TXT_CANCEL         "Cancel"
#define TXT_HOSTIP         "Host IP:"
#define TXT_PASSWORD       "Password:"
#define TXT_USERNAME       "User Name:"
#define TXT_SHOWMENU       "hide or show the menu bar"
#define TXT_SINGLEROUNDWIN "%s has won the round."
#define TXT_SINGLEWINNER   "%s has won the match.  Would you like to play again?"
#define TXT_ROLL           "Roll:"
#define TXTLEN_ROLL        5
#define TXTLEN_HOSTIP      8
#define TXTLEN_PASSWORD    9
#define TXTLEN_USERNAME    10

// object velocities /////////////////////////////////////////////////////
#define VEL_TEAMBAR        .08
#define VEL_CHATBAR        .2
#define VEL_MENUBAR        .5
#define VEL_NORMALNAMEBAR  .5
#define VEL_CHATNAMEBAR    .5
#define VEL_NORMALDICE     .2
#define VEL_NETDICE        .1
#define VEL_ROLLBAR        .2
#define VEL_TIMEBAR        .2
#define VEL_SELECTRING     .05
#define VEL_SPLASH         .5
#define VEL_MOVE           .05
#define VEL_SHOWMARBLE     .03
#define VEL_HIDEMARBLE     .03
#define VEL_GLOW           .05
#define VEL_EXPLODE        .02

// waiting times /////////////////////////////////////////////////////////
#define WAIT_NAMEFLASH     200
#define WAIT_GRAPH         10
#define WAIT_NUMUPDATE     100
#define WAIT_SPLASH        3000
#define WAIT_UTILBARS      1000
#define WAIT_SHOWMSG       1000
#define WAIT_NEWNETGAME    1000

// box states & flags ////////////////////////////////////////////////////
#define BOX_INACTIVE       0     // should not be drawn or updated
#define BOX_IDLE           1     // minding its own busness
#define BOX_MOVING         2     // crusin' along
#define BOX_FINISHED       3     // done with the crusin'

// button definitions ////////////////////////////////////////////////////
#define BTN_MENUA          0     // is a menu button
#define BTN_MENUB          1     // dialog menu button
#define BTN_TEXT           2     // is a text button
#define BTN_CHECKBOX       3     // check box button
#define BTN_SHOWMENU       6     // show/hide menu button
#define BTN_ARROW          7     // up arrow button

#define BTN_INACTIVE       0     // should not be drawn or updated
#define BTN_DISABLED       1     // should be drawn, but no input given to it
#define BTN_TRACKMOUSE     2     // tracking the mouse
#define BTN_WASUSED        3     // has been clicked by the user
#define BTN_IDLE           4     // nothing really happening

#define BTN_TRANSPARENT    0x01  // button is transparent
#define BTN_PUSHED         0x02  // is 'pushed'
#define BTN_TOGGLE         0x04  // is a toggle button
#define BTN_TDOWN          0x08  // is in 'down' toggle position
#define BTN_CURWITHIN      0x10  // mouse is within it

#define BTN_USED(x) (x.state == BTN_WASUSED)   // button was used
#define BTN_DOWN(x) (x.flags & BTN_TDOWN? 1:0) // button is toggled down

// Text box definitions //////////////////////////////////////////////////
#define TXT_IDLE           0     // just sitting around
#define TXT_CURWITHIN      1     // the mouse is within it
#define TXT_SELECTED       2     // gotta get those letters from the user

// Window definitions ////////////////////////////////////////////////////
#define WND_INIT           0     // sent before the window is shown
#define WND_DRAW           1     // sent when window needs to draw
#define WND_UPDATE         2     // sent each update loop
#define WND_RENDER         3     // sent each render loop
#define WND_MAINBTN        4     // a main button has been used, extra is button id
#define WND_CUSTBTN        5     // a custom button has been used, extra is button id
#define WND_TEXTBOX        6     // a text box has been selected


//————————————————————————————————————————————————————————————————————————
// Structures
//————————————————————————————————————————————————————————————————————————

// Utility bar ///////////////////////////////////////////////////////////
struct BAR {
   double x, y      ; // x & y chords
   double xvel, yvel; // x & y velocity in pixels/tick
   long   xmax, ymax; // max x & y chords
   long   xmin, ymin; // min x & y chords

   long  extra; // extra data used by specific objects

   UCHAR state; // object's current status
   UCHAR flags; // object's flags

   LPDDS surf ; // pointer to source surface
   DWORD ltick; // time of last frame   
   RECT  rclip; // rectangle to clip to
   RECT  rsrc ; // source rectangle
};

// Button structure //////////////////////////////////////////////////////
struct BTN {
   char  text[64]; // button text
   char *msg;     // pointer to a string with message text
   long  extra;  // extra data

   RECT  rdest;  // destination surface button location
   long  xscrn;  // screen x choordinate
   long  yscrn;  // screen y choordinate

   LPDDS ddsdst; // destination surface

   UCHAR type ;  // button type
   UCHAR state;  // button status
   UCHAR flags;  // button flags
};

// Text box control structure ////////////////////////////////////////////
struct TXT {
   char  text[32]; // currnent text in box
   bool  center; // should the text be centered?
   UCHAR state; // state of box
   UCHAR extra; // extra data
   RECT  rdest; // surface rectangle
   RECT  rscrn; // screen rectangle
   
   void (*key)(TXT*); // used to intrpret keys customly
};

// Number control structure //////////////////////////////////////////////
struct NUM {
   bool disabled;
   long max  ; // maximum value
   long min  ; // minimum value
   long incr ; // increment used each time up or down button is clicked
   long value; // current value

   long x, y  ; // x & y chords
   long textw ; // text width
   long labelw; // label width

   char  title[16]; // control's title
   BTN   btns[2]  ; // button used to increase/decrease value
   DWORD ltick    ; // last time of update

   void (*numcust)(long, LPSTR); // used to create custom text value
};

// Window data structure /////////////////////////////////////////////////
struct WND {
   char  title[32]; // window title

   long  rval ; // return value
   long  xscrn; // x screen chord
   long  yscrn; // y screen chord
   long  xsurf; // x surface chord
   long  ysurf; // y surface chord
   long  width; // width
   long  hight; // height

   char  cbox ; // currently selected text box

   UCHAR nbtns; // number of buttons
   UCHAR ntxts; // number of text boxes
   UCHAR nnums; // number of number controls
   TXT  *ptxts; // pointer to text boxes
   BTN  *pbtns; // pointer to custom buttons
   NUM  *pnums; // pointer to number controls
   BTN   mbtns[3]; // main text buttons on bottom

   long (*loop)(WND*, UCHAR, UCHAR); // pointer to window defined loop
};

// Bitmaped font data structure //////////////////////////////////////////
struct FNT {
   UCHAR cellw;
   UCHAR cellh;
   UCHAR charh;
   UCHAR lwidths[91];
   LPDDS dds;
};


//————————————————————————————————————————————————————————————————————————
// Type definitions
//————————————————————————————————————————————————————————————————————————

// Pointer to function for window call loop //////////////////////////////
typedef long (*WNDLOOP)(WND*, UCHAR, UCHAR);

// Pointer to a function to calculate string for number controls /////////
typedef void (*NUMCUST)(long, LPSTR);


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// misc. globals /////////////////////////////////////////////////////////
extern RECT R_SCREEN;
extern void (*NameDraw)(void);

// fonts /////////////////////////////////////////////////////////////////
extern FNT gFntTitle   ;
extern FNT gFntNames   ;
extern FNT gFntTxt     ;
extern FNT gFntTxtHil  ;
extern FNT gFntTxtDim  ;
extern FNT gFntTxtSmall;

// chat data /////////////////////////////////////////////////////////////
extern char gsChatText[4][64];
extern char gsChatSend[64];
extern long gsChatSendPos;

// utility bars //////////////////////////////////////////////////////////
extern BAR gBarTeams;
extern BAR gBarMenu;
extern BAR gBarChat;
extern BAR gBarRoll;
extern BAR gBarDice[2];
extern BAR gBarNames[4];

// window data ///////////////////////////////////////////////////////////
extern WND gWndA;


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// Game User Interface Code //////////////////////////////////////////////
void GameUpdateUI(void);
void GameRenderUI(void);

// setup functions ///////////////////////////////////////////////////////
void GameInitBarsA(void);
void GameInitBarsB(void);

// Menu functions ////////////////////////////////////////////////////////
void MenuEnable(UCHAR, bool);
void MenuPush(UCHAR);
void MenuDraw(void);
void MenuInit(void);

// Chat Bar Functions ////////////////////////////////////////////////////
void ChatDraw(void);
void ChatAddText(UCHAR, char*);
void ChatDrawText(void);
void ChatDrawSendText(void);
void ChatInit(void);

// Name bar functions ////////////////////////////////////////////////////
void NameSetup(void);
void NameSelect(void);
void NameChangeGraph(UCHAR, long);
void NameSelectDraw(UCHAR, UCHAR);
void NameDrawA(void);
void NameDrawB(void);
void NameDrawGraph(UCHAR);
void NameDrawColorBar(UCHAR, long, long, long, long);
void NameDrawGreyBar(UCHAR, UCHAR, long, long, long, long);

// dice, & team bar functions ////////////////////////////////////////////
void DiceDraw(void);
void TeamDraw(void);

// Utility bars functions ////////////////////////////////////////////////
void BarShow(BAR*, bool, bool);
void BarUpdate(BAR*);
void BarRender(BAR*);

// Window functions //////////////////////////////////////////////////////
long Wnd(UCHAR, WNDLOOP);
void WndLoop(void);
void WndUpdate(void);
void WndClose(long);
void WndDraw(WND*);
void WndRedraw(void);
void WndGroup(WND*, char*, long, long, long, long);
void WndHLine(WND*, long);

// Number control functions //////////////////////////////////////////////
void NumSetBounds(NUM*, long, long);
void NumSetValue(NUM*, long);
void NumUpdate(NUM*);
void NumDraw(NUM*);
void NumDrawData(NUM*);

// Button functions //////////////////////////////////////////////////////
void BtnEnable(BTN*, bool);
void BtnUpdate(BTN*);
void BtnDraw(BTN*);

// Text box functions ////////////////////////////////////////////////////
long TxtCheckKeys(char*, long*, long);
void TxtUpdate(TXT*);
void TxtDraw(TXT*);

// Font Functions ////////////////////////////////////////////////////////
void FntLoad(FNT*, UCHAR);
void FntDelete(FNT*);
long FntRestore(FNT*, UCHAR);
long FntStrWidth(FNT*, char*, long);
long FntDraw(FNT*, LPDDS, char*, long, long, long);
void FntDrawCentered(FNT*, LPDDS, char*, RECT*);
void FntDrawWrap(FNT*, LPDDS, char*, RECT*);

#endif // #ifndef USER_HEADER_INCLUDED