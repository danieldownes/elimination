/*******************************************************************************/
// board.cpp - Main Code For The Game Board
//
// Written By : Justin Hoffman
// Date       : July 19, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "main.h"
#include "graph.h"
#include "sound.h"
#include "board.h"
#include "computer.h"
#include "window.h"

#include "file.h"


// Globals ///////////////////////////////////////////////////////////////
int      g_board[16][16]; // the location of things on the board
SPRITE   g_sprBullet;     // the bullet sprite
SPRITE   g_sprMarbles;    // a linked list of the marble sprites
SPRITE   g_sprMoves[8];   // the possible moves sprites
MARBLEID g_selected;      // the selected marble
MARBLEID g_moves[8];      // the possible moves


/*-----------------------------------------------------------------------------*/
// Create & Close games
/*-----------------------------------------------------------------------------*/

// initilize the board and related variables /////////////////////////////
void Board_NewGame(void)
{
   int a, b;
   int i, j;

   // reset the player data
   for(i = 0; i < 4; i++)
   {
      g_marbles[i] = 10;          // set the marbles to 10
      g_score[i]   = 0;           // set the score to 0
      g_pFlags[i] &= ~PLAYER_OUT; // remove the out flag
   }

   // 'just to be safe' code
   Marble_DeleteAll();                      // delete all of the marbles
   g_selected.a = g_selected.b = D_INVALID; // clear the selected marble
   g_sprBullet.status = MARBLE_IDLE;        // set the bullets status
   g_settings &= ~SET_MARBLE_JUMPING;       // remove the marble jumping flag

   // set up the board and sprites
   for(i = 0; i < 16; i++)
   {
      for(j = 0; j < 16; j++)
      {
         // invalid marble
         if(INVALID_MARBLE(i, j)) g_board[i][j] = MARBLE_INVALID;

         // top marble (red)
         else if(j == 0 && (g_pFlags[0] & PLAYER_ACTIVE)) Marble_New(FALSE, 0, i, j);

         // right marble (teal)
         else if(i == 15 && (g_pFlags[1] & PLAYER_ACTIVE)) Marble_New(FALSE, 1, i, j);

         // bottom marble (purple)
         else if(j == 15 && (g_pFlags[2] & PLAYER_ACTIVE)) Marble_New(FALSE, 2, i, j);

         // left marble (tan)
         else if(i == 0 && (g_pFlags[3] & PLAYER_ACTIVE)) Marble_New(FALSE, 3, i, j);

         // middle marble
         else g_board[i][j] = MARBLE_SPACE;
      }
   }

   // create the sinkholes
   for(i = 0; i < g_nSinkholes; i++)
   {
      // find the next sinkhole
      do
      {
         a = RAND(14) + 1;
         b = RAND(14) + 1;
      } while(g_board[a][b] != MARBLE_SPACE);

      // set the board item
      g_board[a][b] = MARBLE_SINKHOLE;
   }
}


// close up the game /////////////////////////////////////////////////////
void Board_CloseGame(void)
{
   int i, j;

   // put the first item of the list into temp
   SPRITE *temp = g_sprMarbles.next;

   // set the statuses of the marbles to decending
   while(temp != &g_sprMarbles)
   {
      temp->frame     = 0;
      temp->sound     = FALSE;
      temp->wait      = WAIT_MARBLE_DECENDING;
      temp->status    = MARBLE_DECENDING;
      
      // move on to the next item
      temp = temp->next; 
   }

   // clear the board variable
   for(i = 0; i < 16; i++) for(j = 0; j < 16; j++) g_board[i][j] = MARBLE_INVALID;

   // clear the moves
   for(i = 0; i < 8; i++)
   {
      g_moves[i].a = g_moves[i].b = D_INVALID;
      g_sprMoves[i].status = MOVE_INACTIVE;
   }

   // clear the selected marble
   g_selected.a = g_selected.b = D_INVALID;

   // disable the bullet
   g_sprBullet.status = MARBLE_IDLE;
}


// Make sure the player can move /////////////////////////////////////////
BOOL Board_CanMove(void)
{
   // local variables
   int i, j, k, l, x, y;
   
   // dice values
   const int dice[] = {g_diceB[0].value + 1, g_diceB[1].value + 1};

   // directions to check
   const int dirX[] = {0, 1, 0, -1};
   const int dirY[] = {-1, 0, 1, 0};

   for(i = 0; i < 16; i++)
   {
      for(j = 0; j < 16; j++)
      {
         if(g_board[i][j] == g_cColor)
         {
            // check all of the directions for a valid move
            for(k = 0; k < 4; k++)
            {
               // cycle through both dice
               for(l = 0; l < 2; l++)
               {
                  x = dirX[k] * dice[l] + i;
                  y = dirY[k] * dice[l] + j;

                  // make sure value is between 1 and 14
                  if(0 < x && x < 15 && 0 < y && y < 15)
                  {
                     // see if it is a possible move
                     if(g_board[x][y] > MARBLE_INVALID) return TRUE;
                  }
               } // end for l
            } // end for k
         }
      } // end for j
   } // end for i

   // return we didn't find any moves
   return FALSE;
}


// Check to see if any players have been knocked out /////////////////////
int Board_CheckDeadPlayers(void)
{
   int flags = 0;

   for(int i = 0; i < 4; i++)
   {
      // see if the player is active, is still in the game, but has no marbles
      if((g_pFlags[i] & PLAYER_ACTIVE) && !(g_pFlags[i] & PLAYER_OUT) && g_marbles[i] == 0)
      {
         // remove the player from the game
         PInfo_KillPlayer(ColorToPlayer(i));

         // see if it was the current player
         if(i == g_cColor) flags |= 0x01;

         // check to see if the game is over
         if(Board_CheckGameOver())
         {
            flags |= 0x02;
            return flags;
         }
      }
   }

   // if 0x01 is set, the current player is out
   // if 0x02 is set, the game is over
   return flags;
}


// Check to see if the game is over //////////////////////////////////////
BOOL Board_CheckGameOver(void)
{
   char szText[256];

   int i;
   int remain = D_INVALID; // the player that remains
   int outA = 0;           // number of out players on team1
   int outB = 0;           // number of out players on team2

   // if there are no teams 
   if(g_teams.nTeamA + g_teams.nTeamB == 0)
   {
      for(i = 0; i < 4; i++)
      {
         // get the number of out players
         if((g_pFlags[i] & PLAYER_ACTIVE) && (g_pFlags[i] & PLAYER_OUT)) outA++;

         // record this player as the remaining one
         else if(g_pFlags[i] & PLAYER_ACTIVE) remain = i;
      }

      // all the players have lost, it is a draw
      if(outA >= g_nPlayers)
      {
         Board_GameOver(TXT_GO_ALL_LOSE);
         return TRUE;
      }

      // only one player remains, the game is over
      else if(outA >= g_nPlayers - 1)
      {
         wsprintf(szText, TXT_GO_ONE_WIN, g_name[remain]);
         Board_GameOver(szText);
         return TRUE;
      }
   }
   else
   {
      for(i = 0; i < 2; i++)
      {
         // get the number of out players on teamA
         if((g_pFlags[g_teams.teamA[i]] & PLAYER_ACTIVE) &&
            (g_pFlags[g_teams.teamA[i]] & PLAYER_OUT)) outA++;

         // get the number of out players on teamB
         if((g_pFlags[g_teams.teamB[i]] & PLAYER_ACTIVE) &&
            (g_pFlags[g_teams.teamB[i]] & PLAYER_OUT)) outB++;
      }

      // both teams are out
      if((outA + outB) >= g_nPlayers)
      {
         Board_GameOver(TXT_GO_ALL_LOSE);
         return TRUE;
      }

      // teamA is out
      else if(outA >= g_teams.nTeamA)
      {
         if(g_teams.nTeamA == 1) wsprintf(szText, TXT_GO_ONE_WIN,
            g_name[g_teams.teamB[0]]);

         else wsprintf(szText, TXT_GO_TWO_WIN, g_name[g_teams.teamB[0]],
            g_name[g_teams.teamB[1]]);

         Board_GameOver(szText);
         return TRUE;
      }

      // teamB is out
      else if(outB >= g_teams.nTeamB)
      {
         if(g_teams.nTeamB == 1) wsprintf(szText, TXT_GO_ONE_WIN,
            g_name[g_teams.teamA[0]]);

         else wsprintf(szText, TXT_GO_TWO_WIN, g_name[g_teams.teamA[0]],
            g_name[g_teams.teamA[1]]);

         Board_GameOver(szText);
         return TRUE;
      }
   }

   // return the game is not over
   return FALSE;
}


// tell the user the game is over ////////////////////////////////////////
void Board_GameOver(char* text)
{
   // see if the game is already over
   bool gameOver = FLAG_SET(g_settings, SET_GAME_OVER);

   // set the game over flag
   g_settings |= SET_GAME_OVER;

   // make sure the game is not already over
   if(!gameOver)
   {
      // set the game state
      g_gameState = (Dlg_MsgBox(text, "Game Over", "New Game", "Exit") == 0?
         STATE_INITGAME : STATE_SHUTDOWN);
   }

}


/*-----------------------------------------------------------------------------*/
// Marble Functions
/*-----------------------------------------------------------------------------*/

// Draw a marble /////////////////////////////////////////////////////////
void Marble_Draw(SPRITE *marble)
{
   RECT r;

   long x = Marble_X(marble->a);
   long y = Marble_Y(marble->b);

   if(IS_TOP_MARBLE(marble->a, marble->b)) Marble_DrawLine(x, y);
   
   // draw the marble
   Rect_SetDimen(&r, 25, 16 * marble->owner, 25, 16);
   DD_BltFast(ddsBack, ddsItems, x, y, &r, DDBLTFAST_SRCCOLORKEY);
}


// Update a jumping marble ///////////////////////////////////////////////
void Marble_UpdateJump(SPRITE* marble)
{
   RECT   r;
   double frame;                     // the current frame
   DWORD  thisTick = GetTickCount(); // record this tick
   int    trim     = 0;              // amout to trim off the bottom if the rectangle

   // make sure the game is not paused
   if(!(g_settings & SET_PAUSED))
   {
      // figure out how much time to add to the frame
      marble->frame += (thisTick - marble->lastTick);
   }
   
   // re-set the last tick
   marble->lastTick = thisTick;

   // figure out what frame this is
   frame = (double)(marble->frame > 500? 500 : marble->frame);

   // get the chords of the jump
   long sx = Marble_X(marble->a) + 2;
   long sy = Marble_Y(marble->b);
   long dx = Marble_X(marble->da) + 2;
   long dy = Marble_Y(marble->db);

   // get the width & height of jump
   double wi = dx - sx; // destX - sourceX
   double he = dy - sy; // destY - sourceY 
   
   // get the width & height scale of the jump
   double wiSc = wi / 500;
   double heSc = he / 500;

   // get the current x & y positions of the marble
   long x = (long)(wiSc * frame + sx);
   long y = (long)((.0012*frame*frame)-(0.6*frame)+(double)(heSc*frame))+sy;

   // make sure to trim the rect if needed so the marble doesn't hang out the bottom
   if(y > (sy - 6) && frame < 50)       trim = (sy - 5) - y;
   else if(y > (dy - 6) && frame > 450) trim = (dy - 5) - y;

   // draw the marble
   Rect_SetDimen(&r, 25, 20 * marble->owner + 64, 21, 21 + trim);
   DD_BltClip(ddsBack, ddsItems, x, y, &R_SCREEN, r, DDBLTFAST_SRCCOLORKEY);

   // draw the bottom of the marbles, if it is nessary
   if(frame < 50)  Marble_DrawBottom(sx - 2, sy);
   if(frame > 450) Marble_DrawBottom(dx - 2, dy);
   
   // if frame is equal to or greater than 1000 the marble is done moving
   if(frame >= 500)
   {

#ifdef ELIM_DEBUG
      IO_DebugWrite("move: land", (DWORD)marble);
#endif

      marble->status = MARBLE_IDLE;
      Marble_DoneMoving(marble);
   }
}


// move a marble to the bottom of the chain so it will be drawn on top ///
void Marble_MoveToBottom(SPRITE* marble)
{
   // edit the links around marble
   marble->next->prev = marble->prev;
   marble->prev->next = marble->next;

   // edit marble's links
   marble->prev = g_sprMarbles.prev;
   marble->next = &g_sprMarbles;

   // edit the 'bottom' of the loop
   g_sprMarbles.prev->next = marble;
   g_sprMarbles.prev = marble;
}


// create a new marble sprite ////////////////////////////////////////////
void Marble_New(BOOL sound, int owner, int a, int b)
{
   // add this marble to g_board
   g_board[a][b] = owner;

   // create the new marble
   SPRITE *marble = new SPRITE;

   // setup the marble structure
   marble->a         = a;
   marble->b         = b;
   marble->owner     = owner;
   marble->sound     = sound;
   marble->frame     = 0;
   marble->wait      = WAIT_MARBLE_ASCENDING;
   marble->lastTick  = GetTickCount();
   marble->status    = MARBLE_ASCENDING;

   // edit marble's pointers
   marble->next = g_sprMarbles.next;
   marble->prev = &g_sprMarbles;
   
   // insert marble into the chain
   g_sprMarbles.next->prev = marble;
   g_sprMarbles.next = marble;
}


// delete a marble ///////////////////////////////////////////////////////
void Marble_Delete(SPRITE* marble)
{
   // re-link the chain
   marble->prev->next = marble->next;
   marble->next->prev = marble->prev;

   // delete the sprite
   delete marble;
}


// search for the sprite with the matching a and b chords ////////////////
SPRITE* Marble_Search(int a, int b)
{
   // put the first item of the list into temp
   SPRITE *temp = g_sprMarbles.next;

   // search the list
   while(temp != &g_sprMarbles)
   {
      if(temp->a == a && temp->b == b) return temp;
      temp = temp->next;
   }

   // return failure
   return NULL;
}


// Clear all of the marble sprites ///////////////////////////////////////
void Marble_DeleteAll(void)
{
   SPRITE *temp = NULL, *del = NULL;

   // put the first item of the list into temp
   temp = g_sprMarbles.next;

   // start deleting
   while(temp != &g_sprMarbles)
   {
      del  = temp;       // save the item
      temp = temp->next; // move on to the next item
      delete del;        // delete the item
   }

   // set the g_sprMarbles pointers to itself
   g_sprMarbles.next = g_sprMarbles.prev = &g_sprMarbles;
}


/*-----------------------------------------------------------------------------*/
// Board User Interface
/*-----------------------------------------------------------------------------*/

// Select a marble ///////////////////////////////////////////////////////
void Marble_Select(int a, int b)
{
   char szText[128];
   char *szColor[] = {TXT_RED, TXT_TEAL, TXT_PURPLE, TXT_TAN};
   static int notTurn = 0;

   // see if it was not the current players marble
   if(g_board[a][b] != g_cColor)
   {
      DS_Play(dsoUnallowed, D_CENTERCHANNEL);
      if(++notTurn > 2)
      {
         wsprintf(szText, TXT_NOT_TURN, g_name[g_cColor], szColor[g_cColor]);
         Dlg_MsgBox(szText, TXT_GENTITLE, TXT_OK, NULL);
      }
      return;
   }

   // re-set not turn
   notTurn = 0;

   // play the sound
   DS_Play(dsoSelect, BOARD_MARBLE_X(a));

   // select the marble
   g_selected.a = a;
   g_selected.b = b;
   
   // clear the moves list
   for(int i = 0; i < 8; i++) g_moves[i].a = g_moves[i].b = D_INVALID;

   // get the new moves
   const int dirA[] = {0, 1, 0, -1}; // x direction
   const int dirB[] = {-1, 0, 1, 0}; // y direction

   // increment by two because each loop calculates two moves - die 1 and die 2
   for(i = 0; i < 4; i++)
   {
      // die 1
      if(g_diceB[0].value != D_INVALID)
      {
         g_moves[2 * i].a = dirA[i] * (g_diceB[0].value + 1) + a;
         g_moves[2 * i].b = dirB[i] * (g_diceB[0].value + 1) + b;
      }

      // die 2
      if(g_diceB[1].value != D_INVALID)
      {
         g_moves[2 * i + 1].a = dirA[i] * (g_diceB[1].value + 1) + a;
         g_moves[2 * i + 1].b = dirB[i] * (g_diceB[1].value + 1) + b;
      }
   }

   // make sure we want to show the moves
   if(g_settings & SET_SHOW_MOVES)
   {
      // setup the moves sprites
      for(i = 0; i < 8; i++)
      {
         if(INVALID_MOVE(g_moves[i].a, g_moves[i].b))
         {
            g_sprMoves[i].status = MOVE_INACTIVE;
         }
         else
         {
            // set the location of the sprite
            g_sprMoves[i].a        = g_moves[i].a;
            g_sprMoves[i].b        = g_moves[i].b;

            // set the variables in the sprites
            g_sprMoves[i].frame    = 0;
            g_sprMoves[i].status   = MOVE_SELECTING;
            g_sprMoves[i].lastTick = GetTickCount();
         }
      }
   }
}


// UnSelect a marble /////////////////////////////////////////////////////
void Marble_UnSelect(void)
{
   // unselect the marble
   g_selected.a = g_selected.b = D_INVALID;

   for(int i = 0; i < 8; i++)
   {
      // clear the moves
      g_moves[i].a = g_moves[i].b = D_INVALID;
      
      // unselect the moves if it is not being clicked
      if(g_sprMoves[i].status != MOVE_CLICKING)
         g_sprMoves[i].status = MOVE_INACTIVE;
   }
}


// check to see if a marble needs to be removed //////////////////////////
BOOL Marble_CheckHit(SPRITE* marble)
{
   int a, b;

   // find the distance moved in x and y directions
   int distA = (marble->da - marble->a);
   int distB = (marble->db - marble->b);
 
   // figure out what way the marble moved for x and y
   int dirA = distA == 0? 0 : (distA > 0? 1 : -1);
   int dirB = distB == 0? 0 : (distB > 0? 1 : -1);

   // add distA to distB because one of them should be zero
   for(int i = 0; i < abs(distA + distB); i++)
   {
      a = dirA * i + marble->a;
      b = dirB * i + marble->b;

      // make sure the marble is valid
      if(!INVALID_MARBLE(a, b))
      {
         // see if the marbles are enemies
         if(Marble_Enemy(g_cColor, g_board[a][b]))
         {

#ifdef ELIM_DEBUG
            IO_DebugWrite("bull: jump", (DWORD)&g_sprBullet);
#endif

            // initilize the bullet and set it in motion
            g_sprBullet.lastTick = GetTickCount();
            g_sprBullet.status   = MARBLE_JUMPING;
            g_sprBullet.frame    = 0;
            g_sprBullet.a        = marble->a;
            g_sprBullet.b        = marble->b;
            g_sprBullet.da       = a;
            g_sprBullet.db       = b;

            // return a marble was hit
            return TRUE;
         }
      }
   }

   // return a marble was not hit
   return FALSE;
}


// determine if the marbles are enemies //////////////////////////////////
BOOL Marble_Enemy(int color, int check)
{
    // if not even a marble
    if(check > 99) return FALSE;

    // if there are no teams, everyone else is enemy
    else if(g_teams.nTeamA + g_teams.nTeamB == 0 && color != check) return TRUE;
    
    // see if color is on teamA
    else if(g_teams.teamA[0] == color || g_teams.teamA[1] == color)
        return (g_teams.teamB[0] == check || g_teams.teamB[1] == check);
    
    // see if color is on teamB
    else if(g_teams.teamB[0] == color || g_teams.teamB[1] == color)
        return (g_teams.teamA[0] == check || g_teams.teamA[1] == check);

    // we should not ever get here
    return FALSE;
}


// Move a marble /////////////////////////////////////////////////////////
void Marble_Move(int sa, int sb, int da, int db)
{
#ifdef ELIM_DEBUG
   extern error_detect;
   if(g_settings & SET_MARBLE_JUMPING) error_detect = TRUE;
#endif

   // set the marble_moving flag
   g_settings |= SET_MARBLE_JUMPING;

   // find the marble in the linked list
   SPRITE* marble = Marble_Search(sa, sb);

   // set the destination points
   marble->da = da;
   marble->db = db;

   // setup the marble to initilize a jump
   marble->owner    = g_board[sa][sb];
   marble->status   = MARBLE_INIT_JUMP;

   // move the marble to the bottom of the list so it will be drawn on top
   Marble_MoveToBottom(marble);
}


// Preform tasks after moving a marble ///////////////////////////////////
void Marble_DoneMoving(SPRITE *marble)
{
   BOOL marbleMade = FALSE;                     // TRUE if a new marble was made
   int  save = g_board[marble->da][marble->db]; // save value of the space landed on
   int  distance;                               // the distance of the jump

   //////////////////////////////////////////////////////////////////
   // Sink the marble
   if(save == MARBLE_SINKHOLE)
   {
#ifdef ELIM_DEBUG
      IO_DebugWrite("sink: start", (DWORD)marble);
#endif ELIM_DEBUG

      // setup the marble to sink
      marble->lastTick = GetTickCount();
      marble->frame    = 0;
      marble->sound    = TRUE;
      marble->status   = MARBLE_DECENDING;
      marble->wait     = WAIT_MARBLE_DECENDING;
      
      // update the player graph
      PInfo_UpdateGraph(g_cPlayer, -1);
   }


   //////////////////////////////////////////////////////////////////
   // Edit the variables

   // find the distance of the jump, add both ^x and ^y because 1 will be zero
   distance = abs((marble->da - marble->a) + (marble->db - marble->b));

   // set the marble souce
   g_board[marble->a][marble->b] = MARBLE_SPACE;

   // set the board item to its new owner
   if(g_board[marble->da][marble->db] == MARBLE_SINKHOLE)
      g_board[marble->da][marble->db] = MARBLE_SPACE;
   else
      g_board[marble->da][marble->db] = marble->owner;

   // set the marbles location
   marble->a = marble->da;
   marble->b = marble->db;


   //////////////////////////////////////////////////////////////////
   // Create a new marble, see if one was really made
   if(save == MARBLE_FREE) marbleMade = Marble_Free(marble->a);
   

   //////////////////////////////////////////////////////////////////
   // Update the dice

   // disable the appropriate die
   if(distance == g_diceB[0].value + 1)
   {
      // set the value to invalid
      g_diceB[0].value = D_INVALID;                 
      
      // change the source rectangle
      Rect_SetDimen(&g_diceB[0].rectSource, 20 * g_cColor, 120, 21, 21); 
   }
   else if(distance == g_diceB[1].value + 1)
   {
      // set the value to invalid
      g_diceB[1].value = D_INVALID;

      // change the source rectangle
      Rect_SetDimen(&g_diceB[1].rectSource, 20 * g_cColor, 120, 21, 21);
   }


   //////////////////////////////////////////////////////////////////
   // Set things back up
   
   // if the marble is sinking, we have nothing left to do for now,
   // it will be delt with when the marble sinks.
   // also, this should make sure that there is a free marble forming
   if(save != MARBLE_SINKHOLE && !marbleMade) Marble_UpdatePlayers();
}


// update the players statuses ///////////////////////////////////////////
BOOL Marble_UpdatePlayers(void)
{
#ifdef ELIM_DEBUG
   IO_DebugWrite("call: UpdatePlayers", 0);
#endif

   int result;
   char szText[128];

   // remove the marble moving flag
   g_settings &= ~SET_MARBLE_JUMPING;

   // Take out players if needed, make sure to change
   // players if the current player has been wiped out.
   result = Board_CheckDeadPlayers();

   // the game is over
   if(result & 0x02) return FALSE;
   
   // the current player is out. NEVER make goNext un-true if it is true
   if(result & 0x01)
   {
      NextPlayer();

      // no more changes to these marbles and players lives need to be made
      return TRUE;
   }

   // see if we should move on to the next player
   if(g_diceB[0].value == D_INVALID && g_diceB[1].value == D_INVALID)
   {
      NextPlayer();
   }
   else
   {
      // see what should be done about the next marble
      // see if the player has any valid moves
      if(!Board_CanMove())
      {
         // display a different message for a computer player
         if(g_pFlags[g_cColor] & PLAYER_IS_PC)
            wsprintf(szText, TXT_NO_PC_MOVES, g_name[g_cColor]);
         else wsprintf(szText, TXT_NO_MOVES, g_name[g_cColor]);
         
         Dlg_MsgBox(szText, TXT_GENTITLE, "OK", NULL);
         NextPlayer();
         return TRUE;
      }

      // move the second marble
      PC_ExecuteMove();
   }

   // return the game didn't end
   return TRUE;
}


// create a free marble //////////////////////////////////////////////////
BOOL Marble_Free(int landedA)
{
   // locals
   char szText[128];
   int zero    = 0;
   int fifteen = 15;
   int *i, *j;      // points to the vars containing x & y pos of marble
   int count;       // used to cycle through 'behind the line'
   int chosen;      // the randomly chosen marble
   int index = 0;   // the current number of found marbles
   int list[10][2]; // a list of places to put a free guy


   switch(g_cColor)
   {
   // figure out how the counters will work
   case 0: i = &count;   j = &zero;    break; // top (red)
   case 1: i = &fifteen; j = &count;   break; // right (teal)
   case 2: i = &count;   j = &fifteen; break; // bottom (purple)
   case 3: i = &zero;    j = &count;   break; // left (tan)
   }

   // make a list of places to put the new marble
   for(count = 3; count < 13; count++)
   {
      if(g_board[*i][*j] == MARBLE_SPACE)
      {
         list[index][0] = *i;
         list[index][1] = *j;
         index++;
      }
   }

   // update the score
   PInfo_UpdateScore(g_cPlayer, SCORE_FREE);

   // see if there was NOT a place for the marble to go
   if(index == 0)
   {
      // make sure it is a human player to say the text
      if(!(g_pFlags[g_cColor] & PLAYER_IS_PC))
      {
         wsprintf(szText, TXT_NO_NEW_MARBLE, g_name[g_cColor]);
         Dlg_MsgBox(szText, TXT_GENTITLE, TXT_OK, NULL);
      }

      // return that no marble was made
      return FALSE;
   }

   // play the sound
   DS_Play(dsoFree, BOARD_MARBLE_X(landedA));

   // update the graph
   PInfo_UpdateGraph(g_cPlayer, 1);

   // create the new marble
   chosen = RAND(index);
   Marble_New(TRUE, g_cColor, list[chosen][0], list[chosen][1]);

   // return that we made a marble
   return TRUE;
}


// The mouse was pushed down on the board ////////////////////////////////
void Board_MouseDown(long x, long y)
{
   int move;

   // if the game is not idle; don't allow mouse clicks
   if(GameIsBusy()) return;

   // get the marble that was clicked on
   MARBLEID marble = Board_MouseToMarble(x, y);

   if(!INVALID_MARBLE(marble.a, marble.b))
   {
      switch(g_board[marble.a][marble.b])
      {
      case 0:
      case 1:
      case 2:
      case 3:
         // unselect the marble if it is already selected
         if(marble.a == g_selected.a && marble.b == g_selected.b) Marble_UnSelect();
         else Marble_Select(marble.a, marble.b);
         break;

      case MARBLE_SINKHOLE:
      case MARBLE_FREE:
      case MARBLE_SPACE:
         move = Board_FindMove(marble.a, marble.b);
         if(move != D_INVALID)
         {
            // set the status of the move to clicking
            if(g_settings & SET_SHOW_MOVES) g_sprMoves[move].status = MOVE_CLICKING;

            // move the marble
            Marble_Move(g_selected.a, g_selected.b, marble.a, marble.b);
            Marble_UnSelect();
         }
         break;
      }
   }
   else Marble_UnSelect();
}


// Get the marble that was clicked on ////////////////////////////////////
MARBLEID Board_MouseToMarble(long x, long y)
{
   MARBLEID res;
   RECT ra, rb, rc, rd, re;

   Rect_SetPoint(&ra, 173,   4, 467,  19); // top
   Rect_SetPoint(&rb, 547,  67, 571, 262); // right
   Rect_SetPoint(&rc, 173, 310, 467, 325); // bottom
   Rect_SetPoint(&rd,  69,  67,  93, 262); // left
   Rect_SetPoint(&re, 113,  27, 527, 302); // middle

   if(Rect_Within(&ra, x, y))      {res.a = MOUSE_X_TO_ID(x); res.b = 0;}
   else if(Rect_Within(&rb, x, y)) {res.a = 15;               res.b = MOUSE_Y_TO_ID(y);}
   else if(Rect_Within(&rc, x, y)) {res.a = MOUSE_X_TO_ID(x); res.b = 15;}
   else if(Rect_Within(&rd, x, y)) {res.a = 0;                res.b = MOUSE_Y_TO_ID(y);}
   else if(Rect_Within(&re, x, y)) {res.a = MOUSE_X_TO_ID(x); res.b = MOUSE_Y_TO_ID(y);}

   return res;
}


// search through the moves to see if we can find a match ////////////////
int Board_FindMove(int a, int b)
{
   for(int i = 0; i < 8; i++)
   {
      if(g_moves[i].a == a && g_moves[i].b == b &&
         !INVALID_MOVE(g_moves[i].a, g_moves[i].b)) return i;
   }

   // return no move was found
   return D_INVALID;
}


/*-----------------------------------------------------------------------------*/
// update board objects positions
/*-----------------------------------------------------------------------------*/

// update the possible moves /////////////////////////////////////////////
void Board_UpdateMoves(void)
{
   RECT r;

   // get the current tick
   DWORD thisTick = GetTickCount();

   // go through all of the sprites
   for(int i = 0; i < 8; i++)
   {
      switch(g_sprMoves[i].status)
      {
      // move the frame though the selecting frames
      case MOVE_SELECTING:
         if(thisTick - g_sprMoves[i].lastTick >= WAIT_MOVE_SELECT)
         {
            g_sprMoves[i].lastTick = thisTick;
            if(++g_sprMoves[i].frame > 7)
               g_sprMoves[i].status = MOVE_IDLE;
         }
         break;

      // move through the clicking frames
      case MOVE_CLICKING:
         if(thisTick - g_sprMoves[i].lastTick >= WAIT_MOVE_SELECT)
         {
            g_sprMoves[i].lastTick = thisTick;
            if(++g_sprMoves[i].frame > 12)
               g_sprMoves[i].status = MOVE_INACTIVE;
         }
         break;
      }
      
      // draw the move
      if(g_sprMoves[i].status != MOVE_INACTIVE)
      {
         Rect_SetDimen(&r, 0, g_sprMoves[i].frame * 11, 25, 11);
         DD_BltFast(ddsBack, ddsItems, BOARD_MARBLE_X(g_sprMoves[i].a),
            BOARD_MARBLE_Y(g_sprMoves[i].b) + 5, &r, DDBLTFAST_SRCCOLORKEY);
      }
   }
}


// update the glowing 'f's ///////////////////////////////////////////////
void Board_UpdateGlow(void)
{
   static DWORD frame = 0;
   static DWORD lastTick = GetTickCount();

   int i, j;
   int color;
   RECT r = {50, 45, 61, 58};
   PALETTEENTRY glow;

   // draw all of the 'f's
   for(i = 0; i < 16; i++)
   {
      for(j = 0; j < 16; j++)
      {
         if(g_board[i][j] == MARBLE_FREE)
         {
            DD_BltFast(ddsBack, ddsItems, BOARD_MARBLE_X(i) + 8,
               BOARD_MARBLE_Y(j), &r, DDBLTFAST_SRCCOLORKEY);
         }
      }
   }

   // change the palette entry
   if(GetTickCount() - lastTick >= WAIT_GLOW)
   {
      // edit the time
      lastTick = GetTickCount();

      // edit the frame
      if(++frame > 50) frame = 0;

      // create the color
      color = frame > 25? 50 - frame : frame;
      glow.peRed   = color;
      glow.peGreen = 7 * color;
      glow.peBlue  = 10 * color;
      glow.peFlags = PC_NOCOLLAPSE;

      // edit the palette
      ddpMain->SetEntries(0, CLR_FREEGUYS, 1, &glow);
   }
}


// update all of the marbles /////////////////////////////////////////////
void Board_UpdateMarbles(void)
{
   // if the game is paused, don't update the marbles
   if(g_settings & SET_PAUSED) return;

   long    x;                         // used for panning of sound
   int     result   = 0;              // is the current player out? is the game over?
   BOOL    check    = FALSE;          // check the game over
   DWORD   thisTick = GetTickCount(); // store the curren time
   SPRITE *temp     = &g_sprMarbles;  // put the first item into temp
   SPRITE *save     = NULL;           // use this to delete items

   // loop through all of the marbles
   do
   {
      // move on to the next item
      temp = temp->next;

      // advance the frame if needed
      if((temp->status == MARBLE_ASCENDING ||
          temp->status == MARBLE_DECENDING ||
          temp->status == MARBLE_EXPLODING) &&
          thisTick - temp->lastTick >= temp->wait)
      {
         temp->lastTick = thisTick;
         temp->frame++;
      }

      // get the marbles x location for sounds
      x = Marble_X(temp->a);

      switch(temp->status)
      {
      // draw the marble moving up
      case MARBLE_ASCENDING:
         if(temp->frame > 15)
         {
            // we are done moving
            temp->status = MARBLE_IDLE;
            if(temp->sound)
            {
               // play the sound and setup the next marble
               DS_Play(dsoMarbleUp, x);
               if(Marble_UpdatePlayers()) return;
            }
         }
         break;

      // draw the marble going down
      case MARBLE_DECENDING:
         if(temp->frame > 14)
         {  
            check = temp->sound;

#ifdef ELIM_DEBUG
            if(check) IO_DebugWrite("sink: end", (DWORD)temp);
#endif

            // delete the marble
            save = temp;
            temp = temp->next;
            Marble_Delete(save);

            // check over the game's status if it is a single marble
            if(check)
            {
               DS_Play(dsoSink, x); 
               if(Marble_UpdatePlayers()) return;
            }
         }
         break;

      // draw the marble exploding
      case MARBLE_EXPLODING:
         if(temp->frame > 11)
         {
#ifdef ELIM_DEBUG
            IO_DebugWrite("exp: finish", (DWORD)temp);
#endif

            // delete the marble
            save = temp;
            temp = temp->next;
            Marble_Delete(save);
         }
         break;

      // initilize the marble jumping
      case MARBLE_INIT_JUMP:
         if(Marble_CheckHit(temp))
         {
#ifdef ELIM_DEBUG
            IO_DebugWrite("move: check", (DWORD)temp);
#endif

            // we can't jump just yet, the bullet is still moving.
            // this marble will be signaled when the bullet is done
            temp->status = MARBLE_IDLE;
         }
         else
         {
#ifdef ELIM_DEBUG
            IO_DebugWrite("move: jump", (DWORD)temp);
#endif

            // start the marble jumping
            temp->status   = MARBLE_JUMPING;
            temp->lastTick = thisTick;
            temp->frame    = 0;
         }
         break;
      }      
   } while(temp->next != &g_sprMarbles);
}


// update the bullet object //////////////////////////////////////////////
void Board_UpdateBullet(void)
{
   RECT r;
   SPRITE *hit;                     // the marble that was hit
   DWORD thisTick = GetTickCount(); // get the current tick

   // make sure the game is not paused
   if(!(g_settings & SET_PAUSED))
   {
      // figure out what to add to the frame count
      g_sprBullet.frame += (thisTick - g_sprBullet.lastTick);
   }

   // re-set lastTick
   g_sprBullet.lastTick = thisTick;

   // get what frame we are on
   double frame = (double)(g_sprBullet.frame > 500? 500 : g_sprBullet.frame);

   // marble source and dest chords
   long sx = Marble_X(g_sprBullet.a) + 10;
   long sy = Marble_Y(g_sprBullet.b) - 2;
   long dx = Marble_X(g_sprBullet.da) + 10;
   long dy = Marble_Y(g_sprBullet.db) - 2;

   double wi = dx - sx; // destX - sourceX
   double he = dy - sy; // destY - sourceY 

   double wiSc = wi / 500;
   double heSc = he / 500;

   long x = (long)(wiSc * frame + sx);
   long y = (long)((.0012*frame*frame)-(0.6*frame)+(heSc*frame)) + sy;

   // draw the bullet
   Rect_SetPoint(&r, 50, 58, 55, 63);
   DD_BltClip(ddsBack, ddsItems, x, y, &R_SCREEN, r, DDBLTFAST_SRCCOLORKEY);

   // check the bullet's frame
   if(g_sprBullet.frame >= 500)
   {
#ifdef ELIM_DEBUG
      IO_DebugWrite("bull: land", (DWORD)&g_sprBullet);
#endif

      // find the marble that was hit
      hit = Marble_Search(g_sprBullet.da, g_sprBullet.db);

      // set the marble to explode
      hit->frame    = 0;
      hit->lastTick = thisTick;
      hit->wait     = WAIT_EXPLODE;
      hit->status   = MARBLE_EXPLODING;

      // play the exploding sound
      DS_Play(dsoExplode, BOARD_MARBLE_X(g_sprBullet.da));

      // clear the marble from g_board
      g_board[g_sprBullet.da][g_sprBullet.db] = MARBLE_FREE;

      // change the graph
      PInfo_UpdateGraph(ColorToPlayer(hit->owner), -1);

      // change the score
      PInfo_UpdateScore(g_cPlayer, SCORE_HIT);

      // attempt to jump again
      Marble_Search(g_sprBullet.a, g_sprBullet.b)->status = MARBLE_INIT_JUMP;
      
      // de-activate the bullet
      g_sprBullet.status = MARBLE_IDLE;
   }
}

/*-----------------------------------------------------------------------------*/
// render board objects
/*-----------------------------------------------------------------------------*/

// render the marbles ////////////////////////////////////////////////////
void Board_RenderMarbles(void)
{
   RECT r;
   long x, y;

   // put the first item into temp
   SPRITE *temp = g_sprMarbles.next;

   // loop through all of the marbles
   while(temp != &g_sprMarbles)
   {
      // get the marbles x and y points
      x = Marble_X(temp->a);
      y = Marble_Y(temp->b);

      switch(temp->status)
      {
      // draw the marble moving up
      case MARBLE_ASCENDING:
         Rect_SetDimen(&r, 25, 20 * temp->owner + 64, 21, temp->frame + 1);
         DD_BltFast(ddsBack, ddsItems, x + 2, y + (15 - temp->frame), &r, DDBLTFAST_SRCCOLORKEY);

         // draw the bottom of the marble
         Marble_DrawBottom(x, y);
         break;

      // draw the marble going down
      case MARBLE_DECENDING:
         Rect_SetDimen(&r, 25, 20 * temp->owner + 64, 21, (16 - temp->frame));
         DD_BltFast(ddsBack, ddsItems, x + 2, y + temp->frame, &r, DDBLTFAST_SRCCOLORKEY);

         // draw the bottom of the marble
         Marble_DrawBottom(x, y);
         break;

      // draw the marble exploding
      case MARBLE_EXPLODING:
         // draw the marble if needed
         if(temp->frame < 6) Marble_Draw(temp);

         // draw the explotion
         Rect_SetDimen(&r, 0, 24 * temp->frame, 29, 25);
         DD_BltFast(ddsBack, ddsExplode, x - 2, y - 7, &r, DDBLTFAST_SRCCOLORKEY);
         break;

      // draw the jump
      case MARBLE_JUMPING:
         Marble_UpdateJump(temp);
         break;

      // the marble is not moving
      case MARBLE_INIT_JUMP:
      case MARBLE_IDLE:
         Marble_Draw(temp);
         break;
      }
      
      // move on to the next item
      temp = temp->next;
   }
}


/*-----------------------------------------------------------------------------*/
// Main board objects updating/rendering
/*-----------------------------------------------------------------------------*/

// Render the board to the screen ////////////////////////////////////////
void Board_Render(void)
{
   static DWORD frame    = 0; // frame count for selected marble
   static DWORD lastTick = GetTickCount(); // last selected marble frame

   // source rectangle of objects
   RECT r = {0, 0, 517, 344};

   // draw the board
   DD_BltFast(ddsBack, ddsBoard, 62, 5, &r, DDBLTFAST_SRCCOLORKEY);

   // Update the possible moves
   Board_UpdateMoves();

   // update the glowing 'f'
   Board_UpdateGlow();

   // update the marbles
   Board_UpdateMarbles();
   Board_RenderMarbles();

   // update the bullet
   if(g_sprBullet.status == MARBLE_JUMPING) Board_UpdateBullet();

   // Update the selected marble
   if(!INVALID_MARBLE(g_selected.a, g_selected.b))
   {
      if(GetTickCount() - lastTick >= 25)
      {
         lastTick = GetTickCount();   // reset the time
         if(++frame > 23) frame = 0; // advance the frame
      }

      // draw the current frame
      Rect_SetDimen(&r, 0, frame * 19, 27, 19);
      DD_BltFast(ddsBack, ddsSelect, Marble_X(g_selected.a) - 1,
         Marble_Y(g_selected.b) - 2, &r, DDBLTFAST_SRCCOLORKEY);
   }
}