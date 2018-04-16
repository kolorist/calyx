#pragma once

#include <stdaliases.h>

#include "context.h"

namespace calyx {
	// insigned will be init here
	extern void									initialize();
	extern void									run(event_buffer_t* i_evtBuffer);
	// insigned will be de-init here
	extern void									clean_up();

	//------------------------------------------
	// common entry point of main thread
	void										main_thread_func(voidptr i_data);
}
