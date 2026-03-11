/**************************************************************/
// class.h - Misc. Class Definitions
// Written By:		Justin Hoffman
// Date Written:	Feb 26, 1999 - April 7, 1999
/**************************************************************/


/////////////////////////////////
// Setup
#ifndef gd_class_header_included
#define gd_class_header_included

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////
// Externals
long chdBoardX(int a);
long chdBoardY(int a);


/////////////////////////////////
// Class Definitions
typedef struct gs_curve
{
	long sx;
	long sy;
	long dx;
	long dy;
	long px;
	long py;
	long pw;
	long ph;
	long mx;
	long my;
	long wait;
} *LPgs_curve;

typedef class gs_selring
{
	public:
		long x;
		long y;
		long sx;
		BOOL setup(int a, int b, BOOL isF)
		{
			x=(chdBoardX(a)-2);
			y=(chdBoardY(b)-1);
			sx=(isF? 24:0);
			return TRUE;
		}
} *LPgs_selring;

#endif // #ifndef gd_class_header_included