#include "calyx/platform/windows/system.h"

#include <helich.h>
#include <refrain2.h>

#include <clover/Logger.h>
#include <clover/SinkTopic.h>
#include <clover/VSOutputSink.h>

#include <calyx/context.h>
#include <calyx/life_cycle.h>
#include <calyx/memory.h>
#include <calyx/platform/windows/event_defs.h>

#include <Windows.h>

namespace calyx {
namespace platform {
namespace windows {

static windows_context_attribs					s_ctx_attribs;
windows_context_attribs* get_windows_context_attribs()
{
	return &s_ctx_attribs;
}

//----------------------------------------------

static bool										s_running;
static floral::thread							s_main_thread;
static event_buffer_t							s_event_buffer;

LRESULT CALLBACK window_proc_render_in_worker_thread(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
{
	LOG_TOPIC("platform_event");
	switch (i_msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:				// if we use CS_DBLCLKS window style, when double clicking, we receive: WM_LBUTTONDOWN -> WM_LBUTTONUP -> WM_LBUTTONDBLCLK -> WM_LBUTTONUP
		{
			break;
		}

		case WM_LBUTTONUP:
			{
				break;
			}

		case WM_RBUTTONDOWN:
			{
				break;
			}

		case WM_RBUTTONUP:
			{
				break;
			}

		case WM_MOUSEMOVE:
			{
				break;
			}

		case WM_MOUSEWHEEL:
			{
				break;
			}

		case WM_CHAR:
			{
				break;
			}

		case WM_KEYDOWN:
			{
				break;
			}

		case WM_KEYUP:
			{
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

		case WM_SETFOCUS:
		{
			CLOVER_DEBUG("Window gained focus");
			break;
		}

		case WM_KILLFOCUS:
		{
			CLOVER_DEBUG("Window lost focus");
			break;
		}

		case WM_SIZE:
		{
			InvalidateRgn(s_ctx_attribs.hwnd, 0, 0);
			switch (i_wparam)
			{
				case SIZE_MINIMIZED:
				{
					CLOVER_DEBUG("Window had been minimized");
					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::pause;
						s_event_buffer.push(eve);
						flush_mainthread();
					}

					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::surface_destroyed;
						s_event_buffer.push(eve);
						flush_mainthread();
					}
					break;
				}
				case SIZE_RESTORED:
				{
					CLOVER_DEBUG("Window had been restored");
					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::resume;
						s_event_buffer.push(eve);
						try_wake_mainthread();
					}

					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::surface_ready;
						s_event_buffer.push(eve);
						try_wake_mainthread();
					}
					break;
				}
			}
			break;
		}
			
		case WM_PAINT:
			{
				PAINTSTRUCT paintStruct;
				HDC hDC = BeginPaint(s_ctx_attribs.hwnd, &paintStruct);
				EndPaint(s_ctx_attribs.hwnd, &paintStruct);
				break;
			}

		default:
			{
				return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
			}

	}
	return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
}

#if 0
LRESULT CALLBACK window_proc_render_in_main_thread(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
{
	LOG_TOPIC("platform_event");
	// NOTE: please note that sizeof(WPARAM) and sizeof(LPARAM) in 32-bit system is 32-bit while in 64-bit system
	// they are both 64-bit accordingly
	switch (i_msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:				// if we use CS_DBLCLKS window style, when double clicking, we receive: WM_LBUTTONDOWN -> WM_LBUTTONUP -> WM_LBUTTONDBLCLK -> WM_LBUTTONUP
			{
				interact_event_t newEvent;
				newEvent.event_type = interact_event_e::cursor_interact;
				newEvent.payload = CLX_MOUSE_LEFT_BUTTON | CLX_MOUSE_BUTTON_PRESSED;
				s_event_buffer.push(newEvent);
				break;
			}

		case WM_LBUTTONUP:
			{
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
				//CLOVER_VERBOSE("Character received: 0x%x - ASCII: '%c'", keyCode, (c8)keyCode);
				break;
			}

		case WM_KEYDOWN:
			{
				interact_event_t newEvent;
				newEvent.event_type = interact_event_e::key_input;
				u32 keyCode = u32(i_wparam & 0x000000FF);
				u32 keyStatus = CLX_KEY;
				if (i_lparam & 0x40000000)
					keyStatus |= CLX_KEY_HELD;
				else keyStatus |= CLX_KEY_PRESSED;
				newEvent.payload = keyStatus | (keyCode << 4);
				s_event_buffer.push(newEvent);
				break;
			}

		case WM_KEYUP:
			{
				interact_event_t newEvent;
				newEvent.event_type = interact_event_e::key_input;
				u32 keyCode = u32(i_wparam & 0x000000FF);
				u32 keyStatus = CLX_KEY;
				newEvent.payload = keyStatus | (keyCode << 4);
				s_event_buffer.push(newEvent);
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
				InvalidateRgn(s_ctx_attribs.hwnd, 0, 0);
				break;
			}
			
		case WM_PAINT:
			{
				PAINTSTRUCT paintStruct;
				HDC hDC = BeginPaint(s_ctx_attribs.hwnd, &paintStruct);
				EndPaint(s_ctx_attribs.hwnd, &paintStruct);
				break;
			}

		default:
			{
				return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
			}

	}
	FLORAL_ASSERT_MSG_ONLY("Should never come here");
	return DefWindowProc(i_hwnd, i_msg, i_wparam, i_lparam);
}
#endif


void initialize()
{
	context_attribs* commonCtx = get_context_attribs();
	FLORAL_ASSERT_MSG(commonCtx->window_title != nullptr, "No title for window");
	FLORAL_ASSERT_MSG(commonCtx->window_width > 0, "Window's width must be greater than 0");
	FLORAL_ASSERT_MSG(commonCtx->window_height > 0, "Window's height must be greater than 0");

	subsystems* subSystems = get_subsystems();
	allocators_t* allocators = get_allocators();

	// init essential systems
	// helich
	helich::init_memory_system();
	clover::InitializeVSOutput("vs", clover::LogLevel::Verbose);

	// init sub-systems: generic worker threads
	subSystems->task_manager = allocators->subsystems_allocator.allocate<refrain2::TaskManager>();
	subSystems->task_manager->Initialize(2);
	subSystems->task_manager->StartAllTaskingThreads();

	// log window configs
	CLOVER_VERBOSE("Window Title: %s", commonCtx->window_title);
	CLOVER_VERBOSE("Window Position: offset (%d; %d), rect (%d; %d)",
			commonCtx->window_offset_left,
			commonCtx->window_offset_top,
			commonCtx->window_width,
			commonCtx->window_height);

	// now create the window
	HINSTANCE hInst = GetModuleHandle(0);
	WNDCLASSEX winClass;
	winClass.cbSize = sizeof(WNDCLASSEX);
	winClass.style = 0;						// we can use CS_DBLCLKS to enable double click message
	//if (commonCtx->render_in_main_thread)
		//winClass.lpfnWndProc = &window_proc_render_in_main_thread;
	//else
		winClass.lpfnWndProc = &window_proc_render_in_worker_thread;
	winClass.cbClsExtra = 0;
	winClass.cbWndExtra = 0;
	winClass.hInstance = hInst;
	winClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	winClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	winClass.hbrBackground = nullptr;
	winClass.lpszMenuName = nullptr;
	winClass.lpszClassName = TEXT("calyx");
	winClass.hIconSm = nullptr;

	RegisterClassEx(&winClass);

	RECT r;
	r.left = commonCtx->window_offset_left;
	r.top = commonCtx->window_offset_top;
	r.right = commonCtx->window_width + commonCtx->window_offset_left;
	r.bottom = commonCtx->window_height + commonCtx->window_offset_top;
	AdjustWindowRectEx(&r, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

	s_ctx_attribs.hwnd = CreateWindowEx(
			WS_EX_OVERLAPPEDWINDOW,
			TEXT("calyx"),
			TEXT(commonCtx->window_title),
			WS_OVERLAPPEDWINDOW,
			r.left, r.top, r.right - r.left, r.bottom - r.top,
			nullptr, nullptr, hInst, nullptr);
}

void run()
{
	ShowWindow(s_ctx_attribs.hwnd, SW_SHOWNORMAL);
	UpdateWindow(s_ctx_attribs.hwnd);

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

	DestroyWindow(s_ctx_attribs.hwnd);;
}

void clean_up()
{
}

}
}
}
