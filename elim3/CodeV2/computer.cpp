/*******************************************************************************/
// computer.cpp - Main Code For The Game Board
//
// Written By : Justin Hoffman
// Date       : July 31, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "game.h"
#include "window.h"
#include "computer.h"

// Definitions ///////////////////////////////////////////////////////////
#define PC_MAX_HIT   4 // maximum hit accepted


//————————————————————————————————————————————————————————————————————————
// Main Game Programming Interface
//————————————————————————————————————————————————————————————————————————

// master move executor //////////////////////////////////////////////////
void PcExecuteMove(void)
{
   //////////////////////////////////////////////////////////////////
   // Local Variables

   AI_MOVE  moves[320];             // the main list of moves
   AI_MOVE* offMoves[320] = {NULL}; // sorts the main list for offensive moves
   AI_MOVE* defMoves[320] = {NULL}; // sorts the main list for defensive moves
   AI_MOVE* freMoves[320] = {NULL}; // sorts the main list for free moves
   AI_MOVE* choosen;                // the chosen move

   int maxOffHit; // maximum offensive hit
   int maxDefHit; // maximum defensice hit
   int maxJumped; // maximum number hit

   int i, j, k, l; // looping variables
   int x, y;       // destination move chords
   int index = 0;  // list index counter

   const int dirX[] = {0, 1, 0, -1}; // direction to move per loop in x units
   const int dirY[] = {-1, 0, 1, 0}; // direction to move per loop in y units
   const int dice[] = {gBarDice[0].extra + 1, gBarDice[1].extra + 1}; // dice values


   //////////////////////////////////////////////////////////////////
   // Create A List of moves
   
   for(i = 0; i < 16; i++) {
      for(j = 0; j < 16; j++) {

         // see if the board item matches the current player
         if(gBoard[i][j] == pcur) {

            // loop through all 4 directions
            for(k = 0; k < 4; k++) {

               // loop through both the dice
               for(l = 0; l < 2; l++) {

                  // figure out where the move will go
                  x = dirX[k] * dice[l] + i;
                  y = dirY[k] * dice[l] + j;

                  // it is on the board
                  if(0 < x && x < 15 && 0 < y && y < 15) {

                     // it is a valid move
                     if(gBoard[x][y] > 3) {

                        // set the moves chords
                        moves[index].sx = i;
                        moves[index].sy = j;
                        moves[index].dx = x;
                        moves[index].dy = y;
                        
                        // see if the move lands on a free guy
                        moves[index].free = (gBoard[x][y] == MARBLE_FREE);

                        // figure out who i will jump and how many
                        GetJumped(i, j, x, y, &moves[index].nJumped, moves[index].jumped);
                        
                        // figure the prob of getting hit if i stay
                        GetHitProb(i, j, &moves[index].nDefHit,
                           &moves[index].pDefHit, moves[index].defHit,
                           moves[index].jumped, 0); // last param is zero so we dont check it

                        // figure the prov of getting hit if i move
                        GetHitProb(x, y, &moves[index].nOffHit,
                           &moves[index].pOffHit, moves[index].offHit,
                           moves[index].jumped, moves[index].nJumped);

                        // move on to the next index value   
                        index++;
                     }
                  }
               } // end for l
            } // end for k
         }
      } // end for j
   } // end for i


   //////////////////////////////////////////////////////////////////
   // Sort the moves
   maxOffHit = GetMaxOffHit(moves, index);
   maxDefHit = GetMaxDefHit(moves, index);
   maxJumped = GetMaxJumped(moves, index);

   SortOff(moves, offMoves, index, maxOffHit, maxDefHit, maxJumped);
   SortDef(moves, defMoves, index, maxOffHit, maxDefHit, maxJumped);
   SortFre(moves, freMoves, index, maxOffHit, maxDefHit, maxJumped);

   //////////////////////////////////////////////////////////////////
   // Choose a move

   // error, no moves were found
#ifdef GAME_DEBUG
   if(index <= 0) {
      DebugWrite("computer unable to locate any moves");
      return;
   }
#endif

   // attemp to find a good move
   if((choosen = Move_LDefence(defMoves, index, maxDefHit)) == NULL)         // light defence
      if((choosen = Move_Offence(offMoves, index, maxOffHit)) == NULL)       // offence
         if((choosen = Move_Free(freMoves, index)) == NULL)                  // free moves
            if((choosen = Move_HDefence(defMoves, index, maxDefHit)) == NULL)// heavy defence move
               choosen = &moves[RAND(index)];                                // random move

   // check for error in the move
#ifdef GAME_DEBUG
   if(gBoard[choosen->sx][choosen->sy] != pcur) {
      DebugWrite("The AI was trying to move an invalid marble");
      return;
   }
#endif

   // move the marble
   GameMoveMarbleEx(choosen->sx, choosen->sy, choosen->dx, choosen->dy);
}


//————————————————————————————————————————————————————————————————————————
// Probability Calculations
//————————————————————————————————————————————————————————————————————————

// find the location of marbles and probability of getting jumped there //
static void GetHitProb(
int sx,         // x location of this marble
int sy,         // y location of this marble
int* num,       // number of marbles that might hit this marble
int* prob,      // probablilty any marbles will hit this marble
int loc[20][2], // marbles that could hit this move
int hit[5][2],  // marbles this move will hit
int nHit)       // number of moves this marble will hit
{
   int i, j;      // looping variables
   int x, y;      // chord variables
   int index = 0; // current index of loc
   const int dirX[] = {0, 1, 0, -1}; // direction to move per loop in x units
   const int dirY[] = {-1, 0, 1, 0}; // direction to move per loop in y units
   
   // set the probablity to zero
   *prob = 0;
   *num  = 0;

   // if the marble is behind the line, it can not be hit
   if(1 > sx || sx > 14 || 1 > sy || sy > 14)
   {
      // clear the locations
      for(i = 0; i < 20; i++) for(j = 0; j < 2; j++) loc[i][j] = -1;

      // no further calculations are needed
      return;
   }

   // look for hits in all directions
   for(i = 0; i < 4; i++)
   {
      for(j = 0; j < 5; j++)
      {
         // get the x & y of marble we are to look at
         x = dirX[i] * j + sx;
         y = dirY[i] * j + sy;

         // make sure the marble is even on the board
         if(-1 < x && x < 16 && -1 < y && y < 16)
         {
            // see if the marble is an enemy
            if(gBoard[x][y] != MARBLE_INVALID && gBoard[x][y] < 4)
            {
               if(MarbleIsEnemy(pcur, gBoard[x][y]))
               {
                  // record the location
                  loc[index][0] = x;
                  loc[index][1] = y;
               
                  // add the probabliliy of this marble hitting it
                  if(!IsInHitList(x, y, nHit, hit))
                     *prob += GetHitProbB(sx, sy, x, y);

                  // move on to next index item
                  index++;
               }
            }
         }
      } // end for j      
   } // end for i

   // set the number to the proper value
   *num = index;
}


// if marble will be jumped, it souldn't be considered dangerous /////////
BOOL IsInHitList(int x, int y, int nHit, int hit[5][2])
{
   // go through the list looking for a match
   for(int i = 0; i < nHit; i++) if(hit[i][0] == x && hit[i][1] == y) return TRUE;

   // no match found
   return FALSE;
}


// calculate the probablility of marbleB jumping marbleA /////////////////
static int GetHitProbB(int ax, int ay, int bx, int by)
{
   int distMarble;   // the distance between the marbles
   int distEdge;     // the distance frome the edge of the board

   // get the distance of marbleA from marbleB, add ^x & ^y because one is zero
   distMarble = 6 - abs((bx - ax) + (by - ay));

   // get the distance of marbleA from the edge oppisite marbleB
   if(ay == by) // the move is horisontal
   { 
      if(bx > ax) distEdge = ax - 1; // check from the left
      else distEdge = 14 - ax;       // check from the right
   }
   else
   {
      if(by > ay) distEdge = ay - 1; // check from the top
      else distEdge = 14 - ay;       // check from the bottom
   }

   // return the smaller number
   return (distEdge > distMarble? distMarble : distEdge);
}


// get the location and number of marbles jumped /////////////////////////
static void GetJumped(int sx, int sy, int dx, int dy, int* num, int loc[5][2])
{
   int index = 0; // indexing variable
   int x, y;      // location variables

   // find the distance travilied in x & y units
   int distX = dx - sx;
   int distY = dy - sy;

   // calculate the direction from sx & sy in x & y units
   int dirX = distX == 0? 0 : (distX > 0? 1 : -1);
   int dirY = distY == 0? 0 : (distY > 0? 1 : -1);

   // find the location of enemy marbles. add distX + distY because one will be zero
   for(int i = 0; i < abs(distX + distY); i++)
   {
      // find the x & y of what we are to look at
      x = dirX * i + sx;
      y = dirY * i + sy;

      // see if the marble is an enemy
      if(gBoard[x][y] != MARBLE_INVALID && gBoard[x][y] < 4)
      {
         if(MarbleIsEnemy(pcur, gBoard[x][y]))
         {
            // record the location
            loc[index][0] = x;
            loc[index][1] = y;

            // go to next index
            index++;
         }
      }
   }

   // set the number of jumped marbles
   *num = index;
}


//————————————————————————————————————————————————————————————————————————
// Get Maximums
//————————————————————————————————————————————————————————————————————————

// Get the maximum offense hit in the list ///////////////////////////////
static int GetMaxOffHit(AI_MOVE moves[320], int nMoves)
{
   int max = 0;

   // see if this item is greater than max
   for(int i = 0; i < nMoves; i++) if(moves[i].pOffHit > max) max = moves[i].pOffHit;

   return max;
}


// Get the maximum defence hit in the list ///////////////////////////////
static int GetMaxDefHit(AI_MOVE moves[320], int nMoves)
{
   int max = 0;

   // see if this item is greater than max
   for(int i = 0; i < nMoves; i++) if(moves[i].pDefHit > max) max = moves[i].pDefHit;

   return max;
}


// get the maximum jumped number in the list /////////////////////////////
static int GetMaxJumped(AI_MOVE moves[320], int nMoves)
{
   int max = 0;

   // see if this item is greater than max
   for(int i = 0; i < nMoves; i++) if(moves[i].nJumped > max) max = moves[i].nJumped;

   return max;
}


//————————————————————————————————————————————————————————————————————————
// Move Sorting
//————————————————————————————————————————————————————————————————————————

// Sort the offensive moves //////////////////////////////////////////////
static void SortOff(
AI_MOVE  moves[320],
AI_MOVE* offMoves[320],
int nMoves,    // number of moves
int maxOffHit, // maximum offensive hit
int maxDefHit, // maximum defensive hit
int maxJumped) // maximum number jumped
{
   int i, j, k, l, m, index = 0; // counters

   // sort by maxJumped (greatest to least)
   for(i = maxJumped; i > -1; i--)
   {
      // sort by pOffHit (least to greatest)
      for(j = 0; j <= maxOffHit; j++)
      {
         // sort by pDefHit (greatest to least)
         for(k = maxDefHit; k > -1; k--)
         {
            // sort by <if free>, free comes first
            for(l = 1; l > -1; l--)
            {
               // go through all of the moves
               for(m = 0; m < nMoves; m++)
               {
                  // make sure all of the criteria is met
                  if(moves[m].free == l && moves[m].pDefHit == k &&
                     moves[m].pOffHit == j && moves[m].nJumped == i)
                  {
                     offMoves[index++] = &moves[m];
                  }
               } // end for m
            } // end for l
         } // end for k
      } // end for j
   } // end for i
}


// Sort the defensive moves //////////////////////////////////////////////
static void SortDef(
AI_MOVE  moves[320],
AI_MOVE* defMoves[320],
int nMoves,    // number of moves
int maxOffHit, // maximum offensive hit
int maxDefHit, // maximum defensive hit
int maxJumped) // maximum number jumped
{
   int i, j, k, l, m, index = 0; // counters

   // sort by nDefHit (greatest to least)
   for(i = maxDefHit; i > -1; i--)
   {
      // sort by nOffHit (least to greatest)
      for(j = 0; j <= maxOffHit; j++)
      {
         // sort by maxJumped (greatest to least)
         for(k = maxJumped; k > -1; k--)
         {
            // sort by <if free>, free comes first
            for(l = 1; l > -1; l--)
            {
               // go through all of the moves
               for(m = 0; m < nMoves; m++)
               {
                  // make sure all of the criteria is met
                  if(moves[m].free == l && moves[m].nJumped == k &&
                     moves[m].pOffHit == j && moves[m].pDefHit == i)
                  {
                     defMoves[index++] = &moves[m];
                  }
               } // end for m
            } // end for l
         } // end for k
      } // end for j
   } // end for i
}


// Sort the 'Free' moves /////////////////////////////////////////////////
static void SortFre(
AI_MOVE  moves[320],
AI_MOVE* offMoves[320],
int nMoves,    // number of moves
int maxOffHit, // maximum offensive hit
int maxDefHit, // maximum defensive hit
int maxJumped) // maximum number jumped
{
   int i, j, k, l, m, index = 0; // counters

   // sort by <if free>, free comes first
   for(i = 1; i > -1; i--)
   {
      // sort by pOffHit (least to greatest)
      for(j = 0; j <= maxOffHit; j++)
      {
         // sort by pDefHit (least to greatest)
         for(k = maxDefHit; k > -1; k--)
         {
            // sort by maxJumped (greatest to least)
            for(l = maxJumped; l > -1; l--)
            {
               // go through all of the moves
               for(m = 0; m < nMoves; m++)
               {
                  // make sure all of the criteria is met
                  if(moves[m].nJumped == l && moves[m].pDefHit == k &&
                     moves[m].pOffHit == j && moves[m].free == i)
                  {
                     offMoves[index++] = &moves[m];
                  }
               } // end for m
            } // end for l
         } // end for k
      } // end for j
   } // end for i
}

//————————————————————————————————————————————————————————————————————————
// Try Moves
//————————————————————————————————————————————————————————————————————————

// take a light look at defence //////////////////////////////////////////
static AI_MOVE* Move_LDefence(AI_MOVE* defMoves[320], int nMoves, int maxDefHit)
{
   int i = 0;
   AI_MOVE* result;
   
   // see if the need to defend is great enough
   if(maxDefHit < PC_MAX_HIT) return NULL;

   // see if it is possible to jump any marbles giving trouble
   while(defMoves[i]->pDefHit == maxDefHit)
   {      
      // see if we can find a jump to take care of it
      if((result = Move_CrossOffAndDef(defMoves[i], defMoves, nMoves)) != NULL)
      {
         // check the amout of danger we are in when we land
         if(result->pOffHit < PC_MAX_HIT) return result;
      }

      // move to the next item if we still have one
      if(++i >= nMoves) break;
   }

   // return that we haven't found a good move yet
   return NULL;
}


// take a look at offensive moves ////////////////////////////////////////
static AI_MOVE* Move_Offence(AI_MOVE* offMoves[320], int nMoves, int maxOffHit)
{
   int i, j = 0;

   // there are no known offensive moves
   if(maxOffHit == 0) return NULL;

   // look for the best jump. i > 0 because we want there to BE a jump
   for(i = maxOffHit; i > 0; i--)
   {
      // go through all of the moves checking the
      // probablity of getting jumped when we land
      while(offMoves[j]->nJumped == i)
      {
         // if the move wont 'cost' too much, or if we will jump enough to compensate
         if(offMoves[j]->pOffHit < PC_MAX_HIT || offMoves[j]->nJumped > 1) return offMoves[j];

         // move on to the next item
         if(++j >= nMoves) break;
      }
   }

   // return that we haven't found a good move here
   return NULL;
}


// look at free moves ////////////////////////////////////////////////////
static AI_MOVE* Move_Free(AI_MOVE* freMoves[320], int nMoves)
{
   int i;            // counter
   int *x, *y;       // used as chords
   int zero    = 0;  // used for finding marbles behind the line
   int fifteen = 15; // used for finding marbles behind the line
   int lineNum = 0;  // number of marbles behind the line

   // no 'free' moves were found
   if(!freMoves[0]->free) return NULL;

   // figure out how to count the number of marbles behind the line
   switch(pcur)
   {
   case 0: x = &i;       y = &zero;    break;
   case 1: x = &fifteen; y = &i;       break;
   case 2: x = &i;       y = &fifteen; break;
   case 3: x = &zero;    y = &i;       break;
   }

   // count the number of marbles behind the line
   for(i = 3; i < 12; i++) if(gBoard[*x][*y] != MARBLE_SPACE) lineNum++;

   // go through all of the moves looking for the best off hit
   i = 0;
   while(freMoves[i]->free)
   {
      // see if the cost of the move will not be too high & if the behind the
      // line is not full or the marble is FROM behind the line
      if(freMoves[i]->pOffHit < PC_MAX_HIT && (lineNum < 10 || 
         (1 > freMoves[i]->sx || freMoves[i]->sx > 14 ||
          1 > freMoves[i]->sy || freMoves[i]->sy > 14))) return freMoves[i];

      // move to the next item if there is one
      if(++i >= nMoves) break;
   }

   // return that we haven't found a good move
   return NULL;
}


// take a heavy look at defensve moves ///////////////////////////////////
static AI_MOVE* Move_HDefence(AI_MOVE* defMoves[320], int nMoves, int maxDefHit)
{
   int i, j;
   int index = 0;      // indexing variable
   int min   = 100;    // minimum off hit
   AI_MOVE* list[320]; // list of 'minimum' moves

   // look for a marble that needs to be move the most.
   for(i = maxDefHit; i > 1; i--)
   {
      j = 0;

      // look for a move where the potential of getting
      // jumped if we move is less than the potential of
      // getting jumped if we stay.
      do
      {
         // see if the move will not 'cost' too much
         if(defMoves[j]->pDefHit == i && defMoves[j]->pOffHit < PC_MAX_HIT)
            return defMoves[j];

      } while(++j < nMoves);
   }

   // find the move with the least possibility of getting jumped
   for(i = 0; i < nMoves; i++)
      if(defMoves[i]->pOffHit < min) min = defMoves[i]->pOffHit;

   // make a list of moves with least reps.
   for(i = 0; i < nMoves; i++)
      if(defMoves[i]->pOffHit == min) list[index++] = defMoves[i];

   // return a random item from the list
   return list[RAND(index)];
}


// see if any offensive move will take care of defence problem ///////////
static AI_MOVE* Move_CrossOffAndDef(AI_MOVE* move, AI_MOVE* match[320], int nMoves)
{
   int i, j, k; // looping variables
   
   // go through all of the moves
   for(i = 0; i < nMoves; i++)
   {
      // go throug all of the jumped marbles
      for(j = 0; j < match[i]->nJumped; j++)
      {
         // look at all of the offending marbles
         for(k = 0; k < move->nDefHit; k++)
         {
            // see if the two match
            if(match[i]->jumped[j][0] == move->defHit[k][0] &&
               match[i]->jumped[j][1] == move->defHit[k][1])
            {
               // see how likley we are to get jumped when we land
               if(match[i]->pOffHit < PC_MAX_HIT) return match[i];
            }
         }
      }
   }

   // return we didn't find any
   return NULL;
}