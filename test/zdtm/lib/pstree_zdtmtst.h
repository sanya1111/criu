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
	pid_t __saved_root_pid_var = __CURRENT_TASK; \
	size_t __futex_id_counter = 1 ;\
	size_t __tasks_counter = 0;\
	pid_t __tasks[__MAX_TASKS]; \
	task_waiter_t __task_waiter; \
	task_waiter_init(&__task_waiter);

#define DO_IN_TASK(PID_VAR, OPERATION) \
	if (PID_VAR == __CURRENT_TASK) {\
		OPERATION;\
	}

#define DO_IN_TASK_SYNC(PID_VAR, OPERATION) \
	if (PID_VAR == __CURRENT_TASK) { \
		OPERATION; \
		size_t i = 0; \
		for (i = 0; i < __tasks_counter  ; i++) \
			task_waiter_complete(&__task_waiter, __futex_id_counter); \
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++;

#define FAIL  \
	fail(); \
	return 1;

#define PASS  \
	if (__saved_root_pid_var == __CURRENT_TASK)\
		pass();\
	return 0;

#define ASSERT(ASSERTION) \
	if (!(ASSERTION)) { \
		FAIL; \
	}

#define ASSERT_IN_TASK(PID_VAR, ASSERTION) \
	DO_IN_TASK(PID_VAR, ASSERT(ASSERTION));

#define ASSERT_IN_TASK_SYNC(PID_VAR, ASSERTION) \
	DO_IN_TASK_SYNC(PID_VAR, ASSERT(ASSERTION));

#define FORK(PID_VAR_PARENT, PID_VAR_CHILD) \
	pid_t PID_VAR_CHILD = __CHILD_DISABLED; \
	if (PID_VAR_PARENT == __CURRENT_TASK) { \
		PID_VAR_CHILD = test_fork(); \
		if (PID_VAR_CHILD == __CURRENT_TASK) { \
			size_t i; \
			for (i = 0; i < __MAX_TASKS; i++) \
				__tasks[i] = __CHILD_DISABLED; \
			PID_VAR_PARENT = __CHILD_DISABLED; \
			__saved_root_pid_var = __CHILD_DISABLED ; \
		} else {\
			size_t i ; \
			for(i = 0; i < __tasks_counter  ; i++) \
				task_waiter_complete(&__task_waiter, __futex_id_counter); \
			ASSERT(PID_VAR_CHILD != -1); \
		}\
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++; \
	__tasks[__tasks_counter ] = PID_VAR_CHILD; \
	__tasks_counter++;

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

#define CR_START \
	if (__saved_root_pid_var == __CURRENT_TASK)\
		test_daemon(); \
	test_waitsig(); \
	int __temp_result_cr_start = __TEST_PROPAGATE_SIG_SUCCESS;\
	TEST_PROPAGATE_SIG(__temp_result_cr_start); \
	ASSERT(__temp_result_cr_start ==  __TEST_PROPAGATE_SIG_SUCCESS);

#endif /* Test_ZDTM_LIB_MAP_F_H_ */
