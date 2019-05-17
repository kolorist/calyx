#include <floral.h>

#include <calyx/context.h>
#include <calyx/life_cycle.h>
#include <calyx/platform/android/system.h>

#include <clover/SinkTopic.h>
#include <clover/Logger.h>

void android_pre_init()
{
	using namespace calyx;

	context_attribs* commonCtx = get_context_attribs();
	memset(commonCtx, 0, sizeof(context_attribs));
	setup_surface(commonCtx);
}

void android_init()
{
	using namespace calyx;

	platform::android::initialize();
	platform::android::run();
}
