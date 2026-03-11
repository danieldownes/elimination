/**************************************************************/
// pcPlayer.h - pcPlayer.cpp Definitions
// Written By:		Justin Hoffman
// Date Written:	March 6, 1999 - April 7, 1999
/**************************************************************/


/////////////////////////////////
// Setup
#ifndef gd_pcPlayer_header_included
#define gd_pcPlayer_header_included

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////
// Definitions
#define pcd_chipid					2 // id of pc's chip on board array
#define pcd_landonchip				1 // landed on a chip
#define pcd_landonspace				2 // landed on a space
#define pcd_landonoppon				3 // landed on oponent
#define pcd_landonfree				4 // landed on a free marble
#define pcd_landoffboard			5 // didn't land on board


/////////////////////////////////
// Externals
extern int gg_board[15][10];
extern int gg_docChips[2][10];
extern int gg_freeguys[2][20];
extern int gg_roll[2];


/////////////////////////////////
// Computer Move Class
typedef class gs_pcmove
{
	private:
		BOOL hitProb();
		BOOL needMove();
		int getChipDist(int a, int b, int ed[2][4]);
	public:
		int sx;
		int sy;
		int dx;
		int dy;
		int landOn;
		int maxHitO;
		int maxHitD;
		int edO[2][4];
		int edD[2][4];
		int jumped[2][10];
		int numJumped;
		BOOL setup(int Vsx, int Vsy, int dirX, int dirY, int die);
} *LPgs_pcmove;


/////////////////////////////////
// Function Prototypes
BOOL startPcPlayer(void);								// start the pc player

namespace ppl
{
	BOOL makeMoves(void);						// make moves list and pick one
	BOOL singleMove(void);						// make a single move
	int getMoves(gs_pcmove mvs[2][80]);			// make a list of possible moves

	int sortOffMoves(gs_pcmove mvs[2][80],		// organize offensive moves
		LPgs_pcmove offList[], int* numO);
	int sortDefMoves(gs_pcmove mvs[2][80],		// try to avoid being jumped
		LPgs_pcmove defList[], int* numD);
	int sortMoveType(gs_pcmove mvs[2][80],		// organize moves of specified type
		LPgs_pcmove lmvs[], int type,
		int* numLeast);
	BOOL crossDefAndOff(LPgs_pcmove dMvs[],		// see if a chip in the way can be jumped
		LPgs_pcmove oMvs[], int maxO, int dlItem,
		int res[]);

	BOOL defMove(LPgs_pcmove dMvs[],			// try defensive moves
		LPgs_pcmove oMvs[], int maxD, int numD,
		int maxO, int numO, BOOL* nm);
	BOOL offMove(LPgs_pcmove oMvs[], int maxO,	// try offensive moves
		int numO, BOOL* nm);
	BOOL fMove(gs_pcmove mvs[2][80], BOOL* nm);	// try to land on 'f'
	BOOL randMove(gs_pcmove mvs[2][80], BOOL* nm); // random move
	BOOL exeMove(LPgs_pcmove move, BOOL* error);// execute a move

	BOOL pcWait(DWORD ticks);					// make the computer wait
}

#endif // #ifndef gd_pcPlayer_header_included