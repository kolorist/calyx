#include "life_cycle.h"

namespace calyx {

	void main_thread_func(voidptr i_data)
	{
		initialize();
		run();
		clean_up();
	}

}
