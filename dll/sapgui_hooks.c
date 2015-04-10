/*
* SAPGUI MultiLogon
* (c) 2015 Guilherme Maeda
* http://abap.ninja
*
* For the full license information, view the LICENSE file that was distributed
* with this source code.
*/
#define WIN32_LEAN_AND_MEAN

/*---------------------------------------------------------------------
   Important:
	Don't allocate memory using malloc/free, or use any library function
	that does this (like sprintf). Instead, use the msvcr* functions
	(like wsprintf).
---------------------------------------------------------------------*/

#include <stdio.h>
#include <windows.h>
#include <windef.h>
#include <windowsx.h>
#include "hook\hook.h"
#include "sapgui_hooks.h"
#include "dllmain.h"
#include "resources.h"

#define IS_KEY_PRESSED(vk) (GetAsyncKeyState(vk) & 0x8000)

#define ASSERT_HOOK_INSTALL(name, lib, expName) \
	if (! HOOK_INSTALL(name, lib, expName)) { \
		MessageBox(NULL, "Hook() failed: " #name, APPLICATION_NAME, MB_ICONERROR); \
		return FALSE; \
	}

#define GET_THIS(this) _asm { mov this, ecx }
#define SET_THIS(this) _asm { mov ecx, this }


static HINSTANCE ghThisInstance = NULL;
static HHOOK ghAboutDialogHook = NULL;
static HWND ghAboutDialogWnd;
static WNDPROC ghAboutDialogOriginalWndProc;

/*---------------------------------------------------------------------
   Declare SAPGUI DLL hooks
---------------------------------------------------------------------*/

// Saplgpad (Logon Pad) hooks
HOOK_DECLARE(CSplashWindow_OnPaint, void, __thiscall, (void));

// SAPpcfvd.dll (Version dialog) hooks
HOOK_DECLARE(SapPcVersionDialog2, void, __stdcall, (const char *, const char *, const char *, const char *, const char *, int));

// SAPfewui.dll (User Interface Manager) hooks
HOOK_DECLARE(CUiObject_IsChecked, int, __thiscall, (void));
HOOK_DECLARE(CUiObject_SetChecked, void, __thiscall, (int));

/*---------------------------------------------------------------------
   Hook functions implementations
---------------------------------------------------------------------*/

/**
 * Install/enable the hooks
 */
BOOL InstallSAPGUIHooks (HINSTANCE hInstance) {
	ghThisInstance = hInstance;

	// SAPlgpad.exe (Logon Pad) / Saplogon.exe (Logon)
	if (! HOOK_INSTALL(CSplashWindow_OnPaint, "saplgpad.exe", "?OnPaint@CSplashWindow@@IAEXXZ")) {
		ASSERT_HOOK_INSTALL(CSplashWindow_OnPaint, "saplogon.exe", "?OnPaint@CSplashWindow@@IAEXXZ");
	}

	// SAPfewui.dll (User Interface Manager) hooks
	ASSERT_HOOK_INSTALL(CUiObject_IsChecked, "SAPfewui.dll", "?IsChecked@CUiObject@@QBEHXZ");
	ASSERT_HOOK_INSTALL(CUiObject_SetChecked, "SAPfewui.dll", "?SetChecked@CUiObject@@UAEXH@Z");

	// SAPpcfvd (Version Dialog) hooks
	LoadLibrary("SAPpcfvd.dll"); // DLL is delay-loaded
	ASSERT_HOOK_INSTALL(SapPcVersionDialog2, "SAPpcfvd.dll", "SapPcVersionDialog2");

	// Check if SAPGUI was already running
	if (GetModuleHandle("sapguidrw32.dll")) {
		MessageBox(NULL, APPLICATION_NAME" initialized successfuly!", APPLICATION_NAME,
			MB_ICONINFORMATION|MB_SYSTEMMODAL);
	}
	return TRUE;
}

/**
 * Remove/disable the hooks
 */
BOOL RemoveSAPGUIHooks (void) {
	// SAPlgpad.exe (Logon Pad) / Saplogon.exe (Logon)
	HOOK_UNINSTALL(CSplashWindow_OnPaint);

	// SAPfewui.dll (User Interface Manager) hooks
	HOOK_UNINSTALL(CUiObject_IsChecked);
	HOOK_UNINSTALL(CUiObject_SetChecked);

	// SAPpcfvd (Version Dialog) hooks
	HOOK_UNINSTALL(SapPcVersionDialog2);

	return TRUE;
}


/**
 * Splash window OnPaint method
 * Called when the splash windows is to be painted
 * Here we call the original function to paint the splash window, then paint our
 *  logo bitmap on top of it.
 */
HOOK_STUB_FUNCTION(CSplashWindow_OnPaint, void, __thiscall, (void)) {
	const char * pThis;

	// Initialization
	GET_THIS(pThis);
	HWND hSplashWindow = *(HWND *) (pThis + 0x20);

	// Let the standard painting happen first
	SET_THIS(pThis);
	HOOK_HOP(CSplashWindow_OnPaint)();

	// Now we paint our stuff on top of it
	PaintLogoBitmap(hSplashWindow, -20, 20);
}

/**
 * About dialog function
 * Called when the help->about menu is triggered.
 * Here we setup a Windows Hook (to display our logo on the dialog) and
 *  change the Release parameter value to include the MultiLogon suffix.
 */
HOOK_STUB_FUNCTION(SapPcVersionDialog2, void, __stdcall,
		(const char* szFileName, const char* szComponent, const char* szFileVersion,
		const char* szRelease, const char* szBuild, int iArg6)) {

	// Setup the window hook for this thread
	ghAboutDialogHook = SetWindowsHookEx(WH_CALLWNDPROC, AboutDialogHookProc,
		NULL, GetCurrentThreadId());

	// Show updated version dialog
	char szNewRelease[128];
	wsprintf(szNewRelease, "%s / MultiLogon " APPLICATION_VERSION, szRelease);
	HOOK_HOP(SapPcVersionDialog2)(szFileName, szComponent, szFileVersion, szNewRelease, szBuild, iArg6);
}

/**
 * About dialog alternative procedure
 * This is the window procedure we set up for the About dialog.
 * Here we display our logo bitmap and call the original procedure.
 */
LRESULT CALLBACK AboutDialogWndProc (HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	// On paint and timer messages, call the original procedure first,
	//  then draw the logo on top of the dialog.
	switch (iMessage) {
		case WM_PAINT:
		case WM_NCPAINT:
		case WM_TIMER:
			CallWindowProc(ghAboutDialogOriginalWndProc, hWnd, iMessage, wParam, lParam);
			PaintLogoBitmap(hWnd, -20, 20);
			return TRUE;
	}

	return CallWindowProc(ghAboutDialogOriginalWndProc, hWnd, iMessage, wParam, lParam);
}

/**
 * The windows hook procedure for the About dialog
 * Here we change the dialog procedure (making it our own) and setup the timer.
 */
LRESULT (WINAPI AboutDialogHookProc)(int nCode, WPARAM wParam, LPARAM lParam) {
	PCWPSTRUCT pMsg = (PCWPSTRUCT) lParam;
	LRESULT iRet = CallNextHookEx(ghAboutDialogHook, nCode, wParam, lParam);

	if (nCode != HC_ACTION) // Only process action events
		return iRet;

	switch (pMsg->message) {
		case WM_INITDIALOG:
			// Intercept the dialog procedure
			ghAboutDialogWnd = pMsg->hwnd;
			ghAboutDialogOriginalWndProc = (WNDPROC) GetWindowLong(pMsg->hwnd, GWL_WNDPROC);
			SetWindowLong(pMsg->hwnd, GWL_WNDPROC, (LONG) AboutDialogWndProc);
			SetTimer(pMsg->hwnd, 1, 100, NULL); // Set a timer to draw the logo on the about dialog

			// We're done here, remove the hook
			UnhookWindowsHookEx(ghAboutDialogHook);
			break;
    }

	return iRet;
}

/**
 * UI objects (screen controls) IsCheck method
 * Called to determine if a screen control is checked (eg checkboxes)
 * Here, if Ctrl key is pressed, we invert the checked state of the UIObject.
 * This makes any radiobutton act as a checkbox when clicked (with Ctrl pressed ofc).
 */
HOOK_STUB_FUNCTION(CUiObject_IsChecked, int, __thiscall, (void)) {
	void * pThis;
	int iChecked;

	GET_THIS(pThis);
	iChecked = HOOK_HOP(CUiObject_IsChecked)();

	// If Ctrl is pressed, invert the checked state
	//  (check if not checked and vice versa)
	if (IS_KEY_PRESSED(VK_CONTROL)) {
		SET_THIS(pThis);
		HOOK_HOP(CUiObject_SetChecked)(!iChecked);
		return FALSE;
	}
	else
		return iChecked;
}

/**
 * UI objects (screen controls) SetChecked method
 * Called to change the checked state of a screen control (eg checkboxes)
 * Here, if Ctrl key is pressed, we just ignore the calls.
 * This makes the radiobuttons act as checkboxes, because no radiobuttons are de-checked
 *  when another radiobutton in the same group is checked.
 */
HOOK_STUB_FUNCTION(CUiObject_SetChecked, void, __thiscall, (int iChecked)) {
	void * pThis;

	GET_THIS(pThis);

	// If Ctrl is pressed, ignore this call
	if (IS_KEY_PRESSED(VK_CONTROL))
		return;

	SET_THIS(pThis);
	HOOK_HOP(CUiObject_SetChecked)(iChecked);
}
