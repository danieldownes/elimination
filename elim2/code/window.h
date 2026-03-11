/*******************************************************************************/
// window.h - Header for window.cpp
//
// Written By : Justin Hoffman
// Date       : July 12, 1999 - 
/*******************************************************************************/

#ifndef WINDOW_HEADER_INCLUDED
#define WINDOW_HEADER_INCLUDED


/*-----------------------------------------------------------------------------*/
// Structure definitions
/*-----------------------------------------------------------------------------*/

// Structure for text boxes //////////////////////////////////////////////
typedef struct sTextBox
{
   LPSTR szText;     // pointer to the text to edit

   BOOL enabled;     // can this box be typed in?
   BOOL active;      // is this box selected?

   RECT rectSurface; // rectangle on the surface
   RECT rectScreen;  // rectangle on the screen
} TEXTBOX;


// Structure for text buttons ////////////////////////////////////////////
typedef struct sBtn
{
   char szText[32];  // the text that is on the button
   int location;     // location on the menubar
   int picture;      // picture on the button
   DWORD flags;      // button status flags
   RECT rectSurface; // the rectangle on the surface
   RECT rectScreen;  // the rectangle on the screen
} BTN;


// Main structure used to describe a window //////////////////////////////
typedef struct sWindow
{
   BOOL active;
   LPSTR    szTitle;       // the title of the window
   long     x, y;          // x & y pos, rel to ddsWindow or the screen depending on which window
   long     width, height; // width and height of the window 
   int      returnVal;     // the button that was pushed
   int      selBox;        // the selected text box
   int      nTxtBoxes;     // the number of text boxes
   TEXTBOX  *pTxtBoxes;    // a pointer to an array of text boxes
   BTN      buttons[3];    // an array of buttons
   DWORD    exData;        // a pointer to extra data previously allocated
   void    (*pfnMsg)(DWORD, UINT, WPARAM, LPARAM); // pointer to the windows message function
} WINDOW;


/*-----------------------------------------------------------------------------*/
// Extra data used buy windows
/*-----------------------------------------------------------------------------*/

// Get Names data ////////////////////////////////////////////////////////
typedef struct sEDat_GetNames
{
    int nPlayers;
    int teams[4];
} EDAT_GETNAMES;


// Load games data ///////////////////////////////////////////////////////
typedef struct sEDat_LoadGame
{
   int selected; // the selected game
   
   BOOL active[10];       // is the game active
   char gameName[10][32]; // the game's name
   char names[10][4][32]; // player names
   TEAM teams[10];        // the teams data
} EDAT_LOADGAME;


/*-----------------------------------------------------------------------------*/
// Function Prototypes
/*-----------------------------------------------------------------------------*/

// Window Creation Wrappers //////////////////////////////////////////////
int  Dlg_MsgBox(LPSTR, LPSTR, char*, char*);
BOOL Dlg_GetNames(void);
void Dlg_RollTurn(void);
void Dlg_SaveGame(void);
BOOL Dlg_LoadGame(void);
void Dlg_Help(int);

// Window Messaging Functions ////////////////////////////////////////////
void DlgMsg_MsgBox(DWORD, UINT, WPARAM, LPARAM);

void DlgMsg_GetNames(DWORD, UINT, WPARAM, LPARAM);
void DlgExGN_Draw(EDAT_GETNAMES*);
int  DlgExGN_GetQuad(long, long);
void DlgExGN_ChangeText(int);
void DlgExGN_CheckAct(int, EDAT_GETNAMES*);
void DlgExGN_CheckTeamA(int, EDAT_GETNAMES*);
BOOL DlgExGN_CheckTeamB(EDAT_GETNAMES*);

void DlgMsg_RollTurn(DWORD, UINT, WPARAM, LPARAM);
void DlgMsgExRT_ChangeText(char*);
void DlgMsgExRT_DisablePlayer(int, int);
void DlgMsgExRT_Done(void);

void DlgMsg_SaveGame(DWORD, UINT, WPARAM, LPARAM);

void DlgMsg_LoadGame(DWORD, UINT, WPARAM, LPARAM);
void DlgExLG_Draw(EDAT_LOADGAME*);

void DlgMsg_Help(DWORD, UINT, WPARAM, LPARAM);

// Main Window Functions /////////////////////////////////////////////////
void Win_Create(WINDOW*, char[3][32]);
void Win_CreateBtns(WINDOW*, long, long, long, long, char[3][32]);
void Win_Close(void);
void Win_Render(void);
void Win_Draw(void);
void Win_DrawHLine(WINDOW*, long);
void Win_Message(UINT, WPARAM, LPARAM);
void Win_Loop(BOOL*);

// Text Button functions /////////////////////////////////////////////////
void TxtBtn_Draw(BTN*);
void TxtBtn_MouseDown(BTN*, long, long);
void TxtBtn_MouseMove(BTN*, long, long);
BOOL TxtBtn_MouseUp(BTN*, long, long);
void TxtBtn_Enable(BTN*, BOOL);

// Text Box functions ////////////////////////////////////////////////////
void TxtBox_Create(TEXTBOX *box, LPSTR szText, RECT *rSurface, RECT *rScreen);
void TxtBox_Draw(TEXTBOX *box);
BOOL TxtBox_MouseDown(TEXTBOX *box, long x, long y);
void TxtBox_Char(TEXTBOX *box, int letter);


#endif // ifndef WINDOW_HEADER_INCLUDED