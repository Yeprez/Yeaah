#pragma once






DWORD* p_swap_chain;


void* detourBuffer;
const void* __cdecl DetourFunc(BYTE* src, const BYTE* dest, const DWORD length)
{
	BYTE* jump = new BYTE[length + 5];
	detourBuffer = jump;

	DWORD dwVirtualProtectBackup;
	VirtualProtect(src, length, PAGE_READWRITE, &dwVirtualProtectBackup);

	memcpy(jump, src, length);
	jump += length;

	jump[0] = 0xE9;
	*(DWORD*)(jump + 1) = (DWORD)(src + length - jump) - 5;

	src[0] = 0xE9;
	*(DWORD*)(src + 1) = (DWORD)(dest - src) - 5;

	VirtualProtect(src, length, dwVirtualProtectBackup, &dwVirtualProtectBackup);

	return jump - length;
}






typedef HRESULT(__stdcall* D3D11PresentHook) (IDXGISwapChain* p_swap_chain, UINT SyncInterval, UINT Flags);
D3D11PresentHook phookD3D11Present = NULL;