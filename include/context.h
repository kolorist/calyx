#pragma once

#include <stdaliases.h>
#include <containers/ring_buffer.h>

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

enum class interact_event_e {
	cursor_interact,
	cursor_move,
	scroll,
	character_input,
	key_input,
	window_resize,
	window_lifecycle
};

enum class life_cycle_event_type_e : u32 {
	pause = 0,
	resume,
	display_update,
	focus_gain,
	focus_lost
};

struct interact_event_t {
	interact_event_e							event_type;
	u32											payload;
};

typedef floral::ring_buffer_mt<interact_event_t, stack_allocator_t, 256>	event_buffer_t;

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

extern context_attribs*						g_context_attribs;
extern allocators							g_allocators;
extern subsystems							g_subsystems;

}
