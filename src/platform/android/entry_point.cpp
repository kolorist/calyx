#include <floral.h>

#include "context.h"
#include "platform/android/system.h"

void android_pre_init()
{
	using namespace calyx;
	g_context_attribs.window_offset_left = 0;
	g_context_attribs.window_offset_top = 0;
	g_context_attribs.window_width = 1280;
	g_context_attribs.window_height = 720;
	g_context_attribs.window_title = "calyx app";
}

void android_init()
{
	using namespace calyx;
	platform::android::initialize();
	platform::android::run();
}

void android_update_surface()
{
}

void android_push_touch_event()
{
}

void android_push_key_event()
{
}
