#pragma once

#include <floral/stdaliases.h>
#include <floral/containers/ring_buffer.h>

#include <helich/Allocator.h>
#include <helich/AllocSchemes.h>
#include <helich/TrackingPolicies.h>

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
