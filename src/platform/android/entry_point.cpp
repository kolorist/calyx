#include <floral.h>

#include "platform/android/system.h"

void android_pre_init()
{
	using namespace calyx;
	g_android_context_attribs.window_offset_left = 0;
	g_android_context_attribs.window_offset_top = 0;
	g_android_context_attribs.window_width = 1280;
	g_android_context_attribs.window_height = 720;
	g_android_context_attribs.window_title = "calyx app";
}

void android_init()
{
	using namespace calyx;
	platform::android::initialize();
	platform::android::run();
}