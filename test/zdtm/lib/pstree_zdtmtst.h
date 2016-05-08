#ifndef PSTREE_ZDTMTST_H_
#define PSTREE_ZDTMTST_H_

#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>

#include "zdtmtst.h"

#define __CURRENT_TASK 0
#define __NOT_CURRENT_TASK -2
#define __MAX_CHILDREN_TASKS 100

extern void pstree_test_init_sigaction(void);
extern void pstree_test_kill_children(void);

extern pid_t __children_tasks[__MAX_CHILDREN_TASKS];
extern size_t __children_tasks_count;

size_t  __pstree_tasks_count;
unsigned int  __sync_id_counter;
task_waiter_t __tasks_sync_waiter;
pid_t *__saved_root_task_var;
bool __cr_runned;

#define pstree_test_init(root_task_var, argc, argv) \
	test_init(argc, argv); \
	pstree_test_init_sigaction(); \
	pid_t root_task_var = __CURRENT_TASK; \
	__pstree_tasks_count = 0; \
	__children_tasks_count = 0; \
	__sync_id_counter = 1; \
	__saved_root_task_var = &(root_task_var); \
	__cr_runned = false; \
	task_waiter_init(&__tasks_sync_waiter)

#define pstree_test_fail() {  \
	fail(); \
	pstree_test_kill_children();\
	exit(1); \
}

#define pstree_test_pass() { \
	pstree_test_do_in_task(*__saved_root_task_var, pass()); \
	exit(0); \
}

#define pstree_test_check(assertion) { \
	if (!(assertion)) \
		pstree_test_fail(); \
}

#define pstree_test_check_in_task(task_var, assertion) \
	pstree_test_do_in_task(task_var, pstree_test_check(assertion))

#define pstree_test_do_in_task(task_var, operation) { \
	if (task_var == __CURRENT_TASK) \
		operation; \
}

#define pstree_test_do_in_task_sync(task_var, operation) ({ \
	if (__cr_runned) { \
		pr_perror("using *sync macro after cr_start"); \
	} \
	if (task_var == __CURRENT_TASK) { \
		operation; \
		size_t i; \
		for (i = 0; i <  __pstree_tasks_count; i++) \
			task_waiter_complete( \
				&__tasks_sync_waiter, __sync_id_counter); \
	} else { \
		task_waiter_wait4(&__tasks_sync_waiter, __sync_id_counter); \
	} \
	__sync_id_counter++; \
	pstree_test_check(__sync_id_counter < UINT32_MAX); \
})

#define pstree_test_fork_in_task(task_var_parent, task_var_child) \
	pid_t task_var_child = __NOT_CURRENT_TASK; \
	if (task_var_parent == __CURRENT_TASK) { \
		task_var_child = test_fork(); \
		if (task_var_child == __CURRENT_TASK) { \
			task_var_parent = __NOT_CURRENT_TASK; \
			__children_tasks_count = 0; \
		} else {\
			pstree_test_check(__children_tasks_count < \
							__MAX_CHILDREN_TASKS); \
			pstree_test_check(task_var_child != -1); \
			__children_tasks[__children_tasks_count++] = \
							task_var_child; \
		} \
	} \
	 __pstree_tasks_count++

/* After this command enables async mode.
 * You shouldn't use *sync macroses after that.
 */
#define pstree_test_cr_start() { \
	pstree_test_do_in_task((*__saved_root_task_var), test_daemon()); \
	test_waitsig(); \
	__cr_runned = true; \
	size_t i; \
	for (i = 0; i < __children_tasks_count; i++) { \
		pid_t pid = __children_tasks[i]; \
		if (!kill(pid, SIGTERM)) \
			waitpid(pid, NULL, 0); \
	} \
} \

#endif /* PSTREE_ZDTMTST_H_ */
