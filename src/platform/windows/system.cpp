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
//----------------------------------------------

static windows_context_attribs					s_ctx_attribs;
windows_context_attribs* get_windows_context_attribs()
{
	return &s_ctx_attribs;
}

//----------------------------------------------

static bool										s_running;
static floral::thread							s_main_thread;
static event_buffer_t							s_event_buffer;

//----------------------------------------------

LRESULT CALLBACK window_proc_render_in_worker_thread(HWND i_hwnd, UINT i_msg, WPARAM i_wparam, LPARAM i_lparam)
{
	switch (i_msg) {
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:				// if we use CS_DBLCLKS window style, when double clicking, we receive: WM_LBUTTONDOWN -> WM_LBUTTONUP -> WM_LBUTTONDBLCLK -> WM_LBUTTONUP
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
			eve.interact_event_data.payload = CLX_MOUSE_LEFT_BUTTON | CLX_MOUSE_BUTTON_PRESSED;
			s_event_buffer.push(eve);
			break;
		}

		case WM_LBUTTONUP:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
			eve.interact_event_data.payload = CLX_MOUSE_LEFT_BUTTON;
			s_event_buffer.push(eve);
			break;
		}

		case WM_RBUTTONDOWN:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
			eve.interact_event_data.payload = CLX_MOUSE_RIGHT_BUTTON | CLX_MOUSE_BUTTON_PRESSED;
			s_event_buffer.push(eve);
			break;
		}

		case WM_RBUTTONUP:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
			eve.interact_event_data.payload = CLX_MOUSE_RIGHT_BUTTON;
			s_event_buffer.push(eve);
			break;
		}

		case WM_MOUSEMOVE:
		{
			u32 x = (u32)(i_lparam & 0xFFFF);
			u32 y = (u32)(i_lparam & 0xFFFF0000);
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::cursor_move;
			eve.interact_event_data.payload = x | y;
			s_event_buffer.push(eve);
			break;
		}

		case WM_MOUSEWHEEL:
		{
			break;
		}

		case WM_CHAR:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::character_input;
			u32 keyCode = u32(i_wparam & 0x000000FF);
			eve.interact_event_data.payload = keyCode;
			s_event_buffer.push(eve);
			break;
		}

		case WM_KEYDOWN:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::key_input;
			u32 keyCode = u32(i_wparam & 0x000000FF);
			u32 keyStatus = CLX_KEY;
			if (i_lparam & 0x40000000)
				keyStatus |= CLX_KEY_HELD;
			else keyStatus |= CLX_KEY_PRESSED;
			eve.interact_event_data.payload = keyStatus | (keyCode << 4);
			s_event_buffer.push(eve);
			break;
		}

		case WM_KEYUP:
		{
			event_t eve;
			eve.type = event_type_e::interact;
			eve.interact_event_data.inner_type = interact_event_e::key_input;
			u32 keyCode = u32(i_wparam & 0x000000FF);
			u32 keyStatus = CLX_KEY;
			eve.interact_event_data.payload = keyStatus | (keyCode << 4);
			s_event_buffer.push(eve);
			break;
		}

		case WM_DESTROY:
			{
				LOG_TOPIC("platform_event");
				CLOVER_VERBOSE("Destroying window...");

				event_t eve;
				eve.type = event_type_e::lifecycle;
				eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::stop;
				s_event_buffer.push(eve);
				flush_mainthread();

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
			LOG_TOPIC("platform_event");
			CLOVER_VERBOSE("Window gained focus");
			break;
		}

		case WM_KILLFOCUS:
		{
			LOG_TOPIC("platform_event");
			CLOVER_VERBOSE("Window lost focus");
			break;
		}

		case WM_SIZE:
		{
			InvalidateRgn(s_ctx_attribs.hwnd, 0, 0);
			switch (i_wparam)
			{
				case SIZE_MINIMIZED:
				{
					LOG_TOPIC("platform_event");
					CLOVER_VERBOSE("Window had been minimized");
					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::pause;
						s_event_buffer.push(eve);
						flush_mainthread();
					}

					CLOVER_VERBOSE("GL Surface will be destroyed");
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
					LOG_TOPIC("platform_event");
					CLOVER_VERBOSE("Window had been restored");
					{
						event_t eve;
						eve.type = event_type_e::lifecycle;
						eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::resume;
						s_event_buffer.push(eve);
						try_wake_mainthread();
					}

					CLOVER_VERBOSE("GL Surface is now ready");
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

void initialize()
{
	context_attribs* commonCtx = get_context_attribs();
	FLORAL_ASSERT_MSG(commonCtx->window_title != nullptr, "No title for window");
	FLORAL_ASSERT_MSG(commonCtx->window_width > 0, "Window's width must be greater than 0");
	FLORAL_ASSERT_MSG(commonCtx->window_height > 0, "Window's height must be greater than 0");

	subsystems* subSystems = get_subsystems();
	allocators_t* allocators = get_allocators();
	system_info_t* systemInfo = get_system_info();

	// init information structs
	SYSTEM_INFO nativeSystemInfo;
	GetSystemInfo(&nativeSystemInfo);
	systemInfo->page_size = (size)nativeSystemInfo.dwPageSize;
	systemInfo->num_logical_processors = (u32)nativeSystemInfo.dwNumberOfProcessors;
	systemInfo->primary_screen_width = (u32)GetSystemMetrics(SM_CXSCREEN);
	systemInfo->primary_screen_height = (u32)GetSystemMetrics(SM_CYSCREEN);

	// init essential systems
	// helich
	helich::init_memory_system();
	CLOVER_INIT_THIS_THREAD("platform_thread", clover::LogLevel::Verbose);

	LOG_TOPIC("calyx");
	CLOVER_VERBOSE("Initializing essential systems...");

	// init sub-systems: generic worker threads
	subSystems->task_manager = allocators->subsystems_allocator.allocate<refrain2::TaskManager>();
	subSystems->task_manager->Initialize(2);
	subSystems->task_manager->StartAllTaskingThreads();
	CLOVER_VERBOSE("refrain started");

	// log system info
	CLOVER_INFO("system_info.page_size: %lld bytes", systemInfo->page_size);
	CLOVER_INFO("system_info.num_logical_processors: %d", systemInfo->num_logical_processors);
	CLOVER_INFO("system_info.primary_screen_width: %d", systemInfo->primary_screen_width);
	CLOVER_INFO("system_info.primary_screen_height: %d", systemInfo->primary_screen_height);

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
	CLOVER_VERBOSE("Window created");
}

void run()
{
	LOG_TOPIC("calyx");
	CLOVER_VERBOSE("Starting window...");

	ShowWindow(s_ctx_attribs.hwnd, SW_SHOWNORMAL);
	UpdateWindow(s_ctx_attribs.hwnd);

	// kick off s_main_thread
	s_main_thread.entry_point = &calyx::main_thread_func;
	s_main_thread.ptr_data = &s_event_buffer;
	s_main_thread.start();

	CLOVER_VERBOSE("Kicked off main_thread");

	MSG msg;
	s_running = true;
	CLOVER_VERBOSE("Entering event loop...");
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

	floral::wait_for_thread(s_main_thread);

	DestroyWindow(s_ctx_attribs.hwnd);;
}

void clean_up()
{
}

}
}
}
