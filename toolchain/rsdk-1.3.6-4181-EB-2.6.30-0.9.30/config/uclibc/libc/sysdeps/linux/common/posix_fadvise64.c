/* vi: set sw=4 ts=4: */
/*
 * posix_fadvise64() for uClibc
 * http://www.opengroup.org/onlinepubs/009695399/functions/posix_fadvise.html
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <unistd.h>
#include <errno.h>
#include <endian.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <fcntl.h>

#ifdef __UCLIBC_HAS_LFS__
#ifdef __NR_fadvise64_64

/* 64 bit implementation is cake ... or more like pie ... */
#if __WORDSIZE == 64

#define __NR_posix_fadvise64 __NR_fadvise64_64

#if defined INTERNAL_SYSCALL && ! defined __TARGET_powerpc__
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
  if (len != (off_t) len)
    return EOVERFLOW;
  INTERNAL_SYSCALL_DECL (err);
    int ret = INTERNAL_SYSCALL (posix_fadvise64, err, 5, fd,
                               __LONG_LONG_PAIR ((long) (offset >> 32),
                                                 (long) offset),
                               (off_t) len, advice);
  if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
    return 0;
  return INTERNAL_SYSCALL_ERRNO (ret, err);
}
#else
static __inline__ int syscall_posix_fadvise(int fd, off_t offset1, off_t offset2, off_t len, int advice);
#define __NR_syscall_posix_fadvise64 __NR_posix_fadvise64
_syscall4(int, syscall_posix_fadvise64, int, fd, __off64_t, offset,
          __off64_t, len, int, advice)
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	int ret = syscall_posix_fadvise64(fd, offset, len, advice);
	if (ret == -1)
		return errno;
	return ret;
}
#endif

/* 32 bit implementation is kind of a pita */
#elif __WORDSIZE == 32

#if defined INTERNAL_SYSCALL && ! defined __TARGET_powerpc__
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	INTERNAL_SYSCALL_DECL (err);
	int ret = INTERNAL_SYSCALL (fadvise64_64, err, 6, fd,
								__LONG_LONG_PAIR(offset >> 32, offset &  0xffffffff),
								__LONG_LONG_PAIR(len >> 32, len & 0xffffffff),
								advice);
	if (!INTERNAL_SYSCALL_ERROR_P (ret, err))
		return 0;
	return INTERNAL_SYSCALL_ERRNO (ret, err);
}
#elif defined _syscall6 /* workaround until everyone has _syscall6() */
#define __NR___syscall_fadvise64_64 __NR_fadvise64_64
static __inline__ _syscall6(int, __syscall_fadvise64_64, int, fd,
          unsigned long, high_offset, unsigned long, low_offset,
          unsigned long, high_len, unsigned long, low_len,
          int, advice)
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
	int ret = __syscall_fadvise64_64(fd,
	        __LONG_LONG_PAIR(offset >> 32, offset &  0xffffffff),
	        __LONG_LONG_PAIR(len >> 32, len & 0xffffffff),
	        advice);
	if (ret == -1)
		return errno;
	return ret;
}
#else
#warning neither INTERNAL_SYSCALL nor _syscall6 has been defined for your machine :(
#endif /* INTERNAL_SYSCALL */

#else
#error your machine is neither 32 bit or 64 bit ... it must be magical
#endif

#elif !defined __NR_fadvise64
/* This is declared as a strong alias in posix_fadvise.c if __NR_fadvise64
 * is defined.
 */
int posix_fadvise64(int fd, __off64_t offset, __off64_t len, int advice)
{
#warning This is not correct as far as SUSv3 is concerned.
	return ENOSYS;
}
#endif /* __NR_fadvise64_64 */
#endif /* __UCLIBC_HAS_LFS__ */
