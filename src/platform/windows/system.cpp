#include "platform/windows/system.h"

#include <stdaliases.h>

#include <Allocator.h>
#include <MemoryManager.h>

#include <TaskManager.h>

#include <Windows.h>

#include "context.h"

namespace calyx {
namespace platform {
namespace windows {

	static HWND									s_hwnd;
	static bool									s_running;

	LRESULT CALLBACK window_proc(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
	{
		switch (i_msg) {
			case WM_DESTROY:
				{
					PostQuitMessage(0);
					s_running = false;
					break;
				}
			case WM_CREATE:
				{
					break;
				}
			case WM_SIZE:
				{
					InvalidateRgn(s_hwnd, 0, 0);
					break;
				}
			case WM_PAINT:
				{
					PAINTSTRUCT paintStruct;
					HDC hDC = BeginPaint(s_hwnd, &paintStruct);
					EndPaint(s_hwnd, &paintStruct);
					break;
				}
			default:
				{
					return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
				}
		}
		return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
	}


	void initialize()
	{
		// init essential systems
		// helich
		helich::init_memory_system();

		// init sub-systems
		g_subsystems.task_manager = g_allocators.subsystems_allocator.allocate<refrain2::TaskManager>();
		g_subsystems.task_manager->Initialize(2);
		g_subsystems.task_manager->StartAllTaskingThreads();

		// now create the window
		HINSTANCE hInst = GetModuleHandle(0);
		WNDCLASSEX winClass;
		winClass.cbSize = sizeof(WNDCLASSEX);
		winClass.style = CS_DBLCLKS;
		winClass.lpfnWndProc = &window_proc;
		winClass.cbClsExtra = 0;
		winClass.cbWndExtra = 0;
		winClass.hInstance = hInst;
		winClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
		winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		winClass.hbrBackground = nullptr;
		winClass.lpszMenuName = nullptr;
		winClass.lpszClassName = TEXT("Amborella");
		winClass.hIconSm = nullptr;

		RegisterClassEx(&winClass);

		RECT r;
		r.left = g_context_attribs.window_offset_left;
		r.top = g_context_attribs.window_offset_top;
		r.right = g_context_attribs.window_width + g_context_attribs.window_offset_left;
		r.bottom = g_context_attribs.window_height + g_context_attribs.window_offset_top;
		AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

		s_hwnd = CreateWindowEx(
				WS_EX_OVERLAPPEDWINDOW,
				TEXT("Amborella"),
				TEXT(g_context_attribs.window_title),
				WS_OVERLAPPEDWINDOW,
				r.left, r.top, r.right - r.left, r.bottom - r.top,
				nullptr, nullptr, hInst, nullptr);
	}

	void run()
	{
		ShowWindow(s_hwnd, SW_SHOWNORMAL);
		UpdateWindow(s_hwnd);

		MSG msg;
		s_running = true;
		while (s_running) {
			// platform events
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// note: 
			//	when single buffered: the SwapBuffers does not wait for VSync regardless of SwapIntervalEXT(any_value);
			//	when double buffered: it works normally with VSync enabled

			// when VSync failed, we have the famous DwmFlush() hack to achieve it
			// DwmFlush();
			//SwapBuffers(oglRenderer.OglDC);
		}

		DestroyWindow(s_hwnd);;
	}

	void clean_up()
	{
	}

}
}
}
