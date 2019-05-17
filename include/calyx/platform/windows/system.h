#pragma once

#include <floral.h>

#include <calyx/context.h>

// windows
#include <Windows.h>

namespace calyx {
namespace platform {
namespace windows {

struct windows_context_attribs {
	HWND										hwnd;
	HDC											dc;
};
windows_context_attribs*						get_windows_context_attribs();

//----------------------------------------------

void											initialize();
void											run();
void											clean_up();

}
}
}
