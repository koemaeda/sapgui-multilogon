#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "dllmain.h"
#include "sapgui_hooks.h"
#include "resources.h"

static HMODULE ghThisModule = NULL;
static HBITMAP ghBmpLogo = NULL;
static HBITMAP ghBmpLogoMask = NULL;


BOOL WINAPI DllMain (HINSTANCE hInstance, DWORD iReason, void * pReserved) {
	switch (iReason) {
		case DLL_PROCESS_ATTACH:
			ghThisModule = hInstance;

			// Load our resources
			LoadLogoBitmap();

			// Install SAPGUI hooks //
			if (! InstallSAPGUIHooks(hInstance)) {
				MessageBox(NULL, APPLICATION_NAME" não foi inicializado corretamente.", APPLICATION_NAME, MB_ICONERROR);
				ExitProcess(-1);
			}

			DisableThreadLibraryCalls (hInstance);
			break;

		case DLL_PROCESS_DETACH:
			// Remove SAPGUI hooks //
			if (! RemoveSAPGUIHooks())
				MessageBox(NULL, APPLICATION_NAME" não foi finalizado corretamente.", APPLICATION_NAME, MB_ICONERROR);
			break;
	}

	return TRUE;
}


void LoadLogoBitmap (void) {
	BITMAP bm;

	// Load logo bitmap
	ghBmpLogo = LoadBitmap(ghThisModule, "BMP_LOGO");
	GetObject(ghBmpLogo, sizeof(bm), &bm);

	HDC hdcLogo = CreateCompatibleDC(NULL);
	SelectBitmap(hdcLogo, ghBmpLogo);

	COLORREF iBkColor = GetPixel(hdcLogo, 0, 0); // transparency color

	// Print version information text
	HFONT hFont = CreateFont(12, 0, 0, 0, FW_BLACK, FALSE, FALSE, 0,
                        ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                        DEFAULT_PITCH | FW_NORMAL, "Courier New");
	SelectObject(hdcLogo, hFont);
	SetTextAlign(hdcLogo, TA_CENTER);
	int iTextX = (bm.bmWidth/2 - 1);
	int iTextY = 35;

	SetBkMode(hdcLogo, TRANSPARENT);

	SetTextColor(hdcLogo, RGB(0,0,0)); // black
	TextOut(hdcLogo, iTextX, iTextY, APPLICATION_VERSION, sizeof(APPLICATION_VERSION) - 1);

	// Mask out the background
	ghBmpLogoMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
	HDC hdcLogoMask = CreateCompatibleDC(NULL);
	SelectBitmap(hdcLogoMask, ghBmpLogoMask);

	SetBkColor(hdcLogo, iBkColor); // get the mask color
	BitBlt(hdcLogoMask, 0, 0, bm.bmWidth, bm.bmHeight, hdcLogo, 0, 0, SRCCOPY);

	SetTextColor(hdcLogo, RGB(255,255,255));
	SetBkColor(hdcLogo, RGB(0,0,0));
	BitBlt(hdcLogo, 0, 0, bm.bmWidth, bm.bmHeight, hdcLogoMask, 0, 0, SRCAND);

	// Cleanup
	DeleteDC(hdcLogo);
	DeleteDC(hdcLogoMask);
	DeleteObject(hFont);
}


void PaintLogoBitmap(HWND hWindow, int iX, int iY) {
	RECT stRect;
	BITMAP bm;

	// Calculate relative positions
	GetWindowRect(hWindow, &stRect);
	GetObject(ghBmpLogo, sizeof(bm), &bm);
	if (iX < 0)
		iX = stRect.right - stRect.left - bm.bmWidth + iX;
	if (iY < 0)
		iY = stRect.bottom - stRect.top - bm.bmHeight + iY;

	// Paint the bitmap
	HDC hDC = GetDC(hWindow);
	HDC hdcMem = CreateCompatibleDC(NULL);
	SelectBitmap(hdcMem, ghBmpLogoMask);
	BitBlt(hDC, iX, iY, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND);
	SelectBitmap(hdcMem, ghBmpLogo);
	BitBlt(hDC, iX, iY, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCPAINT);

	DeleteDC(hdcMem);
}


DWORD ShowLastErrorMessage (void) {
	char szMessage[512];
	DWORD iErrorCode;

	// Get the last message code //
	iErrorCode = GetLastError ();

	// Show the system error text //
	if ( FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM, NULL, iErrorCode, 0,
			szMessage, sizeof (szMessage), NULL));
		MessageBox (NULL, szMessage, APPLICATION_NAME, MB_ICONERROR);

	return iErrorCode;
}
