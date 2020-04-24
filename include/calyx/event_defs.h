#pragma once

#if defined(FLORAL_PLATFORM_WINDOWS)
	#include "platform/windows/event_defs.h"
#else
	#include "platform/android/event_defs.h"
#endif