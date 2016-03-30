#ifndef PSTREE_ZDTMTST_WRAPPERS_H_
#define PSTREE_ZDTMTST_WRAPPERS_H_

#include "pstree_zdtmtst.h"

#define mmap_in_task(task_var, mmap_args...) ({\
	void * result = MAP_FAILED; \
	do_in_task_sync(task_var, result = mmap(mmap_args)); \
	assert_in_task(task_var, result != MAP_FAILED); \
	result; \
})

#define munmap_in_task(task_var, munmap_args...) \
	do_in_task_sync(task_var, munmap(munmap_args));

#define mremap_in_task(task_var, mremap_args...) ({\
	void * result = MAP_FAILED; \
	do_in_task_sync(task_var, result = mremap(mremap_args)); \
	assert_in_task(task_var, result != MAP_FAILED); \
	result; \
})

#define datagen_in_task(task_var, datagen_args...) { \
	uint32_t __crc = ~0; \
	do_in_task_sync(task_var, datagen(datagen_args, &(__crc))); \
}

#define datachk_in_task(task_var, datachk_args ...) {\
	int __temp_result_datachk_z = 0;\
	uint32_t __crc = ~0; \
	do_in_task(task_var, __temp_result_datachk_z = datachk(datachk_args, &(__crc))); \
	assert_in_task(task_var, __temp_result_datachk_z == 0); \
}

#endif /* PSTREE_ZDTMTST_WRAPPERS_H_ */
