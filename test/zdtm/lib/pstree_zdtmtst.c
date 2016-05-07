#include "pstree_zdtmtst.h"

pid_t __children_tasks[__MAX_CHILDREN_TASKS];
size_t __children_tasks_count;

static void kill_children_and_exit(int exit_status)
{
	pstree_test_kill_children();
	exit(exit_status);
}

static void sigchld_handler(int sig, siginfo_t *siginfo, void *context)
{
	if (siginfo->si_status)
		kill_children_and_exit(siginfo->si_status);
}

static void sigusr_handler(int sig)
{
	kill_children_and_exit(1);
}

void pstree_test_kill_children(void)
{
	size_t i;

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
		kill_children_and_exit(1);
	}
	signal(SIGUSR2, sigusr_handler);
}
