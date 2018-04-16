#pragma once

#include <stdaliases.h>
#include <thread/Thread.h>

namespace calyx {
namespace platform {
namespace windows {

	void										initialize();
	void										run();
	void										clean_up();

}
}
}
