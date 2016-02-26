#define _GNU_SOURCE
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
#include "zdtmtst.h"

#define MEM_SIZE (1L << 30)
#define PROC_COUNT 1
#define ITERATIONS 32

const char *test_doc    = "Test shared memory";
const char *test_author = "Andrew Vagin <avagin@openvz.org";

int run_child(void **shmems, task_waiter_t *t, int child_num)
{
	uint32_t mem, crc_before, crc_after;

	for (mem = 0; mem < PROC_COUNT; ++mem) {
		int i, pg;
		for (i = 0; i < ITERATIONS; ++i) {
			pg = mrand48() % (MEM_SIZE / PAGE_SIZE);
			crc_before = ~0;
			datagen(shmems[mem] + pg * PAGE_SIZE, PAGE_SIZE, &crc_before);
		}
	}

	task_waiter_complete(t, 1);
	datasum(shmems[child_num], MEM_SIZE, &crc_before);

	test_waitsig();

	datasum(shmems[child_num], MEM_SIZE, &crc_after);

	return crc_before != crc_after;
}

int main(int argc, char ** argv)
{
	pid_t *pids;
	int i, status;
	task_waiter_t *waiters;
	void **shmems;

	pids = malloc(PROC_COUNT * sizeof(*pids));
	if (!pids)
		goto mem_fail;

	waiters = malloc(PROC_COUNT * sizeof(*waiters));
	if (!waiters)
		goto mem_fail;

	shmems = malloc(PROC_COUNT * sizeof(*shmems));
	if (!shmems)
		goto mem_fail;

	test_init(argc, argv);

	for (i = 0; i < PROC_COUNT; ++i)
		task_waiter_init(&waiters[i]);

	for (i = 0; i < PROC_COUNT; ++i) {
		shmems[i] = mmap(NULL, MEM_SIZE, PROT_WRITE | PROT_READ,
				MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		if (shmems[i] == MAP_FAILED)
			goto mem_fail;
	}

	for (i = 0; i < PROC_COUNT; ++i) {
		pids[i] = test_fork();
		if (pids[i] < 0) {
			goto err;
		} else if (pids[i] == 0) {
			if (run_child(shmems, &waiters[i], i))
				return 1;
			return 0;
		}
	}
	for (i = 0; i < PROC_COUNT; ++i)
		task_waiter_wait4(&waiters[i], 1);

	test_daemon();
	test_waitsig();

	for (i = 0; i < PROC_COUNT; ++i) {
		kill(pids[i], SIGTERM);
		wait(&status);
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status))
				goto err;
		} else
			goto err;
	}

	pass();

	return 0;
err:
	if (waitpid(-1, NULL, WNOHANG) == 0)
		for (i = 0; i < PROC_COUNT; ++i) {
			kill(pids[i], SIGTERM);
			wait(NULL);
		}
mem_fail:
	return 1;
}
