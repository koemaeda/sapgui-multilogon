/*
* SAPGUI MultiLogon
* (c) 2015 Guilherme Maeda
* http://abap.ninja
*
* For the full license information, view the LICENSE file that was distributed
* with this source code.
*/
#ifndef WINMAIN_H_INCLUDED
#define WINMAIN_H_INCLUDED

#define APPLICATION_NAME "SAPGUI MultiLogon"

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInst, char * szCmdLine, int iShow);
BOOL GetSAPGUIDirectory(char * szBuffer, int iBufferSize);
DWORD ShowLastErrorMessage (void);
int GetPID (const char* szProcessName);

#endif //WINMAIN_H_INCLUDED
