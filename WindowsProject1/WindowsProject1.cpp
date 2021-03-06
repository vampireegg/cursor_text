// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WindowsProject1.h"

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <ole2.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <crtdbg.h>
#include <stdarg.h>
#include <stdio.h>
#include <cstring>
#include <iostream>

using namespace std;

HINSTANCE g_hinst;                          /* This application's HINSTANCE */
HWND g_hwndChild;                           /* Optional child window */

#include <oleacc.h>

POINT g_pt;
LPTSTR g_pszText;

#define printg(X,...) _RPT1(0,X,__VA_ARGS__)
#define printf(...) printg(__VA_ARGS__,"")

void printBSTR(BSTR bs, string s)
{
	if (bs)
	{
		wcout << "found " << s.c_str() << ":" << bs << "\n";
	}
	else
	{
		cout << "no" << s.c_str() << "\n";
	}
}

void CALLBACK RecalcText(HWND hwnd, UINT, UINT_PTR, DWORD)
{
	POINT pt;
	if (GetCursorPos(&pt) &&
		(pt.x != g_pt.x || pt.y != g_pt.y)) {
		g_pt = pt;
		IAccessible *pacc;
		VARIANT vtChild;
		if (SUCCEEDED(AccessibleObjectFromPoint(pt, &pacc, &vtChild))) 
		{
			//printf("Got no error\n");
			BSTR bsName = NULL;
			BSTR bsValue = NULL;
			BSTR bsDEsc = NULL;
			BSTR bsAction = NULL;
			printf("Name = %s\n", bsName);
			printf("Value = %s\n", bsValue);
			pacc->get_accName(vtChild, &bsName);
			pacc->get_accValue(vtChild, &bsValue);
			pacc->get_accDescription(vtChild, &bsDEsc);
			pacc->get_accDefaultAction(vtChild, &bsAction);

			LPTSTR pszResult;
			DWORD_PTR args[2] = { (DWORD_PTR)(4 ? bsName : L""),
				(DWORD_PTR)(bsValue ? bsValue : L"") };
			if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_STRING |
				FORMAT_MESSAGE_ARGUMENT_ARRAY,
				TEXT("Name: %1!ws!\r\n\r\nValue: %2!ws!"),
				0, 0, (LPTSTR)&pszResult, 0, (va_list*)args)) 
			{
				//char cACharArray[1000] = { 0 };
				//sprintf(cACharArray, "%s", *pszResult);
				//printf("Name2 = %s\n", cACharArray);
				//cout << "abc\n";
				/*for (auto const &name : pszResult)
				{
					std::wcout << name << std::endl;
				}*/
				LocalFree(g_pszText);
				g_pszText = pszResult;
				wcout << pszResult << endl;

				printBSTR(bsName, "name");
				printBSTR(bsValue, "value");
				printBSTR(bsDEsc, "Desc");
				printBSTR(bsAction, "Action");
			
				
				InvalidateRect(hwnd, NULL, TRUE);
			}

			SysFreeString(bsName);
			SysFreeString(bsValue);
			VariantClear(&vtChild);
			pacc->Release();
		}
		else
		{
			printf("Got an error\n");
		}
	}
}
											
											/*
											*  OnSize
											*      If we have an inner child, resize it to fit.
											*/
void
OnSize(HWND hwnd, UINT state, int cx, int cy)
{
	if (g_hwndChild) {
		MoveWindow(g_hwndChild, 0, 0, cx, cy, TRUE);
	}
}

/*
*  OnCreate
*      Applications will typically override this and maybe even
*      create a child window.
*/
BOOL
OnCreate(HWND hwnd, LPCREATESTRUCT lpcs)
{
	SetTimer(hwnd, 1, 1000, RecalcText);
	return TRUE;
}

/*
*  OnDestroy
*      Post a quit message because our application is over when the
*      user closes this window.
*/
void
OnDestroy(HWND hwnd)
{
	PostQuitMessage(0);
}

/*
*  PaintContent
*      Interesting things will be painted here eventually.
*/
void
PaintContent(HWND hwnd, PAINTSTRUCT *pps)
{
	if (g_pszText) {
		RECT rc;
		GetClientRect(hwnd, &rc);
		DrawText(pps->hdc, g_pszText, lstrlen(g_pszText),
			&rc, DT_NOPREFIX | DT_WORDBREAK);
	}
}

/*
*  OnPaint
*      Paint the content as part of the paint cycle.
*/
void
OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	PaintContent(hwnd, &ps);
	EndPaint(hwnd, &ps);
}

/*
*  OnPrintClient
*      Paint the content as requested by USER.
*/
void
OnPrintClient(HWND hwnd, HDC hdc)
{
	PAINTSTRUCT ps;
	ps.hdc = hdc;
	GetClientRect(hwnd, &ps.rcPaint);
	PaintContent(hwnd, &ps);

}

/*
*  Window procedure
*/
LRESULT CALLBACK
WndProc(HWND hwnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg) {

		HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hwnd, WM_SIZE, OnSize);
		HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
	case WM_PRINTCLIENT: OnPrintClient(hwnd, (HDC)wParam); return 0;
	}

	return DefWindowProc(hwnd, uiMsg, wParam, lParam);
}

BOOL
InitApp(void)
{
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hinst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("Scratch");

	if (!RegisterClass(&wc)) return FALSE;

	InitCommonControls();               /* In case we use a common control */

	return TRUE;
}

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hinstPrev,
	LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	HWND hwnd;

	g_hinst = hinst;

	if (!InitApp()) return 0;
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	if (SUCCEEDED(CoInitialize(NULL))) {/* In case we use COM */

		hwnd = CreateWindow(
			TEXT("Scratch"),                /* Class Name */
			TEXT("Scratch"),                /* Title */
			WS_OVERLAPPEDWINDOW,            /* Style */
			CW_USEDEFAULT, CW_USEDEFAULT,   /* Position */
			CW_USEDEFAULT, CW_USEDEFAULT,   /* Size */
			NULL,                           /* Parent */
			NULL,                           /* No menu */
			hinst,                          /* Instance */
			0);                             /* No special parameters */

		ShowWindow(hwnd, nShowCmd);

		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		CoUninitialize();
	}

	return 0;
}
