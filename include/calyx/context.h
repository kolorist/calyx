#pragma once

#include <floral/stdaliases.h>
#include <floral/containers/ring_buffer.h>

#include <helich/allocator.h>
#include <helich/alloc_schemes.h>
#include <helich/tracking_policies.h>

#include <TaskManager.h>

namespace calyx {

struct context_attribs {
	const_cstr									window_title;
	u32											window_offset_left;
	u32											window_offset_top;
	f32											window_scale;		// if it's 0.0f, we prefer window_width and window_height
	u32											window_width;
	u32											window_height;
	c8											data_path[1024];
	bool										render_in_main_thread;
};

struct system_info_t
{
	size										page_size;
	u32											num_logical_processors;
	u32											primary_screen_width;
	u32											primary_screen_height;
};

// platform sub-systems
struct subsystems {
	refrain2::TaskManager*						task_manager;
};

//----------------------------------------------

context_attribs*								get_context_attribs();
subsystems*										get_subsystems();
system_info_t*									get_system_info();

}
