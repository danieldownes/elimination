/************************************************************************/
// utility.cpp - Main DirectDraw and DirectInput utilities
//
// Author: Justin Hoffman
// Date:   5/1/00 - 7/18/2000
/************************************************************************/

// Headers ///////////////////////////////////////////////////////////////
#include "utility.h"
#include "sound.h"
#include "game.h"
#include "user.h"
#include "network.h"


//������������������������������������������������������������������������
// Globals
//������������������������������������������������������������������������

// DirectX objects ///////////////////////////////////////////////////////
       LPDIRECTDRAW         lpdd    = 0; // main DirectDraw object
static LPDIRECTINPUT8       lpdi    = 0; // main DirectInput object
static LPDIRECTINPUTDEVICE8 diKeys  = 0; // DI keyboard object
static LPDIRECTINPUTDEVICE8 diMouse = 0; // DI mouse object

// user interface ////////////////////////////////////////////////////////
UCHAR *key_lfdwn; // key state table for the last frame
UCHAR *key_down ; // key state table
UCHAR  cur_lfdwn; // was the mouse down in the last frame?
UCHAR  cur_down ; // is left mouse button down?
long   cur_x    ; // mouse x chord
long   cur_y    ; // mouse y chord

// resource loading data /////////////////////////////////////////////////
static HANDLE gResFile   = 0;
static long   gResOffset = 0;

// xmp loading data //////////////////////////////////////////////////////
static UCHAR *gXmpBuffer = 0;
static UCHAR *gXmpHeader = 0;
static ULONG  gXmpHeight = 0;
static ULONG  gXmpWidth  = 0;

// misc varables /////////////////////////////////////////////////////////
RECT R_SCREEN = {0, 0, SCREEN_W, SCREEN_H};

// windows interface /////////////////////////////////////////////////////
HINSTANCE gHInst = 0;       // application instance
HWND      gHWnd  = 0;       // window handle
char      gszAppPath[1024]; // path name of the executable


//������������������������������������������������������������������������
// General DirectDraw Stuff
//������������������������������������������������������������������������

// Initilize DirectDraw //////////////////////////////////////////////////
long DDInit(void)
{  
   DDSURFACEDESC ddsd;
   DDSCAPS       ddscaps;

   // create the main DirectDraw object
   if(DirectDrawCreate(0, &lpdd, 0) != DD_OK) {
      DebugSimple("FAILED: DDInit; DirectDrawCreate");
      return 0;
   }

   // set the cooperation level
   if(lpdd->SetCooperativeLevel(gHWnd, DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE) != DD_OK) {
      DebugSimple("FAILED: DDInit; SetCooperativeLevel");
      return 0;
   }

   // set the display mode
   if(lpdd->SetDisplayMode(SCREEN_W, SCREEN_H, 8) != DD_OK) {
      DebugSimple("FAILED: DDInit; SetDisplayMode");
      return 0;
   }

   // setup ddsd to create front buffer
   STRUCT_INIT(ddsd);
   ddsd.dwFlags           = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
   ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
   ddsd.dwBackBufferCount = 1;

   // create the front buffer
   if(lpdd->CreateSurface(&ddsd, &ddsFront, 0) != DD_OK) {
      DebugSimple("FAILED: DDInit; CreateSurface");
      return 0;
   }

   // create the back buffer
   ddscaps.dwCaps = DDSCAPS_BACKBUFFER;
   if(ddsFront->GetAttachedSurface(&ddscaps, &ddsBack) != DD_OK) {
      DebugSimple("FAILED: DDInit; GetAttachedSurface");
      return 0;
   }

   // clear the front & back buffers
   DDClear(ddsFront, 0);
   DDClear(ddsBack , 0);
   return 1;
}


//������������������������������������������������������������������������
// Create DirectDraw surfaces
//������������������������������������������������������������������������

// Create a blank DirectDraw surface /////////////////////////////////////
void DdsCreate(LPDDS *dds, ULONG width, ULONG height)
{
   DDCOLORKEY    ddck;
   DDSURFACEDESC ddsd;

   // setup the surface description
   STRUCT_INIT(ddsd);
   ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
   ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
   ddsd.dwWidth        = width;
   ddsd.dwHeight       = height;

   // create the surface
   if(lpdd->CreateSurface(&ddsd, dds, 0) != DD_OK) {
      DebugSimple("FAILED: DdsCreate; CreateSurface");
      return;
   }

   // clear the surface
   DDClear(*dds, CLR_TRANS);

   // set the new surfaces color key
   ddck.dwColorSpaceHighValue = CLR_TRANS;
   ddck.dwColorSpaceLowValue  = CLR_TRANS;

#ifdef GAME_DEBUG
   DDRVAL_DEBUG((*dds)->SetColorKey(DDCKEY_SRCBLT, &ddck), "DdsCreate (SetColorKey)");
#else
   DDRVAL((*dds)->SetColorKey(DDCKEY_SRCBLT, &ddck));
#endif
}

// Create a DirectDraw surface from an XMP ///////////////////////////////
void DdsCreate(LPDDS *dds, UCHAR id)
{
   XmpLoad(id);
   DdsCreate(dds, gXmpWidth, gXmpHeight);
   XmpCopy(*dds, 0, 0);
   XmpDelete();
}

// Restore a DirectDraw surface //////////////////////////////////////////
long DdsRestore(LPDDS dds)
{
   if(dds->Restore() != DD_OK) return 0;
   DDClear(dds, CLR_TRANS);
   return 1;
}


//������������������������������������������������������������������������
// Drawing
//������������������������������������������������������������������������

// Draw a rectangle //////////////////////////////////////////////////////
void DDRect(LPDDS dds, UCHAR border, UCHAR fill, LPRECT r, UCHAR flags)
{
   UCHAR  *buffer;
   ULONG   lpitch;
   DDBLTFX bltfx;
   long    bottom;
   long    right;
   long    i;

   // Fill the rectangle
   if((flags & DDRECT_NOFILL) == 0) {

      STRUCT_INIT(bltfx);
      bltfx.dwFillColor = fill;

#ifdef GAME_DEBUG
      if(!DDRVAL_DEBUG(dds->Blt(r, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx), "DDRect (Blt)"))
         DebugWrite("ERROR: DDRect: fill = %d, border = %d, r = %d,%d,%d,%d",
            fill, border, r->left, r->top, r->right, r->bottom);
#else
      DDRVAL(dds->Blt(r, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx));
#endif
   }

   // draw the borders
   if((flags & DDRECT_NOBORDER) == 0) {

      DDLock(dds, &buffer, &lpitch);

      // cashe bottom & right values
      bottom = r->bottom - 1;
      right  = r->right  - 1;

      // draw the top & bottom border
      for(i = r->left; i < r->right; i++) {
         buffer[i + r->top * lpitch] = border;
         buffer[i + bottom * lpitch] = border;
      }

      // draw the left and & border
      for(i = r->top; i < r->bottom; i++) {
         buffer[r->left + i*lpitch] = border;
         buffer[right   + i*lpitch] = border;
      }

      DDUnlock(dds);
   }
}

// Draw a line ///////////////////////////////////////////////////////////
void DDLine(LPDDS dds, UCHAR color, long xa, long ya, long xb, long yb)
{
   UCHAR *buffer;
   ULONG  lpitch;
   long   i;

   DDLock(dds, &buffer, &lpitch);

   // See if it is a horisontal line, if so, go from
   // left to right otherwise from top to bottom
   if(ya == yb) {
      for(i = xa; i < xb; i++) buffer[i + ya*lpitch] = color;
   } else if(xa == xb) {
      for(i = ya; i < yb; i++) buffer[xa + i*lpitch] = color;
   }

   DDUnlock(dds);
}


//������������������������������������������������������������������������
// DirectDraw Wrappers
//������������������������������������������������������������������������

// Wrapper for *IDirectDrawSurface->BltFast() ////////////////////////////
void DDBltFast(
   LPDDS  ddsDst, // destination surface
   LPDDS  ddsSrc, // source surface
   ULONG  x     , // destination x chord
   ULONG  y     , // destination y chord
   LPRECT r     , // source rectangle
   DWORD  flags)  // bltfast flags
{
#ifdef GAME_DEBUG
   if(!DDRVAL_DEBUG(ddsDst->BltFast(x, y, ddsSrc, r, flags | DDBLTFAST_WAIT), "DDBltFast (BltFast)"))
      DebugWrite("ERROR: DDBltFast: x = %d, y = %d, r = %d,%d,%d,%d",
         x, y, r->left, r->top, r->right, r->bottom);
#else
   DDRVAL(ddsDst->BltFast(x, y, ddsSrc, r, flags | DDBLTFAST_WAIT));
#endif
}

// Wrapper for *IDirectDrawSurface->BltFast(), with clipping /////////////
void DDBltClip(
   LPDDS  ddsDst, // destination surface
   LPDDS  ddsSrc, // source surface
   long   x,      // destination x chord
   long   y,      // destination y chord
   LPRECT rclip,  // rectangle to fit inside of
   LPRECT rsrc,   // source rectangle
   DWORD  flags)  // bltfast flags
{
   RECT r;
   long dright;
   long dbotom;

   // cashe destination right & bottom values
   dright = x + rsrc->right  - rsrc->left;
   dbotom = y + rsrc->bottom - rsrc->top;

   // make sure some part of dest rect is within rclip, if not, exit now
   if(x >= rclip->right  || dright <= rclip->left ||
      y >= rclip->bottom || dbotom <= rclip->top) return;

   // copy source rectangle
   memcpy(&r, rsrc, sizeof(RECT));

   // check x chord
   if(x < rclip->left) {
      r.left += rclip->left - x;
      x       = rclip->left;
   }

   // check y chord
   if(y < rclip->top) {
      r.top += rclip->top - y;
      y      = rclip->top;
   }

   // check if right & bottom of src need shortened
   if(dright > rclip->right)  r.right  -= dright - rclip->right;
   if(dbotom > rclip->bottom) r.bottom -= dbotom - rclip->bottom;

#ifdef GAME_DEBUG
   if(!DDRVAL_DEBUG(ddsDst->BltFast(x, y, ddsSrc, &r, flags | DDBLTFAST_WAIT), "DDBltClip (Blt)"))
      DebugWrite("ERROR: DDBltClip: x = %d, y = %d, r = %d,%d,%d,%d",
         x, y, r.left, r.top, r.right, r.bottom);
#else
   DDRVAL(ddsDst->BltFast(x, y, ddsSrc, &r, flags | DDBLTFAST_WAIT));
#endif
}

// Clear a surface using *IDirectDrawSurface->Blt() //////////////////////
void DDClear(LPDDS dds, UCHAR color)
{
   DDBLTFX bltfx;
   STRUCT_INIT(bltfx);
   bltfx.dwFillColor = color;

#ifdef GAME_DEBUG
   if(!DDRVAL_DEBUG(dds->Blt(0, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx), "DDClear"))
      DebugWrite("ERROR: DDClear: color = %d", color);
#else
   DDRVAL(dds->Blt(0, 0, 0, DDBLT_COLORFILL | DDBLT_WAIT, &bltfx));
#endif
}


#ifdef GAME_DEBUG

// Wrapper for *IDirectDrawSurface->GetDC() //////////////////////////////
void DDGetDC(LPDDS dds, HDC *hdc)
{
   DDRVAL_DEBUG(dds->GetDC(hdc), "DDGetDC");
}

// Wrapper for *IDirectDrawSurface->ReleaseDC() //////////////////////////
void DDReleaseDC(LPDDS dds, HDC hdc)
{
   DDRVAL_DEBUG(dds->ReleaseDC(hdc), "DDReleaseDC");
}

#endif // #ifdef GAME_DEBUG

// Wrapper for *IDirectDrawSurface->Lock() ///////////////////////////////
void DDLock(LPDDS dds, UCHAR **buff, ULONG *lpitch)
{
   DDSURFACEDESC ddsd;
   STRUCT_INIT(ddsd);

#ifdef GAME_DEBUG
   DDRVAL_DEBUG(dds->Lock(0, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 0), "DDLock");
#else
   DDRVAL(dds->Lock(0, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, 0));
#endif

   // set the pointer to the video memory
   *buff   = (UCHAR*)ddsd.lpSurface;
   *lpitch = ddsd.lPitch;
}

// Wrapper for *IDirectDrawSurface->Unlock() /////////////////////////////
void DDUnlock(LPDDS dds)
{
#ifdef GAME_DEBUG
   DDRVAL_DEBUG(dds->Unlock(0), "DDUnlock");
#else
   DDRVAL(dds->Unlock(0));
#endif
}


//������������������������������������������������������������������������
// XMP functions
//������������������������������������������������������������������������

// Load an XAL ///////////////////////////////////////////////////////////
void XalLoad(PALETTEENTRY palette[], UCHAR id)
{
   ResOpen(id);
   ResRead(palette, 256*sizeof(PALETTEENTRY));
   ResClose();
}

// Load an XMP ///////////////////////////////////////////////////////////
void XmpLoad(UCHAR id)
{
   ResOpen(id);
   XmpRead();
   ResClose();
}

// Read an opended XMP file //////////////////////////////////////////////
void XmpRead(void)
{
   ULONG HeadLen;
   ULONG BuffLen;

   // make sure an xmp is not already open
#ifdef GAME_DEBUG
   if(gXmpHeader != 0 || gXmpBuffer != 0) {
      DebugSimple("ERROR: XmpRead called without XmpDelete called before it");
      return;
   }
#endif

   // open and read the image file
   ResRead(&gXmpHeight, sizeof(gXmpHeight)); // read height
   ResRead(&gXmpWidth,  sizeof(gXmpWidth));  // read width
   ResSeek(sizeof(ULONG), SEEK_CUR);         // skip actual_lenth
   ResRead(&HeadLen, sizeof(HeadLen));       // read header_length
   ResRead(&BuffLen, sizeof(BuffLen));       // read buffer_length

   // allocate and read buffer & header
   gXmpHeader = (UCHAR*)MoreMem(HeadLen);
   gXmpBuffer = (UCHAR*)MoreMem(BuffLen);
   ResRead(gXmpHeader, HeadLen);
   ResRead(gXmpBuffer, BuffLen);
}

// Copy an XMP to a DirectDraw surface ///////////////////////////////////
void XmpCopy(LPDDS dds, ULONG x, ULONG y)
{
   ULONG  lPitch; // surface pitch
   UCHAR *buffer; // surface pointer
   UCHAR  match ; // color to match
   long   size  ; // image size

   long i    ; // pos in buffer
   long j = 0; // pos in comp_buff
   long k = 0; // pos in comp_head
   long l    ; // match length counter
   long m    ; // match lenght counter ajusted for surface pitch
   long n = 0; // distance gone in uncompressed buffer

   DDLock(dds, &buffer, &lPitch);
   size = gXmpWidth * gXmpHeight;

   // set the starting position in the buffer
   i = y*lPitch + x;

   do {
      match = gXmpBuffer[j];

      // see if it is a match & length pair      
      if(gXmpHeader[(ULONG)(k / 8)] & (0x01 << (k % 8))) {

         for(l = m = 0; l < gXmpBuffer[j + 1]; l++) {

            // make sure we go to the proper location in the next line.
            // if(is on edge) compensate position, note that the number
            // being mod'ed cannot be less than zero, that is why lPitch
            // is added to the number being mod'ed
            if((i + m - gXmpWidth - x + lPitch) % lPitch == 0) m += (lPitch - gXmpWidth);

            buffer[i + m] = match; // set the color in the surface buffer
            m++;                   // move forward
         }

         // update varables
         n += l;  i += m;  j += 2;  k += 1;
      
      } else {

         // compensate for being on the edge of the image
         if((i - gXmpWidth - x + lPitch) % lPitch == 0) i += (lPitch - gXmpWidth);

         // update varables
         buffer[i] = match;
         i++; n++; j++; k++;
      }

   } while(n < size);

   DDUnlock(dds);
}

// Restore the DirectDraw surface & reload XMP file ///////////////////////
long XmpReload(LPDDS dds, UCHAR id)
{
   // restore the surface
   if(dds->Restore() != DD_OK) return 0;

   // copy the image back to the surface
   XmpLoad(id);
   XmpCopy(dds, 0, 0);
   XmpDelete();
   return 1;
}

// Delete a previously loaded XMP ////////////////////////////////////////
void XmpDelete(void)
{
   // release the header & buffer
   DELETE_ARRAY(gXmpHeader);
   DELETE_ARRAY(gXmpBuffer);
}


//������������������������������������������������������������������������
// RES functions
//������������������������������������������������������������������������
/*
// Test to see if the resource file is avalable //////////////////////////
long ResAvalable(void)
{
   // attemp to open the file
   if((gResFile = CreateFile(RES_FILENAME, GENERIC_READ, 0, 0,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) return 0;

   // return sucess
   CloseHandle(gResFile);
   return 1;
}
*/
// Open the main resource file and seek to the resource's location ///////
void ResOpen(UCHAR id)
{
   DWORD dwRead;

   // set the current directory
   SetCurrentDirectory(gszAppPath);

   // attemp to open the resource file
   if((gResFile = CreateFile(RES_FILENAME, GENERIC_READ, 0, 0,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) {
      DebugSimple("FAILED: ResOpen; CreateFile");
      return;
   }

   // seek to the begining of the resource
   SetFilePointer(gResFile, id*sizeof(ULONG), 0, FILE_BEGIN);
   ReadFile(gResFile, &gResOffset, sizeof(gResOffset), &dwRead, 0);
   SetFilePointer(gResFile, gResOffset, 0, FILE_BEGIN);
}

// Seek in the resource file /////////////////////////////////////////////
void ResSeek(long offset, UCHAR origin)
{
   if(origin == SEEK_SET) {
      SetFilePointer(gResFile, gResOffset + offset, 0, FILE_BEGIN);
   } else {
      SetFilePointer(gResFile, offset, 0, FILE_CURRENT);
   }
}

// Read/write the resource file //////////////////////////////////////////
void ResRead(void *buffer, ULONG length)
{
   DWORD dwRead;
   ReadFile(gResFile, buffer, length, &dwRead, 0);
}

// Close the resource file ///////////////////////////////////////////////
void ResClose(void)
{
   CloseHandle(gResFile);
}


//������������������������������������������������������������������������
// DirectInput Code
//������������������������������������������������������������������������

// Initilize DirectInput /////////////////////////////////////////////////
long DIInit(void)
{
   if(DirectInput8Create(gHInst, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&lpdi, 0) != DI_OK) {
      DebugSimple("FAILED: DIInit; DirectInputCreate");
      return 0;
   }

   // keyboard input
   if(lpdi->CreateDevice(GUID_SysKeyboard, &diKeys, 0)                             != DI_OK
      || diKeys->SetCooperativeLevel(gHWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK
      || diKeys->SetDataFormat(&c_dfDIKeyboard)                                    != DI_OK
      || diKeys->Acquire()                                                         != DI_OK

   // mouse input
      || lpdi->CreateDevice(GUID_SysMouse, &diMouse, 0)                             != DI_OK
      || diMouse->SetCooperativeLevel(gHWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK
      || diMouse->SetDataFormat(&c_dfDIMouse)                                       != DI_OK
      || diMouse->Acquire()                                                         != DI_OK) return 0;

   // allocate the key state varables
   key_down  = (UCHAR*)MoreMem(256);
   key_lfdwn = (UCHAR*)MoreMem(256);
 
   // set the mouse location to the center of the screen
   cur_x     = (SCREEN_W / 2);
   cur_y     = (SCREEN_H / 2);
   cur_down  = 0;
   cur_lfdwn = 0;
   return 1;
}

// Close DirectInput /////////////////////////////////////////////////////
void DIClose(void)
{
   if(lpdi) {

      // release the mouse
      if(diMouse) {
         diMouse->Unacquire();
         diMouse->Release();
         diMouse = 0;
      }

      // release keyboard
      if(diKeys) {
         diKeys->Unacquire();
         diKeys->Release();
         diKeys = 0;
      }
   
      // release DirectInput
      lpdi->Release();
      lpdi = 0;
   }

   // release keyboard state data
   DELETE_ARRAY(key_down);
   DELETE_ARRAY(key_lfdwn);
}

// Update the mouse and keyboard statuses ////////////////////////////////
void DIGetInput(void)
{
   UCHAR        *temp;
   DIMOUSESTATE  mouse;

   // get mouse data
   if((diMouse->GetDeviceState(sizeof(DIMOUSESTATE), (void*)&mouse) == DIERR_INPUTLOST)) {
#ifdef GAME_DEBUG
      if(diMouse->Acquire() != DI_OK) DebugWrite("FAILED: DIGetInput (diMouse->Aquire)");
#else
      diMouse->Acquire();
#endif
   }

   // translate mouse data
   cur_x    += mouse.lX;
   cur_y    += mouse.lY;
   cur_lfdwn = cur_down;
   cur_down  = mouse.rgbButtons[0]? 1 : 0;

   // make sure mouse is within screen
   if(cur_x >= SCREEN_W) cur_x = (SCREEN_W - 1);
   else if(cur_x < 0)    cur_x = 0;

   if(cur_y >= SCREEN_H) cur_y = (SCREEN_H - 1);
   else if(cur_y < 0)    cur_y = 0;

   // swap keyboard data pointers
   temp      = key_down;
   key_down  = key_lfdwn;
   key_lfdwn = temp;

   // get keyboard data
   if((diKeys->GetDeviceState(256, (void*)key_down) == DIERR_INPUTLOST)) {
#ifdef GAME_DEBUG
      if(diKeys->Acquire() != DI_OK) DebugWrite("FAILED: DIGetInput (diKeys->Aquire)");
#else
      diKeys->Acquire();
#endif
   }
}


//������������������������������������������������������������������������
// Windows Interface
//������������������������������������������������������������������������

// Main window callback //////////////////////////////////////////////////
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   switch(msg) {

   // main window is being bothered by someone else
   case WM_ACTIVATEAPP:

      // suspend the program
      if(wParam == 0) {
         if(prog_state != PROG_SUSPENDED && prog_state != PROG_CLOSE) {
            if(!gbNetwork) GamePause();
            prog_state = PROG_SUSPENDED;
            gbVisible  = 0;
            MusicPause();
         }

      // resume the program
      } else if(prog_state == PROG_SUSPENDED) {
         if(!gbNetwork) GameResume();
         prog_state = PROG_PLAYING;
         gbVisible  = 1;
         MusicResume();
      }
      break;

   // windows wants us to draw something, so i will draw nothing
   case WM_PAINT:
      ValidateRect(hWnd, 0);
      break;

   // check network messages
   case PROG_CHECKDPMSG:
      DpCheckMsg();
      break;

   // main window is really getting bothered by someone else
   case WM_CLOSE:
      PostMessage(hWnd, PROG_KILL, 0, 0);
      break;

   // check to see if CD Audio is now avalable
   case WM_DEVICECHANGE:
      MusicCheckCdChange(wParam, lParam);
      break;

   // check to see if the next song should be started
   case MM_MCINOTIFY:
   case PROG_CHECKMUSIC:
      MusicCheckNext(wParam);
      break;

   // process debug F keys
#ifdef GAME_DEBUG
   case WM_KEYDOWN:
      if(wParam == VK_F1) DebugWriteBmp();
      break;
#endif

   // let windows process all other messages
   default: return DefWindowProc(hWnd, msg, wParam, lParam);
   }

   return 0;
}

// WinMain function //////////////////////////////////////////////////////
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShow)
{
   MSG        msg;
   WNDCLASSEX wc;

   gHInst = hInst;
   
   // create saved directory &
   // get name of current directory
   CreateDirectory("data", NULL);
   GetCurrentDirectory(1024, gszAppPath);

   // setup & register window class
   memset(&wc, 0, sizeof(wc));
   wc.cbSize        = sizeof(wc);
   wc.hInstance     = hInst;
   wc.lpfnWndProc   = WndProc;
   wc.lpszClassName = TXT_WINCLASS;
   //wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
   wc.hIcon         = LoadIcon(hInst, (LPCTSTR)1);
   wc.hCursor       = LoadCursor(0, (LPCTSTR)IDC_ARROW);
   wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
   if(!RegisterClassEx(&wc)) return 0;

   // create main window
   if(!(gHWnd = CreateWindowEx(WS_EX_TOPMOST, TXT_WINCLASS, TXT_WINTITLE,
      WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics(SM_CXSCREEN),
      GetSystemMetrics(SM_CYSCREEN), 0, 0, hInst, 0))) return 0;

   // initilize DirectX & load game stuff
   if(!ProgInit()) return 0;

   // main application event loop
   while(1) {
      if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {

         if(msg.message == PROG_KILL) {
            DestroyWindow(gHWnd);
            break;
         }

         TranslateMessage(&msg);
         DispatchMessage(&msg);
      }
      ProgLoop();
   }

   ProgClose();
   return msg.wParam;
}


#ifdef GAME_DEBUG

//������������������������������������������������������������������������
// Debug/Error functions
//������������������������������������������������������������������������

// Globals ///////////////////////////////////////////////////////////////
long   gMemAllocated = 0; // amount of memory allocated
long   gnMemAllocs   = 0; // number of times memmory is allocated
DWORD  debug_lstfps  = 0; // last tick the framrate was updated
DWORD  debug_start   = 0; // debug start time
UCHAR  debug_swfps   = 0; // should the framerate be shown?
UCHAR  debug_error   = 0; // has an error been detected?
HANDLE debug_file    = 0; // error file
int    debug_fps     = 0; // framerate
int    debug_frames  = 0; // number of frames this second


// Process DirectDraw return values [debug version] //////////////////////
long DDRVAL_DEBUG(HRESULT rval, char *sFunction)
{
   if(rval == DDERR_SURFACELOST) {
      if(!ProgRestoreSurfaces()) return 0;
   } else if(rval != DD_OK) {
      DebugWrite("GENERAL ERROR %d : %s", rval, sFunction);
      return 0;
   }

   return 1;
}

// Open all nessary debug files //////////////////////////////////////////
void DebugOpen(void)
{
   SYSTEMTIME time;
   DWORD      dwActual;
   char       sText[128];

   // get current system time
   GetLocalTime(&time);

   // open debug file
   DeleteFile(DEBUG_FILENAME);
   debug_file = CreateFile(DEBUG_FILENAME, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

   // create header string
   wsprintf(sText, "ELIMINATION 3.0 DEBUG FILE\r\nDATE: %d/%d/%d\r\nTIME: %d:%d\r\n\r\n",
      time.wMonth, time.wDay, time.wYear, time.wHour, time.wMinute);

   // write the header string
   WriteFile(debug_file, sText, lstrlen(sText), &dwActual, 0);

   // initilize globals
   debug_lstfps = debug_start = GetTickCount();
}

// Close the debug files /////////////////////////////////////////////////
void DebugClose(void)
{
   CloseHandle(debug_file);
}

// Draw the frame rate to the screen /////////////////////////////////////
void DebugDrawFps(void)
{
   MEMORYSTATUS memstat    ;
   DDCAPS       ddcaps     ;
   HDC          hdc        ;
   char         sText[128];

   // see if the framerate should be hidden
   if(KEY_CLICK(DIK_F2)) debug_swfps = !debug_swfps;

   // make sure we want the framerate shown
   if(!debug_swfps) return;

   // check to see if its time to reset
   if(gThisTick - debug_lstfps > 1000) {
      debug_fps    = debug_frames;
      debug_lstfps = gThisTick;
      debug_frames = 0;
   }

   debug_frames++;

   // get v-ram data
   STRUCT_INIT(ddcaps);
   lpdd->GetCaps(&ddcaps, 0);

   // get memory data
   memset(&memstat, 0, sizeof(memstat));
   memstat.dwLength = sizeof(MEMORYSTATUS);
   GlobalMemoryStatus(&memstat);

   // create the string
   wsprintf(sText, "FPS: %d - VM Used: %d - M Load: %d - M Free: %d%s",
      debug_fps, (ddcaps.dwVidMemTotal - ddcaps.dwVidMemFree),
      memstat.dwMemoryLoad, memstat.dwAvailPhys,
      debug_error? " - Error(s) Detected":"");

   // draw the FPS and memory data
   DDGetDC(ddsBack, &hdc);
   TextOut(hdc, 0, 0, sText, lstrlen(sText)); 
   DDReleaseDC(ddsBack, hdc);
}

// Write a bitmap of the current screen //////////////////////////////////
void DebugWriteBmp(void)
{
   BITMAPFILEHEADER bmpFileHead ; // bitmap file header
   BITMAPINFOHEADER bmpInfoHead ; // bitmap info header
   PALETTEENTRY     palette[256]; // the palette
   UCHAR           *buff        ; // pointer to front buffer bits
   ULONG            lPitch      ; // surface pitch 
   DWORD            dwActual    ; // actual number of bytes written
   HANDLE           hFile       ; // pointer to the file
   int              i           ; // counter

   // clear the file and info headers 
   memset(&bmpFileHead, 0, sizeof(bmpFileHead));
   memset(&bmpInfoHead, 0, sizeof(bmpInfoHead));

   // setup the file header
   bmpFileHead.bfType    = 0x4D42;
   bmpFileHead.bfSize    = sizeof(BITMAPFILEHEADER);
   bmpFileHead.bfOffBits = 1078;

   // setup the info header
   bmpInfoHead.biSize        = sizeof(BITMAPINFOHEADER);
   bmpInfoHead.biWidth       = SCREEN_W;
   bmpInfoHead.biHeight      = SCREEN_H;
   bmpInfoHead.biSizeImage   = SCREEN_W*SCREEN_H;
   bmpInfoHead.biCompression = BI_RGB;
   bmpInfoHead.biBitCount    = 8;
   bmpInfoHead.biPlanes      = 1;

   // get the palette
   ddpMain->GetEntries(0, 0, 256, palette);

   // translate the palette
   for(i = 0; i < 256; i++) {
      palette[i].peBlue ^= palette[i].peRed;
      palette[i].peRed  ^= palette[i].peBlue;
      palette[i].peBlue ^= palette[i].peRed;
   }

   // create the bitmap file
   if((hFile = CreateFile("1.bmp", GENERIC_WRITE, 0, 0,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) == INVALID_HANDLE_VALUE) return;

   // write the bitmap data
   WriteFile(hFile, &bmpFileHead, sizeof(bmpFileHead), &dwActual, 0);
   WriteFile(hFile, &bmpInfoHead, sizeof(bmpInfoHead), &dwActual, 0);
   WriteFile(hFile, palette, 256*sizeof(PALETTEENTRY), &dwActual, 0);

   // lock the front buffer
   DDLock(ddsFront, &buff, &lPitch);

   // write the image buffer
   for(i = 0; i < SCREEN_H; i++)
      WriteFile(hFile, &buff[(SCREEN_H - 1 - i) * SCREEN_W], SCREEN_W, &dwActual, 0);

   // clean up
   DDUnlock(ddsFront);
   CloseHandle(hFile);
}

// Write to the error file ///////////////////////////////////////////////
void DebugWrite(char *sFormat,...)
{
   va_list va;
   DWORD   time;
   DWORD   tick;
   DWORD   sec;
   DWORD   min;
   DWORD   dwActual;
   char    sText[128];
   char    sFinal[128];

   // get elapsed time since game start
   time = GetTickCount() - debug_start;

   // create error string
   va_start(va, sFormat);
   wvsprintf(sText, sFormat, va);
   va_end(va);

   // determine time values
   min  = (long)(time / 60000);
   sec  = (long)((time - min * 60000) / 1000);
   tick = ((time - min * 60000) % 1000);

   // create final string
   wsprintf(sFinal, "%3.0d:%3.0d:%3.0d, %s\r\n", min, sec, tick, sText);

   // write final string
   WriteFile(debug_file, sFinal, lstrlen(sFinal), &dwActual, 0);
}

#endif // #ifdef GAME_DEBUG


//������������������������������������������������������������������������
// Memory Functions
//������������������������������������������������������������������������

// allocate more memory //////////////////////////////////////////////////
void *MoreMem(ULONG amount)
{
   void *rval;

   // allocate & clean memory
   rval = malloc(amount);
   memset(rval, 0, amount);

   /*
   // debug stuff
#ifdef GAME_DEBUG
   gnMemAllocs++;
   gMemAllocated += amount;
   DebugWrite("+ gnMemAllocs = %d, ptr = %d, gMemAllocated = %d", gnMemAllocs, rval, gMemAllocated);
#endif
   */
   // return memory pointer
   return rval;
}

#ifdef GAME_DEBUG

// delete some memory ////////////////////////////////////////////////////
void LessMem(void *ptr)
{
   /*
   gnMemAllocs--;
   DebugWrite("- gnMemAllocs = %d, ptr = %d", gnMemAllocs, ptr);
   */
   free(ptr);
}

#endif // #ifdef GAME_DEBUG