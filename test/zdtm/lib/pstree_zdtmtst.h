#ifndef PSTREE_ZDTMTST_H_
#define PSTREE_ZDTMTST_H_

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

#define __CURRENT_TASK 0
#define __NOT_CURRENT_TASK -2
#define __MAX_CHILDREN_TASKS 100

#define pstree_test_init(root_task_var, argc, argv) \
	test_init(argc, argv); \
	pid_t root_task_var = __CURRENT_TASK; \
	pid_t * __saved_root_task_var = &(root_task_var); \
	size_t __futex_id_counter = 1 ;\
	size_t __pstree_num_tasks = 0;\
	size_t __children_tasks_counter = 0; \
	pid_t __children_tasks[__MAX_CHILDREN_TASKS]; \
	task_waiter_t __task_waiter; \
	task_waiter_init(&__task_waiter);

#define do_in_task(task_var, operation) \
	if (task_var == __CURRENT_TASK) {\
		operation;\
	}

#define do_in_task_sync(task_var, operation) \
	if (task_var == __CURRENT_TASK) { \
		operation; \
		size_t i; \
		for (i = 0; i < __pstree_num_tasks  ; i++) \
			task_waiter_complete(&__task_waiter, __futex_id_counter); \
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++;

#define FAIL  \
	fail(); \
	exit(1);

#define PASS  \
	if (*__saved_root_task_var == __CURRENT_TASK)\
		pass();\
	exit(0);

#define __assert(assertion) \
	if (!(assertion)) { \
		FAIL; \
	}

#define assert_in_task(task_var, assertion) \
	do_in_task(task_var, __assert(assertion));

#define assert_in_task_sync(task_var, assertion) \
	do_in_task_sync(task_var, __assert(assertion));

#define FORK(task_var_parent, task_var_child) \
	pid_t task_var_child = __NOT_CURRENT_TASK; \
	if (task_var_parent == __CURRENT_TASK) { \
		task_var_child = test_fork(); \
		if (task_var_child == __CURRENT_TASK) { \
			task_var_parent = __NOT_CURRENT_TASK; \
			__children_tasks_counter = 0; \
		} else {\
			__assert(__children_tasks_counter < __MAX_CHILDREN_TASKS); \
			__assert(task_var_child != -1); \
			__children_tasks[__children_tasks_counter++] = task_var_child; \
		}\
	} \
	__pstree_num_tasks++;

#define FORK_SYNC(task_var_parent, task_var_child) \
	pid_t task_var_child = __NOT_CURRENT_TASK; \
	__children_list_entry_create(task_var_child); \
	if (task_var_parent == __CURRENT_TASK) { \
		task_var_child = test_fork(); \
		if (task_var_child == __CURRENT_TASK) { \
			task_var_parent = __NOT_CURRENT_TASK; \
			__children_tasks_counter = 0; \
		} else {\
			size_t i ; \
			for(i = 0; i < __pstree_num_tasks  ; i++) \
				task_waiter_complete(&__task_waiter, __futex_id_counter); \
			__assert(__children_tasks_counter < __MAX_CHILDREN_TASKS); \
			__assert(task_var_child != -1); \
			__children_tasks[__children_tasks_counter++] = task_var_child; \
		}\
	} else { \
		task_waiter_wait4(&__task_waiter, __futex_id_counter); \
	} \
	__futex_id_counter++; \
	__pstree_num_tasks++;

#define __test_propagate_sig { \
	int status; \
	size_t i; \
	for ( i = 0; i < __children_tasks_counter; i++) { \
		pid_t pid = __children_tasks[i]; \
		kill(pid, SIGTERM);\
		waitpid(pid, &status, 0); \
		if (WIFEXITED(status)) { \
			if (WEXITSTATUS(status)) { \
				FAIL; \
			} \
		} else { \
			FAIL; \
		} \
	} \
}

#define cr_start \
	if (*__saved_root_task_var == __CURRENT_TASK)\
		test_daemon(); \
	test_waitsig(); \
	__test_propagate_sig;

#endif /* PSTREE_ZDTMTST_H_ */

