#pragma once

#include <floral/containers/double_queue.h>

namespace calyx
{

enum class interact_event_e : s32
{
	cursor_interact = 0,
	cursor_move,
	scroll,
	character_input,
	key_input,
	orientation_input,
	window_resize,
	window_lifecycle
};

enum class lifecycle_event_type_e : s32
{
	pause = 0,
	resume,
	surface_ready,
	surface_destroyed,
	focus_gain,
	focus_lost,
	stop
};

enum class event_type_e : s32
{
	interact = 0,
	lifecycle
};

struct event_t
{
	event_type_e								type;
	union
	{
		struct
		{
			u64									payload;
			u32									lowpayload;
			f32									extpayload[3];
			interact_event_e					inner_type;
		} interact_event_data;
		struct
		{
			lifecycle_event_type_e				inner_type;
		} lifecycle_event_data;
	};
};

#define CLX_MAX_EVENTS_PER_QUEUE				8192
typedef floral::inplace_spsc_double_queue_lock_based_t<event_t, CLX_MAX_EVENTS_PER_QUEUE> event_buffer_t;

}
