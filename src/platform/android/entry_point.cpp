#include <floral.h>

#include <calyx/context.h>
#include <calyx/life_cycle.h>
#include <calyx/platform/android/system.h>

#include <clover/SinkTopic.h>
#include <clover/Logger.h>

#include <cstring>

namespace floral
{
namespace platform
{
extern void set_working_directory(const_cstr i_wdir);
}
}

void android_pre_init(const char* i_filesDir, const char* i_obbDir, const char* i_externalFilesDir)
{
	using namespace calyx;

	context_attribs* commonCtx = get_context_attribs();
	memset(commonCtx, 0, sizeof(context_attribs));
	setup_surface(commonCtx);
	
	floral::platform::set_working_directory(i_externalFilesDir);
}

void android_init()
{
	using namespace calyx;

	platform::android::initialize();
	platform::android::run();
}
