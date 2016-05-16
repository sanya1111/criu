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

/*
 * Framework for easy creation of ZDTM tests for complicated process trees.
 * Framework provides set of macros and functions that make
 * simultaneous code execution in different ps tree processes sequential.
 * Moreover this code is written and looks like sequential.
 * Thus it simplifies big ps tree tests programming
 * (no manual processes synchronization needed)
 * and significantly improves code readability.
 *
 * Each test within this framework should contain:
 * 1) pstt_init(root, argc, argv)
 * 2) ...(ps tree initialization code)
 * 3) pstt_cr_start() - does ps tree checkpoint-restore
 * 4) ...(ps tree self validation code)
 * 5) pstt_pass() if ok else pstt_fail()
 *
 * So how can we write sequential code that executes in different ps tree
 * processes? First we need to create processes. Creation of processes in
 * the framework is done using pstt_fork_in_task(parent_task, child_task)
 * macro. It is implemented like this:
 * // Fork new task in parent_task and define new variable
 * // child_task that represents forked task.
 * const pid_t __CURRENT_TASK = 0;
 * const pid_t __NOT_CURRENT_TASK = -2;
 *
 * pid_t parent_task = __CURRENT_TASK;
 * pid_t child_task = __NOT_CURRENT_TASK;
 * child_task = fork();
 * if (child_task)
 *     child_task = __NOT_CURRENT_TASK;
 * else
 *     parent_task = __NOT_CURRENT_TASK;
 *
 * Now we want to write code executed in particular task.
 * That's what pstt_do_in_task(task_var, your_code) is used for.
 * It is implemented like this:
 * if (task_var == __CURRENT_TASK) {
 *     your_code;
 * }
 *
 * Now we want to exec int fd2 = open("/tmp/1", ...); code in all tasks.
 * Just write it like this:
 * int fd2 = open("/tmp/1", ...);
 *
 * And the last thing. Suppose you want to execute some code in task2
 * strictly after some other code in task1 has completed. Do it this way:
 * do_in_task_sync(task1, { your_code_here; });
 * do_in_task(task2, { your_code_here; });
 *
 * But there is a restriction. Currently you can't call do_in_task_sync()
 * after pstt_cr_start()
 */

#define __CURRENT_TASK 0
#define __NOT_CURRENT_TASK -2
#define __MAX_CHILDREN_TASKS 100

extern void __pstt_init_sigaction(void);
extern void __pstt_fail(const char *func_name, size_t line, const char *msg);

extern pid_t __children_tasks[__MAX_CHILDREN_TASKS];
extern size_t __children_tasks_count;
extern size_t __pstree_tasks_count;
extern unsigned int  __sync_id_counter;
extern task_waiter_t __tasks_sync_waiter;
extern pid_t *__saved_root_task_var;
extern bool __cr_runned;

#define pstt_init(root_task_var, argc, argv) \
	pid_t root_task_var = __CURRENT_TASK; \
	test_init(argc, argv); \
	__pstt_init_sigaction(); \
	__pstree_tasks_count = 1; \
	__children_tasks_count = 0; \
	__sync_id_counter = 1; \
	__saved_root_task_var = &(root_task_var); \
	__cr_runned = false; \
	task_waiter_init(&__tasks_sync_waiter)

#define pstt_fail() __pstt_fail(__FUNCTION__, __LINE__, "manual fail")

#define pstt_pass() do { \
	pstt_do_in_task(*__saved_root_task_var, pass()); \
	exit(0); \
} while(0)

#define pstt_check(assertion) do { \
	if (!(assertion)) \
		__pstt_fail(__FUNCTION__, __LINE__, "Test check failed: " #assertion); \
} while(0)

#define pstt_check_in_task(task_var, assertion) \
	pstt_do_in_task(task_var, pstt_check(assertion))

#define pstt_do_in_task(task_var, operation) do { \
	if (task_var == __CURRENT_TASK) \
		operation; \
} while(0)

#define __pstt_do_in_task_sync(task_var, operation) do { \
	pstt_check(!__cr_runned); \
	if (task_var == __CURRENT_TASK) { \
		operation; \
		size_t i; \
		for (i = 0; i <  __pstree_tasks_count - 1; i++) \
			task_waiter_complete(&__tasks_sync_waiter, __sync_id_counter); \
	} else { \
		task_waiter_wait4(&__tasks_sync_waiter, __sync_id_counter); \
	} \
	__sync_id_counter++; \
	pstt_check(__sync_id_counter < UINT32_MAX); \
} while(0)

#define pstt_barrier() do { \
	task_waiter_complete(&__tasks_sync_waiter, __sync_id_counter); \
	__sync_id_counter++; \
	__pstt_do_in_task_sync((*__saved_root_task_var), { \
		size_t i = 0; \
		for (i = 0; i < __pstree_tasks_count; i++) \
			task_waiter_wait4(&__tasks_sync_waiter, __sync_id_counter - 1); \
	}); \
} while(0)

#define pstt_do_in_task_sync(task_var, operation) do { \
	pstt_barrier(); \
	__pstt_do_in_task_sync(task_var, operation); \
} while(0)

#define pstt_fork_in_task(task_var_parent, task_var_child) \
	pstt_barrier(); \
	pid_t task_var_child = __NOT_CURRENT_TASK; \
	if (task_var_parent == __CURRENT_TASK) { \
		task_var_child = test_fork(); \
		if (task_var_child == __CURRENT_TASK) { \
			task_var_parent = __NOT_CURRENT_TASK; \
			__children_tasks_count = 0; \
		} else {\
			pstt_check(__children_tasks_count < __MAX_CHILDREN_TASKS); \
			pstt_check(task_var_child != -1); \
			__children_tasks[__children_tasks_count++] = task_var_child; \
		} \
	} \
	 __pstree_tasks_count++

/*
 * Starts CRIU C/R.
 * You can't call do_in_task_sync macro after resume from C/R
 * because ps tree processes don't run simultaneously after C/R.
 */
#define pstt_cr_start() do { \
	pstt_barrier(); \
	pstt_do_in_task((*__saved_root_task_var), test_daemon()); \
	test_waitsig(); \
	__cr_runned = true; \
	size_t i; \
	for (i = 0; i < __children_tasks_count; i++) { \
		pid_t pid = __children_tasks[i]; \
		pstt_check(kill(pid, SIGTERM) == 0); \
		waitpid(pid, NULL, 0); \
	} \
} while(0)

#endif /* PSTREE_ZDTMTST_H_ */
