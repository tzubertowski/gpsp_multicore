
#include <stdint.h>
#include <stddef.h>

#include "memmap.h"

// The JIT cache buffer is allocated via mmap (or win equivalent) so that it
// can be RWX. On top of that, we need the bufer to be "close" to the text
// segment, so that we can perform jumps between the two code blocks.
// Android and some other platforms discourage the usage of sections in the
// binary (ie. on-disk ELF) that are marked as executable and writtable for
// security reasons. Therefore we prefer to use mmap even though it can be
// tricky to map correctly.

// To map a block close to the code, we take the function address as a proxy
// of the text section address, and try to map the cache next to it. This is
// an iterative process of trial and error that is hopefully successful.

// x86-64 has a +/- 2GB offset requirement.
// ARM64 has a +/-128MB offset requirement.
// ARM32 has a +/- 32MB offset requirement (gpsp does not require this).
// MIPS requires blocks to be within the same 256MB boundary (identical 4 MSB)

#ifdef MMAP_JIT_CACHE

#ifdef WIN32

	#include <windows.h>
	#include <io.h>

	void *map_jit_block(unsigned size) {
		unsigned i;
		uintptr_t base = (uintptr_t)(map_jit_block) & (~0xFFFFFULL);
		for (i = 0; i < 256; i++) { 
			int offset = ((i & 1) ? 1 : -1) * (i >> 1) * 1024 * 1024;
			uintptr_t baddr = base + (intptr_t)offset;
			if (!baddr)
				continue;    // Do not map NULL, bad things happen :)

			void *p = VirtualAlloc((void*)baddr, size, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (p == (void*)baddr)
				return p;
			if (p)
				VirtualFree(p, 0, MEM_RELEASE);
		}
		return 0;
	}

	void unmap_jit_block(void *bufptr, unsigned size) {
		VirtualFree(bufptr, 0, MEM_RELEASE);
	}

#else

	#include <sys/mman.h>

	// Posix implementation
	void *map_jit_block(unsigned size) {
		unsigned i;
		uintptr_t base = (uintptr_t)(map_jit_block) & (~0xFFFFFULL);
		/* MIPS j instruction requires target within same 256MB region (top 4 bits
		 * of PC). Accept any mapping whose top 4 bits match the .so base. */
		uintptr_t region_mask = ~((uintptr_t)0x0FFFFFFFUL);
		uintptr_t base_region = base & region_mask;
		for (i = 0; i < 256; i++) {
			int offset = ((i & 1) ? 1 : -1) * (i >> 1) * 1024 * 1024;
			uintptr_t baddr = base + (intptr_t)offset;
			if (!baddr)
				continue;    // Do not map NULL, bad things happen :)

			void *p = mmap((void*)baddr, size, PROT_READ|PROT_WRITE|PROT_EXEC,
			                                   MAP_ANON|MAP_PRIVATE, -1, 0);
			if (p) {
				if (((uintptr_t)p & region_mask) == base_region)
					return p;
				munmap(p, size);
			}
		}
		/* Last resort: unconstrained allocation — JIT j-instructions may fault
		 * if outside region, but worth trying rather than returning NULL. */
		return mmap(NULL, size, PROT_READ|PROT_WRITE|PROT_EXEC,
		            MAP_ANON|MAP_PRIVATE, -1, 0);
	}

	void unmap_jit_block(void *bufptr, unsigned size) {
		munmap(bufptr, size);
	}

#endif /* WIN32 */

#endif /* MMAP_JIT_CACHE */

