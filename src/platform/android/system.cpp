#include "platform/android/system.h"

#include <Allocator.h>
#include <MemoryManager.h>

#include <Logger.h>
#include <SinkTopic.h>
#include <ADBOutputSink.h>

#include <TaskManager.h>

#include "context.h"
#include "life_cycle.h"

namespace calyx {
namespace platform {
namespace android {

	static bool									s_running;

	static floral::thread						s_main_thread;
	static event_buffer_t						s_event_buffer;

	void initialize()
	{
		helich::init_memory_system();
		clover::InitializeADBOutput("adb", clover::LogLevel::Verbose);

		// init essential systems
		// helich
		helich::init_memory_system();
		clover::InitializeADBOutput("adb", clover::LogLevel::Verbose);

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
