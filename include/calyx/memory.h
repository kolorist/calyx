#pragma once

#include <helich/Allocator.h>
#include <helich/AllocSchemes.h>
#include <helich/TrackingPolicies.h>

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
