#include <floral.h>

#include "context.h"
#include "platform/windows/system.h"

s32 main(c8 argc, const_cstr* argv)
{
	using namespace calyx;
	using namespace calyx::platform::windows;

	g_context_attribs.window_offset_left = 30;
	g_context_attribs.window_offset_top = 30;
	g_context_attribs.window_width = 1280;
	g_context_attribs.window_height = 720;

	platform::windows::initialize();
	platform::windows::run();
	platform::windows::clean_up();

	return 0;
}
