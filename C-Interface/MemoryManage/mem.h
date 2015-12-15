#ifndef MEM_H_
#define MEM_H_

extern void *mem_resize(void *ptr, long nbytes) ;
extern void mem_free(void *ptr);
extern void *mem_calloc(long count, long nbytes);
#endif

