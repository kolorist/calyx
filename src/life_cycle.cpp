#include "calyx/life_cycle.h"

#include <clover.h>

namespace calyx {

void main_thread_func(voidptr i_data)
{
	CLOVER_INIT_THIS_THREAD("main_thread", clover::LogLevel::Verbose);
	CLOVER_VERBOSE("Main thread started");

	event_buffer_t* evtBuffer = static_cast<event_buffer_t*>(i_data);
	CLOVER_VERBOSE("Calling user's initialize()");
	initialize();
	CLOVER_VERBOSE("Calling user's run()");
	run(evtBuffer);
	CLOVER_VERBOSE("Calling user's clean_up()");
	clean_up();
}

}
