#include "life_cycle.h"

namespace calyx {

	void main_thread_func(voidptr i_data)
	{
		event_buffer_t* evtBuffer = static_cast<event_buffer_t*>(i_data);
		initialize();
		run(evtBuffer);
		clean_up();
	}

}
