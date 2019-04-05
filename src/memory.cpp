#include "calyx/memory.h"

namespace calyx
{

extern allocators_t								s_allocators;

//----------------------------------------------

allocators_t* get_allocators()
{
	return &s_allocators;
}

}
