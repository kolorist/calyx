#include <floral.h>

#include <calyx/context.h>
#include <calyx/life_cycle.h>
#include <calyx/platform/windows/system.h>

s32 main(s32 argc, const_cstr* argv)
{
	using namespace calyx;

	context_attribs* commonCtx = get_context_attribs();
	memset(commonCtx, 0, sizeof(context_attribs));
	setup_surface(commonCtx);

	platform::windows::initialize();
	platform::windows::run();
	platform::windows::clean_up();

	return 0;
}
