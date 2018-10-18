#pragma once

#include <stdaliases.h>
#include <thread/thread.h>

#include "context.h"

// android
#include <EGL/egl.h>
#include <android/native_window.h>

namespace calyx {

	struct android_context_attribs : context_attribs {
		EGLDisplay								display;
		EGLSurface								surface;
		EGLContext								main_context;
		ANativeWindow*							native_window;
	};

	extern android_context_attribs				g_android_context_attribs;
	
namespace platform {
namespace android {

	void										initialize();
	void										run();
	void										clean_up();

}
}
}

void											android_update_surface(ANativeWindow* i_wnd);
void											android_push_touch_event();
void											android_push_touch_move_event(const u32 i_x, const u32 i_y);
void											android_push_key_event();
