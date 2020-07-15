#pragma once

#include <helich/allocator.h>
#include <helich/alloc_schemes.h>
#include <helich/tracking_policies.h>

namespace calyx
{

// allocators
typedef helich::allocator<helich::stack_scheme, helich::no_tracking_policy>	stack_allocator_t;

// ***
// user-provided
// ***
struct allocators_t {
	stack_allocator_t							subsystems_allocator;
};

//----------------------------------------------

allocators_t*									get_allocators();

}
