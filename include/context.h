#pragma once

#include <stdaliases.h>

#include <Allocator.h>
#include <AllocSchemes.h>
#include <TrackingPolicies.h>

#include <TaskManager.h>

namespace calyx {

	struct context_attribs {
		const_cstr								window_title;
		u32										window_offset_left;
		u32										window_offset_top;
		u32										window_width;
		u32										window_height;
	};

	// allocators
	typedef helich::allocator<helich::stack_scheme, helich::no_tracking_policy>	stack_allocator_t;

	// ***
	// user-provided
	// ***
	struct allocators {
		stack_allocator_t						subsystems_allocator;
	};

	// platform sub-systems
	struct subsystems {
		refrain2::TaskManager*					task_manager;
	};

	extern context_attribs						g_context_attribs;
	extern allocators							g_allocators;
	extern subsystems							g_subsystems;

}
