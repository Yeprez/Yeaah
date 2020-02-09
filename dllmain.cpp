// Standard imports
#include"pch.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <stdlib.h>

// Detours imports
//there will be something like hook
#include"DetourFunc.h"



// DX11 imports
#include <d3d11.h>
#include <D3Dcompiler.h>
#pragma comment(lib, "D3dcompiler.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "winmm.lib")


//ImGUI imports
#include "IMGUI//imgui.h"
#include "IMGUI//imgui_impl_win32.h"
#include "IMGUI//imgui_impl_dx11.h"
#include <mutex>

 
//imgui shit 
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL g_bInitialised = false;
bool g_ShowMenu = false;
//


//main d3d object 
static WNDPROC OriginalWndProcHandler = nullptr;

ID3D11Device* device_pointer;
ID3D11DeviceContext* context_pointer;

ID3D11RenderTargetView* mainRenderTargetView;

HWND window = nullptr;
//



typedef HRESULT(__stdcall* d3d11_present)(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags);
d3d11_present o_d3d11_present = nullptr;

std::once_flag init_d3d;

HRESULT hooked_d3d11_present(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags) {
	std::call_once(init_d3d, [&] {
		p_swap_chain->GetDevice(__uuidof(device_pointer), (void**)&device_pointer);
		device_pointer->GetImmediateContext(&context_pointer);
		});



	//here we can draw with imgui something 

	//imgui shit 
	DXGI_SWAP_CHAIN_DESC sd;
	p_swap_chain->GetDesc(&sd);
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	window = sd.OutputWindow;




	OriginalWndProcHandler = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)hWndProc);
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device_pointer, context_pointer);
	ImGui::GetIO().ImeWindowHandle = window;

	ID3D11Texture2D* pBackBuffer;

	p_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	device_pointer->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
	pBackBuffer->Release();


	g_bInitialised = true;


	//still IMGUI shit 
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();


	ImGui::NewFrame();
	if (g_ShowMenu)
	{
		bool bShow = true;
		ImGui::ShowDemoWindow(&bShow);
	}
	ImGui::EndFrame();

	ImGui::Render();

	context_pointer->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


	return o_d3d11_present(p_swap_chain, sync_interval, flags);
}





//ImGui shit 
LRESULT CALLBACK hWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	ImGuiIO& io = ImGui::GetIO();
	POINT mPos;
	GetCursorPos(&mPos);
	ScreenToClient(window, &mPos);
	ImGui::GetIO().MousePos.x = mPos.x;
	ImGui::GetIO().MousePos.y = mPos.y;

	if (uMsg == WM_KEYUP)
	{
		if (wParam == VK_DELETE)
		{
			g_ShowMenu = !g_ShowMenu;
		}

	}

	if (g_ShowMenu)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(OriginalWndProcHandler, hWnd, uMsg, wParam, lParam);
}







DWORD WINAPI main_thread(LPVOID lpvoid) {

	// this will hook the steam overlay
	// gameoverlayrenderer64.dll + 0x8A450 = hooked_d3d11_present 

	// get base address of the dll
	const auto renderer_handle = reinterpret_cast<uintptr_t>(GetModuleHandleA("GameOverlayRenderer64.dll"));

	// store the address of where we're going to hook
	const auto function_to_hook = renderer_handle + 0x8B9A0;

	
	//create  the default thing 
	HWND hWnd = GetForegroundWindow();
	IDXGISwapChain* p_swap_chain;
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.Windowed = (GetWindowLong(hWnd, GWL_STYLE) & (WS_POPUP != 0)) ? false : true;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;


	if (false(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, &featureLevel, 1
		, D3D11_SDK_VERSION, &swapChainDesc, &p_swap_chain, &device_pointer, NULL, &context_pointer)))
	{
		MessageBoxA(hWnd, "Failed to create directX device and swapchain!", "uBoos?", MB_ICONERROR);
		return NULL;
	}



	//we don`t need it cause we have a adress 
	//phookD3D11Present = (D3D11PresentHook)DetourFunc((BYTE*)pSwapChainVtable[8], (BYTE*)hookD3D11Present, 5);

	//hook 
	// (DetourFunc(reinterpret_cast<LPVOID>(function_to_hook), &hooked_d3d11_present, reinterpret_cast<void**>(&o_d3d11_present)));
	





	//you can find this in DetourFunc.h
	o_d3d11_present = (D3D11PresentHook)DetourFunc((BYTE*)function_to_hook, (BYTE*)hooked_d3d11_present, 5);

    DWORD dwOld;
	VirtualProtect(o_d3d11_present, 2, PAGE_EXECUTE_READWRITE, &dwOld);

	
	//after that we can release it 

	device_pointer->Release();
	context_pointer->Release();
	p_swap_chain->Release();

	return FALSE;
}




//after we have hooked :my version is above 
//HRESULT hooked_d3d11_present(IDXGISwapChain* p_swap_chain, UINT sync_interval, UINT flags) {
//	std::call_once(init_d3d, [&] {
//		p_swap_chain->GetDevice(__uuidof(device_pointer), reinterpret_cast<void**>(&device_pointer));
//		device_pointer->GetImmediateContext(&context_pointer);
//		});
//
//	// draw
//
//	return o_d3d11_present(p_swap_chain, sync_interval, flags);
//}





BOOL WINAPI DllMain(HINSTANCE hinstdll, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		CreateThread(nullptr, 0, main_thread, hinstdll, 0, nullptr);
	}
	return TRUE;
}



