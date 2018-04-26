#include <floral.h>

#include "platform/windows/system.h"

s32 main(c8 argc, const_cstr* argv)
{
	using namespace calyx;

	g_windows_context_attribs.window_offset_left = 30;
	g_windows_context_attribs.window_offset_top = 30;
	g_windows_context_attribs.window_width = 1280;
	g_windows_context_attribs.window_height = 720;
	g_windows_context_attribs.window_title = "calyx app";

	platform::windows::initialize();
	platform::windows::run();
	platform::windows::clean_up();

	return 0;
}
