#pragma once

#include <floral.h>

#include "../../context.h"

#include <Windows.h>

namespace calyx {

	struct windows_context_attribs : context_attribs {
		HWND									hwnd;
		HDC										dc;
	};

	extern windows_context_attribs				g_windows_context_attribs;

namespace platform {
namespace windows {

	void										initialize();
	void										run();
	void										clean_up();

}
}
}
