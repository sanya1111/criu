
#ifndef TEST_ZDTM_LIB_MAP_F_H_
#define TEST_ZDTM_LIB_MAP_F_H_

#define MAX_TASKS 100

#ifdef DEBUG
#define Test_msg(args...) test_msg(args)
#else
#define Test_msg(args...) ;
#endif

#define INIT(ROOT_PID_VAR) \
	pid_t ROOT_PID_VAR = 0; \
	int __futex_id_counter = 1 ;\
	int __process_counter = 0;\
	pid_t __tasks[MAX_TASKS]; \
	task_waiter_t __task_waiter; \
	task_waiter_init(&__task_waiter); \

#define DO_IF(PID_VAR, operation) \
	if(PID_VAR == 0) {\
		operation;\
	} \

#define SYNC(PID_VAR, operation) \
	if (PID_VAR == 0) { \
		operation; \
		int i = 0; \
		for(i = 0; i < __process_counter  ; i++) \
			task_waiter_complete(&__task_waiter, __futex_id_counter); \
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++; \

#define FORK_OPERATION(PID_VAR_PARENT, PID_VAR_CHILD) \
	PID_VAR_CHILD = test_fork(); \
	if(PID_VAR_CHILD == 0) { \
		int i; \
		for(i = 0; i < MAX_TASKS; i++) \
			__tasks[i] = -2; \
		PID_VAR_PARENT = -2; \
	}

#define MMAP_OPERATION(OUT, args...) \
	OUT = mmap(args);

#define MUNMAP_OPERATION(args...) \
	munmap(args);

#define MREMAP_OPERATION(OUT, args...) \
	OUT = mremap(args); \

#define DATAGEN_OPERATION(args...) \
	datagen(args);

#define DATACHK_OPERATION(OUT, args...) \
	OUT = datachk(args);

#define Test_DEMON_OPERATION \
	test_daemon(); \

#define KILL_OPERATION(args ... ) \
	kill_all(args); \



#define TERM_OPERATION(OUT) { \
	int i, status; \
	OUT = 0; \
	for(i = 0; i < __process_counter; i++){ \
		pid_t pid = __tasks[i]; \
		if(pid <= 0) \
			continue; \
		kill(pid, SIGTERM);\
		waitpid(pid, &status, 0); \
		if (WIFEXITED(status)) { \
			if (WEXITSTATUS(status)) { \
				OUT = 1; \
				break; \
			} \
		} else { \
			OUT = 1; \
			break; \
		} \
	} \
} \

#define FAIL  \
	return 1; \

#define FINISH  \
	DO_IF(ROOT, pass());\
	return 0; \

#define CHECK_EQ_OPERATION(VALUE, NEED) \
	if(VALUE != NEED) { \
		FAIL; \
	}\

#define CHECK_NEQ_OPERATION(VALUE, NEED) \
	if(VALUE == NEED) { \
		FAIL; \
	} \


#define CHECK_EQ(PID_VAR, VALUE, NEED) \
	DO_IF(PID_VAR, CHECK_EQ_OPERATION(VALUE, NEED)); \

#define CHECK_NEQ(PID_VAR, VALUE, NOT_NEED) \
	DO_IF(PID_VAR, CHECK_NEQ_OPERATION(VALUE, NOT_NEED)); \

#define FORK(PID_VAR_PARENT, PID_VAR_CHILD) \
	pid_t PID_VAR_CHILD = -2; \
	SYNC(PID_VAR_PARENT, FORK_OPERATION(PID_VAR_PARENT, PID_VAR_CHILD)); \
	CHECK_NEQ(PID_VAR_PARENT, PID_VAR_CHILD, -1); \
	__tasks[__process_counter ] = PID_VAR_CHILD; \
	__process_counter++; \

#define MMAP(PID_VAR, OUT, args...) \
	SYNC(PID_VAR, MMAP_OPERATION(OUT, args)); \
	CHECK_NEQ(PID_VAR, OUT, MAP_FAILED); \

#define MUNMAP(PID_VAR, args...) \
	SYNC(PID_VAR, MUNMAP_OPERATION(args)); \

#define MREMAP(PID_VAR, OUT, args...) \
	SYNC(PID_VAR, MREMAP_OPERATION(OUT, args)); \
	CHECK_NEQ(PID_VAR, OUT, MAP_FAILED); \

#define DATAGEN(PID_VAR, args...) \
	SYNC(PID_VAR, DATAGEN_OPERATION(args)); \

#define DATACHK(PID_VAR, OUT, args...) \
	DO_IF(PID_VAR, DATACHK_OPERATION(OUT, args)); \

#define DATACHK_Z(PID_VAR, CRC, OUT, args...) \
	CRC = ~0; \
	DATACHK(PID_VAR, OUT, args); \

#define DATAGEN_Z(PID_VAR, CRC, args...) \
	CRC = ~0;\
	DATAGEN(PID_VAR, args); \

#define DATACHK_Z_CHECK(PID_VAR, CRC, args ...) {\
	int __temp_result_datachk_z_check = 0;\
	DATACHK_Z(PID_VAR, CRC, __temp_result_datachk_z_check, args);  \
	CHECK_EQ(PID_VAR, __temp_result_datachk_z_check, 0); \
	}\

#define CR_START(ROOT) \
	DO_IF(ROOT, Test_DEMON_OPERATION); \
	test_waitsig(); \
	int __temp_result_cr_start = 0;\
	TERM_OPERATION(__temp_result_cr_start); \
	CHECK_EQ_OPERATION(__temp_result_cr_start, 0); \




#endif /* Test_ZDTM_LIB_MAP_F_H_ */
