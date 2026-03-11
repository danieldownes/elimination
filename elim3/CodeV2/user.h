/************************************************************************/
// user.h 
//
// Author: Justin Hoffman
// Date:   6/7/00 - 7/18/2000
/************************************************************************/

#ifndef USER_HEADER_INCLUDED
#define USER_HEADER_INCLUDED


//————————————————————————————————————————————————————————————————————————
// Game Texts
//————————————————————————————————————————————————————————————————————————

// repeated texts ////////////////////////////////////////////////////////
extern char *TXT_SYSTEMCHAT;
extern char *TXT_PLAYERCHAT;
extern char *TXT_PLRDISCONNECT;
extern char *TXT_MINIMIZEBTN;
extern char *TXT_ALLSAMETEAM;
extern char *TXT_EVERYONENEEDSTEAM;
extern char *TXT_NOTENOUGHFORTEAMS;
extern char *TXT_HELP;
extern char *TXT_GAMEOPTIONS;
extern char *TXT_GENTITLE;
extern char *TXT_NETERROR;
extern char *TXT_GENERROR;
extern char *TXT_OKAY;
extern char *TXT_CANCEL;

// game over texts ///////////////////////////////////////////////////////
#define TXT_ROUNDOVER         "round over"                             
#define TXT_1PLRSWIN          "%s has won"
#define TXT_2PLRSWIN          "%s and %s have won"
#define TXT_3PLRSWIN          "%s, %s, and %s have won"
#define TXT_WINROUND          " the round."
#define TXT_WINGAME           " the game.  Do you want to play again?"
#define TXT_NETEXIT           "Are you sure you want to exit the network game?"
#define TXT_OVERWRITE         "Are you sure you want to overwrite this file?"

// 'please wait' texts ///////////////////////////////////////////////////
#define TXT_GETTINGPLRLIST    "Getting Player List..."
#define TXT_CONNECTING        "Connecting..."
#define TXT_HOSTING           "Hosting..."

// 'invalid action' texts ////////////////////////////////////////////////
#define TXT_ENTERFILENAME     "Please enter a file name."
#define TXT_HASNOMOVES        "%s has no valid moves."
#define TXT_NOSAVEDGAMES      "No saved games were found."
#define TXT_MUSTHAVEONESONG   "There must be at least one active song."
#define TXT_INVALIDCLIPBOARD  "Invalid clipboard data."
#define TXT_NOTENOUGHPLRS     "There are not enough players to play a game."
#define TXT_ENTERSCREENNAME   "You must enter a screen name."
#define TXT_ENTERSESSIONNAME  "You must enter a session name for games using a Local Area Network."

// error texts ///////////////////////////////////////////////////////////
#define TXT_COULDNOTJOIN      "Failed to join the game,"
#define TXT_COULDNOTHOST      "Failed to host a game,"
#define TXT_NORES             "Error: Resource file could not be found."
#define TXT_NOCOM             "Error: Failed to initlize COM."
#define TXT_NODIRECTDRAW      "Error: Failed to initilize DirectDraw"
#define TXT_NODIRECTINPUT     "Error: Failed to initilize DirectInput"
#define TXT_NOPALETTE         "Error: Failed to create screen palette"

// labels ////////////////////////////////////////////////////////////////
#define TXT_FILENAME          "File Name"
#define TXT_CDTRACK           "CD Track %d"
#define TXT_COPYSESSION       "Copy the session name to the clipboard"
#define TXT_COPYIP            "Copy your IP to the clipboard"
#define TXT_STANDARDMIDI      "Standard MIDI"
#define TXT_DIRECTMUSIC       "DirectMusic"
#define TXT_CDMUSIC           "CD Music"
#define TXT_NOMUSIC           "No Music"
#define TXT_USINGLAN          "Using a Local Area Network"
#define TXT_PASTEIP           "Paste an IP or session name from the clipboard"
#define TXT_PLAYSELECTED      "Play Selected Song"
#define TXT_SHOWMENU          "hide/show the menu bar"

// descriptors ///////////////////////////////////////////////////////////
#define TXT_SAVEGAMEAS        "Save this game as:"
#define TXT_DISPLAYSESSION    "Session: %s"
#define TXT_DISPLAYIP         "Your IP: %s"
#define TXT_PLAYEROPTIONS     "Player Options:"
#define TXT_USERNAME          "User Name:"
#define TXT_CHAT              "Chat:"
#define TXT_PLAYERS           "Players:"
#define TXT_SESSION           "Session:"
#define TXT_HOSTIP            "Host IP:"
#define TXT_PASSWORD          "Password:"
#define TXT_ROLL              "Roll:"

// button texts //////////////////////////////////////////////////////////
#define TXT_NEW               "New"
#define TXT_PLAY              "Play"
#define TXT_READY             "Ready"
#define TXT_EXIT              "Exit"

// single characters /////////////////////////////////////////////////////
#define TXT_CURCHAR           "_"
#define TXT_NULLCHAR          '\0'

// string lengths ////////////////////////////////////////////////////////
#define TXTLEN_PLAYSELECTED   18
#define TXTLEN_SAVEGAMEAS     18
#define TXTLEN_ROLL           5
#define TXTLEN_PASSWORD       9
#define TXTLEN_USERNAME       10


//————————————————————————————————————————————————————————————————————————
// Definitions
//————————————————————————————————————————————————————————————————————————

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
#define VEL_SPLASH         .2
#define VEL_MOVE           .05
#define VEL_SHOWMARBLE     .03
#define VEL_HIDEMARBLE     -.03
#define VEL_GLOW           .05
#define VEL_EXPLODE        .02

// waiting times /////////////////////////////////////////////////////////
#define WAIT_NAMEFLASH     200
#define WAIT_GRAPH         10
#define WAIT_NUMUPDATE     100
#define WAIT_SPLASH        3000
#define WAIT_UTILBARS      1000
#define WAIT_SHOWMSG       500
#define WAIT_NEWNETGAME    1000
#define WAIT_CURFLASH      500

// frame numbers /////////////////////////////////////////////////////////
#define NFRAMES_NAMEFLASH  8

// bar states ////////////////////////////////////////////////////////////
#define BAR_INACTIVE       0     // should not be drawn or updated
#define BAR_IDLE           1     // minding its own busness
#define BAR_MOVING         2     // crusin' along
#define BAR_FINISHED       3     // done with the crusin'

// button definitions ////////////////////////////////////////////////////
#define BTN_MENUA          0     // is a menu button
#define BTN_MENUB          1     // dialog menu button
#define BTN_TEXT           2     // is a text button
#define BTN_CHECKBOX       3     // check box button
#define BTN_SHOWMENU       6     // show/hide menu button
#define BTN_ARROW          7     // up/down arrow button
#define BTN_PLAY           8     // play song button
#define BTN_SCROLLARROW    9     // scroll bar arrows

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
   char  text[16]; // currnent text in box
   long  slen;  // length of string in box
   UCHAR state; // state of box
   UCHAR extra; // extra data
   RECT  rdest; // surface rectangle
   RECT  rscrn; // screen rectangle
   
   long (*key)(TXT*); // used to intrpret keys customly
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
typedef void (*NUMCUST)(long, char*);


//————————————————————————————————————————————————————————————————————————
// Global Varables
//————————————————————————————————————————————————————————————————————————

// chat data /////////////////////////////////////////////////////////////
extern char gsChatText[4][80];
extern char gsChatSend[64];
extern long gsChatSendLen;

// cursor data ///////////////////////////////////////////////////////////
extern bool gCurChanged;
extern bool gCurVisible;

// utility bars //////////////////////////////////////////////////////////
extern BAR gBarTeams;
extern BAR gBarMenu;
extern BAR gBarChat;
extern BAR gBarRoll;
extern BAR gBarDice[2];
extern BAR gBarNames[4];

// window data ///////////////////////////////////////////////////////////
extern WND  gWndA;
extern WND *gWndCur;


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// user interface code ///////////////////////////////////////////////////
void GameUpdateCursor(void);
void GameUpdateBars(void);
void GameUpdateMenu(void);
void GameUpdateChat(void);
void GameUpdateName(void);
void GameUpdatePopUp(void);
void GameRenderUi(void);

// chat bar functions ////////////////////////////////////////////////////
void ChatAddText(UCHAR, char*);
void ChatDraw(void);
void ChatDrawText(void);
void ChatDrawSendText(void);
void ChatInit(void);

// menu functions ////////////////////////////////////////////////////////
void MenuEnable(UCHAR, bool);
void MenuPush(UCHAR);
void MenuDraw(void);
void MenuInit(void);

// name bar functions ////////////////////////////////////////////////////
void NameSelect(void);
void NameReGraph(UCHAR, long);
void NameDraw(void);
void NameDrawSelect(UCHAR, UCHAR);
void NameDrawGraph(UCHAR);
void NameDrawColorBar(UCHAR, long, long, long, long);
void NameDrawGreyBar(UCHAR, UCHAR, long, long, long, long);
void NameSetup(void);

// misc. utility bar functions ///////////////////////////////////////////
void BarsDrawDice(void);
void BarsDrawTeam(void);
void BarsInitA(void);
void BarsInitB(void);

// utility bar functions /////////////////////////////////////////////////
void BarShow(BAR*, bool, bool);
void BarSetEnd(BAR*, bool, bool);
void BarUpdate(BAR*);
void BarRender(BAR*);

// window functions //////////////////////////////////////////////////////
long Wnd(UCHAR, WNDLOOP);
void WndLoop(void);
void WndUpdate(void);
void WndRender(void);
void WndClose(long);
void WndDraw(WND*);
void WndReDraw(void);
void WndDrawGroup(char*, long, long, long, long);
void WndDrawLine(WND*, long);

// number control functions //////////////////////////////////////////////
void NumSetValue(NUM*, long);
void NumUpdate(NUM*);
void NumDraw(NUM*);
void NumDrawData(NUM*);

// button functions //////////////////////////////////////////////////////
void BtnEnable(BTN*, bool);
void BtnUpdate(BTN*);
void BtnDraw(BTN*);
void BtnDrawMenu(BTN*, bool, bool);

// text box functions ////////////////////////////////////////////////////
long TxtCheckKeys(char*, long*, long);
void TxtSetValue(TXT*, char*);
void TxtUpdate(TXT*);
void TxtDraw(TXT*);

// font functions ////////////////////////////////////////////////////////
long FntDraw(FNT*, LPDDS, char*, long, long, long);
long FntDrawClip(FNT*, LPDDS, char*, long, long, long);
void FntDrawCenter(FNT*, LPDDS, char*, RECT*);
void FntDrawWrap(FNT*, LPDDS, char*, RECT*);
long FntStrWidth(FNT*, char*, long);
void FntLoad(FNT*, UCHAR);
void FntDelete(FNT*);
long FntRestore(FNT*, UCHAR);

#endif // #ifndef USER_HEADER_INCLUDED