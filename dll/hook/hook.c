// API Hook Library -- Hook functions
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

#include <windows.h>
#include "ollyasm\disasm.h"
#include "hook.h"

BOOL HookInstall (const char * szModule, const char * szFunction, void * pStub, void ** pHopFunction, PHOOKDATA pHookData) {
	DWORD iProtect, iUseless;

	// Get the function address //
	pHookData->pFunction = (unsigned char *) GetProcAddress (GetModuleHandle (szModule), szFunction);
	if (! pHookData->pFunction) return FALSE;

	// Determine the hook size (by disassembling the original function)
	if (! HookGetSize (pHookData))
		return FALSE;

	// Unprotect the first X bytes //
	if (! VirtualProtect (pHookData->pFunction, pHookData->iHookSize, PAGE_EXECUTE_READWRITE, &iProtect))
		return FALSE;

	// Backup the first X bytes //
	memcpy (pHookData->szBackup, pHookData->pFunction, pHookData->iHookSize);

	// Set the first 5 bytes to [JMP pStub] and the rest to NOPs //
	pHookData->pFunction[0] = 0xE9;
	*(DWORD *) &pHookData->pFunction[1] =
		(DWORD) pStub - (DWORD) pHookData->pFunction - 5;
	for (int i=5; i<pHookData->iHookSize; i++)
		pHookData->pFunction[i] = 0x90;

	// Reprotect the first X bytes //
	VirtualProtect (pHookData->pFunction, pHookData->iHookSize, iProtect, &iUseless);

	// Set the hook hop function //
	if (pHookData->szBackup[0] == 0xE9) { // Hooked function is just a JMP
		*(DWORD *) pHopFunction = *(DWORD *) &pHookData->szBackup[1] + (DWORD) pHookData->pFunction + 5;
	}
	else { // Hooked function has normal code
		*(DWORD *) pHopFunction = (DWORD) pHookData->szBackup;
		VirtualProtect (*pHopFunction, pHookData->iHookSize, PAGE_EXECUTE_READWRITE, &iUseless);
	}

	// Set the hook hop helper //
	pHookData->szBackup[pHookData->iHookSize] = 0xE9;
	*(DWORD *) &(pHookData->szBackup[pHookData->iHookSize+1]) =
		(DWORD) pHookData->pFunction - (DWORD) pHookData->szBackup - 5;
	VirtualProtect (pHookData->szBackup, HOOK_BACKUP_SIZE, PAGE_EXECUTE_READWRITE, &iUseless);

	return TRUE;
}

BOOL HookUninstall (PHOOKDATA pHookData) {
	DWORD iProtect, iUseless;

	// Unprotect the first X bytes //
	if (! VirtualProtect (pHookData->pFunction, pHookData->iHookSize, PAGE_EXECUTE_READWRITE, &iProtect))
		return FALSE;

	// Restore the first X bytes from the backup //
	memcpy (pHookData->pFunction, pHookData->szBackup, pHookData->iHookSize);

	// Reprotect the first X bytes //
	VirtualProtect (pHookData->pFunction, pHookData->iHookSize, iProtect, &iUseless);

	return TRUE;
}


BOOL HookGetSize (PHOOKDATA pHookData) {
	t_disasm stDa;
	unsigned char * pCurOp;
	int iOpSize;

	// Disassemble the beginning of the hooked function to determine the correct
	// size to be changed, in order not to have the hop land in the middle of
	// an operand.
	pCurOp = pHookData->pFunction;
	pHookData->iHookSize = 0;
	while (pHookData->iHookSize < 5) { // 5 is the minimum for the JMP
		iOpSize = Disasm(pCurOp, MAXCMDSIZE, 0, &stDa, DISASM_SIZE);
		if (! iOpSize)
			return FALSE;

		pCurOp += iOpSize;
		pHookData->iHookSize += iOpSize;
	}

	return TRUE;
}
