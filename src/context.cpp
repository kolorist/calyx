#include "calyx/context.h"

namespace calyx {

static context_attribs							s_context_attribs;
static subsystems								s_subsystems;

//----------------------------------------------
context_attribs* get_context_attribs()
{
	return &s_context_attribs;
}

subsystems* get_subsystems()
{
	return &s_subsystems;
}

}
