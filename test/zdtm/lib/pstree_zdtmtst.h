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
 *	HOWTO:
 *	1. Each test with this framework should contains body:
 *		1) pstt_init(root, argc, argv)
 *		2) ...(some commands)
 *		3) pstt_cr_start()
 *		4) ...(some commands)
 *		5) pstt_pass()
 *
 *	2. Describing each point of body:
 *		1) pstt_init calls test_init(argc, argv), defined in zdtmtst.h,
 *			than inits framework.
 *			first param is root task var (you can read about them
 *			at 3 paragraph)
 *		2) commands style:
 *			(1) you can write you own command -
 *				it will execute async in all tasks,
 *				opened before.
 *
 *			(2) do_in_task(my_task, my_command):
 *				my_comand will execute inside
 *				my_task, async with tasks, opened before
 *
 *			(3) do_in_task(my_task, {
 *					...commands...
 *				})
 *
 *				will execute commands inside my_task,
 *				async with tasks, opened before
 *
 *			(4) do_in_task_sync(my_task, ...)
 *				is same as do_in_tasks, but commands inside
 *				executes sync with tasks,
 *				opened before:
 *				other tasks are waiting,
 *				while commands inside my_task are running
 *
 *			(5) you can fail(pstt_fail)
 *				test or assert(pstt_check) or
 *				assert in task(pstt_check_in_task)
 *
 *				If assertion fails, it will call fail() in task,
 *				where it fails, and than kill process tree.
 *
 *			(6) you can fork new task from existing task, using
 *				pstt_fork_in_task(old_task, new_task)
 *
 *				(macro calls test_fork() inside old_task,
 *				sync with other tasks,
 *				and assigns new task var(new_task in our case)
 *				with new task)
 *
 *			(7) there are some helpful wrappers
 *				inside pstree_zdtmtst_wrappers.h
 *
 *		3) pstt_cr_start calls test_daemon in the root task
 *			and test_waitsig in all tasks.
 *			When each task wakes up, it wait's
 *			for all it's children finished and than continues
 *			execution
 *
 *		4) same as in 2), but you can't use *sync commands
 *
 *		5) finished point of test, each task exits here.
 *			If test pass,
 *			root task will call pass() before exit.
 *
 *	3. Task vars uses to identify task(or process).
 *		Simply, task vars is pid_t vars with name, you
 *		choose.
 *		Tasks vars creates with pstt_init() (root task)
 *		and pstt_fork_in_task() (other tasks)
 *		inside current scope.
 *
 *	4. All synchronizations is implemented with
 *		task_waiter
 *
 *	5. SIGCHLD and SIGUSR2 are overrided to provide
 *		assertions before pstt_cr_start() call
 */

#define __CURRENT_TASK 0
#define __NOT_CURRENT_TASK -2
#define __MAX_CHILDREN_TASKS 100

extern void pstt_init_sigaction(void);
extern void pstt_kill_children(void);

extern pid_t __children_tasks[__MAX_CHILDREN_TASKS];
extern size_t __children_tasks_count;
extern size_t __pstree_tasks_count;
extern unsigned int  __sync_id_counter;
extern task_waiter_t __tasks_sync_waiter;
extern pid_t *__saved_root_task_var;
extern bool __cr_runned;

#define pstt_init(root_task_var, argc, argv) \
	test_init(argc, argv); \
	pstt_init_sigaction(); \
	pid_t root_task_var = __CURRENT_TASK; \
	__pstree_tasks_count = 0; \
	__children_tasks_count = 0; \
	__sync_id_counter = 1; \
	__saved_root_task_var = &(root_task_var); \
	__cr_runned = false; \
	task_waiter_init(&__tasks_sync_waiter)

#define pstt_fail() {  \
	fail(); \
	pstt_kill_children();\
	exit(1); \
}

#define pstt_pass() { \
	pstt_do_in_task(*__saved_root_task_var, pass()); \
	exit(0); \
}

#define pstt_check(assertion) { \
	if (!(assertion)) \
		pstt_fail(); \
}

#define pstt_check_in_task(task_var, assertion) \
	pstt_do_in_task(task_var, pstt_check(assertion))

#define pstt_do_in_task(task_var, operation) { \
	if (task_var == __CURRENT_TASK) \
		operation; \
}

#define pstt_do_in_task_sync(task_var, operation) ({ \
	pstt_check(!__cr_runned); \
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
	pstt_check(__sync_id_counter < UINT32_MAX); \
})

#define pstt_fork_in_task(task_var_parent, task_var_child) \
	pid_t task_var_child = __NOT_CURRENT_TASK; \
	if (task_var_parent == __CURRENT_TASK) { \
		task_var_child = test_fork(); \
		if (task_var_child == __CURRENT_TASK) { \
			task_var_parent = __NOT_CURRENT_TASK; \
			__children_tasks_count = 0; \
		} else {\
			pstt_check(__children_tasks_count < \
			__MAX_CHILDREN_TASKS); \
			pstt_check(task_var_child != -1); \
			__children_tasks[__children_tasks_count++] = \
			task_var_child; \
		} \
	} \
	 __pstree_tasks_count++

/*
 * After this command enables async mode.
 * You shouldn't use *sync macroses after that.
 */
#define pstt_cr_start() { \
	pstt_do_in_task((*__saved_root_task_var), test_daemon()); \
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
