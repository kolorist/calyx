#pragma once

#include <stdaliases.h>
#include <containers/ring_buffer.h>

#include <Allocator.h>
#include <AllocSchemes.h>
#include <TrackingPolicies.h>

#include <TaskManager.h>

namespace calyx {

struct context_attribs {
	const_cstr									window_title;
	u32											window_offset_left;
	u32											window_offset_top;
	u32											window_width;
	u32											window_height;
	bool										render_in_main_thread;
};

// platform sub-systems
struct subsystems {
	refrain2::TaskManager*						task_manager;
};

//----------------------------------------------

context_attribs*								get_context_attribs();
subsystems*										get_subsystems();

}
