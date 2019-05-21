#include "calyx/context.h"

namespace calyx {

static context_attribs							s_context_attribs;
static subsystems								s_subsystems;
static system_info_t							s_system_info;

//----------------------------------------------
context_attribs* get_context_attribs()
{
	return &s_context_attribs;
}

subsystems* get_subsystems()
{
	return &s_subsystems;
}

system_info_t* get_system_info()
{
	return &s_system_info;
}

}
