#define _GNU_SOURCE
#include <fcntl.h>
#include "zdtmtst.h"
#include "pstree_zdtmtst.h"
#include "pstree_zdtmtst_wrappers.h"

const char *test_doc = "Test anonymous shared memory";
const char *test_author = "Alex Markelov";

/*                                         root
 *                            /                |                 \
 *                      task1                task7             task11
 *                     /     \             /     |    \              \
 *                 task2    task6      task8 task9 task10        task12
 *               /                                                      \
 *             task3                                                     task13
 *            /      \
 *           task4   task5
 */

int main(int argc, char **argv)
{
	pstree_test_init(root, argc, argv);

	/* ROOT */
	const size_t ROOT_MEM_SIZE = PAGE_SIZE * 30;

	void *tmp = MAP_FAILED;

	void *root_mem = pstree_test_mmap_in_task(root, NULL, ROOT_MEM_SIZE,
		PROT_WRITE | PROT_READ, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	pstree_test_fork_in_task(root, task1);
	pstree_test_fork_in_task(root, task7);
	pstree_test_fork_in_task(root, task11);

	pstree_test_munmap_in_task(root, root_mem, ROOT_MEM_SIZE);

	/* TASK1 */
	const size_t TASK1_MEM1_OFFSET = PAGE_SIZE;
	const size_t TASK1_MEM1_SIZE = PAGE_SIZE * 10;
	const size_t TASK1_MEM2_OFFSET = PAGE_SIZE * 29;
	const size_t TASK1_MEM2_SIZE = PAGE_SIZE;

	tmp = pstree_test_mmap_in_task(task1, NULL, TASK1_MEM1_SIZE,
			PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task1_mem1 = pstree_test_mremap_in_task(task1,
		(root_mem + TASK1_MEM1_OFFSET),
		TASK1_MEM1_SIZE, TASK1_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task1, NULL,
		TASK1_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task1_mem2 = pstree_test_mremap_in_task(task1,
		(root_mem + TASK1_MEM2_OFFSET),
		TASK1_MEM2_SIZE, TASK1_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task1, task2);
	pstree_test_fork_in_task(task1, task6);
	pstree_test_munmap_in_task(task1, root_mem, TASK1_MEM1_OFFSET);
	pstree_test_munmap_in_task(task1, (root_mem +
			(TASK1_MEM1_OFFSET + TASK1_MEM1_SIZE)),
			(TASK1_MEM2_OFFSET -
					(TASK1_MEM1_OFFSET + TASK1_MEM1_SIZE)));

	/* TASK6 */

	const size_t TASK6_MEM1_OFFSET = PAGE_SIZE * 2;
	const size_t TASK6_MEM1_SIZE = PAGE_SIZE * 2;
	const size_t TASK6_MEM2_OFFSET = 0;
	const size_t TASK6_MEM2_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task6, NULL,
		TASK6_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task6_mem1 = pstree_test_mremap_in_task(task6,
		(task1_mem1 + TASK6_MEM1_OFFSET),
		TASK6_MEM1_SIZE, TASK6_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task6, NULL,
		TASK6_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task6_mem2 = pstree_test_mremap_in_task(task6,
			(task1_mem1 + TASK6_MEM2_OFFSET),
		TASK6_MEM2_SIZE, TASK6_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	/* TASK2 */
	const size_t TASK2_MEM1_OFFSET = 0;
	const size_t TASK2_MEM1_SIZE = 5 * PAGE_SIZE;
	const size_t TASK2_MEM2_OFFSET = 5 * PAGE_SIZE;
	const size_t TASK2_MEM2_SIZE = 5 * PAGE_SIZE;

	tmp = pstree_test_mmap_in_task(task2, NULL,
		TASK2_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task2_mem1 = pstree_test_mremap_in_task(task2,
		(task1_mem1 + TASK2_MEM1_OFFSET),
		TASK2_MEM1_SIZE, TASK2_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task2, NULL,
		TASK2_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task2_mem2 = pstree_test_mremap_in_task(task2,
		(task1_mem1 + TASK2_MEM2_OFFSET),
		TASK2_MEM2_SIZE, TASK2_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task2, task3);
	/* TASK3 */
	const size_t TASK3_MEM1_OFFSET = 0;
	const size_t TASK3_MEM1_SIZE = PAGE_SIZE * 2;
	const size_t TASK3_MEM2_OFFSET = PAGE_SIZE * 2;
	const size_t TASK3_MEM2_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task3, NULL,
		TASK3_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task3_mem1 = pstree_test_mremap_in_task(task3,
		(task2_mem1 + TASK3_MEM1_OFFSET),
		TASK3_MEM1_SIZE, TASK3_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task3, NULL,
		TASK3_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task3_mem2 = pstree_test_mremap_in_task(task3,
		(task2_mem1 + TASK3_MEM2_OFFSET),
		TASK3_MEM2_SIZE, TASK3_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task3, task4);
	/* TASK4 */
	const size_t TASK4_MEM1_OFFSET = PAGE_SIZE;
	const size_t TASK4_MEM1_SIZE = PAGE_SIZE * 2;
	const size_t TASK4_MEM2_OFFSET = PAGE_SIZE * 3;
	const size_t TASK4_MEM2_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task4, NULL,
		TASK4_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task4_mem1 = pstree_test_mremap_in_task(task4,
		(task2_mem2 + TASK4_MEM1_OFFSET),
		TASK4_MEM1_SIZE, TASK4_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task4, NULL,
		TASK4_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task4_mem2 = pstree_test_mremap_in_task(task4,
		(task2_mem2 + TASK4_MEM2_OFFSET),
		TASK4_MEM2_SIZE, TASK4_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);
	pstree_test_fork_in_task(task3, task5);
	/* TASK 5 */
	const size_t TASK5_MEM1_OFFSET = PAGE_SIZE;
	const size_t TASK5_MEM1_SIZE = PAGE_SIZE * 2;
	const size_t TASK5_MEM2_OFFSET = PAGE_SIZE * 3;
	const size_t TASK5_MEM2_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task5, NULL,
		TASK5_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task5_mem1 = pstree_test_mremap_in_task(task5,
		(task2_mem2 + TASK5_MEM1_OFFSET),
		TASK5_MEM1_SIZE, TASK5_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task5, NULL,
		TASK5_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task5_mem2 = pstree_test_mremap_in_task(task5,
		(task2_mem2 + TASK5_MEM2_OFFSET),
		TASK5_MEM2_SIZE, TASK5_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);
	/* TASK7 */
	const size_t TASK7_MEM1_OFFSET = PAGE_SIZE * 29;
	const size_t TASK7_MEM1_SIZE = PAGE_SIZE;
	const size_t TASK7_MEM2_OFFSET = PAGE_SIZE;
	const size_t TASK7_MEM2_SIZE = PAGE_SIZE * 10;

	tmp = pstree_test_mmap_in_task(task7, NULL,
		TASK7_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task7_mem1 = pstree_test_mremap_in_task(task7,
		(root_mem + TASK7_MEM1_OFFSET),
		TASK7_MEM1_SIZE, TASK7_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task7, NULL,
		TASK7_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task7_mem2 = pstree_test_mremap_in_task(task7,
		(root_mem + TASK7_MEM2_OFFSET),
		TASK7_MEM2_SIZE, TASK7_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task7, task8);
	/* TASK 8 */
	const size_t TASK8_MEM1_OFFSET = 0;
	const size_t TASK8_MEM1_SIZE = PAGE_SIZE * 2;
	const size_t TASK8_MEM2_OFFSET = PAGE_SIZE * 2;
	const size_t TASK8_MEM2_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task8, NULL,
		TASK8_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task8_mem1 = pstree_test_mremap_in_task(task8,
		(task7_mem2 + TASK8_MEM1_OFFSET),
		TASK8_MEM1_SIZE, TASK8_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	tmp = pstree_test_mmap_in_task(task8, NULL,
		TASK8_MEM2_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task8_mem2 = pstree_test_mremap_in_task(task8,
		(task7_mem2 + TASK8_MEM2_OFFSET),
		TASK8_MEM2_SIZE, TASK8_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task7, task9);
	/* TASK 9 */
	const size_t TASK9_MEM1_OFFSET = PAGE_SIZE * 6;
	const size_t TASK9_MEM1_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task9, NULL,
		TASK9_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task9_mem1 = pstree_test_mremap_in_task(task9,
		(task7_mem2 + TASK9_MEM1_OFFSET),
		TASK9_MEM1_SIZE, TASK9_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);
	pstree_test_fork_in_task(task7, task10);
	/* TASK 10 */
	const size_t TASK10_MEM1_OFFSET = PAGE_SIZE * 8;
	const size_t TASK10_MEM1_SIZE = PAGE_SIZE * 2;

	tmp = pstree_test_mmap_in_task(task10,
		NULL, TASK10_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task10_mem1 = pstree_test_mremap_in_task(task10,
		(task7_mem2 + TASK10_MEM1_OFFSET), TASK10_MEM1_SIZE,
		TASK10_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED, tmp);

	/* TASK11 */
	const size_t TASK11_MEM1_OFFSET = PAGE_SIZE * 27;
	const size_t TASK11_MEM1_SIZE = PAGE_SIZE * 3;

	tmp = pstree_test_mmap_in_task(task11, NULL,
		TASK11_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task11_mem1 = pstree_test_mremap_in_task(task11,
		(root_mem + TASK11_MEM1_OFFSET),
		TASK11_MEM1_SIZE, TASK11_MEM1_SIZE,
		MREMAP_MAYMOVE | MREMAP_FIXED,
		tmp);

	pstree_test_fork_in_task(task11, task12);
	/*TASK12 */
	char path[PATH_MAX];
	int fd;
	const size_t TASK12_MEM1_SIZE = PAGE_SIZE;
	void *task12_mem1 = MAP_FAILED;

	pstree_test_do_in_task_sync(task12, {
			snprintf(path, PATH_MAX,
				"/proc/self/map_files/%lx-%lx",
				(unsigned long) task11_mem1,
				(unsigned long) task11_mem1 + TASK11_MEM1_SIZE);
			fd = open(path, O_RDWR);
			pstree_test_check(fd != -1);
			task12_mem1 = mmap(NULL, TASK12_MEM1_SIZE,
				PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
			close(fd);
	});

	pstree_test_fork_in_task(task12, task13);

	pstree_test_munmap_in_task(task12, task11_mem1, TASK11_MEM1_SIZE);
	/* TASK13 */
	const size_t TASK13_MEM1_OFFSET = PAGE_SIZE * 2;
	const size_t TASK13_MEM1_SIZE = PAGE_SIZE;

	tmp = pstree_test_mmap_in_task(task13, NULL,
		TASK13_MEM1_SIZE, PROT_WRITE | PROT_READ,
		MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	void *task13_mem1 = pstree_test_mremap_in_task(task13,
		(task11_mem1 + TASK13_MEM1_OFFSET), TASK13_MEM1_SIZE,
		TASK13_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED, tmp);


	/* DATAGEN */
	pstree_test_datagen_in_task(task5, task5_mem1, TASK5_MEM1_SIZE);

	pstree_test_datagen_in_task(task4, task4_mem2, TASK4_MEM2_SIZE);

	pstree_test_datagen_in_task(task6, task6_mem1, TASK6_MEM1_SIZE);

	pstree_test_datagen_in_task(task5, task3_mem1, TASK3_MEM1_SIZE);

	pstree_test_datagen_in_task(task13, task13_mem1, TASK13_MEM1_SIZE);

	pstree_test_datagen_in_task(task13, task12_mem1, TASK12_MEM1_SIZE);
	/* CRIU START */
	pstree_test_cr_start();

	/* DATACHK */
	pstree_test_datachk_in_task(task4, task4_mem1, TASK4_MEM1_SIZE);
	pstree_test_datachk_in_task(task5, task5_mem1, TASK5_MEM1_SIZE);
	pstree_test_datachk_in_task(task9, task9_mem1, TASK9_MEM1_SIZE);

	pstree_test_datachk_in_task(task4, task4_mem2, TASK4_MEM2_SIZE);
	pstree_test_datachk_in_task(task5, task5_mem2, TASK5_MEM2_SIZE);
	pstree_test_datachk_in_task(task10, task10_mem1, TASK10_MEM1_SIZE);

	pstree_test_datachk_in_task(task6, task6_mem1, TASK6_MEM1_SIZE);
	pstree_test_datachk_in_task(task3, task3_mem2, TASK3_MEM2_SIZE);
	pstree_test_datachk_in_task(task4, task3_mem2, TASK3_MEM2_SIZE);
	pstree_test_datachk_in_task(task5, task3_mem2, TASK3_MEM2_SIZE);
	pstree_test_datachk_in_task(task8, task8_mem2, TASK8_MEM2_SIZE);

	pstree_test_datachk_in_task(task6, task6_mem2, TASK6_MEM2_SIZE);
	pstree_test_datachk_in_task(task3, task3_mem1, TASK3_MEM1_SIZE);
	pstree_test_datachk_in_task(task4, task3_mem1, TASK3_MEM1_SIZE);
	pstree_test_datachk_in_task(task5, task3_mem1, TASK3_MEM1_SIZE);
	pstree_test_datachk_in_task(task8, task8_mem1, TASK8_MEM1_SIZE);

	pstree_test_datachk_in_task(task13, task13_mem1, TASK13_MEM1_SIZE);
	pstree_test_datachk_in_task(task11, (task11_mem1 + TASK13_MEM1_OFFSET),
			TASK13_MEM1_SIZE);
	pstree_test_datachk_in_task(task5, task1_mem2, TASK1_MEM2_SIZE);
	pstree_test_datachk_in_task(task4, task1_mem2, TASK1_MEM2_SIZE);
	pstree_test_datachk_in_task(task3, task1_mem2, TASK1_MEM2_SIZE);
	pstree_test_datachk_in_task(task2, task1_mem2, TASK1_MEM2_SIZE);
	pstree_test_datachk_in_task(task1, task1_mem2, TASK1_MEM2_SIZE);
	pstree_test_datachk_in_task(task7, task7_mem1, TASK7_MEM1_SIZE);
	pstree_test_datachk_in_task(task8, task7_mem1, TASK7_MEM1_SIZE);
	pstree_test_datachk_in_task(task9, task7_mem1, TASK7_MEM1_SIZE);
	pstree_test_datachk_in_task(task10, task7_mem1, TASK7_MEM1_SIZE);

	pstree_test_datachk_in_task(task12, task12_mem1, TASK12_MEM1_SIZE);
	pstree_test_pass();
}
