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
#include <stdarg.h>

#include "zdtmtst.h"
#include "pstree_zdtmtst.h"
#include "pstree_zdtmtst_wrappers.h"

const char *test_doc = "Test shared memory";
const char *test_author = "Alex Markelov";

/*
 *
 *                        [            ROOT                    ]
 * 	                      /            |                       \
 * 	                 CHILD1		       [    CHILD7  ]           CHILD11
 *                  /     \			  /      |       \                \
 *            CHILD2      CHILD6	 CHILD8  CHILD9   CHILD10      CHILD12
 *           /                                                          \
 *     CHILD3                                                           CHILD13
 *    /      \
 *   CHILD4  CHILD5
 */

int main(int argc, char ** argv) {
	INIT(ROOT, argc, argv);

	/* ROOT */
	const size_t ROOT_MEM_SIZE = PAGE_SIZE * 30;

	void * root_mem;
	void * reserved;

	MMAP(ROOT, root_mem, NULL, ROOT_MEM_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	FORK(ROOT, CHILD1);
	FORK(ROOT, CHILD7);
	FORK(ROOT, CHILD11);

	MUNMAP(ROOT, root_mem, ROOT_MEM_SIZE);

	/* CHILD1 */
	const size_t CHILD1_MEM1_OFFSET = PAGE_SIZE;
	const size_t CHILD1_MEM1_SIZE   = PAGE_SIZE * 10;
	const size_t CHILD1_MEM2_OFFSET = PAGE_SIZE * 29;
	const size_t CHILD1_MEM2_SIZE   = PAGE_SIZE;

	void * child1_mem1 = MAP_FAILED;
	MMAP(CHILD1, reserved, NULL, CHILD1_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD1, child1_mem1, (root_mem + CHILD1_MEM1_OFFSET),
			CHILD1_MEM1_SIZE, CHILD1_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	void * child1_mem2 = MAP_FAILED;

	MMAP(CHILD1, reserved, NULL, CHILD1_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD1, child1_mem2, (root_mem + CHILD1_MEM2_OFFSET),
			CHILD1_MEM2_SIZE, CHILD1_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD1, CHILD2);
	FORK(CHILD1, CHILD6);
	MUNMAP(CHILD1, root_mem, CHILD1_MEM1_OFFSET);
	MUNMAP(CHILD1, (root_mem + (CHILD1_MEM1_OFFSET + CHILD1_MEM1_SIZE)),
			(CHILD1_MEM2_OFFSET - (CHILD1_MEM1_OFFSET + CHILD1_MEM1_SIZE)));

	/* CHILD6 */

	const size_t CHILD6_MEM1_OFFSET = PAGE_SIZE * 2;
	const size_t CHILD6_MEM1_SIZE   = PAGE_SIZE * 2;
	const size_t CHILD6_MEM2_OFFSET = 0;
	const size_t CHILD6_MEM2_SIZE   = PAGE_SIZE * 2;

	void * child6_mem1 = MAP_FAILED, *child6_mem2 = MAP_FAILED;
	MMAP(CHILD6, reserved, NULL, CHILD6_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD6, child6_mem1, (child1_mem1 + CHILD6_MEM1_OFFSET),
			CHILD6_MEM1_SIZE, CHILD6_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);
	MMAP(CHILD6, reserved, NULL, CHILD6_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD6, child6_mem2, (child1_mem1 + CHILD6_MEM2_OFFSET),
			CHILD6_MEM2_SIZE, CHILD6_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	/* CHILD2 */
	const size_t CHILD2_MEM1_OFFSET = 0;
	const size_t CHILD2_MEM1_SIZE   = 5 * PAGE_SIZE;
	const size_t CHILD2_MEM2_OFFSET = 5 * PAGE_SIZE;
	const size_t CHILD2_MEM2_SIZE   = 5 * PAGE_SIZE;

	void * child2_mem1, *child2_mem2;

	MMAP(CHILD2, reserved, NULL, CHILD2_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD2, child2_mem1, (child1_mem1 + CHILD2_MEM1_OFFSET),
			CHILD2_MEM1_SIZE, CHILD2_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD2, reserved, NULL, CHILD2_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD2, child2_mem2, (child1_mem1 + CHILD2_MEM2_OFFSET),
			CHILD2_MEM2_SIZE, CHILD2_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD2, CHILD3);
	/* CHILD3 */
	const size_t CHILD3_MEM1_OFFSET = 0;
	const size_t CHILD3_MEM1_SIZE   = PAGE_SIZE * 2;
	const size_t CHILD3_MEM2_OFFSET = PAGE_SIZE * 2;
	const size_t CHILD3_MEM2_SIZE   = PAGE_SIZE * 2;

	void * child3_mem1 = MAP_FAILED, *child3_mem2 = MAP_FAILED;

	MMAP(CHILD3, reserved, NULL, CHILD3_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD3, child3_mem1, (child2_mem1 + CHILD3_MEM1_OFFSET),
			CHILD3_MEM1_SIZE, CHILD3_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD3, reserved, NULL, CHILD3_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD3, child3_mem2, (child2_mem1 + CHILD3_MEM2_OFFSET),
			CHILD3_MEM2_SIZE, CHILD3_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD3, CHILD4);
	/* CHILD4 */
	const size_t CHILD4_MEM1_OFFSET = PAGE_SIZE;
	const size_t CHILD4_MEM1_SIZE   = PAGE_SIZE * 2;
	const size_t CHILD4_MEM2_OFFSET = PAGE_SIZE * 3;
	const size_t CHILD4_MEM2_SIZE   = PAGE_SIZE * 2;

	void * child4_mem1 = MAP_FAILED, *child4_mem2 = MAP_FAILED;

	MMAP(CHILD4, reserved, NULL, CHILD4_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD4, child4_mem1, (child2_mem2 + CHILD4_MEM1_OFFSET),
			CHILD4_MEM1_SIZE, CHILD4_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD4, reserved, NULL, CHILD4_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD4, child4_mem2, (child2_mem2 + CHILD4_MEM2_OFFSET),
			CHILD4_MEM2_SIZE, CHILD4_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);
	FORK(CHILD3, CHILD5);
	/* CHILD 5 */
	const size_t CHILD5_MEM1_OFFSET = PAGE_SIZE;
	const size_t CHILD5_MEM1_SIZE   = PAGE_SIZE * 2;
	const size_t CHILD5_MEM2_OFFSET = PAGE_SIZE * 3;
	const size_t CHILD5_MEM2_SIZE   = PAGE_SIZE * 2;

	void * child5_mem1 = MAP_FAILED, *child5_mem2 = MAP_FAILED;

	MMAP(CHILD5, reserved, NULL, CHILD5_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD5, child5_mem1, (child2_mem2 + CHILD5_MEM1_OFFSET),
			CHILD5_MEM1_SIZE, CHILD5_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD5, reserved, NULL, CHILD5_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD5, child5_mem2, (child2_mem2 + CHILD5_MEM2_OFFSET),
			CHILD5_MEM2_SIZE, CHILD5_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);
	/* CHILD7 */
	const size_t CHILD7_MEM1_OFFSET = PAGE_SIZE * 29;
	const size_t CHILD7_MEM1_SIZE   = PAGE_SIZE;
	const size_t CHILD7_MEM2_OFFSET = PAGE_SIZE;
	const size_t CHILD7_MEM2_SIZE   = PAGE_SIZE * 10;

	void * child7_mem1 = MAP_FAILED, *child7_mem2 = MAP_FAILED;

	MMAP(CHILD7, reserved, NULL, CHILD7_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD7, child7_mem1, (root_mem + CHILD7_MEM1_OFFSET),
			CHILD7_MEM1_SIZE, CHILD7_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD7, reserved, NULL, CHILD7_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD7, child7_mem2, (root_mem + CHILD7_MEM2_OFFSET),
			CHILD7_MEM2_SIZE, CHILD7_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD7, CHILD8);
	/* CHILD 8 */
	const size_t CHILD8_MEM1_OFFSET = 0;
	const size_t CHILD8_MEM1_SIZE   = PAGE_SIZE * 2;
	const size_t CHILD8_MEM2_OFFSET = PAGE_SIZE * 2;
	const size_t CHILD8_MEM2_SIZE   = PAGE_SIZE * 2;

	void * child8_mem1 = MAP_FAILED, *child8_mem2 = MAP_FAILED;

	MMAP(CHILD8, reserved, NULL, CHILD8_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD8, child8_mem1, (child7_mem2 + CHILD8_MEM1_OFFSET),
			CHILD8_MEM1_SIZE, CHILD8_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	MMAP(CHILD8, reserved, NULL, CHILD8_MEM2_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD8, child8_mem2, (child7_mem2 + CHILD8_MEM2_OFFSET),
			CHILD8_MEM2_SIZE, CHILD8_MEM2_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD7, CHILD9);
	/* CHILD 9 */
	const size_t CHILD9_MEM1_OFFSET = PAGE_SIZE * 6;
	const size_t CHILD9_MEM1_SIZE   = PAGE_SIZE * 2;

	void * child9_mem1 = MAP_FAILED;

	MMAP(CHILD9, reserved, NULL, CHILD9_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD9, child9_mem1, (child7_mem2 + CHILD9_MEM1_OFFSET),
			CHILD9_MEM1_SIZE, CHILD9_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);
	FORK(CHILD7, CHILD10);
	/* CHILD 10 */
	const size_t CHILD10_MEM1_OFFSET = PAGE_SIZE * 8;
	const size_t CHILD10_MEM1_SIZE   = PAGE_SIZE * 2;

	void * child10_mem1 = MAP_FAILED;

	MMAP(CHILD10, reserved, NULL, CHILD10_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD10, child10_mem1, (child7_mem2 + CHILD10_MEM1_OFFSET),
			CHILD10_MEM1_SIZE, CHILD10_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	/* CHILD11 */
	const size_t CHILD11_MEM1_OFFSET = PAGE_SIZE * 27;
	const size_t CHILD11_MEM1_SIZE   = PAGE_SIZE * 3;

	void * child11_mem1 = MAP_FAILED;

	MMAP(CHILD11, reserved, NULL, CHILD11_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD11, child11_mem1, (root_mem + CHILD11_MEM1_OFFSET),
			CHILD11_MEM1_SIZE, CHILD11_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);

	FORK(CHILD11, CHILD12);
	/*CHILD12 */
	FORK(CHILD12, CHILD13);
	MUNMAP(CHILD12, child11_mem1, CHILD11_MEM1_SIZE);
	/* CHILD13 */
	const size_t CHILD13_MEM1_OFFSET = PAGE_SIZE * 2;
	const size_t CHILD13_MEM1_SIZE   = PAGE_SIZE;

	void * child13_mem1 = MAP_FAILED;

	MMAP(CHILD13, reserved, NULL, CHILD13_MEM1_SIZE, PROT_WRITE | PROT_READ,
			MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	MREMAP(CHILD13, child13_mem1, (child11_mem1 + CHILD13_MEM1_OFFSET),
			CHILD13_MEM1_SIZE, CHILD13_MEM1_SIZE, MREMAP_MAYMOVE | MREMAP_FIXED,
			reserved);
	/* DATAGEN */
	uint32_t crc;
	DATAGEN_Z(CHILD5, crc, child5_mem1, CHILD5_MEM1_SIZE);

	DATAGEN_Z(CHILD4, crc, child4_mem2, CHILD4_MEM2_SIZE);

	DATAGEN_Z(CHILD6, crc, child6_mem1, CHILD6_MEM1_SIZE);

	DATAGEN_Z(CHILD5, crc, child3_mem1, CHILD3_MEM1_SIZE);

	DATAGEN_Z(CHILD13, crc, child13_mem1, CHILD13_MEM1_SIZE);

	/* CRIU START */
	CR_START;

	/* DATACHK */
	DATACHK_Z_CHECK(CHILD4, crc, child4_mem1, CHILD4_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD5, crc, child5_mem1, CHILD5_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD9, crc, child9_mem1, CHILD9_MEM1_SIZE);

	DATACHK_Z_CHECK(CHILD4, crc, child4_mem2, CHILD4_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD5, crc, child5_mem2, CHILD5_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD10, crc, child10_mem1, CHILD10_MEM1_SIZE);

	DATACHK_Z_CHECK(CHILD6, crc, child6_mem1, CHILD6_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD3, crc, child3_mem2, CHILD3_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD4, crc, child3_mem2, CHILD3_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD5, crc, child3_mem2, CHILD3_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD8, crc, child8_mem2, CHILD8_MEM2_SIZE);

	DATACHK_Z_CHECK(CHILD6, crc, child6_mem2, CHILD6_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD3, crc, child3_mem1, CHILD3_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD4, crc, child3_mem1, CHILD3_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD5, crc, child3_mem1, CHILD3_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD8, crc, child8_mem1, CHILD8_MEM1_SIZE);

	DATACHK_Z_CHECK(CHILD13, crc, child13_mem1, CHILD13_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD11, crc, (child11_mem1 + CHILD13_MEM1_OFFSET),
			CHILD13_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD5, crc, child1_mem2, CHILD1_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD4, crc, child1_mem2, CHILD1_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD3, crc, child1_mem2, CHILD1_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD2, crc, child1_mem2, CHILD1_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD1, crc, child1_mem2, CHILD1_MEM2_SIZE);
	DATACHK_Z_CHECK(CHILD7, crc, child7_mem1, CHILD7_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD8, crc, child7_mem1, CHILD7_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD9, crc, child7_mem1, CHILD7_MEM1_SIZE);
	DATACHK_Z_CHECK(CHILD10, crc, child7_mem1, CHILD7_MEM1_SIZE);

	PASS;
}
