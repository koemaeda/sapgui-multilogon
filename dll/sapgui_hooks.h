/*
* SAPGUI MultiLogon
* (c) 2015 Guilherme Maeda
* http://abap.ninja
*
* For the full license information, view the LICENSE file that was distributed
* with this source code.
*/
#ifndef SAPGUI_HOOKS_H_INCLUDED
#define SAPGUI_HOOKS_H_INCLUDED

BOOL InstallSAPGUIHooks (HINSTANCE hInstance);
BOOL RemoveSAPGUIHooks (void);
void CloseAllSessions(void);

LRESULT (WINAPI AboutDialogHookProc)(int nCode, WPARAM wParam, LPARAM lParam);
DWORD WINAPI SessionThread(void * pArg);
HWND FindSAPWindowByThreadId(DWORD iThreadId);

#endif // SAPGUI_HOOKS_H_INCLUDED
