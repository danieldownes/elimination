/**************************************************************/
// pcPlayer.cpp - Computer Player Code
// Written By:		Justin Hoffman
// Date Written:	March 6, 1999 - April 7, 1999
/**************************************************************/


/////////////////////////////////
// Headers
#include "stdafx.h"
#include "pcPlayer.h"
#include "elim1.h"


/////////////////////////////////
// Class Functions
BOOL gs_pcmove::setup(int Vsx, int Vsy, int dirX, int dirY, int die)
{
	sx=Vsx;
	sy=Vsy;
	numJumped=0;
	dx=(dirX*gg_roll[die]+Vsx);
	dy=(dirY*gg_roll[die]+Vsy);

	// clear jumped chips
	for(int i=0; i<10; i++)
	{
		jumped[0][i]=15;
		jumped[1][i]=15;
	}

	if(dx<0 || dy<0 || dx>14 || dy>9)
	{
		landOn=pcd_landoffboard;
		return FALSE; // invalid move
	}
	else if(iIsFreeGuy(dx, dy)) landOn=pcd_landonfree;
	else if(gg_board[dx][dy]==0) landOn=pcd_landonspace;
	else if(gg_board[dx][dy]==pcd_chipid) landOn=pcd_landonchip;
	else if(gg_board[dx][dy]==1) landOn=pcd_landonoppon;

	// get the number and location of chips that will be jumped
	int x, y;
	for(i=1; i<gg_roll[die]; i++)
	{
		x=(dirX*i+sx);
		y=(dirY*i+sy);

		if(x<0 || y<0 || x>14 || y>9) break;
		else if(gg_board[x][y]==1)
		{
			jumped[0][numJumped]=x;
			jumped[1][numJumped]=y;
			numJumped++;
		}
	}

	// get number of rolls that could get it
	hitProb();
	needMove();
	return TRUE;
}

BOOL gs_pcmove::hitProb(void)
{	
	maxHitO=getChipDist(dx, dy, edO);
	return TRUE;
}

BOOL gs_pcmove::needMove(void)
{
	if(sx==15) // chips behind line can't be hit
	{
		edD[0][0]=15; edD[1][0]=15;
		edD[0][1]=15; edD[1][1]=15;
		edD[0][2]=15; edD[1][2]=15;
		edD[0][3]=15; edD[1][3]=15;
		maxHitD=0;
	}
	else maxHitD=getChipDist(sx, sy, edD);
	return TRUE;
}

int gs_pcmove::getChipDist(int a, int b, int ed[2][4])
{
	int x, y, i, j;
	const int dir[4][2]={{0,-1},{1,0},{0,1},{-1,0}};

	int chipDist, edgeDist[4], res[4];
	edgeDist[0]=(9-b);		// dist from bottom
	edgeDist[1]=a;			// dist from left
	edgeDist[2]=b;			// dist from top
	edgeDist[3]=(14-a);		// dist from right
	
	for(i=0; i<4; i++)
	{
		// clear location of chips
		ed[0][i]=15;
		ed[1][i]=15;

		// find closest enemy and record its location
		chipDist=0;
		for(j=0; j<7; j++)
		{
			x=(a+dir[i][0]*j);
			y=(b+dir[i][1]*j);

			if(x==-1 && gg_docChips[0][y]==1)
			{
				// behind enemy line
				ed[0][i]=-1;
				ed[1][i]=y;
				chipDist=(6-j);

				// make edgeDist larger so res[i]==chipDist
				edgeDist[i]=7;

				break;
			}
			else if(x<0 || y<0 || x>14 || y>9)
			{
				break;
			}
			else if(gg_board[x][y]==1)
			{
				ed[0][i]=x;
				ed[1][i]=y;
				chipDist=(6-j);
				break;
			}
		}
		res[i]=(chipDist>edgeDist[i]? edgeDist[i]:chipDist);
	}

	int max=0;
	for(i=0; i<4; i++) if(res[i]>max) max=res[i];
	return max;
}


/////////////////////////////////
// Search Moves Functions
BOOL startPcPlayer(void)
{
	ppl::pcWait(1000);
	ppl::makeMoves();
	return TRUE;
}

BOOL ppl::makeMoves(void)
{
	BOOL nm=TRUE;
	nm=singleMove();

	if(nm) // if not error or game over
	{
		pcWait(500);
		singleMove();
	}

	return TRUE;
}

BOOL ppl::singleMove(void)
{
	BOOL nm=TRUE;
	gs_pcmove mvs[2][80];
	getMoves(mvs);

	int numO, maxO, numD, maxD;
	LPgs_pcmove oMvs[180];
	LPgs_pcmove dMvs[180];
	
	maxO=sortOffMoves(mvs, oMvs, &numO);
	maxD=sortDefMoves(mvs, dMvs, &numD);

	if(!defMove(dMvs, oMvs, maxD, numD, maxO, numO, &nm))
		if(!offMove(oMvs, maxO, numO, &nm))
			if(!fMove(mvs, &nm))
				if(!randMove(mvs, &nm))
					reportError(4);
	return nm;
}

int ppl::getMoves(gs_pcmove mvs[2][80])
{
	const int dir[4][2]={{0,1},{1,0},{0,-1},{-1,0}};
	int chips[2][20], numChips=0, i, j, k, blck;

	// get location of chips
	for(i=0; i<15; i++) 
	{
		for(j=0; j<10; j++)
		{
			if(gg_board[i][j]==pcd_chipid)
			{
				chips[0][numChips]=i;
				chips[1][numChips]=j;
				numChips++;
			}
		}
	}

	// add in the chips behind the lines
	for(i=0; i<10; i++)
	{
		if(gg_docChips[1][i]==pcd_chipid)
		{
			chips[0][numChips]=15;
			chips[1][numChips]=i;
			numChips++;
		}
	}

	// setup moves
	for(i=0; i<2; i++)
	{
		for(j=0; j<numChips; j++)
		{
			blck=(4*j);
			for(k=0; k<4; k++)
			{
				mvs[i][blck+k].setup(chips[0][j], chips[1][j],
					dir[k][0], dir[k][1], i);
			}
		}

		// clear empty part of array
		for(j=numChips; j<20; j++)
		{
			blck=(4*j);
			for(k=0; k<4; k++) mvs[i][blck+k].landOn=pcd_landoffboard;
		}
	}
	
	return numChips;
}


/////////////////////////////////
// Short Moves
int ppl::sortOffMoves(gs_pcmove mvs[2][80], LPgs_pcmove offList[], int* numO)
{
	// find the maximum value
	int max=0, arrItem=0, i, j, k;
	for(i=0; i<80; i++)
	{
		if(mvs[0][i].landOn!=pcd_landoffboard &&
			mvs[0][i].numJumped>max) max=mvs[0][i].numJumped;
		if(mvs[0][i].landOn!=pcd_landoffboard &&
			mvs[1][i].numJumped>max) max=mvs[1][i].numJumped;
	}
	
	// make the list
	for(i=max; i>-1; i--) // order them according to offence
	{
		for(j=0; j<7; j++) // order them according to defence
		{
			for(k=0; k<80; k++)
			{
				if(mvs[0][k].landOn!=pcd_landoffboard &&
					mvs[0][k].numJumped==i && mvs[0][k].maxHitO==j)
				{
					offList[arrItem]=&mvs[0][k];
					arrItem++;
				}
				if(mvs[1][k].landOn!=pcd_landoffboard && 
					mvs[1][k].numJumped==i && mvs[1][k].maxHitO==j)
				{
					offList[arrItem]=&mvs[1][k];
					arrItem++;
				}
			}
		}
	}

	*numO=(arrItem-1);
	return max;
}

int ppl::sortDefMoves(gs_pcmove mvs[2][80], LPgs_pcmove defList[],
					   int* numD)
{
	int max=0, arrItem=0, i, j, k;

	// get maximum defensive move
	for(i=0; i<80; i++)
	{
		if(mvs[0][i].landOn!=pcd_landoffboard &&
			mvs[0][i].maxHitD>max) max=mvs[0][i].maxHitD;
		if(mvs[0][i].landOn!=pcd_landoffboard &&
			mvs[1][i].maxHitD>max) max=mvs[1][i].maxHitD;
	}

	// sort the list
	for(i=max; i>0; i--) // according to pre defensive
	{
		for(j=0; j<7; j++) // according to post deffensive
		{
			for(k=0; k<80; k++)
			{
				if(mvs[0][k].landOn!=pcd_landoffboard
					&& mvs[0][k].maxHitD==i && mvs[0][k].maxHitO==j)
				{
					defList[arrItem]=&mvs[0][k];
					arrItem++;
				}
				if(mvs[1][k].landOn!=pcd_landoffboard
					&& mvs[1][k].maxHitD==i && mvs[0][k].maxHitO==j)
				{
					defList[arrItem]=&mvs[1][k];
					arrItem++;
				}
			}
		}
	}

	*numD=(arrItem-1);
	return max;
}

int ppl::sortMoveType(gs_pcmove mvs[2][80], LPgs_pcmove lmvs[],
					  int type, int* numLeast)
{
	int arrItem=0, least=6, nl=0, i, j;

	// find the best post defensive
	for(i=0; i<80; i++)
	{
		if(mvs[0][i].landOn==type && mvs[0][i].maxHitO<least)
			least=mvs[0][i].maxHitO;
		if(mvs[1][i].landOn==type && mvs[1][i].maxHitO<least)
			least=mvs[1][i].maxHitO;
	}

	// sort the moves
	for(i=0; i<7; i++)
	{
		for(j=0; j<80; j++)
		{
			if(mvs[0][j].landOn==type && mvs[0][j].maxHitO==i)
			{
				if(i==least) nl++;
				lmvs[arrItem]=&mvs[0][j];
				arrItem++;
			}
			if(mvs[1][j].landOn==type && mvs[1][j].maxHitO==i)
			{
				if(i==least) nl++;
				lmvs[arrItem]=&mvs[1][j];
				arrItem++;
			}
		}
	}

	*numLeast=nl;
	return (arrItem-1);
}

BOOL ppl::crossDefAndOff(LPgs_pcmove dMvs[], LPgs_pcmove oMvs[],
						 int maxO, int dItem, int res[])
{
	int i, j, k, arrItem=0;
	BOOL isMove=FALSE;

	// look through offensive list for match
	for(i=0; i<maxO; i++)
	{
		if(oMvs[i]->landOn!=pcd_landonspace &&
			oMvs[i]->landOn!=pcd_landonfree) continue;
		for(j=0; j<10; j++)
		{
			for(k=0; k<4; k++)
			{
				// if match found
				if(dMvs[dItem]->edD[0][k]!=15 && dMvs[dItem]->edD[1][k]!=15
					&& oMvs[i]->jumped[0][j]==dMvs[dItem]->edD[0][k]
					&& oMvs[i]->jumped[1][j]==dMvs[dItem]->edD[1][k])
				{
					// return offensive moves to use
					isMove=TRUE;
					res[arrItem]=i;

					// only want 10 moves
					if(arrItem++>10) return TRUE;
				}
			}
		}
	}

	return isMove;
}


/////////////////////////////////
// Conduct Move Functions
BOOL ppl::defMove(LPgs_pcmove dMvs[], LPgs_pcmove oMvs[], int maxD,
				  int numD, int maxO, int numO, BOOL* nm)
{
	BOOL error;
	*nm=TRUE;

	if(maxD<2 || numD==-1) return FALSE; // not enough chance of getting hit
	
	int res[10], mvToTke=0;
	if(crossDefAndOff(dMvs, oMvs, numO, 0, res))
	{
		*nm=exeMove(oMvs[res[0]], &error);
	}
	else
	{
		// could not jump enemy so move out of the way
		int a=0;
		while(oMvs[a]->landOn!=pcd_landonfree &&
			oMvs[a]->landOn!=pcd_landonspace && a<=numD) a++;

		if(a>=numD) return FALSE; // no moves found

		*nm=exeMove(dMvs[a], &error);
	}

	return !error;
}

BOOL ppl::offMove(LPgs_pcmove oMvs[], int maxO, int numO, BOOL* nm)
{
	BOOL error;
	*nm=TRUE;

	if(maxO==0) return FALSE; // no known 'offensve' moves

	int a=0;
	while(oMvs[a]->landOn!=pcd_landonfree &&
		oMvs[a]->landOn!=pcd_landonspace && a<=numO) a++;

	if(a>=numO) return FALSE; // no known moves

	*nm=exeMove(oMvs[a], &error);

	return !error;
}

BOOL ppl::fMove(gs_pcmove mvs[2][80], BOOL* nm)
{	
	BOOL error;
	*nm=TRUE;
	LPgs_pcmove lmvs[160];
	int numLeast;

	if(sortMoveType(mvs, lmvs, pcd_landonfree, &numLeast)==-1)
		return FALSE; // no 'f' moves

	// consider if behind line is full
	BOOL isSpace=FALSE;
	for(int i=0; i<10; i++)
		isSpace=(isSpace || gg_docChips[1][i]==0);

	if(!isSpace) return FALSE; // no empty spaces found

	*nm=exeMove(lmvs[randNum(numLeast)], &error);

	return !error;
}

BOOL ppl::randMove(gs_pcmove mvs[2][80], BOOL* nm)
{
	BOOL error;
	*nm=TRUE;
	LPgs_pcmove lmvs[160];
	int numLeast, numItems;

	numItems=sortMoveType(mvs, lmvs, pcd_landonspace, &numLeast);

	if(numItems!=-1) *nm=exeMove(lmvs[randNum(numLeast)], &error);

	if(error || numItems==-1)
	{
		*nm=FALSE;
		return reportError(4);
	}

	return TRUE;
}

BOOL ppl::exeMove(LPgs_pcmove move, BOOL* error)
{
	*error=(move->landOn!=pcd_landonspace &&
		move->landOn!=pcd_landonfree);
	// hopefully we can recover from this error
	if(*error) return TRUE; 
	
	return vMoveChip((move->sx==15? 2:3), move->sx, move->sy,
		move->dx, move->dy);
}


/////////////////////////////////
// Utility Functions
BOOL ppl::pcWait(DWORD ticks)
{
	DWORD st=GetTickCount();
	while(st>(GetTickCount()-ticks)){}
	return TRUE;
}