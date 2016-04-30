#include "pstree_zdtmtst.h"
#include "zdtmtst.h"

pid_t __children_tasks[__MAX_CHILDREN_TASKS];
size_t  __pstree_tasks_count;
size_t __children_tasks_count;
unsigned int  __sync_id_counter;
task_waiter_t __tasks_sync_waiter;
pid_t *__saved_root_task_var;

static void kill_and_exit(int exit_status)
{
	pstree_test_kill_tree();
	exit(exit_status);
}

static void sigchld_handler(int sig, siginfo_t *siginfo, void *context)
{
	if (siginfo->si_status)
		kill_and_exit(siginfo->si_status);
}

static void sigusr_handler(int sig)
{
	kill_and_exit(1);
}

void pstree_test_kill_tree(void)
{
	int i;

	for (i = 0; i < __children_tasks_count; i++)
		kill(__children_tasks[i], SIGUSR2);
}

void pstree_test_init_sigaction(void)
{
	struct sigaction sa = {
		.sa_sigaction = &sigchld_handler,
		.sa_flags	= SA_SIGINFO | SA_RESTART,
	};
	if (sigaction(SIGCHLD, &sa, NULL)) {
		pr_perror("Can't reset SIGCHLD handler for pstree test");
		kill_and_exit(1);
	}
	signal(SIGUSR2, sigusr_handler);
}
