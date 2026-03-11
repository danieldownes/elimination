/*******************************************************************************/
// board.h - Function Prototype And Definitions For board.cpp
//
// Written By : Justin Hoffman
// Date       : July 19, 1999 -
/*******************************************************************************/

#ifndef BOARD_HEADER_INCLUDED
#define BOARD_HEADER_INCLUDED

#include "graph.h"


/*-----------------------------------------------------------------------------*/
// Structure Definitions
/*-----------------------------------------------------------------------------*/

// describe a marble sprite //////////////////////////////////////////////
typedef struct sSprite
{
   sSprite *next, *prev; // links to other nodes
   DWORD   lastTick;     // last time a frame was executed
   DWORD   status;       // current status of the sprite
   DWORD   wait;         // time to wait between frames
   BOOL    sound;        // should a sound be played?
   int     owner;        // owner of the sprite
   int     a, b;         // x and y locations on the board
   int     da, db;       // destination x and y locations on the board
   int     frame;        // current animation frame

   // make sure the links point to this
   sSprite() {next = prev = this;}
} SPRITE;


// describe the location of a marble /////////////////////////////////////
typedef struct sMarbleId
{
   int a, b; // x and y location on the board

   sMarbleId() {a = b = D_INVALID;}
} MARBLEID;


/*-----------------------------------------------------------------------------*/
// Macros
/*-----------------------------------------------------------------------------*/

// determine if the marble is valid //////////////////////////////////////
#define INVALID_MARBLE(a, b) ((a < 3 && b < 3) || (a < 3 && b > 12) || \
                              (a > 12 && b < 3) || (a > 12 && b > 12))

// determine if a move is valid //////////////////////////////////////////
#define INVALID_MOVE(a, b) (INVALID_MARBLE(a, b) || a < 1 || a > 14 || b < 1 || b > 14)

// determine if a marble is in the top rows //////////////////////////////
#define IS_TOP_MARBLE(a, b) (b == 0 || (b == 3 && (3 > a || a > 12)))

// convert a mouse x & y to a marble id a & b
#define MOUSE_X_TO_ID(x) (int)((x - 83) / 30)
#define MOUSE_Y_TO_ID(y) (int)((y - 5) / 20)

// find a marbles x & y
#define BOARD_MARBLE_X(a) (30 * a + 83)
#define BOARD_MARBLE_Y(b) (20 * b + 7)


/*-----------------------------------------------------------------------------*/
// Globals
/*-----------------------------------------------------------------------------*/

extern int      g_board[16][16];
extern SPRITE   g_sprBullet;
extern SPRITE   g_sprMarbles;
extern MARBLEID g_selected;
extern MARBLEID g_moves[8];


/*-----------------------------------------------------------------------------*/
// Function Prototypes
/*-----------------------------------------------------------------------------*/

// Create & Close games //////////////////////////////////////////////////
void Board_NewGame(void);
void Board_CloseGame(void);
BOOL Board_CanMove(void);
int  Board_CheckDeadPlayers(void);
BOOL Board_CheckGameOver(void);
void Board_GameOver(char*);

// Marble Functions //////////////////////////////////////////////////////
void    Marble_Draw(SPRITE *marble);
void    Marble_UpdateJump(SPRITE*);
void    Marble_MoveToBottom(SPRITE*);
void    Marble_New(BOOL, int, int, int);
void    Marble_Delete(SPRITE*);
SPRITE* Marble_Search(int, int);
void    Marble_DeleteAll(void);

// Board User Interface //////////////////////////////////////////////////
void     Marble_Select(int, int);
void     Marble_UnSelect(void);
BOOL     Marble_CheckHit(SPRITE*);
BOOL     Marble_Enemy(int, int);
void     Marble_Move(int, int, int, int);
void     Marble_DoneMoving(SPRITE *marble);
BOOL     Marble_UpdatePlayers(void);
BOOL     Marble_Free(int);
void     Board_MouseDown(long, long);
MARBLEID Board_MouseToMarble(long, long);
int      Board_FindMove(int, int);

// update board objects positions ////////////////////////////////////////
void Board_UpdateMoves(void);
void Board_UpdateGlow(void);
void Board_UpdateMarbles(void);
void Board_UpdateBullet(void);

// render board objects //////////////////////////////////////////////////
void Board_RenderMarbles(void);

// main board objects updating/rendering /////////////////////////////////
void Board_Render(void);


/*-----------------------------------------------------------------------------*/
// Inline functions
/*-----------------------------------------------------------------------------*/

// draw the bottom of a rising/sinking marble ////////////////////////////
inline void Marble_DrawBottom(long x, long y)
{
   RECT r;
   Rect_SetPoint(&r, 25, 145, 50, 150);
   DD_BltFast(ddsBack, ddsItems, x, y + 11, &r, DDBLTFAST_SRCCOLORKEY);
}


// draw the line over marbles on the top row /////////////////////////////
inline void Marble_DrawLine(long x, long y)
{
   DD_Draw_Line(ddsBack, CLR_BACKGROUND, x + 5, y + 1, x + 20, y + 1);
}

// determine the marbles x point /////////////////////////////////////////
inline long Marble_X(int a)
{
  if(a == 0) return 69;        // left
  else if(a == 15) return 547; // right
  else return 30 * a + 83;     // anything else
}


// determine the marbles y point /////////////////////////////////////////
inline long Marble_Y(int b)
{
  if(b == 0) return 4;         // top
  else if(b == 15) return 310; // bottom
  else return 20 * b + 7;      // anything else
}


#endif // ifndef BOARD_HEADER_INCLUDED