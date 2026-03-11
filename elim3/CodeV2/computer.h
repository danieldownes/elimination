/*******************************************************************************/
// computer.h - Function prototype for computer.cpp
//
// Written By : Justin Hoffman
// Date       : 7/31/1999 - 7/18/2000
/*******************************************************************************/

#ifndef COMPUTER_HEADER_INCLUDED
#define COMPUTER_HEADER_INCLUDED

//————————————————————————————————————————————————————————————————————————
// Computer player move structure
//————————————————————————————————————————————————————————————————————————

struct AI_MOVE {
   int free;    // the marble will land on a free guy (used as boolean)
   int nOffHit; // num different marbles that will hit me if i move
   int nDefHit; // num different marbles that will hit me if i stay
   int nJumped; // number of enemy marbles that will be jumped by the move

   int pOffHit; // probablity of getting jumped if i move
   int pDefHit; // probablity of getting jumped if i stay

   // [5 dice in a direction could jump me * there are 4 directions][x & y]
   int offHit[20][2]; // location of marbles that will hit me if i move
   int defHit[20][2]; // location of marbles that will hit me if i stay

   // [a roll of 6 can jump 5 dice][x & y]
   int jumped[5][2]; // location of marbles that will be jumped by the move

   int sx, sy, dx, dy; // chordinates of move
};


//————————————————————————————————————————————————————————————————————————
// Function Prototypes
//————————————————————————————————————————————————————————————————————————

// Main Game Programming Interface ///////////////////////////////////////
void PcExecuteMove(void);

// Probability Calculations //////////////////////////////////////////////
void GetHitProb(int, int, int*, int*, int[20][2], int[5][2], int);
BOOL IsInHitList(int, int, int, int[5][2]);
int  GetHitProbB(int, int, int, int);
void GetJumped(int, int, int, int, int*, int[5][2]);

// Get Maximums //////////////////////////////////////////////////////////
int GetMaxOffHit(AI_MOVE[320], int);
int GetMaxDefHit(AI_MOVE[320], int);
int GetMaxJumped(AI_MOVE[320], int);

// Move sorting //////////////////////////////////////////////////////////
void SortOff(AI_MOVE[320], AI_MOVE*[320], int, int, int, int);
void SortDef(AI_MOVE[320], AI_MOVE*[320], int, int, int, int);
void SortFre(AI_MOVE[320], AI_MOVE*[320], int, int, int, int);

// Try Moves /////////////////////////////////////////////////////////////
AI_MOVE* Move_LDefence(AI_MOVE*[320], int, int);
AI_MOVE* Move_Offence(AI_MOVE*[320], int, int);
AI_MOVE* Move_Free(AI_MOVE*[320], int);
AI_MOVE* Move_HDefence(AI_MOVE*[320], int, int);
AI_MOVE* Move_CrossOffAndDef(AI_MOVE*, AI_MOVE*[320], int);


#endif // ifndef COMPUTER_HEADER_INCLUDED