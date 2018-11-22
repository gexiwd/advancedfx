#include "stdafx.h"

#include <windows.h>

#include <shared/Detours/src/detours.h>

#include <d3d11.h>

#include <tchar.h>

LONG error = NO_ERROR;
HMODULE hD3d11Dll = NULL;

bool g_FbOverride = false;
bool g_FbDepthOverride = false;

HANDLE g_hFbSurface;
HANDLE g_hFbDepthSurface;


typedef ULONG (STDMETHODCALLTYPE * AddReff_t)(ID3D11Device * This);
typedef ULONG (STDMETHODCALLTYPE * Release_t)(ID3D11Device * This);

ID3D11Device *pDevice = NULL;
ID3D11Query * pQuery = NULL;

ULONG g_RefCOunt = 1;
AddReff_t True_AddRef;
Release_t True_Release;

ULONG STDMETHODCALLTYPE My_AddRef(ID3D11Device * This)
{
	g_RefCOunt = True_AddRef(This);

	return g_RefCOunt;
}

ULONG STDMETHODCALLTYPE My_Release(ID3D11Device * This)
{
	if (1 == g_RefCOunt)
	{
		if (pQuery)
		{
			pQuery->Release();
			pQuery = NULL;
		}

		pDevice = NULL;
	}

	g_RefCOunt = True_Release(This);

	return g_RefCOunt;
}


typedef HRESULT (STDMETHODCALLTYPE * CreateRenderTargetView_t)(
	ID3D11Device * This,
	/* [annotation] */
	_In_  ID3D11Resource *pResource,
	/* [annotation] */
	_In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
	/* [annotation] */
	_COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView);

CreateRenderTargetView_t True_CreateRenderTargetView = NULL;

HRESULT STDMETHODCALLTYPE My_CreateRenderTargetView(
	ID3D11Device * This,
	/* [annotation] */
	_In_  ID3D11Resource *pResource,
	/* [annotation] */
	_In_opt_  const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
	/* [annotation] */
	_COM_Outptr_opt_  ID3D11RenderTargetView **ppRTView)
{
	ID3D11Resource * resource = nullptr;
	HRESULT hr = NULL;

	if(g_FbOverride)
	{
		hr = This->OpenSharedResource(g_hFbSurface, __uuidof(ID3D11Resource), (void **)&resource);
		if (!SUCCEEDED(hr))		
		{
			MessageBox(NULL, _T("My_CreateDepthStencilView OpenSharedResource g_hFbSurface failed."), _T("AfxHookUnity"), MB_OK | MB_ICONERROR);
			return hr;
		}		
	}

	return True_CreateRenderTargetView(
		This,
		resource ? resource : pResource,
		resource ? NULL: pDesc,
		ppRTView
	);
}

typedef HRESULT (STDMETHODCALLTYPE * CreateDepthStencilView_t)(
	ID3D11Device * This,
	/* [annotation] */
	_In_  ID3D11Resource *pResource,
	/* [annotation] */
	_In_opt_  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
	/* [annotation] */
	_COM_Outptr_opt_  ID3D11DepthStencilView **ppDepthStencilView);

CreateDepthStencilView_t True_CreateDepthStencilView;

HRESULT STDMETHODCALLTYPE My_CreateDepthStencilView(
	ID3D11Device * This,
	/* [annotation] */
	_In_  ID3D11Resource *pResource,
	/* [annotation] */
	_In_opt_  const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
	/* [annotation] */
	_COM_Outptr_opt_  ID3D11DepthStencilView **ppDepthStencilView)
{
	ID3D11Resource * resource = nullptr;
	HRESULT hr = NULL;

	if (g_FbDepthOverride)
	{
		/*
		TCHAR buf[33] = { 0 };

		_stprintf_s(buf, _T("%i"), pDesc->Format);

		MessageBox(NULL, buf, _T("INFO"), MB_OK);

		if (!pDesc)
		{
			MessageBox(NULL, _T("My_CreateDepthStencilView *pDesc is NULL."), _T("AfxHookUnity"), MB_OK | MB_ICONERROR);
			return hr;
		}

		const_cast<D3D11_DEPTH_STENCIL_VIEW_DESC *>(pDesc)->Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		*/

		hr = This->OpenSharedResource(g_hFbDepthSurface, __uuidof(ID3D11Resource), (void **)&resource);
		if (!SUCCEEDED(hr))
		{
			MessageBox(NULL, _T("My_CreateDepthStencilView OpenSharedResource g_hFbDeptSurface failed."), _T("AfxHookUnity"), MB_OK | MB_ICONERROR);
			return hr;
		}

		return True_CreateDepthStencilView(
			This,
			resource ? resource : pResource,
			resource ? NULL : pDesc,
			ppDepthStencilView
		);
	}

	return True_CreateDepthStencilView(
		This,
		resource ? resource : pResource,
		resource ? NULL : pDesc,
		ppDepthStencilView
	);
}


typedef HRESULT(WINAPI * D3D11CreateDevice_t)(
	_In_opt_        IDXGIAdapter        *pAdapter,
	D3D_DRIVER_TYPE     DriverType,
	HMODULE             Software,
	UINT                Flags,
	_In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
	UINT                FeatureLevels,
	UINT                SDKVersion,
	_Out_opt_       ID3D11Device        **ppDevice,
	_Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
	_Out_opt_       ID3D11DeviceContext **ppImmediateContext
);

D3D11CreateDevice_t TrueD3D11CreateDevice = NULL;

HRESULT WINAPI MyD3D11CreateDevice(
	_In_opt_        IDXGIAdapter        *pAdapter,
	D3D_DRIVER_TYPE     DriverType,
	HMODULE             Software,
	UINT                Flags,
	_In_opt_  const D3D_FEATURE_LEVEL   *pFeatureLevels,
	UINT                FeatureLevels,
	UINT                SDKVersion,
	_Out_opt_       ID3D11Device        **ppDevice,
	_Out_opt_       D3D_FEATURE_LEVEL   *pFeatureLevel,
	_Out_opt_       ID3D11DeviceContext **ppImmediateContext
) {
	HRESULT result = TrueD3D11CreateDevice(pAdapter, DriverType, Software, Flags, pFeatureLevels, FeatureLevels, SDKVersion, ppDevice, pFeatureLevel, ppImmediateContext);

	if (IS_ERROR(result) || NULL == ppDevice)
	{
		error = E_FAIL;
		return result;
	}

	if (NULL == True_CreateRenderTargetView)
	{
		True_AddRef = (AddReff_t)*(void **)((*(char **)(*ppDevice)) + sizeof(void *) * 1);
		True_Release = (Release_t)*(void **)((*(char **)(*ppDevice)) + sizeof(void *) * 2);
		True_CreateRenderTargetView = (CreateRenderTargetView_t)*(void **)((*(char **)(*ppDevice)) + sizeof(void *) * 9);
		True_CreateDepthStencilView = (CreateDepthStencilView_t)*(void **)((*(char **)(*ppDevice)) + sizeof(void *) * 10);

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)True_AddRef, My_AddRef);
		DetourAttach(&(PVOID&)True_Release, My_Release);
		DetourAttach(&(PVOID&)True_CreateRenderTargetView, My_CreateRenderTargetView);
		DetourAttach(&(PVOID&)True_CreateDepthStencilView, My_CreateDepthStencilView);
		error = DetourTransactionCommit();
	}

	if (SUCCEEDED(error))
	{
		D3D11_QUERY_DESC queryDesc = {
			D3D11_QUERY_EVENT, 0
		};
		
		if (FAILED((*ppDevice)->CreateQuery(&queryDesc, &pQuery)))
		{
			pQuery = NULL;
		}
	}

	return result;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		{
			hD3d11Dll = LoadLibrary(_T("d3d11.dll"));

			TrueD3D11CreateDevice = (D3D11CreateDevice_t)GetProcAddress(hD3d11Dll,"D3D11CreateDevice");

			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
			error = DetourTransactionCommit();

		}
		break;
    case DLL_THREAD_ATTACH:
		break;
    case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach(&(PVOID&)TrueD3D11CreateDevice, MyD3D11CreateDevice);
			error = DetourTransactionCommit();

			FreeLibrary(hD3d11Dll);
		}
        break;
    }
    return TRUE;
}

extern "C" __declspec(dllexport) bool AfxHookUnityStatus(void) {
	return NO_ERROR == error;
}


extern "C" __declspec(dllexport) void AfxHookUnityBeginCreateRenderTexture(HANDLE fbSharedHandle, HANDLE fbDepthSharedHandle)
{
	g_hFbSurface = fbSharedHandle;
	g_hFbDepthSurface = fbDepthSharedHandle;

	g_FbOverride = true;
	g_FbDepthOverride = true;
}

extern "C" __declspec(dllexport) void AfxHookUnityEndCreateRenderTexture()
{
	g_FbDepthOverride = false;
	g_FbOverride = false;
}

extern "C" __declspec(dllexport) void AfxHookUnityWaitForGPU()
{
	ID3D11DeviceContext * pImmediateContext = nullptr;

	if (pDevice && pQuery && (pDevice->GetImmediateContext(&pImmediateContext), pImmediateContext))
	{
		pImmediateContext->End(pQuery);

		while (S_FALSE == pImmediateContext->GetData(pQuery, NULL, 0, 0))
			;
	}
}
