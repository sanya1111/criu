#ifndef PSTREE_ZDTMTST_WRAPPERS_H_
#define PSTREE_ZDTMTST_WRAPPERS_H_

#include "pstree_zdtmtst.h"

#define pstt_mmap_in_task(task_var, mmap_args...) ({ \
	void *result = MAP_FAILED; \
	pstt_do_in_task_sync(task_var, { \
		result = mmap(mmap_args); \
		pstt_check(result != MAP_FAILED); \
	}); \
	result; \
})

#define pstt_munmap_in_task(task_var, munmap_args...) \
	pstt_do_in_task_sync(task_var, \
		pstt_check(munmap(munmap_args) == 0))

#define pstt_mremap_in_task(task_var, mremap_args...) ({\
	void *result = MAP_FAILED; \
	pstt_do_in_task_sync(task_var, { \
		result = mremap(mremap_args); \
		pstt_check(result != MAP_FAILED); \
	}); \
	result; \
})

#define pstt_datagen_in_task(task_var, datagen_args...) { \
	uint32_t crc = ~0; \
	pstt_do_in_task_sync(task_var, datagen(datagen_args, &(crc))); \
}

#define pstt_datachk_in_task(task_var, datachk_args ...) {\
	int res = 0;\
	uint32_t crc = ~0; \
	pstt_do_in_task(task_var, res = datachk(datachk_args, &(crc))); \
	pstt_check_in_task(task_var, res == 0); \
}

#endif /* PSTREE_ZDTMTST_WRAPPERS_H_ */
