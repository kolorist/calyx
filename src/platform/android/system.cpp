#include "calyx/platform/android/system.h"

#include <helich.h>
#include <refrain2.h>

#include <clover/Logger.h>
#include <clover/SinkTopic.h>
#include <clover/ADBOutputSink.h>

#include <TaskManager.h>

#include <calyx/context.h>
#include <calyx/life_cycle.h>
#include <calyx/memory.h>
#include <calyx/platform/android/event_defs.h>

#include <unistd.h>

namespace calyx {
namespace platform {
namespace android {

static android_context_attribs					s_ctx_attribs;
android_context_attribs* get_android_context_attribs()
{
	return &s_ctx_attribs;
}

//----------------------------------------------

static bool										s_running;
static floral::thread							s_main_thread;
static event_buffer_t							s_event_buffer;

void initialize()
{
	context_attribs* commonCtx = get_context_attribs();
	subsystems* subSystems = get_subsystems();
	allocators_t* allocators = get_allocators();
	system_info_t* systemInfo = get_system_info();

	// init information structs
	systemInfo->page_size = (size)sysconf(_SC_PAGESIZE);
	systemInfo->num_logical_processors = (u32)sysconf(_SC_NPROCESSORS_ONLN);

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
	CLOVER_INFO("system_info.primary_screen_width: <waiting for native surface>");
	CLOVER_INFO("system_info.primary_screen_height: <waiting for native surface>");

	// log window configs
	CLOVER_VERBOSE("Window Title: %s", commonCtx->window_title);
	CLOVER_VERBOSE("Window Position: offset (%d; %d), rect (%d; %d)",
			commonCtx->window_offset_left,
			commonCtx->window_offset_top,
			commonCtx->window_width,
			commonCtx->window_height);
}

void run()
{
	LOG_TOPIC("calyx");

	// kick off s_main_thread
	s_main_thread.entry_point = &calyx::main_thread_func;
	s_main_thread.ptr_data = &s_event_buffer;
	s_main_thread.start();
	CLOVER_VERBOSE("Kicked off main_thread");
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

	LOG_TOPIC("platform_event");
	if (i_wnd) {
		CLOVER_VERBOSE("Update Surface at address: 0x%x", (aptr)i_wnd);
	} else {
		CLOVER_VERBOSE("Update Surface: nullptr");
	}
}

void android_will_destroy_surface(ANativeWindow* i_wnd)
{
	using namespace calyx;
	using namespace calyx::platform::android;
	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("SurfaceDestroyed received.");

	// TODO: mutex this!!!
	android_context_attribs* androidCtx = get_android_context_attribs();
	androidCtx->native_window = nullptr;

	event_t eve;
	eve.type = event_type_e::lifecycle;
	eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::surface_destroyed;
	s_event_buffer.push(eve);
	flush_mainthread();
}

void android_created_surface(ANativeWindow* i_wnd)
{
	using namespace calyx;
	using namespace calyx::platform::android;
	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("SurfaceCreated received.");

	// TODO: mutex this!!!
	android_context_attribs* androidCtx = get_android_context_attribs();
	androidCtx->native_window = i_wnd;

	event_t eve;
	eve.type = event_type_e::lifecycle;
	eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::surface_ready;
	s_event_buffer.push(eve);
	try_wake_mainthread();
}

void android_push_pause_event()
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("Pause Event received.");

	event_t eve;
	eve.type = event_type_e::lifecycle;
	eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::pause;
	s_event_buffer.push(eve);
	flush_mainthread();
}

void android_push_resume_event()
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("Resume Event received.");

	event_t eve;
	eve.type = event_type_e::lifecycle;
	eve.lifecycle_event_data.inner_type = lifecycle_event_type_e::resume;
	s_event_buffer.push(eve);
	try_wake_mainthread();
}

void android_push_focus_event(bool i_hasFocus)
{
	using namespace calyx;
	using namespace calyx::platform::android;

	LOG_TOPIC("platform_event");
	if (i_hasFocus) {
		CLOVER_VERBOSE("Application gained focus.");
	} else {
		CLOVER_VERBOSE("Application lost focus.");
	}
}

//----------------------------------------------
// These functions runs at the same thread with JNI thread
void android_push_touch_down_event(const u32 i_pointerId, const u32 i_x, const u32 i_y)
{
	using namespace calyx;
	using namespace calyx::platform::android;
	{
		u32 x = i_x;
		u32 y = i_y << 16;
		event_t eve;
		eve.type = event_type_e::interact;
		eve.interact_event_data.inner_type = interact_event_e::cursor_move;
		eve.interact_event_data.payload = x | y;
		eve.interact_event_data.lowpayload = i_pointerId;
		s_event_buffer.push(eve);
	}
	{
		event_t eve;
		eve.type = event_type_e::interact;
		eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
		eve.interact_event_data.payload = CLX_TOUCH_DOWN;
		eve.interact_event_data.lowpayload = i_pointerId;
		s_event_buffer.push(eve);
	}
}

void android_push_touch_up_event(const u32 i_pointerId, const u32 i_x, const u32 i_y)
{
	using namespace calyx;
	using namespace calyx::platform::android;
	// should we emit a touch move event before touch up?
	{
		event_t eve;
		eve.type = event_type_e::interact;
		eve.interact_event_data.inner_type = interact_event_e::cursor_interact;
		eve.interact_event_data.payload = CLX_TOUCH_UP;
		eve.interact_event_data.lowpayload = i_pointerId;
		s_event_buffer.push(eve);
	}
}

void android_push_touch_move_event(const u32 i_pointerId, const u32 i_x, const u32 i_y)
{
	using namespace calyx;
	using namespace calyx::platform::android;
	u32 x = i_x;
	u32 y = i_y << 16;
	event_t eve;
	eve.type = event_type_e::interact;
	eve.interact_event_data.inner_type = interact_event_e::cursor_move;
	eve.interact_event_data.payload = x | y;
	eve.interact_event_data.lowpayload = i_pointerId;
	s_event_buffer.push(eve);
}

void android_push_key_event(const u32 i_keyCode)
{
	using namespace calyx;
	using namespace calyx::platform::android;

	event_t eve;
	eve.type = event_type_e::interact;
	eve.interact_event_data.inner_type = interact_event_e::key_input;
	eve.interact_event_data.payload = i_keyCode;
	s_event_buffer.push(eve);
}

void android_push_orientation_event(const f32 i_azimuth, const f32 i_pitch, const f32 i_roll)
{
	using namespace calyx;
	using namespace calyx::platform::android;

	event_t eve;
	eve.type = event_type_e::interact;
	eve.interact_event_data.inner_type = interact_event_e::orientation_input;
	eve.interact_event_data.payload = 0;
	eve.interact_event_data.extpayload[0] = i_azimuth;
	eve.interact_event_data.extpayload[1] = i_pitch;
	eve.interact_event_data.extpayload[2] = i_roll;
	s_event_buffer.push(eve);
}
