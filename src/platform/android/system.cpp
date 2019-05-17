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
void android_push_touch_event()
{
}

void android_push_touch_move_event(const u32 i_x, const u32 i_y)
{
	using namespace calyx::platform::android;
	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("touch: %d - %d", i_x, i_y);
}

void android_push_key_event()
{
	using namespace calyx::platform::android;
	LOG_TOPIC("platform_event");
	CLOVER_VERBOSE("key");
}
