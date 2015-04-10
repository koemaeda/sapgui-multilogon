/*
* SAPGUI MultiLogon
* (c) 2015 Guilherme Maeda
* http://abap.ninja
*
* For the full license information, view the LICENSE file that was distributed
* with this source code.
*/
#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include "winmain.h"
#include "resources.h"


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInst, char * szCmdLine, int iShow) {
	char szDirPath[MAX_PATH];
	char szExePath[MAX_PATH];
	char szDllPath[MAX_PATH];

	///////////////////////////////////////////////////////////////////
	//  Find SAP Logon and the MultiLogon DLL
	///////////////////////////////////////////////////////////////////

	// Get the SAPGUI directory
	if (! GetSAPGUIDirectory(szDirPath, sizeof(szDirPath))) {
		MessageBox(NULL, "Initialization failed! sapgui.exe was not found.", APPLICATION_NAME, MB_ICONERROR);
		return -1;
	}

	// Check if saplogon.exe/saplgpad.exe exists
	sprintf(szExePath, "%s\\saplogon.exe", szDirPath);
	if (GetFileAttributes (szExePath) == INVALID_FILE_ATTRIBUTES) {
		// Maybe saplgpad.exe then?
		sprintf(szExePath, "%s\\saplgpad.exe", szDirPath);
		if (GetFileAttributes (szExePath) == INVALID_FILE_ATTRIBUTES) {
			MessageBox(NULL, "Initialization failed! sapgui.exe was not found.", APPLICATION_NAME, MB_ICONERROR);
			return -1;
		}
	}

	// Get the DLL filename //
	GetCurrentDirectory (sizeof(szDllPath), szDllPath);
	strcat (szDllPath, "\\sapgui_multilogon.dll");
	if (GetFileAttributes (szDllPath) == INVALID_FILE_ATTRIBUTES) {
		MessageBox(NULL, "Initialization failed! sapgui_multilogon.dll was not found.", APPLICATION_NAME, MB_ICONERROR);
		return -1;
	}

	///////////////////////////////////////////////////////////////////
	//  Launch SAP Logon
	///////////////////////////////////////////////////////////////////

	HANDLE hProcess, hMainThread = NULL;

	// Check if SAP Logon is already running
	int iPID = GetPID("saplogon.exe");
	if (! iPID)
		iPID = GetPID("saplgpad.exe");
	if (iPID) {
		// Process found, open it to inject the library
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, iPID);
		if (! hProcess) {
			ShowLastErrorMessage();
			return -1;
		}
	}
	else {
		// Create the SAP Logon process //
		STARTUPINFO stStartupInfo = { sizeof(STARTUPINFO) };
		PROCESS_INFORMATION stProcInfo;
		if(! CreateProcess(szExePath, NULL, NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, szDirPath, &stStartupInfo, &stProcInfo)) {
			ShowLastErrorMessage();
			return -1;
		}
		hProcess = stProcInfo.hProcess;
		hMainThread = stProcInfo.hThread;
	}

	///////////////////////////////////////////////////////////////////
	//  Inject the MultiLogon DLL
	///////////////////////////////////////////////////////////////////

	// Copy the DLL path to the remote process //
	int iSize = strlen (szDllPath) + 1;
	void * pRemoteLibraryPath = VirtualAllocEx (hProcess, NULL, iSize, MEM_COMMIT, PAGE_READWRITE);
	if (! WriteProcessMemory (hProcess, pRemoteLibraryPath, szDllPath, iSize, NULL)) {
		ShowLastErrorMessage();
		return -1;
	}

	// Create a remote LoadLibraryA suspended thread //
	HANDLE hInjThread = CreateRemoteThread (hProcess, NULL, 0,
		(LPTHREAD_START_ROUTINE) GetProcAddress (GetModuleHandle ("kernel32"),
		"LoadLibraryA"), pRemoteLibraryPath, 0, NULL);
	if (! hInjThread) return FALSE;

	ResumeThread(hInjThread);
	ResumeThread(hMainThread);

	CloseHandle(hInjThread);
	CloseHandle(hProcess);
	CloseHandle(hMainThread);

	return 0;
}

BOOL GetSAPGUIDirectory(char * szBuffer, int iBufferSize) {
	char szDirPath[MAX_PATH];
	char szExePath[MAX_PATH];

	// Check if the directory we're running in has sapgui.exe
	GetCurrentDirectory(sizeof(szDirPath), szDirPath);
	sprintf(szExePath, "%s\\sapgui.exe", szDirPath);
	if (GetFileAttributes (szExePath) != INVALID_FILE_ATTRIBUTES) {
		strncpy(szBuffer, szDirPath, iBufferSize);
		return TRUE;
	}

	// Look for it in Program Files
	ExpandEnvironmentStrings("%ProgramFiles%", szDirPath, sizeof(szDirPath));
	strcat(szDirPath, "\\SAP\\FrontEnd\\SAPgui");
	sprintf(szExePath, "%s\\sapgui.exe", szDirPath);
	if (GetFileAttributes (szExePath) != INVALID_FILE_ATTRIBUTES) {
		strncpy(szBuffer, szDirPath, iBufferSize);
		return TRUE;
	}

	return FALSE;
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


int GetPID (const char* szProcessName) {
	HANDLE hSnapshot;
	PROCESSENTRY32 stProcess;

	// Compare with all processes //
	hSnapshot = CreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) return FALSE;
	stProcess.dwSize = sizeof (PROCESSENTRY32);

	if (Process32First (hSnapshot, &stProcess))
		do
			if (! _stricmp (stProcess.szExeFile, szProcessName))
				return stProcess.th32ProcessID;
		while (Process32Next (hSnapshot, &stProcess));

	CloseHandle (hSnapshot);
	return 0;
}
