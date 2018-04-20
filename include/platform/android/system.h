#pragma once

#include <stdaliases.h>
#include <thread/thread.h>

namespace calyx {
namespace platform {
namespace android {

	void										initialize();
	void										run();
	void										clean_up();

}
}
}
