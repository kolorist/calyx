#include "platform/android/system.h"

#include <helich.h>
#include <refrain2.h>

#include <Logger.h>
#include <SinkTopic.h>
#include <ADBOutputSink.h>

#include <TaskManager.h>

#include "context.h"
#include "life_cycle.h"

namespace calyx {

android_context_attribs							g_android_context_attribs;

namespace platform {
namespace android {

static bool										s_running;

static floral::thread							s_main_thread;
static event_buffer_t							s_event_buffer;

void initialize()
{
	// init essential systems
	// helich
	helich::init_memory_system();
	clover::InitializeADBOutput("adb", clover::LogLevel::Verbose);

	// init sub-systems
	g_subsystems.task_manager = g_allocators.subsystems_allocator.allocate<refrain2::TaskManager>();
	g_subsystems.task_manager->Initialize(2);
	g_subsystems.task_manager->StartAllTaskingThreads();

	// log window configs
	CLOVER_VERBOSE("Window Title: %s", g_android_context_attribs.window_title);
	CLOVER_VERBOSE("Window Position: offset (%d; %d), rect (%d; %d)",
			g_android_context_attribs.window_offset_left,
			g_android_context_attribs.window_offset_top,
			g_android_context_attribs.window_width,
			g_android_context_attribs.window_height);

	// init global variables
	g_context_attribs = static_cast<context_attribs*>(&g_android_context_attribs);
}

void run()
{
	// setup event buffer
	s_event_buffer.assign_allocator(&g_allocators.subsystems_allocator);

	// kick off s_main_thread
	s_main_thread.entry_point = &calyx::main_thread_func;
	s_main_thread.ptr_data = &s_event_buffer;
	s_main_thread.start();
}

void clean_up()
{
}

}
}
}

void android_update_surface(ANativeWindow* i_wnd)
{
	LOG_TOPIC("platform_event");
	using namespace calyx;
	g_android_context_attribs.native_window = i_wnd;
	CLOVER_INFO("surface updated");
}

void android_push_touch_event()
{
}

void android_push_touch_move_event(const u32 i_x, const u32 i_y)
{
	LOG_TOPIC("platform_event");
	using namespace calyx::platform::android;
	calyx::interact_event_t newEvent;
	newEvent.event_type = calyx::interact_event_e::cursor_move;
	newEvent.payload = i_x | (i_y << 16);
	s_event_buffer.push(newEvent);
}

void android_push_key_event()
{
	LOG_TOPIC("platform_event");
	using namespace calyx::platform::android;
	CLOVER_VERBOSE("key");
}
