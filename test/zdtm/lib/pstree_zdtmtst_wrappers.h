#ifndef PSTREE_ZDTMTST_WRAPPERS_H_
#define PSTREE_ZDTMTST_WRAPPERS_H_

#include "pstree_zdtmtst.h"

#define mmap_in_task(task_var, mmap_args...) ({ \
	void *result = MAP_FAILED; \
	do_in_task_sync(task_var, { \
		result = mmap(mmap_args); \
		pstree_test_check(result != MAP_FAILED); \
	}); \
	result; \
})

#define munmap_in_task(task_var, munmap_args...) \
	do_in_task_sync(task_var, pstree_test_check(munmap(munmap_args) == 0))

#define mremap_in_task(task_var, mremap_args...) ({\
	void *result = MAP_FAILED; \
	do_in_task_sync(task_var, { \
		result = mremap(mremap_args); \
		pstree_test_check(result != MAP_FAILED); \
	}); \
	result; \
})

#define datagen_in_task(task_var, datagen_args...) { \
	uint32_t crc = ~0; \
	do_in_task_sync(task_var, datagen(datagen_args, &(crc))); \
}

#define datachk_in_task(task_var, datachk_args ...) {\
	int res = 0;\
	uint32_t crc = ~0; \
	do_in_task(task_var, res = datachk(datachk_args, &(crc))); \
	pstree_test_check_in_task(task_var, res == 0); \
}

#endif /* PSTREE_ZDTMTST_WRAPPERS_H_ */
