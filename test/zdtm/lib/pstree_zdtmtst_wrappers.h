#ifndef TEST_ZDTM_LIB_PSTREE_ZDTMTST_WRAPPERS_H_
#define TEST_ZDTM_LIB_PSTREE_ZDTMTST_WRAPPERS_H_
#include "pstree_zdtmtst.h"

#define MMAP(PID_VAR, OUT, args...) \
	DO_IN_TASK_SYNC(PID_VAR, OUT = mmap(args)); \
	ASSERT_IN_TASK(PID_VAR, OUT != MAP_FAILED);

#define MUNMAP(PID_VAR, args...) \
	DO_IN_TASK_SYNC(PID_VAR, munmap(args););

#define MREMAP(PID_VAR, OUT, args...) \
	DO_IN_TASK_SYNC(PID_VAR, OUT = mremap(args)); \
	ASSERT_IN_TASK(PID_VAR, OUT != MAP_FAILED);

#define DATAGEN(PID_VAR, args...) \
	DO_IN_TASK_SYNC(PID_VAR, datagen(args));

#define DATACHK(PID_VAR, OUT, args...) \
	DO_IN_TASK(PID_VAR, OUT = datachk(args));

#define DATACHK_Z(PID_VAR, CRC, OUT, args...) \
	CRC = ~0; \
	DATACHK(PID_VAR, OUT, args, (&CRC));

#define DATAGEN_Z(PID_VAR, CRC, args...) \
	CRC = ~0;\
	DATAGEN(PID_VAR, args, (&CRC));

#define DATACHK_Z_CHECK(PID_VAR, CRC, args ...) {\
	int __temp_result_datachk_z = 0;\
	DATACHK_Z(PID_VAR, CRC, __temp_result_datachk_z, args);  \
	ASSERT_IN_TASK(PID_VAR, __temp_result_datachk_z == 0); \
}



#endif
