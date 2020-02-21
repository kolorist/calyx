#pragma once

#include <floral/stdaliases.h>
#include <floral/thread/thread.h>

#include <calyx/context.h>

// android
#include <EGL/egl.h>
#include <android/native_window.h>

namespace calyx {
namespace platform {
namespace android {

struct android_context_attribs {
	EGLDisplay									display;
	EGLSurface									surface;
	EGLConfig									config;
	EGLContext									main_context;
	ANativeWindow*								native_window;
};
android_context_attribs*						get_android_context_attribs();

//----------------------------------------------

void											initialize();
void											run();
void											clean_up();

}
}
}

void											android_push_pause_event();
void											android_push_resume_event();
void											android_push_focus_event(bool i_hasFocus);
void											android_update_surface(ANativeWindow* i_wnd);
void											android_will_destroy_surface(ANativeWindow* i_wnd);
void											android_created_surface(ANativeWindow* i_wnd);

void											android_push_touch_down_event(const u32 i_pointerId, const u32 i_x, const u32 i_y);
void											android_push_touch_up_event(const u32 i_pointerId, const u32 i_x, const u32 i_y);
void											android_push_touch_move_event(const u32 i_pointerId, const u32 i_x, const u32 i_y);
void											android_push_key_event(const u32 i_keyCode);
void											android_push_orientation_event(const f32 i_azimuth, const f32 i_pitch, const f32 i_roll);
