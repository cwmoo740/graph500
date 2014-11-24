/* -*- mode: C; mode: folding; fill-column: 70; -*- */
/* Copyright 2010,  Georgia Institute of Technology, USA. */
/* See COPYING for license. */
#include "compat.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

#if !defined(MAP_HUGETLB)
#define MAP_HUGETLB 0
#endif
#if !defined(MAP_POPULATE)
#define MAP_POPULATE 0
#endif
#if !defined(MAP_NOSYNC)
#define MAP_NOSYNC 0
#endif

#if 0
/* Included in the generator. */
void *
xmalloc (size_t sz)
{
	void *out;
	if (!(out = malloc (sz))) {
		perror ("malloc failed");
		abort ();
	}
	return out;
}
#else
extern void *xmalloc(size_t);
#endif

#if defined(__MTA__)||defined(USE_MMAP_LARGE)||defined(USE_MMAP_LARGE_EXT)
#define MAX_LARGE 32
static int n_large_alloc = 0;
static struct {
	void * p;
	size_t sz;
	int fd;
}large_alloc[MAX_LARGE];

static int installed_handler = 0;
static void (*old_abort_handler)(int);

static void
exit_handler (void)
{
	int k;
	for (k = 0; k < n_large_alloc; ++k) {
		if (large_alloc[k].p)
		munmap (large_alloc[k].p, large_alloc[k].sz);
		if (large_alloc[k].fd >= 0)
		close (large_alloc[k].fd);
		large_alloc[k].p = NULL;
		large_alloc[k].fd = -1;
	}
}

static void
abort_handler (int passthrough)
{
	exit_handler ();
	if (old_abort_handler) old_abort_handler (passthrough);
}
#endif

#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS MAP_ANON
#endif

void *
xmalloc_large(size_t sz) {
	return xmalloc(sz);
}

void xfree_large(void *p) {
	free(p);
}

void *
xmalloc_large_ext(size_t sz) {
	return xmalloc_large(sz);
}

/*
 void
 mark_large_unused (void *p)
 {
 #if !defined(__MTA__)
 int k;
 for (k = 0; k < n_large_alloc; ++k)
 if (p == large_alloc[k].p)
 posix_madvise (large_alloc[k].p, large_alloc[k].sz, POSIX_MADV_DONTNEED);
 #endif
 }

 void
 mark_large_willuse (void *p)
 {
 #if !defined(__MTA__)
 int k;
 for (k = 0; k < n_large_alloc; ++k)
 if (p == large_alloc[k].p)
 posix_madvise (large_alloc[k].p, large_alloc[k].sz, POSIX_MADV_WILLNEED);
 #endif
 }
 */
