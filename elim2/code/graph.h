/*******************************************************************************/
// graph.h - Function prototypes and data types for graph.cpp
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 - 
/*******************************************************************************/

#ifndef GRAPH_HEADER_INCLUDED
#define GRAPH_HEADER_INCLUDED

// Headers ///////////////////////////////////////////////////////////////
#include "main.h"


// Globals ///////////////////////////////////////////////////////////////
extern LPDIRECTDRAW lpdd;


// XMP data structure ////////////////////////////////////////////////////
typedef struct sXmpData
{
   ULONG width;
   ULONG height;
   UCHAR* buffer;
   UCHAR* header;
} XMPDATA;


// RES data structure ////////////////////////////////////////////////////
struct RES {
   UCHAR id;
   ULONG offset;
   int   file;
};


/*-----------------------------------------------------------------------------*/
// Function Prototypes
/*-----------------------------------------------------------------------------*/

// GDI Graphic Functions /////////////////////////////////////////////////
void GDI_DrawText(LPDDS dds, COLORREF color, LPRECT r, LPSTR s, DWORD flags);

// DirectDraw Setup //////////////////////////////////////////////////////
BOOL InitDirectDraw(void);
void RestoreDirectDraw(void);

// Main DirectDraw Functions /////////////////////////////////////////////
void DD_CreateSurface(LPDDS*, UINT);
void DD_CreateSurface(LPDDS*, int, int, UCHAR);
void DD_Lock(LPDDS dds, UCHAR** surface_buffer, DWORD* lPitch);
void DD_UnLock(LPDDS dds);

/*
UCHAR DD_ColorMatch(LPDDS dds, COLORREF color);
COLORREF DD_ColorMatch(LPDDS dds, UCHAR color);
*/

// Special effects & drawing /////////////////////////////////////////////
void DD_Effect_Fade(void);
void DD_Draw_Rect(LPDDS, RECT*, UCHAR, UCHAR, int);
void DD_Draw_Line(LPDDS, UCHAR, long, long, long, long);

// Wrapper functions /////////////////////////////////////////////////////
void DD_Blt(LPDDS ddsDst, LPDDS ddsSrc, LPRECT rDst, LPRECT rSrc, DWORD flags, LPDDBLTFX bltFx);
void DD_BltFast(LPDDS ddsDst, LPDDS ddsSrc, DWORD x, DWORD y, LPRECT rSrc, DWORD flags);
void DD_BltClip(LPDDS ddsDst, LPDDS ddsSrc, long x, long y, LPRECT rClip, RECT rSrc, DWORD flags);
void DD_Flip(LPDDS);
void DD_GetDC(LPDDS, HDC*);
void DD_ReleaseDC(LPDDS, HDC);

// X image functions /////////////////////////////////////////////////////
void XAL_Load(UINT, PALETTEENTRY[256]); 
void XMP_Load(XMPDATA*, UINT);
void XMP_Copy(LPDDS dds, XMPDATA*, ULONG, ULONG);
void XMP_ReLoad(LPDDS dds, UINT resId);
void XMP_Delete(XMPDATA*);

// RES functions /////////////////////////////////////////////////////////
bool ResAvalable(void);
void ResOpen(RES*, UCHAR);
void ResSeek(RES*, long, UCHAR);
void ResRead(RES*, void*, ULONG);
void ResClose(RES*);

// Process result from DirectDraw functions //////////////////////////////
inline BOOL DDResult(HRESULT ddrval)
{
   switch(ddrval)
   {
   case DD_OK:                 return 0; // success
   case DDERR_WASSTILLDRAWING: return 1; // keep trying
   case DDERR_SURFACELOST:               // restore the surfaces and try again
      RestoreDirectDraw();
      return 1;
   default: return 0;                    // an error occured
   }

   // we should never get here
   return 0;
}

#endif // ifndef GRAPH_HEADER_INCLUDED