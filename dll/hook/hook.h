// API Hook Library -- Header file
//
// Copyright (C) 2010 Guilherme Maeda
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef HOOK_H_INCLUDED
#define HOOK_H_INCLUDED

/***********************************************************************************\
* How the hook works:                                                               *
*  __________       __________       __________       __________                    *
* |          |---->||  Jump  ||---->|          |---->| Original |                   *
* |          |     ||________||     |          |     | X bytes  |                   *
* |          |     |          |<---------------------|__________|                   *
* | Calling  |     |  Hooked  |     |   Stub   |       Hook hop                     *
* | Process  |     | Function |     | Function |        helper                      *
* |          |     |__________|---->|          |                                    *
* |          |                      |          |                                    *
* |__________|<---------------------|__________|                                    *
*                                                                                   *
* 1) The process calls the hooked function.                                         *
* 2) The hooked function jumps to the stub function.                                *
* 3) The stub function calls the hooked function's original first bytes.            *
* 4) The first bytes jump to the hooked function right after the hook. (Hook hop)   *
* 5) After the hooked function returns, the stub function returns to the process.   *
\***********************************************************************************/

/*
 * Macro to declare the necessary objects for a hook:
 *  name: The hook name (must be a valid C variable/function name).
 *  ret_type: Return type of the hooked function.
 *  call_conv: Calling convention of the hooked function.
 *  params: The function parameters (enclosed in parenthesis).
 *
 * Usage example: HOOK_DECLARE(MyMessageBeep, BOOL, WINAPI, (UINT));
 */
#define HOOK_DECLARE(name, ret_type, call_conv, params) \
	ret_type call_conv name##_Stub params; \
	static ret_type (call_conv * name##_Hop) params; \
	static HOOKDATA name##_HookData;

/*
 * Macro to install/enable a hook:
 *  name: The hook name (must be the same name used in HOOK_DECLARE).
 *  lib: The DLL library name.
 *  expName: The hooked function's exported name in the library.
 *
 * Usage example: INSTALL_HOOK(MyMessageBeep, "User32.dll", "MessageBeep");
 */
#define HOOK_INSTALL(name, lib, expName) \
	HookInstall(lib, expName, (void *) name##_Stub, (void **) &name##_Hop, &name##_HookData)

/*
 * Macro to uninstall/disable a hook:
 *  name: The hook name (must be the same name used in HOOK_DECLARE).
 *
 * Usage example: HOOK_UNINSTALL(MyMessageBeep);
 */
#define HOOK_UNINSTALL(name) \
	HookUninstall(&name##_HookData)

/*
 * Macro to declare the Stub function implementation:
 *  name: The hook name (must be the same name used in HOOK_DECLARE).
 *  ret_type: Return type of the hooked function (eg DWORD).
 *  call_conv: Calling convention of the hooked function (eg __stdcall).
 *  params: The function parameters (enclosed in parenthesis).
 *
 * Usage example:
 *  HOOK_STUB_FUNCTION(MyMessageBeep, BOOL, WINAPI, (UINT uType)) {
 *      // Make every beep sound like an error lol
 *      return HOOK_HOP(MyMessageBeep) (MB_ICONERROR);
 *  }
 */
#define HOOK_STUB_FUNCTION(name, ret_type, call_conv, params) \
	ret_type call_conv name##_Stub params

/*
 * Macro to use the Hop function:
 *  name: The hook name (must be the same name used in HOOK_DECLARE).
 *
 * Usage example: HOOK_HOP(MyMessageBeep)(MB_ICONERROR);
 */
#define HOOK_HOP(name) name##_Hop


#ifndef __thiscall
#define __thiscall __stdcall
#endif // __thiscall

#define HOOK_BACKUP_SIZE 32

// Hook control structure
typedef struct _HOOKDATA {
	unsigned char * pFunction;
	void ** pHopFunction;
	DWORD iHopAddress;
	int iHookSize;
	unsigned char szBackup[HOOK_BACKUP_SIZE];
} HOOKDATA, * PHOOKDATA;


// API functions
BOOL HookInstall (const char * szModule, const char * szFunction, void * pStub, void ** pHopFunction, PHOOKDATA pHookData);
BOOL HookUninstall (PHOOKDATA pHookData);

// Internal functions
BOOL HookGetSize (PHOOKDATA pHookData);


#endif // HOOK_H_INCLUDED
