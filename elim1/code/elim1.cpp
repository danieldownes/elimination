/**************************************************************/
// elim1.cpp - Main Code
// Written By:		Justin Hoffman
// Date Written:	Feb 2, 1999 - April 7, 1999
/**************************************************************/

/////////////////////////////////
// Headers
#include "stdafx.h"
#include "resource.h"
#include "class.h"
#include "elim1.h"
#include "globals.h"
#include "pcPlayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <mmsystem.h>


/////////////////////////////////
// Setup Functions
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if(!InitInstance(hInstance, nCmdShow)) return FALSE;
	HACCEL hAccelTable=LoadAccelerators(hInstance, (LPCTSTR)IDC_PCT01);

	// Main message loop:
	while(GetMessage(&msg, NULL, 0, 0)) 
	{
		if(!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize			=sizeof(WNDCLASSEX); 
	wcex.style			=(CS_HREDRAW | CS_VREDRAW);
	wcex.lpfnWndProc	=(WNDPROC)WndProc;
	wcex.cbClsExtra		=0;
	wcex.cbWndExtra		=0;
	wcex.hInstance		=hInstance;
	wcex.hIcon			=LoadIcon(hInstance, (LPCTSTR)IDI_PCT01);
	wcex.hCursor		=LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	=ggb_lightgrey;
	wcex.lpszMenuName	=(LPCSTR)IDC_PCT01;
	wcex.lpszClassName	=gd_winclass;
	wcex.hIconSm		=LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	long x=(long)(GetSystemMetrics(SM_CXSCREEN)/2-249);
	long y=(long)(GetSystemMetrics(SM_CYSCREEN)/2-148.5);
	gg_hInst=hInstance; // Store instance handle in a global variable
	DWORD winStyle=(WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED | WS_BORDER);
	HWND hWnd=CreateWindow(gd_winclass, gd_wintitle, winStyle,
		x, y, 495, 297, NULL, NULL, hInstance, NULL);

	if(!hWnd)
	{
		reportError(1, FALSE);
		return FALSE;
	}

	gg_hwndMain=hWnd;
	ggh_maind=GetDC(gg_hwndMain);
	srand((unsigned int)GetTickCount());

	GetCurrentDirectory(256, gg_path);
	openSettings();
	makeBitmaps();

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	if(DialogBox(gg_hInst, (LPCTSTR)grd_names, gg_hwndMain,
		(DLGPROC)namesProc)==IDCANCEL)
	{
		DestroyWindow(gg_hwndMain);
		return TRUE;
	}

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	
	checkMenus();
	initBoard();
	pBoard();
	if(gg_options[2]) mNextSong();
	DialogBox(gg_hInst, (LPCTSTR)grd_roll, gg_hwndMain, (DLGPROC)rollWinProc);

	aScrollScreen();
	gg_canpaint=TRUE;

	SetTimer(gg_hwndMain, 1, 100, (TIMERPROC)TimerProc); // animate selected chip
	SetTimer(gg_hwndMain, 2, 75, (TIMERPROC)TimerProc); // animate 'f'
	rollDice();

	if(gg_options[4] && gg_player==1) startPcPlayer();
	return TRUE;
}


////////////////////////////////////////////////////////////////
// Window Procedures

/////////////////////////////////
// About Box
LRESULT CALLBACK aboutProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT com=LOWORD(wParam);
	switch(message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		if(com==IDOK || com==IDCANCEL) 
		{
			EndDialog(hDlg, com);
			return TRUE;
		}
		break;
	}
    return FALSE;
}

/////////////////////////////////
// Custom Box
LRESULT CALLBACK custProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;
	BOOL oneBtn;
	HWND btna;
	UINT com=LOWORD(wParam);
	switch(message)
	{
	case WM_INITDIALOG:

		// get item info
		btna=GetDlgItem(hDlg, gdi_buttona);
		oneBtn=(ggcb_numdat[1]);
		hIcon=LoadIcon(gg_hInst, (LPCTSTR)ggcb_numdat[0]);

		// set dlg items
		ShowWindow(btna, (oneBtn? SW_HIDE:SW_SHOW));
		SetDlgItemText(hDlg, gdi_text, ggcb_text[0]);
		SetDlgItemText(hDlg, gdi_buttona, ggcb_text[1]);
		SetDlgItemText(hDlg, gdi_buttonb, ggcb_text[oneBtn? 1:2]);
		SendDlgItemMessage(hDlg, gdi_icon, STM_SETICON, (WPARAM)hIcon, 0);
		DeleteObject(hIcon);
		return TRUE;
	case WM_COMMAND:
		if(com==gdi_buttona || com==gdi_buttonb || com==IDCANCEL) 
		{
			EndDialog(hDlg, com);
			return TRUE;
		}
		break;
	}
    return FALSE;
}

/////////////////////////////////
// Roll Dice Window
LRESULT CALLBACK rollWinProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch(message)
	{
	case WM_INITDIALOG:
		long wx, wy;
		wx=(long)(GetSystemMetrics(SM_CXSCREEN)/2-84.5);
		wy=(long)(GetSystemMetrics(SM_CYSCREEN)/2-82.5);

		int i, j, y;
		char buf[100];
		sprintf(buf, "%s has won the roll.", gg_names[gg_player]);

		HWND hwndIcon[2];
		hwndIcon[0]=GetDlgItem(hDlg, gdi_die1);
		hwndIcon[1]=GetDlgItem(hDlg, gdi_die2);
			
		HICON hBitA, hBitB;
		hBitA=LoadIcon(gg_hInst, (LPCTSTR)(gri_die1+gg_preroll[0]));
		hBitB=LoadIcon(gg_hInst, (LPCTSTR)(gri_die1+gg_preroll[1]));

		SendDlgItemMessage(hDlg, gdi_die1, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hBitA);
		SendDlgItemMessage(hDlg, gdi_die2, STM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hBitB);

		SetDlgItemText(hDlg, gdi_name1, gg_names[0]);
		SetDlgItemText(hDlg, gdi_name2, gg_names[1]);
		SetWindowPos(hwndIcon[0], NULL, -23, 11, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		SetWindowPos(hwndIcon[1], NULL, -23, 37, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		
		SetWindowPos(hDlg, NULL, wx, wy, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		ShowWindow(hDlg, SW_SHOW);
		UpdateWindow(hDlg);
		waitTicks(250);

		for(i=0; i<2; i++)
		{
			y=(26*i+11);
			for(j=0; j<18; j++)
			{
				SetWindowPos(hwndIcon[i], NULL, (2*j-23),
					y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				UpdateWindow(hwndIcon[i]);
				waitTicks(15);
			}
		}

		SetDlgItemText(hDlg, gdi_winer, buf);
		DeleteObject(hBitA);
		DeleteObject(hBitB);

		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}

/////////////////////////////////
// Get Names Dialog
LRESULT CALLBACK namesProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT com=LOWORD(wParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, gdi_name1, gg_names[0]);
		SetDlgItemText(hDlg, gdi_name2, 
			(gg_options[4]? "Computer":gg_names[1]));
		SendDlgItemMessage(hDlg, gdi_name2, EM_SETREADONLY,
			(WPARAM)gg_options[4], 0);
		CheckDlgButton(hDlg, gdi_pcplayer, (gg_options[4]? BST_CHECKED:BST_UNCHECKED));
		return TRUE;
	case WM_COMMAND:
		if(com==gdi_play || com==IDCANCEL)
		{
			// ajust global variables
			GetDlgItemText(hDlg, gdi_name1, gg_names[0], 15);
			GetDlgItemText(hDlg, gdi_name2, gg_names[1], 15);
			gg_options[4]=(IsDlgButtonChecked(hDlg, 
				gdi_pcplayer)==BST_CHECKED);

		}

		switch(com)
		{
		case gdi_play:
			EndDialog(hDlg, com);
			break;
		case IDCANCEL:
			if(customBox(hDlg, gri_question, grs_question,
				"Are you sure you want to exit?", "Exit",
				"Cancel")==gd_btnokay)
			{
				writeSettings();
				EndDialog(hDlg, com);
			}
			break;
		case gdi_pcplayer:
			BOOL isPc;
			isPc=IsDlgButtonChecked(hDlg, gdi_pcplayer);
			SendDlgItemMessage(hDlg, gdi_name2, EM_SETREADONLY,
				(WPARAM)isPc, 0);
			if(isPc) SetDlgItemText(hDlg, gdi_name2, "Computer");
			else SetDlgItemText(hDlg, gdi_name2, gg_names[1]);
			break;
		case gdi_options:
			DialogBox(gg_hInst, (LPCTSTR)grd_options,
				hDlg, (DLGPROC)optionsProc);
			break;
		case gdi_help:
			DialogBox(gg_hInst, (LPCTSTR)grd_htp,
				hDlg, (DLGPROC)htpProc);
			break;
		}
		return TRUE;
		break;
	}
    return FALSE;
}


/////////////////////////////////
// Options Dialog
LRESULT CALLBACK optionsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT com=LOWORD(wParam);
	switch(message)
	{
	case WM_INITDIALOG:
		UINT speed;
		speed=(gg_framedevide==1? gdi_fastpc:(gg_framedevide==gd_fdeva? gdi_medpc:gdi_slowpc));
		CheckRadioButton(hDlg, gdi_fastpc, gdi_slowpc, speed);
		CheckDlgButton(hDlg, gdi_sound, (gg_options[1]? BST_CHECKED:BST_UNCHECKED));
		CheckDlgButton(hDlg, gdi_music, (gg_options[2]? BST_CHECKED:BST_UNCHECKED));
		CheckDlgButton(hDlg, gdi_sm, (gg_options[3]? BST_CHECKED:BST_UNCHECKED));
		SetDlgItemInt(hDlg, gdi_numholes, gg_numholes, FALSE);
		return TRUE;
	case WM_COMMAND:
		switch(com)
		{
		case gdi_ok:
			int num;
			num=GetDlgItemInt(hDlg, gdi_numholes, NULL, FALSE);
			if(num<0 || num>50)
			{
				customBox(hDlg, gri_flag, grs_notturn,
					"The number of sinkholes must be between 0 and 50.",
					"OK", NULL);
				return TRUE;
			}

			// get animation speed
			if(IsDlgButtonChecked(hDlg, gdi_fastpc)==BST_CHECKED)
				gg_framedevide=1;
			else if(IsDlgButtonChecked(hDlg, gdi_medpc)==BST_CHECKED)
				gg_framedevide=gd_fdeva;
			else gg_framedevide=gd_fdevb;

			// get other options
			gg_options[1]=(IsDlgButtonChecked(hDlg, gdi_sound)==BST_CHECKED);
			gg_options[2]=(IsDlgButtonChecked(hDlg, gdi_music)==BST_CHECKED);
			gg_options[3]=(IsDlgButtonChecked(hDlg, gdi_sm)==BST_CHECKED);
			gg_numholes=GetDlgItemInt(hDlg, gdi_numholes, NULL, FALSE);
			EndDialog(hDlg, com);

			// save the settings
			writeSettings();
			break;
		case IDCANCEL:

			// close dialog without making changes
			EndDialog(hDlg, com);
			break;
		}
		return TRUE;
		break;
	}
    return FALSE;
}


/////////////////////////////////
// How To play Dialog
LRESULT CALLBACK htpProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char buf[5120]={NULL};
	switch(message)
	{
	case WM_INITDIALOG:
		HANDLE file;
		DWORD wtn;
		char fileName[256];
		char* LfileName;
		LfileName=fileName;
		
		SetCurrentDirectory(gg_path);
		GetFullPathName("help.txt", 256, fileName, &LfileName);
		file=CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL, NULL);

		if(file==INVALID_HANDLE_VALUE)
		{
			reportError(5);
			DestroyWindow(hDlg);
			return TRUE;
		}

		ReadFile(file, buf, 5120, &wtn, NULL);
		CloseHandle(file);

		SetDlgItemText(hDlg, gdi_help, buf);
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}

    return FALSE;
}


/////////////////////////////////
// Process Messages Sent To Main Window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	int wmId, wmEvent;

	switch (message) 
	{
	case WM_COMMAND:
		wmId   =LOWORD(wParam);
		wmEvent=HIWORD(wParam);

		// Parse the menu selections
		switch (wmId)
		{
		case gm_ng: newGame(); break;						// start a new game
		case gm_exit: wantToQuit(hWnd); break;				// exit program					
		case gm_sound: changeOption(gm_sound, 1); break;	// turn on/off sound
		case gm_fastani:									// change speed of animation
		case gm_medani:
		case gm_slowani:
			changeAniSpeed(wmId);
			break;
		case gm_music:										// turn on/off music
			changeOption(gm_music, 2);
			if(gg_options[2])
			{
				gg_midopt[1]=1;
				gg_midopt[2]=0;
				mNextSong();
			}
			else
			{
				gg_midopt[1]=0;
				mPauseMidi();
			}
			break;
		case gm_showm:				// toggle showmoves	
			pMoves(!gg_options[3]); // shut it off
			changeOption(gm_showm, 3);
			pMoves(gg_options[3]);	// turn it on
			break;
		case gm_htp:
			DialogBox(gg_hInst, (LPCTSTR)grd_htp, hWnd, (DLGPROC)htpProc);
			break;
		case gm_about:
			DialogBox(gg_hInst, (LPCTSTR)grd_about, hWnd, (DLGPROC)aboutProc);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_LBUTTONDOWN: mseDown(LOWORD(lParam), HIWORD(lParam)); break;
	case WM_MOUSEMOVE: mseMove(LOWORD(lParam), HIWORD(lParam)); break;
	case WM_LBUTTONUP: mseUp(LOWORD(lParam), HIWORD(lParam)); break;
	case WM_CLOSE: wantToQuit(hWnd); break;
	case WM_PAINT:
		BeginPaint(hWnd, &ps);

		if(gg_canpaint)
			BitBlt(ps.hdc, 0, 0, 489, 254, ggh_maina, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);
		break;
	case WM_ACTIVATEAPP:
		gg_suspended=!(BOOL)wParam;
		if(gg_suspended) mPauseMidi();
		else if(gg_options[2] && !gg_midopt[1]) mResumeMidi();
		break;
	case MM_MCINOTIFY:
		if(gg_options[2] && gg_midopt[1]) mNextSong();
		break;
	case WM_DESTROY:
		mCloseMidi();
		destBrushes();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	switch(idEvent)
	{
	case 1: aSelect(); break;
	case 2: aFreeGuys(); break;
	}
	return;
}


////////////////////////////////////////////////////////////////
// Game Functions

/////////////////////////////////
// Game Initalization
BOOL makeBitmaps(void)
{
	// create dcs
	ggh_maina	=CreateCompatibleDC(ggh_maind);
	ggh_paint	=CreateCompatibleDC(ggh_maind);
	ggh_masks	=CreateCompatibleDC(ggh_maind);
	ggh_eff		=CreateCompatibleDC(ggh_maind);
	ggh_select	=CreateCompatibleDC(ggh_maind);
	ggh_selani	=CreateCompatibleDC(ggh_maind);
	ggh_fglow	=CreateCompatibleDC(ggh_maind);

	// create bitaps
	ggm_main	=CreateCompatibleBitmap(ggh_maind, 492, 254);
	ggm_paint	=(HBITMAP)LoadImage(gg_hInst, (LPCTSTR)grb_paint, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
	ggm_masks	=(HBITMAP)LoadImage(gg_hInst, (LPCTSTR)grb_masks, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
	ggm_eff		=(HBITMAP)LoadImage(gg_hInst, (LPCTSTR)grb_eff, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_DEFAULTCOLOR);
	ggm_select	=CreateCompatibleBitmap(ggh_maind, 22, 102);
	ggm_selani	=CreateCompatibleBitmap(ggh_maind, 48, 55);
	ggm_fglow	=CreateCompatibleBitmap(ggh_maind, 9, 11);

	// select bitmaps into dcs
	SelectObject(ggh_maina, ggm_main);
	SelectObject(ggh_paint, ggm_paint);
	SelectObject(ggh_masks, ggm_masks);
	SelectObject(ggh_eff, ggm_eff);
	SelectObject(ggh_select, ggm_select);
	SelectObject(ggh_selani, ggm_selani);
	SelectObject(ggh_fglow, ggm_fglow);

	return TRUE;
}

BOOL checkMenus(void)
{
	HMENU mainMenu=GetMenu(gg_hwndMain);

	// misc options
	CheckMenuItem(mainMenu, gm_sound, (gg_options[1]? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_music, (gg_options[2]? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_showm, (gg_options[3]? MF_CHECKED:MF_UNCHECKED));

	// animation speed
	CheckMenuItem(mainMenu, gm_fastani, (gg_framedevide==1? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_medani, (gg_framedevide==gd_fdeva? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_slowani, (gg_framedevide==gd_fdevb? MF_CHECKED:MF_UNCHECKED));
	return TRUE;
}

BOOL initBoard(void)
{
	BOOL sinks[15][10]={0};
	int ra=randNum(15), rb=randNum(10), i, j;
	
	// clear variables
	for(i=0; i<15; i++) for(j=0; j<10; j++) gg_board[i][j]=0;
	for(i=0; i<9 ; i++) gg_selected[0][i]=gg_selected[1][i]=15;
	for(i=0; i<20; i++) gg_freeguys[0][i]=gg_freeguys[1][i]=15;
	for(i=0; i<25; i++) gg_holes[0][i]=gg_holes[1][i]=15;
	
	for(i=0; i<10; i++)
	{
		gg_docChips[0][i]=1;
		gg_docChips[1][i]=2;
	}

	// pick sink holes
	for(i=0; i<gg_numholes; i++)
	{
		while(sinks[ra][rb])
		{
			ra=randNum(15);
			rb=randNum(10);
		}
		sinks[ra][rb]=TRUE;
		gg_holes[0][i]=ra;
		gg_holes[1][i]=rb;
	}

	// clear some more
	gg_chipsremain[0]=gg_chipsremain[1]=10;
	gg_score[0]=gg_score[1]=0;
	gg_numfreeguys=0;
	gg_notturn=0;
	gg_seltype=0;
	gg_mousedown=FALSE; // just in case

	// roll dice to see who goes first
	do
	{
		gg_preroll[0]=randNum(6);
		gg_preroll[1]=randNum(6);
	}
	while(gg_preroll[0]==gg_preroll[1]);
	
	// set player to highest die roll
	gg_player=(gg_preroll[0]>gg_preroll[1]? 0:1);
	return TRUE;
}


/////////////////////////////////
// User Interface
BOOL mseDown(long x, long y)
{
	if(x>12 && y>25 && x<33 && y<193)			// tan line
		cSelect(1, 0, (int)((y-26)/17));
	else if(x>455 && y>25 && x<476 && y<193)	// teal line
		cSelect(2, 0, (int)((y-26)/17));
	else if(x>44 && y>25 && x<444 && y<193)		// chips on board
		cSelect(3, (int)((x-45)/27), (int)((y-26)/17));

	gg_mousedown=TRUE;
	return TRUE;
}

BOOL mseMove(long x, long y)
{
	if(!gg_mousedown || !gg_pushspace[0] || !gg_options[3]) return FALSE;
	if(sWithinClickedSpace(x, y))
	{
		if(!gg_pushspace[1])
		{
			gg_pushspace[1]=1;
			aPushSpace();
		}
	}
	else
	{
		gg_pushspace[1]=0;
		long a=(gg_pushspace[4]? 24:0);
		BitBlt(ggh_maind, gg_pushpoint.x, gg_pushpoint.y, 24, 11,
			ggh_selani, a, 44, SRCCOPY);
	}

	return TRUE;
}

BOOL mseUp(long x, long y)
{
	if(gg_pushspace[0])
	{
		if(gg_options[3])
		{
			long a=(gg_pushspace[4]? 24:0);
			BitBlt(ggh_maind, gg_pushpoint.x, gg_pushpoint.y, 24, 11,
				ggh_selani, a, 44, SRCCOPY);
		}

		if(sWithinClickedSpace(x, y))
		{
			vMoveChip(gg_seltype, gg_selected[0][0], gg_selected[1][0],
				gg_pushspace[2], gg_pushspace[3]);
		}
		gg_pushspace[0]=0;
		gg_pushspace[1]=0;
	}

	ReleaseCapture();
	gg_mousedown=FALSE;
	return TRUE;
}


/////////////////////////////////
// Painting
BOOL pBoard(void)
{
	RECT ra;
	int i, j;
	
	// background
	pRect(&ggh_maina, &ggp_bkborder, &ggb_background, 0, 0, 489, 220);
	ra.left=0; ra.top=220; ra.right=489; ra.bottom=253;
	FillRect(ggh_maina, &ra, ggb_greyd);

	// main board
	pRect(&ggh_maina, &ggp_black, &ggb_greya, 9, 9, 480, 212);

	// medium stripe
	ra.left=10; ra.top=195; ra.right=479; ra.bottom=196;
	FillRect(ggh_maina, &ra, ggb_greyb);

	// front panel
	ra.left=10; ra.top=196; ra.right=479; ra.bottom=211;
	FillRect(ggh_maina, &ra, ggb_greyc);

	// draw rectangles
	pRect(&ggh_maina, &ggp_darkgrey, &ggb_lightgrey, 12, 25, 477, 29);
	pRect(&ggh_maina, &ggp_darktan, &ggb_lighttan, 37, 31, 41, 193);
	pRect(&ggh_maina, &ggp_darkteal, &ggb_lightteal, 448, 31, 452, 193);

	// draw starting chips
	pStart(0); 
	pStart(1);

	// draw spaces
	for(i=0; i<15; i++) for(j=0; j<10; j++)
		BitBlt(ggh_maina, (27*i+45), (17*j+31), 20, 9, ggh_paint, 61, 131, SRCCOPY);

	// paint names
	aName(0, FALSE);
	aName(1, FALSE);

	// draw graphs
	aGraph(0, FALSE, 0);
	aGraph(1, FALSE, 0);

	return TRUE;
}

BOOL pRect(HDC* hdc, HPEN* pen, HBRUSH* brush, long a, long b, long c, long d)
{
	SelectObject(*hdc, *pen);
	SelectObject(*hdc, *brush);
	Rectangle(*hdc, a, b, c, d);
	return TRUE;
}

BOOL pStart(int p)
{
	int i;
	long a=(p*14);
	long x=(p*443+13), y;

	for(i=0; i<10; i++)
	{
		y=(17*i+26);
		BitBlt(ggh_maina, x, y, 20, 14, ggh_paint, 57, a, SRCINVERT);
		BitBlt(ggh_maina, x, y, 20, 14, ggh_masks, 16, 0, SRCAND);
		BitBlt(ggh_maina, x, y, 20, 14, ggh_paint, 57, a, SRCINVERT);
	}
	return TRUE;
}

BOOL pSelect(int p, long x, long y)
{
	gg_aselnum=0;
	pCreateSel(p, x, y);
	BitBlt(ggh_maina, x, y, 22, 17, ggh_select, 0, 0, SRCCOPY);
	BitBlt(ggh_maind, x, y, 22, 17, ggh_select, 0, 0, SRCCOPY);
	return TRUE;
}

BOOL pUnselect()
{
	if(!gg_seltype) return FALSE;
	BitBlt(ggh_maina, gg_selchord[0], gg_selchord[1], 22, 17, ggh_select, 0, 85, SRCCOPY);
	BitBlt(ggh_maind, gg_selchord[0], gg_selchord[1], 22, 17, ggh_select, 0, 85, SRCCOPY);
	return TRUE;
}

BOOL pMoves(BOOL select)
{
	if(!gg_options[3]) return FALSE;
	long x, dx, dy;
	BOOL isP[12];

	pCreateSelSpace(&ggh_eff, select, 53, 91);

	for(int i=1; i<9; i++)
		isP[i-1]=iIsFreeGuy(gg_selected[0][i], gg_selected[1][i]);

	if(select) aMoves(&ggh_selani, isP);

	for(i=1; i<9; i++)
	{
		if(gg_selected[0][i]==15 || gg_selected[1][i]==15) continue;
		x=(isP[i-1]? 24:0);
		dx=(chdBoardX(gg_selected[0][i])-2);
		dy=(chdBoardY(gg_selected[1][i])-1);
		BitBlt(ggh_maind, dx, dy, 24, 11, ggh_selani, x, 44, SRCCOPY);
		BitBlt(ggh_maina, dx, dy, 24, 11, ggh_selani, x, 44, SRCCOPY);
	}

	return TRUE;
}

BOOL pCreateSelSpace(HDC* hdc, BOOL select, long x, long y)
{
	long ax=(select? 41:59), ay=(select? 31:130);
	long by, cy=(chdFreePY()+3);

	BitBlt(ggh_selani, 0, 0, 24, 44, *hdc, x, y, SRCCOPY);
	BitBlt(ggh_selani, 24, 0, 24, 44, *hdc, x, y, SRCCOPY);
	BitBlt(ggh_selani, 0, 44, 24, 11, ggh_paint, ax, ay, SRCCOPY);
	BitBlt(ggh_selani, 24, 44, 24, 11, ggh_paint, ax, ay, SRCCOPY);

	// paint the 'f'
	for(int i=0; i<5; i++)
	{
		by=(11*i);
		BitBlt(ggh_selani, 32, by, 11, 8, ggh_eff, 53, cy, SRCINVERT);
		BitBlt(ggh_selani, 32, by, 11, 8, ggh_eff, 53, 80, SRCAND);
		BitBlt(ggh_selani, 32, by, 11, 8, ggh_eff, 53, cy, SRCINVERT);
	}

	return TRUE;
}

BOOL pCreateSel(int p, long x, long y)
{
	// draw chips
	for(int i=0; i<6; i++)
		BitBlt(ggh_select, 0, (i*17), 22, 17, ggh_maind, x, y, SRCCOPY);

	// draw selection rings
	BitBlt(ggh_select, 0, 0, 22, 101, ggh_paint, 65, 28, SRCINVERT);
	BitBlt(ggh_select, 0, 0, 22, 101, ggh_masks, 0, 16, SRCAND);
	BitBlt(ggh_select, 0, 0, 22, 101, ggh_paint, 65, 28, SRCINVERT);

	return TRUE;
}

BOOL pIcon(HDC* hdc, int p)
{
	BOOL pc=((gg_options[4] && p==1));
	long sy=(pc? 91:75), my=(pc? 123:107);

	BitBlt(*hdc, 3, 2, 16, 16, ggh_paint, 42, sy, SRCINVERT);
	BitBlt(*hdc, 3, 2, 16, 16, ggh_paint, 42, my, SRCAND);
	BitBlt(*hdc, 3, 2, 16, 16, ggh_paint, 42, sy, SRCINVERT);
	return TRUE;
}

BOOL pSpace(int t, int a, int b)
{
	long ax, ay, by;
	ax=(t==3? chdBoardX(a):chdDocX(t));
	ay=(chdBoardY(b)-5);
	by=(b==0? 112:126);
	BitBlt(ggh_maina, ax, ay, 20, 14, ggh_paint, 61, by, SRCCOPY);
	BitBlt(ggh_maind, ax, ay, 20, 14, ggh_paint, 61, by, SRCCOPY);

	return TRUE;
}

BOOL pChip(int t, int a, int b)
{
	long ax, ay, by, cy;
	ax=(t==3? chdBoardX(a):chdDocX(t));
	ay=(chdBoardY(b)-5);
	by=(b==0? 112:126);
	cy=(14*gg_player);

	HDC hdca=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 20, 14);
	SelectObject(hdca, hbita);

	BitBlt(hdca, 0, 0, 20, 14, ggh_paint, 61, by, SRCCOPY);
	BitBlt(hdca, 0, 0, 20, 14, ggh_paint, 57, cy, SRCINVERT);
	BitBlt(hdca, 0, 0, 20, 14, ggh_masks, 16, 0, SRCAND);
	BitBlt(hdca, 0, 0, 20, 14, ggh_paint, 57, cy, SRCINVERT);
	BitBlt(ggh_maina, ax, ay, 20, 14, hdca, 0, 0, SRCCOPY);
	BitBlt(ggh_maind, ax, ay, 20, 14, hdca, 0, 0, SRCCOPY);

	DeleteDC(hdca);
	DeleteObject(hbita);
	return TRUE;
}


BOOL pAddFreeGuy(int a, int b)
{
	long x=(chdBoardX(a)+6);
	long y=(chdBoardY(b)-4);
	long px=chdFreePY();
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 9, 11);
	SelectObject(hdca, hbita);

	BitBlt(hdca, 0, 0, 9, 11, ggh_maina, x, y, SRCCOPY);
	BitBlt(hdca, 0, 0, 9, 11, ggh_eff, 53, px, SRCINVERT);
	BitBlt(hdca, 0, 0, 9, 11, ggh_eff, 53, 77, SRCAND);
	BitBlt(hdca, 0, 0, 9, 11, ggh_eff, 53, px, SRCINVERT);
	BitBlt(ggh_maina, x, y, 9, 11, hdca, 0, 0, SRCCOPY);
	BitBlt(ggh_maind, x, y, 9, 11, hdca, 0, 0, SRCCOPY);

	DeleteDC(hdca);
	DeleteObject(hbita);
	return TRUE;
}

BOOL pScore(HDC* hdc, BOOL grey, long x, long y, int p)
{
	RECT ra; ra.left=(x+164); ra.top=(y+1); ra.right=(x+195); ra.bottom=(y+18);
	int pvBkMode=SetBkMode(*hdc, TRANSPARENT);
	HFONT pvFont=(HFONT)SelectObject(*hdc, ggf_maina);
	COLORREF pvTextColor=SetTextColor(*hdc, ((p!=gg_player || grey)?
		gc_darkgrey:(p==0? gc_darktan:gc_darkteal)));

	char buf[10];
	sprintf(buf, "%d", gg_score[p]);
	FillRect(*hdc, &ra, ((p!=gg_player || grey)?
		ggb_lightgrey:(p==0? ggb_lighttan:ggb_lightteal)));
	DrawText(*hdc, buf, strlen(buf), &ra, gd_rightcenter);

	SetBkMode(*hdc, pvBkMode);
	SelectObject(*hdc, pvFont);
	SetTextColor(*hdc, pvTextColor);
	return TRUE;
}


/////////////////////////////////
// Animation
BOOL aSelect(void)
{
	if(!gg_seltype) return FALSE;
	BitBlt(ggh_maind, gg_selchord[0], gg_selchord[1],
		22, 17, ggh_select, 0, (gg_aselnum*17), SRCCOPY);
	gg_aselnum=(gg_aselnum==4? 0:(gg_aselnum+1));
	return TRUE;
}

BOOL aMoves(HDC* hdca, BOOL isP[])
{
	int i, j;
	gs_selring rgs[12];

	// get location data
	for(i=1; i<9; i++)
		rgs[i-1].setup(gg_selected[0][i], gg_selected[1][i], isP[i-1]);

	// execute animation
	for(i=0; i<4; i++)
	{
		for(j=0; j<8; j++)
		{
			if(gg_selected[0][j+1]==15 || gg_selected[1][j+1]==15) continue;
			BitBlt(ggh_maind, rgs[j].x, rgs[j].y, 24, 11, *hdca,
				rgs[j].sx, (11*i), SRCCOPY);
			waitTicks(15);
		}
	}

	return TRUE;
}

BOOL aPushSpace(void)
{
	if(!gg_options[3]) return FALSE;
	int x=(gg_pushspace[4]? 24:0);
	for(int i=0; i<4; i++)
	{
		BitBlt(ggh_maind, gg_pushpoint.x, gg_pushpoint.y, 24, 11,
		ggh_selani, x, (11*i), SRCCOPY);
		waitTicks(25);
	}
	return TRUE;
}

BOOL aCurve(LPgs_curve cd)
{
	int i; long x, y;
	double wi, he, wiSc, heSc;

	// create bitmaps
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HDC hdcb=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, cd->pw, cd->ph);
	HBITMAP hbitb=CreateCompatibleBitmap(ggh_maind, cd->pw, cd->ph);
	SelectObject(hdca, hbita);
	SelectObject(hdcb, hbitb);

	wi=((cd->dx)-(cd->sx)); // destX-sourceX
	he=((cd->sy)-(cd->dy)); // sourceY-destY
	wiSc=(wi/15);
	heSc=(he/15);

	for(i=1; i<15; i++)
	{
		x=(long)(wiSc*i+(cd->sx));
		y=(long)(-15*i+i*i-heSc*i+cd->sy);//(cd->sy-(-i*i+15*i+heSc*i));

		BitBlt(hdca, 0, 0, cd->pw, cd->ph, ggh_maind, x, y, SRCCOPY);
		BitBlt(hdcb, 0, 0, cd->pw, cd->ph, ggh_maind, x, y, SRCCOPY);
		BitBlt(hdcb, 0, 0, cd->pw, cd->ph, ggh_paint, cd->px, cd->py, SRCINVERT);
		BitBlt(hdcb, 0, 0, cd->pw, cd->ph, ggh_masks, cd->mx, cd->my, SRCAND);
		BitBlt(hdcb, 0, 0, cd->pw, cd->ph, ggh_paint, cd->px, cd->py, SRCINVERT);
		BitBlt(ggh_maind, x, y, cd->pw, cd->ph, hdcb, 0, 0, SRCCOPY);

		waitTicks(cd->wait);

		BitBlt(ggh_maind, x, y, cd->pw, cd->ph, hdca, 0, 0, SRCCOPY);
	}

	DeleteDC(hdca);
	DeleteDC(hdcb);
	DeleteObject(hbita);
	DeleteObject(hbitb);
	return TRUE;
}

BOOL aExplode(int a, int b)
{
	long x=(chdBoardX(a)-4);
	long y=(chdBoardY(b)-7);

	// create offscreen bitmap with animation frames
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 27, 171);
	SelectObject(hdca, hbita);

	for(int i=0; i<9; i++)
		BitBlt(hdca, 0, (19*i), 27, 19, ggh_maina, x, y, SRCCOPY);
	BitBlt(hdca, 0, 0, 26, 171, ggh_eff, 1, 0, SRCINVERT);
	BitBlt(hdca, 0, 0, 26, 171, ggh_eff, 27, 0, SRCAND);
	BitBlt(hdca, 0, 0, 26, 171, ggh_eff, 1, 0, SRCINVERT);
	
	sndPlay(grs_explode);

	// draw the animation
	for(i=(gg_board[a][b]==0? 0:1); i<9; i++)
	{
		BitBlt(ggh_maind, x, y, 27, 19, hdca, 0, (19*i), SRCCOPY);
		waitTicks(75);
	}

	// clean up
	DeleteDC(hdca);
	DeleteObject(hbita);
	return TRUE;
}

BOOL aSinkChip(int a, int b, BOOL down)
{
	int t=(gg_player+1);
	int py=(15*gg_player);
	int ctr=(down? 1:-1), i;
	long sy=(b==0? 112:126);
	long x=(down? chdBoardX(a):chdDocX(t));
	long y=(chdBoardY(b)-5);

	// make offscreen bitmap
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 20, 91);
	SelectObject(hdca, hbita);

	for(i=0; i<7; i++) // paint background space
		BitBlt(hdca, 0, (13*i), 20, 13, ggh_paint, 61, sy, SRCCOPY);

	for(i=0; i<7; i++)
		BitBlt(hdca, 2, (15*i), 16, 13, ggh_paint, 41, py, SRCINVERT);
	BitBlt(hdca, 2, 0, 16, 91, ggh_eff, 62, 0, SRCAND);
	for(i=0; i<7; i++)
		BitBlt(hdca, 2, (15*i), 16, 13, ggh_paint, 41, py, SRCINVERT);

	// conduct animation
	for(i=(down? 0:6); (i>-1 && i<7); i+=ctr)
	{
		waitTicks(50); // wait first so sound is imediate
		BitBlt(ggh_maind, x, y, 20, 13, hdca, 0, (13*i), SRCCOPY);
	}

	if(down)
	{
		pSpace(3, a, b); 
		sndPlay(grs_sinkchip);
	}
	else pChip(t, 0, b);
	
	DeleteDC(hdca);
	DeleteObject(hbita);
	return TRUE;
}

BOOL aName(int p, BOOL scroll)
{
	RECT ra; ra.left=21; ra.top=0; ra.right=185; ra.bottom=20;
	int sl=strlen(gg_names[p]);

	// get destination x chord
	long x=(277*p+6), xx;

	// setup
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HDC hdcb=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 201, 20);
	HBITMAP hbitb=CreateCompatibleBitmap(ggh_maind, 201, 20);
	SelectObject(hdca, hbita);
	SelectObject(hdcb, hbitb);
	SelectObject(hdca, ggf_maina);
	SelectObject(hdcb, ggf_maina);
	SetBkMode(hdca, TRANSPARENT);
	SetBkMode(hdcb, TRANSPARENT);
	SetTextColor(hdca, gc_darkgrey);
	SetTextColor(hdcb, (p==0? gc_darktan:gc_darkteal));

	// draw background
	pRect(&hdca, &ggp_darkgrey, &ggb_lightgrey, 0, 0, 200, 20);
	pIcon(&hdca, p);
	DrawText(hdca, gg_names[p], strlen(gg_names[p]), &ra, gd_leftcenter);
	pScore(&hdca, TRUE, 0, 0, p);

	if(p==gg_player)
	{
		for(int i=(scroll? 0:20); i<21; i++)
		{
			xx=(i*10);
			pRect(&hdcb, (p==0? &ggp_darktan:&ggp_darkteal),
				(p==0? &ggb_lighttan:&ggb_lightteal), 0, 0, xx, 20);
			pIcon(&hdcb, p);
			DrawText(hdcb, gg_names[p], sl, &ra, gd_leftcenter);
			pScore(&hdcb, FALSE, 0, 0, p);

			BitBlt(hdca, 0, 0, xx, 20, hdcb, 0, 0, SRCCOPY);
			if(scroll) BitBlt(ggh_maind, x, 227, 200, 20, hdca, 0, 0, SRCCOPY);
			waitTicks(15);
		}

		// draw to offscreen version of the window
		BitBlt(ggh_maina, x, 227, 200, 20, hdca, 0, 0, SRCCOPY);
	}
	else 
	{
		BitBlt(ggh_maina, x, 227, 200, 20, hdca, 0, 0, SRCCOPY);
		if(scroll) BitBlt(ggh_maind, x, 227, 200, 20, hdca, 0, 0, SRCCOPY);
	}

	DeleteDC(hdca);
	DeleteDC(hdcb);
	DeleteObject(hbita);
	DeleteObject(hbitb);
	return TRUE;
}

BOOL aGraph(int p, BOOL scroll, int dir)
{
	char num[2];
	sprintf(num, "%d", (gg_chipsremain[p]+dir));
	int numLen=strlen(num);
	long x=(264*p+12), gx=(10*gg_chipsremain[p]), ix;
	RECT ra; ra.left=0; ra.top=-1; ra.right=201; ra.bottom=11;

	// create offscreen bitmaps
	HDC hdca=CreateCompatibleDC(ggh_maind);
	HDC hdcb=CreateCompatibleDC(ggh_maind);
	HBITMAP hbita=CreateCompatibleBitmap(ggh_maind, 201, 11);
	HBITMAP hbitb=CreateCompatibleBitmap(ggh_maind, 201, 11);
	SelectObject(hdca, hbita);
	SelectObject(hdcb, hbitb);

	SetTextColor(hdca, gc_darkgrey);
	SetTextColor(hdcb, (p==0? gc_darktan:gc_darkteal));
	SetBkMode(hdca, TRANSPARENT);
	SetBkMode(hdcb, TRANSPARENT);
	SelectObject(hdca, ggf_mainb);
	SelectObject(hdcb, ggf_mainb);

	HPEN* penColor=(p==0? &ggp_darktan:&ggp_darkteal);
	HBRUSH* bshColor=(p==0? &ggb_lighttan:&ggb_lightteal);

	// foreground
	pRect(&hdca, &ggp_darkgrey, &ggb_lightgrey, 0, 0, 201, 11);
	DrawText(hdca, num, numLen, &ra, gd_centercenter);
	if(scroll)
	{
		for(int i=1; i<11; i++)
		{
			ix=(i*dir+gx);
			pRect(&hdca, &ggp_darkgrey, &ggb_lightgrey, 0, 0, 201, 11);
			DrawText(hdca, num, numLen, &ra, gd_centercenter);
			pRect(&hdcb, penColor, bshColor, 0, 0, ix, 11);
			DrawText(hdcb, num, numLen, &ra, gd_centercenter);

			BitBlt(hdca, 0, 0, ix, 11, hdcb, 0, 0, SRCCOPY);
			BitBlt(ggh_maind, x, 12, 201, 11, hdca, 0, 0, SRCCOPY);
			waitTicks(20);
		}
		BitBlt(ggh_maina, x, 12, 201, 11, hdca, 0, 0, SRCCOPY);
	}
	else
	{
		if(gg_chipsremain[p])
		{
			pRect(&hdcb, penColor, bshColor, 0, 0, (gx+1), 11);
			DrawText(hdcb, num, numLen, &ra, gd_centercenter);
			BitBlt(hdca, 0, 0, (gx+1), 11, hdcb, 0, 0, SRCCOPY);
		}
		BitBlt(ggh_maina, x, 12, 201, 11, hdca, 0, 0, SRCCOPY);
	}

	DeleteDC(hdca);
	DeleteDC(hdcb);
	DeleteObject(hbita);
	DeleteObject(hbitb);
	return TRUE;
}

BOOL aDie(BOOL up, int p, int die, int num)
{
	RECT ra; 
	long a=(20*p), b=(die==50? 120:(20*die-20));
	long c=(up? 226:256);
	int i, d=(up? 2:-2);

	ra.left=(24*num+222); ra.right=(ra.left+21);
	ra.top=c; ra.bottom=(ra.top+21);

	for(i=1; i<16; i++)
	{
		FillRect(ggh_maind, &ra, ggb_greyd);
		ra.top=(d*i+c); ra.bottom=(ra.top+21);
		BitBlt(ggh_maind, ra.left, ra.top, 21, 21, ggh_paint, a, b, SRCCOPY);
		waitTicks(5);
	}

	BitBlt(ggh_maina, ra.left, (d*15+c), 21, 21, ggh_paint, a, b, SRCCOPY);
	return TRUE;
}

BOOL aScrollScreen(void)
{
	int a, b;
	if(randNum(2)==0) {a=(-2*randNum(2)+1); b=0;}
	else {a=0; b=(-2*randNum(2)+1);}
	long x=(488*a);
	long y=(253*b);
	double sx=(24.4*-a);
	double sy=(12.65*-b);

	for(int i=0; i<21; i++)
	{
		BitBlt(ggh_maind, (int)(sx*i+x), (int)(sy*i+y), 489, 253, ggh_maina, 0, 0, SRCCOPY);
		waitTicks(15);
	}

	return TRUE;
}

BOOL aFreeGuys(void)
{
	if(gg_numfreeguys==0) return FALSE;
	
	gg_pselnum=(gg_pselnum==12? 0:(gg_pselnum+1));
	long x, y, py=(chdFreePY());

	for(int i=0; i<gg_numfreeguys; i++)
	{
		x=(chdBoardX(gg_freeguys[0][i])+6);
		y=(chdBoardY(gg_freeguys[1][i])-4);
		BitBlt(ggh_fglow, 0, 0, 9, 11, ggh_maina, x, y, SRCCOPY);
		BitBlt(ggh_fglow, 0, 0, 9, 11, ggh_eff, 53, py, SRCINVERT);
		BitBlt(ggh_fglow, 0, 0, 9, 11, ggh_eff, 53, 77, SRCAND);
		BitBlt(ggh_fglow, 0, 0, 9, 11, ggh_eff, 53, py, SRCINVERT);
		BitBlt(ggh_maind, x, y, 9, 11, ggh_fglow, 0, 0, SRCCOPY);
	}

	return TRUE;
}


/////////////////////////////////
// Misc Game Functions
BOOL rollDice(void)
{
	gg_roll[0]=(randNum(6)+1);
	gg_roll[1]=(randNum(6)+1);
	aDie(FALSE, gg_player, gg_roll[0], 0);
	aDie(FALSE, gg_player, gg_roll[1], 1);
	return TRUE;
}

BOOL changeScore(int p, long cge)
{
	long x=(277*p+6);
	gg_score[p]+=cge;
	pScore(&ggh_maina, FALSE, x, 227, p);
	pScore(&ggh_maind, FALSE, x, 227, p);
	return TRUE;
}

BOOL nextPlayer(void)
{
	gg_notturn=0;

	aDie(TRUE, gg_player, gg_roll[0], 0);
	aDie(TRUE, gg_player, gg_roll[1], 1);
	gg_player=(1-gg_player);
	
	// re-draw names
	aName((1-gg_player), TRUE);
	aName(gg_player, TRUE);

	rollDice();
	if(iCanMove() && gg_options[4] && gg_player==1) startPcPlayer();
	return TRUE;
}

BOOL gameOver(void)
{
	if(gg_chipsremain[0]!=0 && gg_chipsremain[1]!=0) return FALSE;
	int p=(gg_chipsremain[0]==0? 0:1);
	char buf[256];
	sprintf(buf, "%s has won the game! Do you want to play again?",
		gg_names[1-p]);
	if(customBox(gg_hwndMain, gri_question, grs_question,
		buf, "Quit", "New Game")==gd_btncancel) newGame();
	else
	{
		writeSettings();
		DestroyWindow(gg_hwndMain);
	}
	return TRUE;
}


/////////////////////////////////
// Selection Functions
BOOL cSelect(int t, int a, int b)
{
	char buf[256]={NULL};
	int p=(gg_player+1);
	int chip=(t==3? gg_board[a][b]:gg_docChips[t-1][b]);

	if(t==3 && chip==0) sClickSpace(a, b);
	else if(t && chip==p)
	{
		// make sure it is not already selected
		if((t==3 && gg_seltype==3 && a==gg_selected[0][0]
			&& b==gg_selected[1][0]) || ((t==1 || t==2)
			&& gg_seltype==t && b==gg_selected[1][0]))
		{
			cUnselect();
			return FALSE;
		}

		gg_notturn=0;
		long x, y;
 
		x=((t==3? chdBoardX(a):chdDocX(t))-1);
		y=(chdBoardY(b)-7);
	
		// unselect previous chip
		if(gg_seltype)
		{
			pUnselect();
			pMoves(FALSE);
		}

		// update variables
		sndPlay(grs_select);
		gg_seltype=t;
		gg_selchord[0]=x;
		gg_selchord[1]=y;
		gg_selected[0][0]=a;
		gg_selected[1][0]=b;
		pSelect(0, x, y);
		iMoves(t, a, b);
		pMoves(TRUE);
	}
	else if(chip)
	{
		gg_notturn++;
		sndPlay(grs_notturn);
		if(gg_notturn>2)
		{
			if(gg_options[4] && gg_player==0)
			{
				sprintf(buf, "%s, you cannot move the computer's "
					"marbles.  Click on a gold marble.", gg_names[0]);
			}
			else
			{
				sprintf(buf, "It is not %s's turn, click on a %s marble.",
					gg_names[1-gg_player], (gg_player==0? "gold":"teal"));
			}
			customBox(gg_hwndMain, gri_flag, grs_notturn, buf, "OK", NULL);
		}
	}

	return TRUE;
}

BOOL cUnselect(void)
{
	pUnselect();
	pMoves(FALSE);
	gg_seltype=0;
	for(int i=0; i<9; i++) gg_selected[0][i]=gg_selected[1][i]=15;
	return TRUE;
}


/////////////////////////////////
// Select Spaces
BOOL sClickSpace(int a, int b)
{
	if(sIsMove(a, b))
	{
		SetCapture(gg_hwndMain);
		gg_pushspace[0]=1;
		gg_pushspace[1]=1;
		gg_pushspace[2]=a;
		gg_pushspace[3]=b;
		gg_pushspace[4]=iIsFreeGuy(a, b);

		gg_pushpoint.x=(chdBoardX(a)-2);
		gg_pushpoint.y=(chdBoardY(b)-1);

		pCreateSelSpace(&ggh_paint, TRUE, 41, 31);
		aPushSpace();
	}
	else if(!gg_options[3]) sndPlay(grs_notturn);

	return TRUE;
}

BOOL sIsMove(int a, int b)
{
	BOOL res=FALSE;
	for(int i=1; i<9; i++) res=(res || (gg_selected[0][i]==a
		&& gg_selected[1][i]==b));
	return res;
}

BOOL sWithinClickedSpace(long x, long y)
{
	return (x>=gg_pushpoint.x && y>=gg_pushpoint.y
		&& x<=(gg_pushpoint.x+23) && y<=(gg_pushpoint.y+10));
}


/////////////////////////////////
// Move Chips
BOOL vMoveChip(int t, int sa, int sb, int da, int db)
{
	cUnselect();
	vShootChip(t, sa, sb, da, db);
	
	gs_curve cd;
	cd.sx=((t==3? chdBoardX(sa):chdDocX(t))+3);
	cd.sy=(chdBoardY(sb)-5);
	cd.dx=(chdBoardX(da)+3);
	cd.dy=(chdBoardY(db)-5);
	cd.px=41;
	cd.py=(15*gg_player);
	cd.pw=16;
	cd.ph=16;
	cd.mx=0;
	cd.my=0;
	cd.wait=30;

	// update board variable
	if(t==3) gg_board[sa][sb]=0;
	else gg_docChips[t-1][sb]=0;
	gg_board[da][db]=(gg_player+1);

	pSpace(t, sa, sb);
	aCurve(&cd);
	pChip(3, da, db);
	vSinkChip(da, db);
	if(iIsFreeGuy(da, db)) vTakeFreeGuy(da, db);
	if(gameOver()) return FALSE;
	if(vUpdateDice(t, sa, sb, da, db))
	{
		waitTicks(750);
		nextPlayer();
		return TRUE;
	}
	return iCanMove();
}

BOOL vShootChip(int t, int sa, int sb, int da, int db)
{
	if(t!=3) sa=(t==1? -1:15);

	gs_curve cd;
	int notP, a, b, wa, wb, ha, hb, i, max;
	notP=(-gg_player+2);
	wa=(da-sa); ha=(db-sb); // height and width of jump
	wb=(wa==0? 0:(wa>0? 1:-1));
	hb=(ha==0? 0:(ha>0? 1:-1));
	max=abs(wa==0? ha:wa);

	for(i=0; i<max; i++)
	{
		a=(sa+wb*i);
		b=(sb+hb*i);

		if(a<0 || b<0 || a>14 || b>10) continue;
		if(gg_board[a][b]==notP)
		{

			cd.sx=((t==3? chdBoardX(sa):chdDocX(t))+7);
			cd.sy=(chdBoardY(sb)-7);
			cd.dx=(chdBoardX(a)+7);
			cd.dy=(chdBoardY(b)-7);
			cd.px=59;
			cd.py=91;
			cd.pw=5;
			cd.ph=5;
			cd.mx=22;
			cd.my=14;
			cd.wait=40;
			
			aCurve(&cd);
			pSpace(3, a, b);
			aExplode(a, b);

			gg_board[a][b]=0;
			vAddFreeGuy(a, b);

			aGraph((1-gg_player), TRUE, -1);
			gg_chipsremain[1-gg_player]--;
			changeScore(gg_player, gdss_kill);
		}
	}

	return TRUE;
}

BOOL vSinkChip(int a, int b)
{
	for(int i=0; i<gg_numholes; i++)
	{
		if(a==gg_holes[0][i] && b==gg_holes[1][i])
		{
			gg_board[a][b]=0;
			gg_holes[0][i]=gg_holes[1][i]=15;
			aSinkChip(a, b, TRUE);
			aGraph(gg_player, TRUE, -1);
			gg_chipsremain[gg_player]--;
			break;
		}
	}
	return TRUE;
}

BOOL vUpdateDice(int t, int sa, int sb, int da, int db)
{
	if(t!=3) sa=(t==1? -1:15);
	int a=abs((da-sa)+(db-sb)); // v+h (one will be zero)

	long x=(20*gg_player);
	BOOL disA, disB;
	disA=(a==gg_roll[0]);
	
	// so doubles don't clear both dice
	disB=(a==gg_roll[1] && a!=gg_roll[0]); 

	if(disA)
	{
		gg_roll[0]=50;
		BitBlt(ggh_maina, 222, 226, 21, 21, ggh_paint, x, 120, SRCCOPY);
		BitBlt(ggh_maind, 222, 226, 21, 21, ggh_paint, x, 120, SRCCOPY);
	}
	if(disB)
	{
		gg_roll[1]=50;
		BitBlt(ggh_maina, 246, 226, 21, 21, ggh_paint, x, 120, SRCCOPY);
		BitBlt(ggh_maind, 246, 226, 21, 21, ggh_paint, x, 120, SRCCOPY);
	}

	return (gg_roll[0]==50 && gg_roll[1]==50);
}

BOOL vAddFreeGuy(int a, int b)
{
	gg_freeguys[0][gg_numfreeguys]=a;
	gg_freeguys[1][gg_numfreeguys]=b;
	gg_numfreeguys++;
	pAddFreeGuy(a, b);
	return TRUE;
}

BOOL vTakeFreeGuy(int a, int b)
{
	char bfr[256];
	BOOL res=FALSE;

	// remove free guy from the list
	for(int i=0; i<20; i++)
		if(a==gg_freeguys[0][i] && b==gg_freeguys[1][i]) break;

	if(i>19) return reportError(7);

	for(; i<=gg_numfreeguys; i++)
	{
		gg_freeguys[0][i]=gg_freeguys[0][i+1];
		gg_freeguys[1][i]=gg_freeguys[1][i+1];
	}

	for(i=0; i<10; i++) if(gg_docChips[gg_player][i]==0) break;

	gg_numfreeguys--;
	sndPlay(grs_freeguy);
	changeScore(gg_player, gdss_new);

	if(i<10)
	{
		gg_docChips[gg_player][i]=(gg_player+1);
		aSinkChip(0, i, FALSE);
		aGraph(gg_player, TRUE, 1);
		gg_chipsremain[gg_player]++;
	}
	else if(!gg_options[4] || gg_player==0) // don't aleart if pc player
	{
		sprintf(bfr, "Sorry %s, there was no place for the free marble to form.",
			gg_names[gg_player]);
		customBox(gg_hwndMain, gri_darn, grs_darn, bfr, "OK", NULL);
	}
	return TRUE;
}


/////////////////////////////////
// Information Functions
BOOL iCanMove(void)
{
	BOOL res=iSearchMoves();
	if(!res)
	{
		char buf[256];
		if(gg_options[4] && gg_player==1) sprintf(buf,
			"The computer has no moves.");
		else sprintf(buf, "Sorry %s, no moves could be detected.",
			gg_names[gg_player]);
		customBox(gg_hwndMain, gri_darn, grs_darn, buf, "OK", NULL);
		nextPlayer();
	}
	return res;
}

BOOL iSearchMoves(void)
{
	int i, j, k; BOOL res=FALSE;
	int p=(gg_player+1);
	int dcDir=(-2*gg_player+1);
	int dcStart=(16*gg_player-1);
	const int dir[4][2]={{0,-1},{1,0},{0,1},{-1,0}};

	// scan behind the lines
	for(i=0; i<10; i++)
	{
		if(gg_docChips[gg_player][i]==0) continue;
		res=(res || gg_board[dcStart+dcDir*gg_roll[0]][i]==0
			|| gg_board[dcStart+dcDir*gg_roll[1]][i]==0);
	}

	if(res) return TRUE; // if we already found a move quit

	// scan the board
	for(i=0; i<15; i++)
	{
		for(j=0; j<10; j++)
		{
			if(gg_board[i][j]!=p) continue; // must be a chip
			for(k=0; k<4; k++)
			{
				res=(res || iBoardItem((i+dir[k][0]*gg_roll[0]), (j+dir[k][1]*gg_roll[0]))==0
					|| iBoardItem((i+dir[k][0]*gg_roll[1]), (j+dir[k][1]*gg_roll[1]))==0);
			}
		}
	}

	return res;
}

BOOL iMoves(int t, int a, int b)
{
	int da[2], db[2], c, i;
	const int dir[4][2]={{0,-1},{1,0},{0,1},{-1,0}};

	if(t!=3) a=(16*t-17); // ajust x if behind line

	// clear moves
	for(i=1; i<9; i++) gg_selected[0][i]=gg_selected[1][i]=15;

	// get moves
	for(i=0; i<4; i++)
	{
		c=(2*i);
		da[0]=(dir[i][0]*gg_roll[0]+a);
		da[1]=(dir[i][1]*gg_roll[0]+b);
		db[0]=(dir[i][0]*gg_roll[1]+a);
		db[1]=(dir[i][1]*gg_roll[1]+b);

		if(iBoardItem(da[0], da[1])==0)
		{
			gg_selected[0][c+1]=da[0];
			gg_selected[1][c+1]=da[1];
		}
		if(iBoardItem(db[0], db[1])==0)
		{
			gg_selected[0][c+2]=db[0];
			gg_selected[1][c+2]=db[1];
		}
	}

	return TRUE;
}

int iBoardItem(int a, int b)
{
	return ((a>-1 && b>-1 && a<15 && b<10)? gg_board[a][b]:-1);
}

BOOL iIsFreeGuy(int a, int b)
{
	if(a<0 || b<0 || a>14 || b>10) return FALSE;
	BOOL res=FALSE;
	for(int i=0; i<20; i++) res=(res || (gg_freeguys[0][i]==a
		&& gg_freeguys[1][i]==b));
	return res;
}


/////////////////////////////////
// Chordinate Functions
long chdDocX(int a) {return (443*a-430);}
long chdBoardX(int a) {return (27*a+45);}
long chdBoardY(int b) {return (17*b+31);}
long chdFreePY(void) {return (11*(gg_pselnum<7? gg_pselnum:(12-gg_pselnum)));}


/////////////////////////////////
// Menu Functions
BOOL changeOption(UINT menuItem, int arritem)
{
	gg_options[arritem]=(!gg_options[arritem]);
	CheckMenuItem(GetMenu(gg_hwndMain), menuItem,
		(gg_options[arritem]? MF_CHECKED:MF_UNCHECKED));
	return TRUE;
}

BOOL changeAniSpeed(UINT menuItem)
{
	HMENU mainMenu=GetMenu(gg_hwndMain);

	gg_framedevide=(menuItem==gm_fastani? 1:(menuItem==gm_medani? gd_fdeva:gd_fdevb));
	CheckMenuItem(mainMenu, gm_fastani, (gg_framedevide==1? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_medani, (gg_framedevide==gd_fdeva? MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(mainMenu, gm_slowani, (gg_framedevide==gd_fdevb? MF_CHECKED:MF_UNCHECKED));
	return TRUE;
}

BOOL newGame(void)
{
	aDie(TRUE, gg_player, gg_roll[0], 0);
	aDie(TRUE, gg_player, gg_roll[1], 1);

	initBoard();
	pBoard();

	SetCursor(LoadCursor(NULL, IDC_WAIT));
	DialogBox(gg_hInst, (LPCTSTR)grd_roll, gg_hwndMain, (DLGPROC)rollWinProc);

	gg_canpaint=FALSE;
	aScrollScreen();
	gg_canpaint=TRUE;
	rollDice();
	if(gg_options[4] && gg_player==1) startPcPlayer();
	return TRUE;
}


/////////////////////////////////
// Utility Functions
int customBox(HWND hWnd, UINT iconId, UINT soundId, 
			  char* text, char* btna, char* btnb)
{
	DWORD ans;
	int res;
	sprintf(ggcb_text[0], "%s", text);
	sprintf(ggcb_text[1], "%s", btna);
	sprintf(ggcb_text[2], "%s", btnb);
	ggcb_numdat[0]=iconId;
	ggcb_numdat[1]=(btnb==NULL);
	if(soundId) sndPlay(soundId);
	ans=DialogBox(gg_hInst, (LPCTSTR)grd_custom, hWnd, (DLGPROC)custProc);
	res=((btnb!=NULL && ans==gdi_buttona)? gd_btnokay:gd_btncancel);

	return res;
}

BOOL reportError(int errorNum, BOOL win)
{
	HWND hWnd=(win? gg_hwndMain:NULL);
	char buf[256];
	switch(errorNum)
	{
	case 1: sprintf(buf, "An error has occurred during"
		" creation of the main window."); break;
	case 2: sprintf(buf, "Unable to save the settings - make"
		" sure this disk is not locked."); break;
	case 3: sprintf(buf, "Unable to load settings - the"
		" default settings will be used."); break;
	case 4: sprintf(buf, "The computer player has errored"
		" and cannot move."); break;
	case 5: sprintf(buf, "Unable to locate help file."); break;
	case 7: sprintf(buf, "Error: 01"); break;
	default: sprintf(buf, "Unknown Error."); break;
	}
	customBox(hWnd, gri_error, grs_error, buf, "OK", NULL);
	return FALSE;
}

long randNum(long n)
{
	long a;
	a=rand();
	while(a>=32766) a=rand();
	return(static_cast<long>(a/(32767/n)));
}

BOOL sndPlay(DWORD id)
{
	if(gg_options[1]) PlaySound((LPCTSTR)id, gg_hInst, SND_RESOURCE | SND_ASYNC);
	return TRUE;
}

BOOL waitTicks(DWORD ticks)
{
	DWORD sTcks=GetTickCount();
	DWORD a=(unsigned long)(ticks/gg_framedevide);
	while((GetTickCount()-a)<sTcks){}
	return TRUE;
}

BOOL wantToQuit(HWND hWnd)
{
	if(customBox(hWnd, gri_question, grs_question, "Are you sure you want to exit?",
		"Exit", "Cancel")==gd_btncancel) return FALSE;
	writeSettings();
	DestroyWindow(gg_hwndMain);

	return TRUE;
}

BOOL destBrushes(void)
{
	// delete brushes
	DeleteObject(ggb_greya);
	DeleteObject(ggb_greyb);
	DeleteObject(ggb_greyc);
	DeleteObject(ggb_greyd);
	DeleteObject(ggb_lightgrey);
	DeleteObject(ggb_lighttan);
	DeleteObject(ggb_lightteal);
	DeleteObject(ggb_background);

	// delete pens
	DeleteObject(ggp_black);
	DeleteObject(ggp_bkborder);
	DeleteObject(ggp_darkgrey);
	DeleteObject(ggp_darktan);
	DeleteObject(ggp_darkteal);

	// delete fonts
	DeleteObject(ggf_maina);
	DeleteObject(ggf_mainb);

	// delete dcs
	ReleaseDC(gg_hwndMain, ggh_maind);
	DeleteDC(ggh_maina);
	DeleteDC(ggh_paint);
	DeleteDC(ggh_masks);
	DeleteDC(ggh_eff);
	DeleteDC(ggh_select);
	DeleteDC(ggh_selani);
	DeleteDC(ggh_fglow);

	// delete bitmaps
	DeleteObject(ggm_main);
	DeleteObject(ggm_paint);
	DeleteObject(ggm_masks);
	DeleteObject(ggm_eff);
	DeleteObject(ggm_select);
	DeleteObject(ggm_selani);
	DeleteObject(ggm_fglow);

	return TRUE;
}


/////////////////////////////////
// Midi Functions
BOOL mMakeRandList(void)
{
	int i, j=randNum(5);
	for(i=0; i<gd_nummidi; i++) gg_midlist[i]=0;
	for(i=1; i<=gd_nummidi; i++)
	{
		while(gg_midlist[j]!=0) j=randNum(gd_nummidi);
		gg_midlist[j]=i;
	}
	return TRUE;
}

BOOL mNextSong(void)
{ 
	BOOL res=FALSE;
	int i=0;

	while(!res)
	{
		if(i++>=gd_nummidi) return FALSE;

		gg_midopt[2]=(gg_midopt[2]==gd_nummidi? 1:(gg_midopt[2]+1)); // change mid number
		if(gg_midopt[0] && gg_midopt[2]==1) mMakeRandList(); // make list

		// play song
		res=mPlayMidi();
	}

	return TRUE;
}

BOOL mPlayMidi(void)
{
	char fileName[256];
	char string[256];
	sprintf(fileName, "%s%d.mid", gd_midtitle, mCurentSong());
	sprintf(string, "open %s type sequencer alias MUSIC", fileName);
	
	SetCurrentDirectory(gg_path);

	if(mciSendString("close all", NULL, 0, NULL)) return FALSE;
	if(mciSendString(string, NULL, 0, NULL)) return FALSE;
	if(mciSendString("play MUSIC from 0 notify", NULL, 0, gg_hwndMain)) return FALSE;
	return TRUE;
}

BOOL mPauseMidi(void)
{
	gg_midopt[1]=0;
    if(mciSendString("stop MUSIC", NULL, 0, NULL)) return FALSE;
    return TRUE;
}

BOOL mResumeMidi(void)
{
	gg_midopt[1]=1;
    if(mciSendString("play MUSIC notify", NULL, 0, gg_hwndMain)) return FALSE;
    return TRUE;
}

BOOL mCloseMidi(void)
{
	if(mciSendString("close all", NULL, 0, NULL)) return FALSE;
    return TRUE;
}

int mCurentSong(void)
{
	return (gg_midopt[0]? gg_midlist[gg_midopt[2]-1]:gg_midopt[2]);
}


/////////////////////////////////
// File Writing Functions
BOOL openSettings(void)
{
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	char fileName[256];
	char* LfileName=fileName;
	int readDat[8]={0};
	HANDLE file;
	DWORD wtn;

	SetCurrentDirectory(gg_path);
	GetFullPathName("settings.stg", 256, fileName, &LfileName);
	file=CreateFile(fileName, GENERIC_READ, 0, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if(file==INVALID_HANDLE_VALUE)
	{
		reportError(3);
		return FALSE;
	}
	
	ReadFile(file, &readDat, 32, &wtn, NULL);
	ReadFile(file, &gg_names[0], 15, &wtn, NULL);
	ReadFile(file, &gg_names[1], 15, &wtn, NULL);
	CloseHandle(file);

	if(readDat[0]==123) // if file exists
	{
		gg_options[1]	=readDat[1];
		gg_options[2]	=readDat[2];
		gg_options[3]	=readDat[3];
		gg_options[4]	=readDat[4];
		gg_midopt[0]	=readDat[5];
		gg_framedevide	=(readDat[6]==1? 1:(readDat[6]==2? gd_fdeva:gd_fdevb));
		gg_numholes		=readDat[7];
	}
	else writeSettings();
	return TRUE;
}

BOOL writeSettings(void)
{
	SetCursor(LoadCursor(NULL, IDC_WAIT));

	char fileName[256];
	char* LfileName=fileName;
	int wrteDat[8];
	HANDLE file;
	DWORD wtn;

	SetCurrentDirectory(gg_path);
	GetFullPathName("settings.stg", 256, fileName, &LfileName);
	int aniSpeed=(gg_framedevide==1? 1:(gg_framedevide==gd_fdeva? 2:3));

	file=CreateFile(fileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	if(file==INVALID_HANDLE_VALUE)
	{
		reportError(2);
		return FALSE;
	}

	wrteDat[0]=123; // use to see if file existed
	wrteDat[1]=gg_options[1];
	wrteDat[2]=gg_options[2];
	wrteDat[3]=gg_options[3];
	wrteDat[4]=gg_options[4];
	wrteDat[5]=gg_midopt[0];
	wrteDat[6]=aniSpeed;
	wrteDat[7]=gg_numholes;

	WriteFile(file, &wrteDat, 32, &wtn, NULL);
	WriteFile(file, &gg_names[0], 15, &wtn, NULL);
	WriteFile(file, &gg_names[1], 15, &wtn, NULL);
	CloseHandle(file);
	return TRUE;
}