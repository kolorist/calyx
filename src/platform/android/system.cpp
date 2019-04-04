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
#if 0
	g_subsystems.task_manager = g_allocators.subsystems_allocator.allocate<refrain2::TaskManager>();
	g_subsystems.task_manager->Initialize(2);
	g_subsystems.task_manager->StartAllTaskingThreads();
#endif

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

//----------------------------------------------
// These functions runs at the same thread with JNI thread
void android_update_surface(ANativeWindow* i_wnd)
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("lifecycle");
	if (i_wnd) {
		CLOVER_VERBOSE("Update Surface at address: 0x%x", (aptr)i_wnd);
	} else {
		CLOVER_VERBOSE("Update Surface: nullptr");
	}

	g_android_context_attribs.native_window = i_wnd;
	interact_event_t newEvent;
	newEvent.event_type = interact_event_e::window_lifecycle;
	newEvent.payload = (u32)life_cycle_event_type_e::display_update;
	s_event_buffer.push(newEvent);
}

void android_push_pause_event()
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("lifecycle");
	CLOVER_VERBOSE("Pause Event received.");

	interact_event_t newEvent;
	newEvent.event_type = interact_event_e::window_lifecycle;
	newEvent.payload = (u32)life_cycle_event_type_e::pause;
	s_event_buffer.push(newEvent);
}

void android_push_resume_event()
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("lifecycle");
	CLOVER_VERBOSE("Resume Event received.");

	interact_event_t newEvent;
	newEvent.event_type = interact_event_e::window_lifecycle;
	newEvent.payload = (u32)life_cycle_event_type_e::resume;
	s_event_buffer.push(newEvent);
}

void android_push_focus_event(bool i_hasFocus)
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("lifecycle");

	interact_event_t newEvent;
	newEvent.event_type = interact_event_e::window_lifecycle;
	if (i_hasFocus) {
		newEvent.payload = (u32)life_cycle_event_type_e::focus_gain;
		CLOVER_VERBOSE("Application gained focus.");
	} else {
		newEvent.payload = (u32)life_cycle_event_type_e::focus_lost;
		CLOVER_VERBOSE("Application lost focus.");
	}
	s_event_buffer.push(newEvent);
}

//----------------------------------------------
// These functions runs at the same thread with JNI thread
void android_push_touch_event()
{
}

void android_push_touch_move_event(const u32 i_x, const u32 i_y)
{
	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("touch: %d - %d", i_x, i_y);
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
