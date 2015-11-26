#include <stdio.h>
#include <stdlib.h>

/* malloc that:
   - tracks size of object (if it's already big enough, don't need to realloc)
   - allocates in increasing powers of 2

   === Blocks allocated with emalloc() must be freed with efree() ===

   testing:
     gcc -DTEST -g3 -std=c99 -Wall -Wextra -Wno-unused-parameter -Wpedantic -o emalloc emalloc.c
   building:
     gcc -O2 -shared -fPIC -o emalloc.so emalloc.c

*/

void *emalloc(void *, size_t);
void  efree(void *);

void *emalloc(void *buf, size_t size)
{
	size_t sizet_size = sizeof(size_t);
	int sizet_bits = (sizet_size * 8) - 1; // all bits set less than size
	void *newptr; 
	/* pointer arithmetic is illegal on void pointers so I cheat */
	char *origptr = NULL;
	/* newsize = next highest power of 2 
	   this is shamelessly lifted from the stanford bithacks page, 
	   and modified (by me) to handle integers of 8 byte size in addition to 4 byte.
	   This change uses additional operations but size_t can be 4 or 8 bytes.
	   I did avoid the use of conditionals/branching, at least.
	   http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */

	size_t newsize = size - 1;
	/* 4 or 8 byte */
	newsize |= newsize >> 1;
	newsize |= newsize >> 2;
	newsize |= newsize >> 4;
	newsize |= newsize >> 8;
	newsize |= newsize >> 16;
	/* 8 byte, if necessary */
	newsize |= newsize >> (32 & sizet_bits);
	newsize++;

	//printf("size %lu, newsize %lu\n", size, newsize);

	if (buf)
	{
		origptr = (char *)buf - sizet_size;
		size_t origsize = *(size_t *)origptr;
		//printf("buf exists, current size %lu, reallocating\n", origsize);
		if (origsize >= size)
		{
			//printf("wanted %lu, already had %lu, not changed\n", size, origsize);
			return buf;
		}
		else
		{
			//printf("wanted %lu, already had %lu, not enough!\n", size, origsize);
			;
		}

		newptr = realloc(origptr, newsize + sizet_size);
	}
	else
		newptr = malloc(newsize + sizet_size);

	if (newptr)
	{
		*(size_t *)newptr = newsize;
		buf = (char *)newptr + sizet_size;
	}
	else
	{
		//printf("wanted %lu, but malloc FAILED\n", size);
		return NULL;
	}

	//printf("wanted %lu, returning buffer of size %lu\n", size, newsize);
	return buf;
}

void efree(void *buf)
{
	/* pointer arithmetic is illegal on void pointers so I cheat. */
	char *foo = (char *)buf - sizeof(size_t);
	free(foo);
	return;
}

#ifdef TEST
int main(int argc, char**argv)
{
	char *buf = NULL;
	for (int i = 1; i < 1000000; i += 10000)
	{
		buf = emalloc(buf, i);
		sprintf(buf, "%d", i);
		size_t bsize = *(size_t *)(buf - sizeof(size_t));
		printf("emalloc(): returned %p, buf %s, buffer size %lu\n", buf, buf, bsize);
	}
	efree(buf);
	buf = NULL;
}
#endif
