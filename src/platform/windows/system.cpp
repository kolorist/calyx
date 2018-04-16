#include "platform/windows/system.h"

#include <stdaliases.h>

#include <Allocator.h>
#include <MemoryManager.h>

#include <Logger.h>
#include <SinkTopic.h>
#include <VSOutputSink.h>

#include <TaskManager.h>

#include <Windows.h>

#include "context.h"
#include "life_cycle.h"

namespace calyx {
namespace platform {
namespace windows {

	static HWND									s_hwnd;
	static bool									s_running;

	floral::thread								s_main_thread;
	event_buffer_t								s_event_buffer;

	LRESULT CALLBACK window_proc(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
	{
		LOG_TOPIC("platform_event");
		// NOTE: please note that sizeof(WPARAM) and sizeof(LPARAM) in 32-bit system is 32-bit while in 64-bit system
		// they are both 64-bit accordingly
		switch (i_msg) {
			case WM_LBUTTONDOWN:
				{
					CLOVER_VERBOSE("Mouse left: DOWN");
					s_event_buffer.push(1);
					break;
				}
			case WM_LBUTTONUP:
				{
					CLOVER_VERBOSE("Mouse left: UP");
					s_event_buffer.push(2);
					break;
				}
			case WM_RBUTTONDOWN:
				{
					CLOVER_VERBOSE("Mouse right: DOWN");
					s_event_buffer.push(3);
					break;
				}
			case WM_RBUTTONUP:
				{
					CLOVER_VERBOSE("Mouse right: UP");
					s_event_buffer.push(4);
					break;
				}
			case WM_MOUSEMOVE:
				{
					u32 x = (u32)(i_lparam & 0xFFFF);
					u32 y = (u32)((i_lparam & 0xFFFF0000) >> 16);
					// NOTE: log spamming!!!
					CLOVER_VERBOSE("Mouse move: (%d; %d)", x, y);
					s_event_buffer.push(6);
					break;
				}
			case WM_MOUSEWHEEL:
				{
					// NOTE: signed-int bit shifting is implementation-dependant
					//s32 delta = (s32)((i_wparam & 0xFFFF0000) >> 16);
					s32 delta = (s32)(i_wparam & 0xFFFF0000) / 65536;
					f32 deltaF = (f32)delta / 120.0f;
					CLOVER_VERBOSE("Mouse wheel delta: %4.2f", deltaF);
					break;
				}
			case WM_CHAR:
				{
					u32 keyCode = (u32)i_wparam;
					CLOVER_VERBOSE("Character received: 0x%x - ASCII: '%c'", keyCode, (c8)keyCode);
					s_event_buffer.push(5);
					break;
				}
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
		clover::InitializeVSOutput("vs", clover::LogLevel::Verbose);

		// init sub-systems
		g_subsystems.task_manager = g_allocators.subsystems_allocator.allocate<refrain2::TaskManager>();
		g_subsystems.task_manager->Initialize(2);
		g_subsystems.task_manager->StartAllTaskingThreads();

		// log window configs
		CLOVER_VERBOSE("Window Title: %s", g_context_attribs.window_title);
		CLOVER_VERBOSE("Window Position: offset (%d; %d), rect (%d; %d)",
				g_context_attribs.window_offset_left,
				g_context_attribs.window_offset_top,
				g_context_attribs.window_width,
				g_context_attribs.window_height);

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

		// setup event buffer
		s_event_buffer.assign_allocator(&g_allocators.subsystems_allocator);

		// kick off s_main_thread
		s_main_thread.entry_point = &calyx::main_thread_func;
		s_main_thread.ptr_data = &s_event_buffer;
		s_main_thread.start();

		MSG msg;
		s_running = true;
		while (s_running) {
			// platform events
			//if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (GetMessage(&msg, nullptr, 0, 0)) {
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
