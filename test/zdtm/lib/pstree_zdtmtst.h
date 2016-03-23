#ifndef TEST_ZDTM_LIB_PSTREE_ZDTMTST_H_
#define TEST_ZDTM_LIB_PSTREE_ZDTMTST_H_
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <stdarg.h>

#define __MAX_TASKS 100

#define __CURRENT_TASK 0

#define __CHILD_DISABLED -2

#define INIT(ROOT_PID_VAR, argc, argv) \
	test_init(argc, argv); \
	pid_t ROOT_PID_VAR = __CURRENT_TASK; \
	size_t __futex_id_counter = 1 ;\
	size_t __tasks_counter = 0;\
	pid_t __tasks[__MAX_TASKS]; \
	task_waiter_t __task_waiter; \
	task_waiter_init(&__task_waiter);

#define DO_IF(PID_VAR, OPERATION) \
	if (PID_VAR == __CURRENT_TASK) {\
		OPERATION;\
	}

#define DO_IN_TASK(PID_VAR, OPERATION) \
	if (PID_VAR == __CURRENT_TASK) { \
		OPERATION; \
		size_t i = 0; \
		for(i = 0; i < __tasks_counter  ; i++) \
			task_waiter_complete(&__task_waiter, __futex_id_counter); \
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++;

#define MMAP_OPERATION(OUT, args...) \
	OUT = mmap(args);

#define MUNMAP_OPERATION(args...) \
	munmap(args);

#define MREMAP_OPERATION(OUT, args...) \
	OUT = mremap(args);

#define DATAGEN_OPERATION(args...) \
	datagen(args);

#define DATACHK_OPERATION(OUT, args...) \
	OUT = datachk(args);

#define TEST_DAEMON_OPERATION \
	test_daemon();

#define FAIL  \
	fail(); \
	return 1;

#define PASS(ROOT)  \
	DO_IF(ROOT, pass());\
	return 0;

#define ASSERT_EQ_OPERATION(VALUE, NEED) \
	if (VALUE != NEED) { \
		FAIL; \
	}

#define ASSERT_NEQ_OPERATION(VALUE, NEED) \
	if (VALUE == NEED) { \
		FAIL; \
	}

#define ASSERT_EQ(PID_VAR, VALUE, NEED) \
	DO_IF(PID_VAR, ASSERT_EQ_OPERATION(VALUE, NEED));

#define ASSERT_NEQ(PID_VAR, VALUE, NOT_NEED) \
	DO_IF(PID_VAR, ASSERT_NEQ_OPERATION(VALUE, NOT_NEED));

#define FORK(PID_VAR_PARENT, PID_VAR_CHILD) \
	pid_t PID_VAR_CHILD = __CHILD_DISABLED; \
	if (PID_VAR_PARENT == __CURRENT_TASK) { \
		PID_VAR_CHILD = test_fork(); \
		if (PID_VAR_CHILD == __CURRENT_TASK) { \
			size_t i; \
			for (i = 0; i < __MAX_TASKS; i++) \
				__tasks[i] = __CHILD_DISABLED; \
			PID_VAR_PARENT = __CHILD_DISABLED; \
		} else {\
			size_t i ; \
			for(i = 0; i < __tasks_counter  ; i++) \
				task_waiter_complete(&__task_waiter, __futex_id_counter); \
			ASSERT_NEQ_OPERATION(PID_VAR_CHILD, -1); \
		}\
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++; \
	__tasks[__tasks_counter ] = PID_VAR_CHILD; \
	__tasks_counter++;

#define MMAP(PID_VAR, OUT, args...) \
	DO_IN_TASK(PID_VAR, MMAP_OPERATION(OUT, args)); \
	ASSERT_NEQ(PID_VAR, OUT, MAP_FAILED);

#define MUNMAP(PID_VAR, args...) \
	DO_IN_TASK(PID_VAR, MUNMAP_OPERATION(args));

#define MREMAP(PID_VAR, OUT, args...) \
	DO_IN_TASK(PID_VAR, MREMAP_OPERATION(OUT, args)); \
	ASSERT_NEQ(PID_VAR, OUT, MAP_FAILED);

#define DATAGEN(PID_VAR, args...) \
	DO_IN_TASK(PID_VAR, DATAGEN_OPERATION(args));

#define DATACHK(PID_VAR, OUT, args...) \
	DO_IF(PID_VAR, DATACHK_OPERATION(OUT, args));

#define DATACHK_Z(PID_VAR, CRC, OUT, args...) \
	CRC = ~0; \
	DATACHK(PID_VAR, OUT, args, (&CRC));

#define DATAGEN_Z(PID_VAR, CRC, args...) \
	CRC = ~0;\
	DATAGEN(PID_VAR, args, (&CRC));

#define DATACHK_Z_CHECK(PID_VAR, CRC, args ...) {\
	int __temp_result_datachk_z_ASSERT = 0;\
	DATACHK_Z(PID_VAR, CRC, __temp_result_datachk_z_ASSERT, args);  \
	ASSERT_EQ(PID_VAR, __temp_result_datachk_z_ASSERT, 0); \
}

#define __TEST_PROPAGATE_SIG_SUCCESS 0
#define __TEST_PROPAGATE_SIG_FAIL 1

#define TEST_PROPAGATE_SIG(OUT) { \
	size_t i; \
	int status; \
	OUT = __TEST_PROPAGATE_SIG_SUCCESS; \
	for (i = 0; i < __tasks_counter; i++) { \
		pid_t pid = __tasks[i]; \
		if (pid == __CHILD_DISABLED || pid == __CURRENT_TASK) \
			continue; \
		kill(pid, SIGTERM);\
		waitpid(pid, &status, 0); \
		if (WIFEXITED(status)) { \
			if (WEXITSTATUS(status)) { \
				OUT = __TEST_PROPAGATE_SIG_FAIL; \
				break; \
			} \
		} else { \
			OUT = __TEST_PROPAGATE_SIG_FAIL; \
			break; \
		} \
	} \
}

#define CR_START(ROOT) \
	DO_IF(ROOT, TEST_DAEMON_OPERATION); \
	test_waitsig(); \
	int __temp_result_cr_start = __TEST_PROPAGATE_SIG_SUCCESS;\
	TEST_PROPAGATE_SIG(__temp_result_cr_start); \
	ASSERT_EQ_OPERATION(__temp_result_cr_start, __TEST_PROPAGATE_SIG_SUCCESS);

#endif /* Test_ZDTM_LIB_MAP_F_H_ */
