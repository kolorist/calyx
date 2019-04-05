#pragma once

#include <stdaliases.h>

#include "context.h"
#include "events.h"

namespace calyx {
// called from input thread
extern void										setup_surface(context_attribs* io_commonCtx);
extern void										try_wake_mainthread();
extern void										flush_mainthread();

// called from main thread
extern void										initialize();
extern void										run(event_buffer_t* i_evtBuffer);
extern void										clean_up();

//------------------------------------------
// common entry point of main thread
void											main_thread_func(voidptr i_data);
}
