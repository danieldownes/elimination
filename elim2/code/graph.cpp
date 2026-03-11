/*******************************************************************************/
// graph.cpp - Game Graphics Functions
//
// Written By : Justin Hoffman
// Date       : July 9, 1999 - 
/*******************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "main.h"
#include "elimres.h"
#include "graph.h"
#include "window.h"

// Globals ///////////////////////////////////////////////////////////////
LPDIRECTDRAW lpdd = NULL;


/*-----------------------------------------------------------------------------*/
// GDI Graphic Functions
/*-----------------------------------------------------------------------------*/

// Generic function for drawing text to a surface ////////////////////////
void GDI_DrawText(LPDDS dds, COLORREF color, LPRECT r, LPSTR s, DWORD flags)
{
   HDC hdc;

   // retreave the DC
   DD_GetDC(dds, &hdc); 

   // set color and mode
   SetTextColor(hdc, color);
   SelectObject(hdc, g_fontSmall);
   SetBkMode(hdc, TRANSPARENT);

   // draw the text
   DrawText(hdc, s, strlen(s), r, flags);

   // release the DC
   DD_ReleaseDC(dds, hdc);
}


/*-----------------------------------------------------------------------------*/
// DirectDraw Setup
/*-----------------------------------------------------------------------------*/

// Initilize DirectSound /////////////////////////////////////////////////
BOOL InitDirectDraw(void)
{
   DDSURFACEDESC ddsd;
   DDSCAPS ddscaps;

   // create the DirectDrawObject
   if(DirectDrawCreate(NULL, &lpdd, NULL) != DD_OK) return FALSE;

   // set the cooperation level
   if(lpdd->SetCooperativeLevel(g_hWnd, DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE |
      DDSCL_ALLOWREBOOT) != DD_OK) return FALSE;

   // set the display mode
   if(lpdd->SetDisplayMode(SCREEN_W, SCREEN_H, SCREEN_BPP) != DD_OK) return FALSE;

   // setup ddsd to create front buffer
   ddsd.dwSize            = sizeof(DDSURFACEDESC);
   ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
   ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
   ddsd.dwBackBufferCount = 1;

   // create the front buffer
   if(lpdd->CreateSurface(&ddsd, &ddsFront, NULL) != DD_OK) return FALSE;

   // create the back buffer
   ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
   if(ddsFront->GetAttachedSurface(&ddscaps, &ddsBack) != DD_OK) return FALSE;
  
   // return success
   return TRUE;
}


// Restore DirectDraw ////////////////////////////////////////////////////
void RestoreDirectDraw(void)
{
   // make sure we restore ALL of the surfaces
   ddsFront->Restore();
   ddsBoard->Restore();
   ddsDice->Restore();
   ddsExplode->Restore();
   ddsItems->Restore();
   ddsMenu->Restore();
   ddsSelect->Restore();

   ddsWindow->Restore();
   ddsMenubar->Restore();
   ddsTeams->Restore();
   ddsRegTime->Restore();
   ddsPInfo->Restore();

   // just to be safe, set the pallette again
   ddsFront->SetPalette(ddpMain);
   ddpMain->SetEntries(0, 0, 256, g_palMain);

   
   // put the bitmaps back onto the surfaces
   XMP_ReLoad(ddsBoard  , XMP_BOARD);
   XMP_ReLoad(ddsDice   , XMP_DICE);
   XMP_ReLoad(ddsExplode, XMP_EXPLODE);
   XMP_ReLoad(ddsItems  , XMP_ITEMS);
   XMP_ReLoad(ddsMenu   , XMP_MENU);
   XMP_ReLoad(ddsSelect , XMP_SELECT);

   // re-draw the intro screen
   if(g_gameState == STATE_INTRO) ddpMain->SetEntries(0, 0, 256, g_palIntro);

   // re-draw the window
   if(g_settings & SET_WINDOW) Win_Draw();

   // re-draw other blank surfaces
   if(!(g_settings & SET_REGISTERED)) RegTime_Draw();
   Menu_Draw();
   Teams_Draw();
   PInfo_Draw();

   // re-select the current player
   PInfo_Select(g_cPlayer);
}


/*-----------------------------------------------------------------------------*/
// Main DirectDraw Functions
/*-----------------------------------------------------------------------------*/

// Create a DirectDraw surface from a bitmap /////////////////////////////
void DD_CreateSurface(LPDDS* dds, UINT resId)
{
   XMPDATA xmp;

   /// Load the xmp
   XMP_Load(&xmp, resId);

   // create the surface
   DD_CreateSurface(dds, xmp.width, xmp.height, CLR_TRANSPARENT);

   // copy the image onto the buffer
   XMP_Copy(*dds, &xmp, 0, 0);

   // release the xmp data
   XMP_Delete(&xmp);
}


// Create a blank DirectDraw surface /////////////////////////////////////
void DD_CreateSurface(LPDDS* pdds, int width, int height, UCHAR cKey)
{
   DDBLTFX       bltFx;
   DDCOLORKEY    colorKey;
   DDSURFACEDESC ddsd;

   // set up the surface description
   ZeroMemory(&ddsd, sizeof(ddsd));
   ddsd.dwSize         = sizeof(ddsd);
   ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   ddsd.dwWidth        = width;
   ddsd.dwHeight       = height;
   
   // create the surface
   if(lpdd->CreateSurface(&ddsd, pdds, NULL) != DD_OK) return;
   
   // setup bltFx
   ZeroMemory(&bltFx, sizeof(DDBLTFX));
   bltFx.dwSize      = sizeof(DDBLTFX);
   bltFx.dwFillColor = cKey;

   // fill the new surface with it's color key
   DD_Blt(*pdds, NULL, NULL, NULL, DDBLT_COLORFILL, &bltFx);

   // set the new surfaces colorkey
   colorKey.dwColorSpaceHighValue = cKey;
   colorKey.dwColorSpaceLowValue  = cKey;
   while(DDResult((*pdds)->SetColorKey(DDCKEY_SRCBLT, &colorKey))) ;
}


// Lock a DirectDraw surface /////////////////////////////////////////////
void DD_Lock(LPDDS dds, UCHAR** surface_buffer, DWORD* lPitch)
{
   DDSURFACEDESC ddsd;

   // clear the description structure
   memset(&ddsd, 0, sizeof(DDSURFACEDESC));
   ddsd.dwSize = sizeof(DDSURFACEDESC);

   // lock the buffer
   while(DDResult(dds->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL))) ;

   // set the pointer to the video memory
   *surface_buffer = (UCHAR*)ddsd.lpSurface;
   *lPitch         = ddsd.lPitch;
}


// Unlock a DirectDraw surface ///////////////////////////////////////////
void DD_UnLock(LPDDS dds)
{
   while(DDResult(dds->Unlock(NULL))) ;
}

/*
// Convert an RGB color to one of the avalable colors on the surface /////
UCHAR DD_ColorMatch(LPDDS dds, COLORREF color)
{
   // a crude function -- if at all possible,
   // use the accual palette entry number instead of this
   HDC      hdc;
   COLORREF save;
   UCHAR*   surface_buf;
   UCHAR    result;
   DWORD    lPitch;

   // have GDI's SetPixel do the matching
   DD_GetDC(dds, &hdc);
   save = SetPixel(hdc, 0, 0, color);
   DD_ReleaseDC(dds, hdc);

   // Figure out what color was accually printed
   DD_Lock(dds, &surface_buf, &lPitch);
   result = *(UCHAR*)surface_buf;
   DD_UnLock(dds);

   // put the origional color back
   DD_GetDC(dds, &hdc);
   SetPixel(hdc, 0, 0, save);
   DD_ReleaseDC(dds, hdc);

   // return our result
   return result;
}

// Convert a color on a surface to an RGB value //////////////////////////
COLORREF DD_ColorMatch(LPDDS dds, UCHAR color)
{
   // Like the previous function, sort of crude, so attempt
   // to supply the real color if it can be done
   COLORREF result;
   UCHAR*   surface_buf;
   UCHAR    save;
   DWORD    lPitch;
   HDC      hdc;

   // lock the buffer
   DD_Lock(dds, &surface_buf, &lPitch);

   // save the current color
   save = *(UCHAR*)surface_buf;

   // put down the color to test
   surface_buf[0] = color;

   // unlock the buffer
   DD_UnLock(dds);

   // use GetPixel to determing the COLORREF version
   DD_GetDC(dds, &hdc);
   result = GetPixel(hdc, 0, 0);
   DD_ReleaseDC(dds, hdc);

   // reset the color that was there
   DD_Lock(dds, &surface_buf, &lPitch);
   surface_buf[0] = save;
   DD_UnLock(dds);

   // return the result
   return result;
}
*/

/*-----------------------------------------------------------------------------*/
// Special Effects & Drawing
/*-----------------------------------------------------------------------------*/

// Fade to white - then set the palette to the main colors ///////////////
void DD_Effect_Fade(void)
{
   DDBLTFX bltFx;
   int red, green, blue;            // used for fading
   PALETTEENTRY color;              // temporary color
   PALETTEENTRY curnt_pal[256];     // the current palette

   // copy the global palettes into new locations
   memcpy(curnt_pal, g_palIntro, 256 * sizeof(PALETTEENTRY));

   // counter variables
   int i, pal_etry;
   
   // do this 64 times
   for(i = 0; i < 64; i++)
   {
      // loop thru all palette entries
      for(pal_etry = 0; pal_etry < 256; pal_etry++)
      {
         // get the entry data
         color = curnt_pal[pal_etry];

         // make 32 bit copy of color
         red   = color.peRed;
         green = color.peGreen;
         blue  = color.peBlue; 

         // change the colors
         if((red += 4) >=255)   red   = 255;
         if((green += 4) >=255) green = 255;
         if((blue += 4) >=255)  blue  = 255;
  
         // store colors back
         color.peRed   = red;
         color.peGreen = green;
         color.peBlue  = blue;

         // set the color to a diminished intensity
         curnt_pal[pal_etry] = color;

      } // end for pal_reg

      // write the palette back out
      ddpMain->SetEntries(0, 0, 256, curnt_pal);

      // wait for a few millaseconds
      Sleep(5);
   }
   
   // setup bltFx
   ZeroMemory(&bltFx, sizeof(DDBLTFX));
   bltFx.dwSize      = sizeof(DDBLTFX);
   bltFx.dwFillColor = CLR_BACKGROUND;

   // Clear the main surface
   DD_Blt(ddsFront, NULL, NULL, NULL, DDBLT_COLORFILL, &bltFx);

   // set the palette to the main one
   ddpMain->SetEntries(0, 0, 256, g_palMain);
}


// Draw a rectangle on surface ///////////////////////////////////////////
void DD_Draw_Rect(
LPDDS dds,     // surface to draw on
RECT* r,       // rectangle to draw in
UCHAR fill,    // palette entry used for fill
UCHAR border,  // palette entry used for border
int lineW)     // width of the border
{   
   UCHAR* surface_buf;
   DWORD  lPitch;
   int    i, j;

   // lock the surface
   DD_Lock(dds, &surface_buf, &lPitch);

   
   // draw the borders
   if(lineW > 0 && border != CLR_TRANSPARENT)
   {
      // draw the top and border: from left to right, then top to bottom
      for(i = r->left; i < r->right; i++)
      {
         // top border
         for(j = r->top; j < r->top + lineW; j++) surface_buf[i + j*lPitch] = border;

         // bottom border
         for(j = r->bottom - lineW; j < r->bottom; j++) surface_buf[i + j*lPitch] = border;
      }

      // draw the left and right border: from top to bottom then left to right
      for(i = r->top + lineW; i < r->bottom - lineW; i++)
      {
         // draw the top border
         for(j = r->left; j < r->left + lineW; j++) surface_buf[j + i*lPitch] = border;

         // draw the right border
         for(j = r->right - lineW; j < r->right; j++) surface_buf[j + i*lPitch] = border;
      }
   }

   // Fill the rectangle
   if(fill != CLR_TRANSPARENT)
   {
      // fill the rectangle: from left to right, then top to bottom
      for(i = r->left + lineW; i < r->right - lineW; i++)
      {
         for(j = r->top + lineW; j < r->bottom - lineW; j++)
         {
            surface_buf[i + j*lPitch] = fill;
         }
      }
   }

   DD_UnLock(dds);
}


// Draw a strait line ////////////////////////////////////////////////////
void DD_Draw_Line(LPDDS dds, UCHAR color, long xa, long ya, long xb, long yb)
{
   UCHAR* surface_buf;
   DWORD  lPitch;
   int    i;

   // lock the buffer
   DD_Lock(dds, &surface_buf, &lPitch);

   // if it is a horisontal line
   if(ya == yb)
   {
      // go from left to right
      for(i = xa; i < xb; i++) surface_buf[i + ya*lPitch] = color;
   }

   // else if it is a vertical line
   else if(xa == xb)
   {
      // go from top to bottom
      for(i = ya; i < yb; i++) surface_buf[xa + i*lPitch] = color;
   }

   // unlock the buffer
   DD_UnLock(dds);
}


/*-----------------------------------------------------------------------------*/
// DirectDraw surface wrappers
/*-----------------------------------------------------------------------------*/

// Blt ///////////////////////////////////////////////////////////////////
void DD_Blt(
LPDDS ddsDst, // destination surface
LPDDS ddsSrc, // source surface
LPRECT rDst,  // destination rectangle
LPRECT rSrc,  // source rectangle
DWORD flags,
LPDDBLTFX bltFx)
{
   flags |= DDBLT_WAIT;
   while(DDResult(ddsDst->Blt(rSrc, ddsSrc, rDst, flags, bltFx))) ;
}


// BltFast ///////////////////////////////////////////////////////////////
void DD_BltFast(
LPDDS  ddsDst, // destination surface
LPDDS  ddsSrc, // source surface
DWORD  x,      // destination x
DWORD  y,      // destination y
LPRECT rSrc,   // pointer to source rectangle
DWORD  flags)
{
   flags |= DDBLTFAST_WAIT;
   while(DDResult(ddsDst->BltFast(x, y, ddsSrc, rSrc, flags))) ;
}


// BltFast with rectangle clipping ///////////////////////////////////////
void DD_BltClip(
LPDDS  ddsDst, // destination surface
LPDDS  ddsSrc, // source surface
long   x,      // destination x
long   y,      // destination y
LPRECT rClip,  // destination rectangle to fit into
RECT   rSrc,   // source rectangle to edit
DWORD  flags)
{
   // get the width and height of the rectangle
   long width  = rSrc.right - rSrc.left;
   long height = rSrc.bottom - rSrc.top;

   // make sure some part of rSrc is within rClip, if not, exit now
   if(x >= rClip->right  || x + width  <= rClip->left ||
      y >= rClip->bottom || y + height <= rClip->top) return;

   // see if x needs to be changed
   if(x < rClip->left)
   {
      rSrc.left += (rClip->left - x);
      x = rClip->left;
   }

   // see if y needs to be changed
   if(y < rClip->top)
   {
      rSrc.top += (rClip->top - y);
      y = rClip->top;
   }

   // see if the right of rSrc needs to be shortened
   if(x + width > rClip->right) rSrc.right -= (x + width - rClip->right);

   // see if the bottom of rSrc needs to be shortened
   if(y + height > rClip->bottom) rSrc.bottom -= (y + height - rClip->bottom);

   // do the blitting operation
   flags |= DDBLTFAST_WAIT;
   while(DDResult(ddsDst->BltFast(x, y, ddsSrc, &rSrc, flags))) ;
}


// Flip //////////////////////////////////////////////////////////////////
void DD_Flip(LPDDS dds)
{
   while(DDResult(dds->Flip(NULL, DDFLIP_WAIT))) ;
}



// GetDC /////////////////////////////////////////////////////////////////
void DD_GetDC(LPDDS dds, HDC *hdc)
{
   while(DDResult(dds->GetDC(hdc))) ;
}


// ReleaseDC /////////////////////////////////////////////////////////////
void DD_ReleaseDC(LPDDS dds, HDC hdc)
{
   while(DDResult(dds->ReleaseDC(hdc))) ;
}


/*-----------------------------------------------------------------------------*/
// X image functions
/*-----------------------------------------------------------------------------*/

// Load a palette file ///////////////////////////////////////////////////
void XAL_Load(UINT resId, PALETTEENTRY palette[256])
{
   RES res;
   ResOpen(&res, resId);
   ResRead(&res, palette, 256 * sizeof(PALETTEENTRY));
   ResClose(&res);
}


// Load an xmp file as a resource ////////////////////////////////////////
void XMP_Load(XMPDATA* xmp, UINT resId)
{
   RES   res;
   ULONG headLen;
   ULONG buffLen;

   ResOpen(&res, resId);

   // read the image file
   ResRead(&res, &xmp->height, sizeof(xmp->height)); // read height
   ResRead(&res, &xmp->width,  sizeof(xmp->width));  // read width
   ResSeek(&res, sizeof(ULONG), SEEK_CUR);           // skip actual_lenth
   ResRead(&res, &headLen, sizeof(headLen));         // read header_length
   ResRead(&res, &buffLen, sizeof(buffLen));         // read buffer_length
   
   // allocate and read buffer & header
   xmp->header = new UCHAR[headLen];
   xmp->buffer = new UCHAR[buffLen];
   ResRead(&res, xmp->header, headLen);
   ResRead(&res, xmp->buffer, buffLen);

   ResClose(&res);
}


// copy an xmp file to a buffer //////////////////////////////////////////
void XMP_Copy(LPDDS dds, XMPDATA* data, ULONG x, ULONG y)
{
   ULONG  lPitch; // surface pitch
   UCHAR *buffer; // surface pointer
   UCHAR  match;  // color to match
   LONG   size;   // image size
   
   LONG i = 0; // pos in buffer
   LONG j = 0; // pos in comp_buff
   LONG k = 0; // pos in comp_head
   LONG l = 0; // match length counter
   LONG m = 0; // match lenght counter ajusted for surface pitch
   LONG n = 0; // distance gone in buffer

   DD_Lock(dds, &buffer, &lPitch);    // lock the DirectDraw surface
   size = data->width * data->height; // calculate the image size
   i = (y * lPitch) + x;              // set the starting position in the buffer

   // write the xmp data to the buffer, offsetting by x and y
   do
   {
      // get the color
      match = data->buffer[j];

      // see if it is a match & length pair
      if(data->header[k / 8] & 0x01 << (k % 8))
      {
         m = 0; // reset m

         for(l = 0; l < data->buffer[j + 1]; l++)
         {
            // make sure we go to the proper location in the next line.
            // if(is on edge) compensate position, note that the number
            // being mod'ed cannot be less than zero, that is why lPitch
            // is added to the number being mod'ed
            if((i + m - data->width - x + lPitch) % lPitch == 0)
               m += (lPitch - data->width);

            buffer[i + m] = match; // set the color in the surface buffer
            m++;                   // move forward
         }

         n += l; // move distance in uncompressed image
         i += m; // move pos in surface buffer
         j += 2; // move pos in compressed buffer 
         k += 1; // move pos in compressed header
      }
      else
      {     
         // make sure we go to the proper location in the next line.
         // if(is on edge) compensate position, note that the number
         // being mod'ed cannot be less than zero, that is why lPitch
         // is added to the number being mod'ed
         if((i - data->width - x + lPitch) % lPitch == 0)
            i += (lPitch - data->width);

         // set the color in the surface buffer
         buffer[i] = match;

         i++; // move pos in surface buffer
         n++; // move distance in uncomressed image
         j++; // move pos in compressed buffer
         k++; // move pos in compressed header
      }

   } while(n < size);

   // unlock the surface
   DD_UnLock(dds);
}

// copy the specified resource to the DirectDraw Surface - used in restore
void XMP_ReLoad(LPDDS dds, UINT resId)
{
   XMPDATA xmp;
   XMP_Load(&xmp, resId);     // load the image
   XMP_Copy(dds, &xmp, 0, 0); // copy the image to the buffer
   XMP_Delete(&xmp);          // release the xmp image
}


// delete the data in an xmp /////////////////////////////////////////////
void XMP_Delete(XMPDATA* xmp)
{
   // release the image buffer
   delete [] xmp->header;
   delete [] xmp->buffer;
}


/*——————————————————————————————————————————————————————————————————————*/
// RES functions
/*——————————————————————————————————————————————————————————————————————*/

// Test to see if the resource file is avalable //////////////////////////
bool ResAvalable(void)
{
   int file;
   if((file = _open(FILE_RESDATA, _O_BINARY | _O_RDONLY)) == -1) return 0;
   _close(file);
   return 1;
}

// Open the main resource file and seek to the resource's location ///////
void ResOpen(RES *res, UCHAR id)
{
   res->id = id;

   // move to the begining of the data for this particular resource ID
   if((res->file = _open(FILE_RESDATA,  _O_BINARY | _O_RDONLY)) == -1) return;

   _lseek(res->file, id*sizeof(ULONG), SEEK_SET);
   _read(res->file, &res->offset, sizeof(res->offset));
   _lseek(res->file, res->offset, SEEK_SET);
}

// Seek in the resource file /////////////////////////////////////////////
void ResSeek(RES *res, long offset, UCHAR origin)
{
   if(origin == SEEK_SET) {
      _lseek(res->file, res->offset + offset, SEEK_SET);
   } else _lseek(res->file, offset, SEEK_CUR);
}

// Read/write the resource file //////////////////////////////////////////
void ResRead(RES *res, void *buffer, ULONG length)
{
   _read(res->file, buffer, length);
}

// Close the resource file ///////////////////////////////////////////////
void ResClose(RES *res)
{
   _close(res->file);
}