#include "platform/windows/system.h"

#include <helich.h>
#include <refrain2.h>

#include <Logger.h>
#include <SinkTopic.h>
#include <VSOutputSink.h>

#include "life_cycle.h"
#include "platform/windows/event_defs.h"

#include <Windows.h>

namespace calyx {

	windows_context_attribs						g_windows_context_attribs;

namespace platform {
namespace windows {

	static HWND									s_hwnd;
	static bool									s_running;

	static floral::thread						s_main_thread;
	static event_buffer_t						s_event_buffer;

	LRESULT CALLBACK window_proc(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
	{
		LOG_TOPIC("platform_event");
		// NOTE: please note that sizeof(WPARAM) and sizeof(LPARAM) in 32-bit system is 32-bit while in 64-bit system
		// they are both 64-bit accordingly
		switch (i_msg) {
			case WM_LBUTTONDOWN:
			case WM_LBUTTONDBLCLK:				// if we use CS_DBLCLKS window style, when double clicking, we receive: WM_LBUTTONDOWN -> WM_LBUTTONUP -> WM_LBUTTONDBLCLK -> WM_LBUTTONUP
				{
					CLOVER_VERBOSE("left down\n");
					interact_event_t newEvent;
					newEvent.event_type = interact_event_e::cursor_interact;
					newEvent.payload = CLX_MOUSE_LEFT_BUTTON | CLX_MOUSE_BUTTON_PRESSED;
					s_event_buffer.push(newEvent);
					break;
				}

			case WM_LBUTTONUP:
				{
					CLOVER_VERBOSE("left up\n");
					interact_event_t newEvent;
					newEvent.event_type = interact_event_e::cursor_interact;
					newEvent.payload = CLX_MOUSE_LEFT_BUTTON;
					s_event_buffer.push(newEvent);
					break;
				}

			case WM_RBUTTONDOWN:
				{
					interact_event_t newEvent;
					newEvent.event_type = interact_event_e::cursor_interact;
					newEvent.payload = CLX_MOUSE_RIGHT_BUTTON | CLX_MOUSE_BUTTON_PRESSED;
					s_event_buffer.push(newEvent);
					break;
				}

			case WM_RBUTTONUP:
				{
					interact_event_t newEvent;
					newEvent.event_type = interact_event_e::cursor_interact;
					newEvent.payload = CLX_MOUSE_RIGHT_BUTTON;
					s_event_buffer.push(newEvent);
					break;
				}

			case WM_MOUSEMOVE:
				{
					u32 x = (u32)(i_lparam & 0xFFFF);
					u32 y = (u32)(i_lparam & 0xFFFF0000);
					interact_event_t newEvent;
					newEvent.event_type = interact_event_e::cursor_move;
					newEvent.payload = x | y;
					s_event_buffer.push(newEvent);
					break;
				}

			case WM_MOUSEWHEEL:
				{
					// NOTE: signed-int bit shifting is implementation-dependant
					//s32 delta = (s32)((i_wparam & 0xFFFF0000) >> 16);
					s32 delta = (s32)(i_wparam & 0xFFFF0000) / 65536;
					f32 deltaF = (f32)delta / 120.0f;
					break;
				}

			case WM_CHAR:
				{
					u32 keyCode = (u32)i_wparam;
					CLOVER_VERBOSE("Character received: 0x%x - ASCII: '%c'", keyCode, (c8)keyCode);
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

		// init sub-systems: generic worker threads
		g_subsystems.task_manager = g_allocators.subsystems_allocator.allocate<refrain2::TaskManager>();
		g_subsystems.task_manager->Initialize(2);
		g_subsystems.task_manager->StartAllTaskingThreads();

		// log window configs
		CLOVER_VERBOSE("Window Title: %s", g_windows_context_attribs.window_title);
		CLOVER_VERBOSE("Window Position: offset (%d; %d), rect (%d; %d)",
				g_windows_context_attribs.window_offset_left,
				g_windows_context_attribs.window_offset_top,
				g_windows_context_attribs.window_width,
				g_windows_context_attribs.window_height);

		// now create the window
		HINSTANCE hInst = GetModuleHandle(0);
		WNDCLASSEX winClass;
		winClass.cbSize = sizeof(WNDCLASSEX);
		winClass.style = 0;						// we can use CS_DBLCLKS to enable double click message
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
		r.left = g_windows_context_attribs.window_offset_left;
		r.top = g_windows_context_attribs.window_offset_top;
		r.right = g_windows_context_attribs.window_width + g_windows_context_attribs.window_offset_left;
		r.bottom = g_windows_context_attribs.window_height + g_windows_context_attribs.window_offset_top;
		AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

		s_hwnd = CreateWindowEx(
				WS_EX_OVERLAPPEDWINDOW,
				TEXT("Amborella"),
				TEXT(g_windows_context_attribs.window_title),
				WS_OVERLAPPEDWINDOW,
				r.left, r.top, r.right - r.left, r.bottom - r.top,
				nullptr, nullptr, hInst, nullptr);

		// init global variables
		g_windows_context_attribs.hwnd = s_hwnd;
		g_context_attribs = static_cast<context_attribs*>(&g_windows_context_attribs);
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
