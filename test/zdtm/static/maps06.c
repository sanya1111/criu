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
#include "map_f.h"
#include <stdarg.h>

#define MEM_SIZE (1L << 30)
#define MEM_OFFSET (1L << 29)
#define MEM_OFFSET2 (MEM_SIZE - PAGE_SIZE)
#define MEM_OFFSET3 (20 * PAGE_SIZE)

const char *test_doc	= "Test shared memory";
const char *test_author	= "Andrew Vagin <avagin@openvz.org";

int main(int argc, char ** argv)
{
	test_init(argc, argv);
	INIT(ROOT);






	void * m, * p = MAP_FAILED,
			*p2 = MAP_FAILED,
			*p3 = MAP_FAILED;

	MMAP(ROOT, m, NULL, PAGE_SIZE * 2 , PROT_WRITE | PROT_READ,
					MAP_SHARED | MAP_ANONYMOUS, -1, 0);

	FORK(ROOT, CHILD);
	FORK(ROOT, CHILD2);
	MREMAP(CHILD2, p2, (char *) m , PAGE_SIZE *2, PAGE_SIZE *2 , MREMAP_MAYMOVE);
	FORK(CHILD2, CHILD3);



	MREMAP(CHILD, p, (char *)m , PAGE_SIZE + 1 , PAGE_SIZE + 1, MREMAP_MAYMOVE);
	MUNMAP(ROOT, m, PAGE_SIZE);
	MREMAP(CHILD3, p3, p2 + PAGE_SIZE, 2, 4, MREMAP_MAYMOVE);

	SYNC(CHILD, (((char*)p)[PAGE_SIZE])=12);


	CR_START(ROOT);







	CHECK_EQ(CHILD3, (((char*)p3)[0]), 12);


	FINISH;
}
