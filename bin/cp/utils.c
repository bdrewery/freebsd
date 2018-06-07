/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
#if 0
static char sccsid[] = "@(#)utils.c	8.3 (Berkeley) 4/1/94";
#endif
#endif /* not lint */
#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <sys/acl.h>
#include <sys/param.h>
#include <sys/stat.h>
#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
#include <sys/mman.h>
#endif

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fts.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "extern.h"

#define	cp_pct(x, y)	((y == 0) ? 0 : (int)(100.0 * (x) / (y)))

/*
 * Memory strategy threshold, in pages: if physmem is larger then this, use a 
 * large buffer.
 */
#define PHYSPAGES_THRESHOLD (32*1024)

/* Maximum buffer size in bytes - do not allow it to grow larger than this. */
#define BUFSIZE_MAX (2*1024*1024)

/*
 * Small (default) buffer size in bytes. It's inefficient for this to be
 * smaller than MAXPHYS.
 */
#define BUFSIZE_SMALL (MAXPHYS)

static char *buf = NULL;
static size_t bufsize;

static void
prepare_buf(void)
{
	if (buf == NULL) {
		/*
		 * Note that buf and bufsize are static. If
		 * malloc() fails, it will fail at the start
		 * and not copy only some files. 
		 */ 
		if (sysconf(_SC_PHYS_PAGES) > PHYSPAGES_THRESHOLD)
			bufsize = MIN(BUFSIZE_MAX, MAXPHYS * 8);
		else
			bufsize = BUFSIZE_SMALL;
		buf = malloc(bufsize);
		if (buf == NULL)
			err(1, "Not enough memory");
	}
}

static void
find_zero_region(const char *p, size_t len, size_t blksize, size_t *zrbeg,
    size_t *zrend)
{
	const char *beg, *end, *pend;

	if (blksize == 0)
		abort();

	/*
	 * The algorithm below is optimized not to inspect every byte of the
	 * input by skipping ahead blksize bytes at a time when it finds a
	 * mismatch, and then backtracking on a potential match.
	 */

	pend = p + len;
	end = p;
	for (;;) {
		/* Wind up, find a zero. */
		for (; end < pend && *end != 0; end += blksize)
			;
		if (end >= pend)
			break;

		/* So, end must be in bounds, and *end must be zero. */

		beg = end;
		if (beg > p) {
			/* Search backward for beginning of region. */
			do {
				beg--;
			} while (*beg == 0);
			beg++; /* Advance again to first zero byte. */
		}
		/* Search forward for end of region. */
		do {
			end++;
		} while (end < pend && *end == 0);

		/* Return this region if it was big enough. */
		if ((size_t)(end - beg) >= blksize) {
			*zrbeg = beg - p;
			*zrend = end - p;
			return;
		}

		end += blksize - 1;
	}
	/* Return an empty region positioned at the end. */
	*zrbeg = len;
	*zrend = len;
}

#if 0
static _Thread_local sigjmp_buf find_zero_region_mmap_env;

static void
find_zero_region_mmap_sigbus(int sigbus)
{
	siglongjmp(find_zero_region_mmap_env, EFAULT);
}

/*
 * A wrapper for find_zero_region which is safe to use on a file-backed mmap,
 * even if the file is truncated.  In that case, it returns EFAULT.
 *
 * Check ucontext(3), might be better than longjmp?
 */
static int
find_zero_region_mmap(const char *p, size_t len, size_t blksize, size_t *zrbeg,
    size_t *zrend)
{
	struct sigaction act = {}, oact;
	int error;

	act.sa_handler = find_zero_region_mmap_sigbus;
	act.sa_flags = SA_RESETHAND;
	act.sa_
	if (sigaction(SIGBUS, &act, &oact) != 0) {
		error = errno;
		goto out;
	}

	error = sigsetjmp(env, 0);
	if (error != 0)
		goto out;

	find_zero_region(p, len, blksize, zrbeg, zrend);
 out:
	return error;
}
#endif

static int
do_write(int to_fd, const char *p, size_t wresid, off_t *wtotal,
    const char *from_path, const char *to_path, off_t expected)
{
	ssize_t wcount;

	for (wcount = 0; wresid > 0; wresid -= wcount, p += wcount) {
		wcount = write(to_fd, p, wresid);
		if (wcount <= 0)
			break;
		*wtotal += wcount;
		if (info) {
			info = 0;
			(void)fprintf(stderr, "%s -> %s %3d%%\n",
			    from_path, to_path, cp_pct(*wtotal, expected));
		}
		if ((size_t)wcount >= wresid)
			break;
	}
	return (wcount < 0 || (size_t)wcount != wresid);
}

static int
copy_file_real(const FTSENT *entp, int from_fd, int to_fd)
{
	struct stat to_st;
	const struct stat *fs;
	off_t next, rpos, wpos;
	size_t blksize;
	ssize_t rcount;
	int rval;
	bool can_iseek, can_oseek, in_sparse_tail, owe_otrunc;
#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
	char *p;
	off_t mapbase;
	size_t maplen, nmaplen, mapoff;
	bool can_mmap, owe_iseek;
#endif

	fs = entp->fts_statp;
	rpos = wpos = 0;
	blksize = 512;
	rval = 0;
	can_iseek = true;
	can_oseek = true;
	owe_otrunc = false; /* Have seeked, but need write or ftruncate. */
#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
	p = MAP_FAILED;
	can_mmap = S_ISREG(fs->st_mode) && fs->st_size > 0;
	owe_iseek = false;
#endif

	/* Optimize empty files. */
	if (fs->st_size == 0)
		goto out;

	/*
	 * The general idea is try a few optimizations, but if they fail to
	 * fall back to read(2)/write(2).  The optimizations are:
	 *  - Use lseek SEEK_DATA to skip sparse regions in the input.
	 *  - Use lseek to skip sparse regions in the output.
	 *  - Use mmap to avoid a copy.
	 *
	 * rpos is the position to read from the from_fd.  It is usually the
	 * same as what the seek offset would be.  Likewise wpos is the
	 * position to write to the to_fd.  When rpos > wpos, there is either
	 * buffered data or a gap of zeros in between.
	 */

	if (fstat(to_fd, &to_st) == 0) {
		can_oseek = S_ISREG(to_st.st_mode) || S_ISBLK(to_st.st_mode) ||
		    S_ISCHR(to_st.st_mode);
		if (to_st.st_blksize > 0)
			blksize = to_st.st_blksize;
	}

	for (;;) {
		/* Try to skip ahead to the next non-sparse region. */
		if (can_iseek) {
			next = lseek(from_fd, rpos, SEEK_DATA);
			if (next < 0 && errno == ENXIO) {
				in_sparse_tail = true;
			} else if (next < 0) {
				can_iseek = false;
			} else {
				rpos = next;
				in_sparse_tail = false;
#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
				owe_iseek = false;
#endif
			}
		}
		/* If we seeked on the input, try seeking on the output. */
		if (can_oseek && rpos > wpos) {
			if (lseek(to_fd, rpos, SEEK_SET) != rpos) {
				can_oseek = false;
			} else {
				wpos = rpos;
				owe_otrunc = true;
			}
		}
		/* Write residual zero region normally, if oseek failed. */
		if (rpos > wpos) {
			prepare_buf();
			memset(buf, 0, MIN(bufsize, (uintmax_t)(rpos - wpos)));
			do {
				if (do_write(to_fd, buf,
				    MIN(bufsize, (uintmax_t)(rpos - wpos)),
				    &wpos, entp->fts_path, to.p_path,
				    fs->st_size))
					goto fail;
			} while (rpos > wpos);
			owe_otrunc = false;
		}

#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
		/*
		 * Mmap and write.  This is really a minor hack, but it wins
		 * some CPU back.  Some filesystems, such as smbnetfs, don't
		 * support mmap, so this is a best-effort attempt.
		 *
		 * Using mmap(2) here is tricky due to possible races with
		 * truncate.  Mapping a page past EOF is not allowed and
		 * results in ENXIO.  Even after establishing a mapping, a
		 * truncate may occur and invalidate it, causing EFAULT on
		 * write (or SIGBUS if we were to touch it, which we don't).
		 * When that occurs, fall back to read.
		 */
		if (can_mmap && !in_sparse_tail && wpos < fs->st_size) {
			mapbase = rounddown(rpos, PAGE_SIZE);
			mapoff = rpos - mapbase;
			// XXX explain remapping.
			// XXX at least on INVARIANTS kernel, no difference
			// from plain unmap at end.
			nmaplen = MIN(8 * 1024 * 1024, fs->st_size - mapbase);
			if (nmaplen != maplen) {
				munmap(p, maplen);
				maplen = nmaplen;
				p = mmap(NULL, maplen, PROT_READ,
				    MAP_PREFAULT_READ | MAP_SHARED, from_fd,
				    mapbase);
			} else {
				p = mmap(p, maplen, PROT_READ,
				    MAP_FIXED | MAP_PREFAULT_READ | MAP_SHARED,
				    from_fd, mapbase);
			}
			if (p == MAP_FAILED) {
				can_mmap = false;
				continue;
			}
			if (do_write(to_fd, p + mapoff, maplen - mapoff, &wpos,
			    entp->fts_path, to.p_path, fs->st_size)) {
				/*
				 * The write failed, but it may have partially
				 * succeeded.  Try to resync the seek offsets.
				 * If either fd is not seekable, we're stuck.
				 * XXX Avoid this by checking that both are
				 * seekable before attempting mmap?
				 * XXX Or do we always get a short write count,
				 * should we just continue?
				 */
				wpos = lseek(to_fd, 0, SEEK_CUR);
				if (wpos < rpos)
					goto fail;
				can_mmap = false;
			}
			if (wpos > rpos)
				owe_otrunc = false;
			if (wpos == fs->st_size)
				break;
			rpos = wpos;
			/* Lazily take care of the seek ourselves. */
			owe_iseek = true;
			//munmap(p, maplen);/* TODO MAP_FIXED unmap/remap */
			//p = MAP_FAILED;
			continue;
		}
		/* Need to seek the input before we issue a read(2). */
		if (owe_iseek) {
			owe_iseek = false;
			if (lseek(from_fd, rpos, SEEK_SET) != rpos)
				goto fail;
		}
#endif
		prepare_buf();
		rcount = read(from_fd, buf, bufsize);
		if (rcount == 0)
			break;
		else if (rcount < 0)
			goto fail;
		rpos += rcount;
		/*
		 * If we are in the sparse tail of a file, verify that we are
		 * reading zeros and try to seek ahead if so.  Unfortunately
		 * we can't determine the size of the sparse tail from lseek(2)
		 * and trying to determine the size with fstat and just
		 * truncating to there has a race that can wrongly populate
		 * to_fd with zeros at offsets where from_fd did not have them.
		 */
		for (size_t i = 0; i < (size_t)rcount; ) {
			size_t zrbeg, zrend;
			if (in_sparse_tail && can_oseek) {
				find_zero_region(buf + i, rcount - i, blksize,
				    &zrbeg, &zrend);
			} else
				zrbeg = zrend = rcount;
			if (zrbeg > 0) {
				if (do_write(to_fd, buf + i, zrbeg, &wpos,
				    entp->fts_path, to.p_path, fs->st_size))
					goto fail;
				owe_otrunc = false;
			}
			if (zrend > zrbeg) {
				next = wpos + (zrend - zrbeg);
				if (lseek(to_fd, next, SEEK_SET) != next) {
					can_oseek = false;
					break;
				}
				owe_otrunc = true;
			}
			i = zrend;
		}
	}
	if (owe_otrunc) {
		if (ftruncate(to_fd, wpos) < 0)
			goto fail;
	}
 out:
#ifdef VM_AND_BUFFER_CACHE_SYNCHRONIZED
	if (p != MAP_FAILED)
		munmap(p, maplen);
	/*
	 * XXX Old code failed if mmunmap failed, and had this comment.  Why?
	 * "Some systems don't unmap on close(2)."
	 * This is just a leak, right?  Anyway does munmap ever fail?
	 */
#endif
	return (rval);

 fail:
	warn("%s", to.p_path);
	rval = 1;
	goto out;
}

int
copy_file(const FTSENT *entp, int dne)
{
	struct stat *fs;
	int ch, checkch, from_fd, rval, to_fd;

	from_fd = to_fd = -1;
	if (!lflag && !sflag &&
	    (from_fd = open(entp->fts_path, O_RDONLY, 0)) == -1) {
		warn("%s", entp->fts_path);
		return (1);
	}

	fs = entp->fts_statp;

	/*
	 * If the file exists and we're interactive, verify with the user.
	 * If the file DNE, set the mode to be the from file, minus setuid
	 * bits, modified by the umask; arguably wrong, but it makes copying
	 * executables work right and it's been that way forever.  (The
	 * other choice is 666 or'ed with the execute bits on the from file
	 * modified by the umask.)
	 */
	if (!dne) {
#define YESNO "(y/n [n]) "
		if (nflag) {
			if (vflag)
				printf("%s not overwritten\n", to.p_path);
			rval = 1;
			goto done;
		} else if (iflag) {
			(void)fprintf(stderr, "overwrite %s? %s", 
			    to.p_path, YESNO);
			checkch = ch = getchar();
			while (ch != '\n' && ch != EOF)
				ch = getchar();
			if (checkch != 'y' && checkch != 'Y') {
				(void)fprintf(stderr, "not overwritten\n");
				rval = 1;
				goto done;
			}
		}

		if (fflag) {
			/*
			 * Remove existing destination file name create a new
			 * file.
			 */
			(void)unlink(to.p_path);
			if (!lflag && !sflag) {
				to_fd = open(to.p_path,
				    O_WRONLY | O_TRUNC | O_CREAT,
				    fs->st_mode & ~(S_ISUID | S_ISGID));
			}
		} else if (!lflag && !sflag) {
			/* Overwrite existing destination file name. */
			to_fd = open(to.p_path, O_WRONLY | O_TRUNC, 0);
		}
	} else if (!lflag && !sflag) {
		to_fd = open(to.p_path, O_WRONLY | O_TRUNC | O_CREAT,
		    fs->st_mode & ~(S_ISUID | S_ISGID));
	}

	if (!lflag && !sflag && to_fd == -1) {
		warn("%s", to.p_path);
		rval = 1;
		goto done;
	}

	rval = 0;

	if (!lflag && !sflag) {
		rval = copy_file_real(entp, from_fd, to_fd);
	} else if (lflag) {
		if (link(entp->fts_path, to.p_path)) {
			warn("%s", to.p_path);
			rval = 1;
		}
	} else if (sflag) {
		if (symlink(entp->fts_path, to.p_path)) {
			warn("%s", to.p_path);
			rval = 1;
		}
	}

	/*
	 * Don't remove the target even after an error.  The target might
	 * not be a regular file, or its attributes might be important,
	 * or its contents might be irreplaceable.  It would only be safe
	 * to remove it if we created it and its length is 0.
	 */

	if (!lflag && !sflag) {
		if (pflag && setfile(fs, to_fd))
			rval = 1;
		if (pflag && preserve_fd_acls(from_fd, to_fd) != 0)
			rval = 1;
		if (close(to_fd)) {
			warn("%s", to.p_path);
			rval = 1;
		}
	}

done:
	if (from_fd != -1)
		(void)close(from_fd);
	return (rval);
}

int
copy_link(const FTSENT *p, int exists)
{
	int len;
	char llink[PATH_MAX];

	if (exists && nflag) {
		if (vflag)
			printf("%s not overwritten\n", to.p_path);
		return (1);
	}
	if ((len = readlink(p->fts_path, llink, sizeof(llink) - 1)) == -1) {
		warn("readlink: %s", p->fts_path);
		return (1);
	}
	llink[len] = '\0';
	if (exists && unlink(to.p_path)) {
		warn("unlink: %s", to.p_path);
		return (1);
	}
	if (symlink(llink, to.p_path)) {
		warn("symlink: %s", llink);
		return (1);
	}
	return (pflag ? setfile(p->fts_statp, -1) : 0);
}

int
copy_fifo(struct stat *from_stat, int exists)
{

	if (exists && nflag) {
		if (vflag)
			printf("%s not overwritten\n", to.p_path);
		return (1);
	}
	if (exists && unlink(to.p_path)) {
		warn("unlink: %s", to.p_path);
		return (1);
	}
	if (mkfifo(to.p_path, from_stat->st_mode)) {
		warn("mkfifo: %s", to.p_path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1) : 0);
}

int
copy_special(struct stat *from_stat, int exists)
{

	if (exists && nflag) {
		if (vflag)
			printf("%s not overwritten\n", to.p_path);
		return (1);
	}
	if (exists && unlink(to.p_path)) {
		warn("unlink: %s", to.p_path);
		return (1);
	}
	if (mknod(to.p_path, from_stat->st_mode, from_stat->st_rdev)) {
		warn("mknod: %s", to.p_path);
		return (1);
	}
	return (pflag ? setfile(from_stat, -1) : 0);
}

int
setfile(struct stat *fs, int fd)
{
	static struct timespec tspec[2];
	struct stat ts;
	int rval, gotstat, islink, fdval;

	rval = 0;
	fdval = fd != -1;
	islink = !fdval && S_ISLNK(fs->st_mode);
	fs->st_mode &= S_ISUID | S_ISGID | S_ISVTX |
	    S_IRWXU | S_IRWXG | S_IRWXO;

	tspec[0] = fs->st_atim;
	tspec[1] = fs->st_mtim;
	if (fdval ? futimens(fd, tspec) : utimensat(AT_FDCWD, to.p_path, tspec,
	    islink ? AT_SYMLINK_NOFOLLOW : 0)) {
		warn("utimensat: %s", to.p_path);
		rval = 1;
	}
	if (fdval ? fstat(fd, &ts) :
	    (islink ? lstat(to.p_path, &ts) : stat(to.p_path, &ts)))
		gotstat = 0;
	else {
		gotstat = 1;
		ts.st_mode &= S_ISUID | S_ISGID | S_ISVTX |
		    S_IRWXU | S_IRWXG | S_IRWXO;
	}
	/*
	 * Changing the ownership probably won't succeed, unless we're root
	 * or POSIX_CHOWN_RESTRICTED is not set.  Set uid/gid before setting
	 * the mode; current BSD behavior is to remove all setuid bits on
	 * chown.  If chown fails, lose setuid/setgid bits.
	 */
	if (!gotstat || fs->st_uid != ts.st_uid || fs->st_gid != ts.st_gid)
		if (fdval ? fchown(fd, fs->st_uid, fs->st_gid) :
		    (islink ? lchown(to.p_path, fs->st_uid, fs->st_gid) :
		    chown(to.p_path, fs->st_uid, fs->st_gid))) {
			if (errno != EPERM) {
				warn("chown: %s", to.p_path);
				rval = 1;
			}
			fs->st_mode &= ~(S_ISUID | S_ISGID);
		}

	if (!gotstat || fs->st_mode != ts.st_mode)
		if (fdval ? fchmod(fd, fs->st_mode) :
		    (islink ? lchmod(to.p_path, fs->st_mode) :
		    chmod(to.p_path, fs->st_mode))) {
			warn("chmod: %s", to.p_path);
			rval = 1;
		}

	if (!gotstat || fs->st_flags != ts.st_flags)
		if (fdval ?
		    fchflags(fd, fs->st_flags) :
		    (islink ? lchflags(to.p_path, fs->st_flags) :
		    chflags(to.p_path, fs->st_flags))) {
			warn("chflags: %s", to.p_path);
			rval = 1;
		}

	return (rval);
}

int
preserve_fd_acls(int source_fd, int dest_fd)
{
	acl_t acl;
	acl_type_t acl_type;
	int acl_supported = 0, ret, trivial;

	ret = fpathconf(source_fd, _PC_ACL_NFS4);
	if (ret > 0 ) {
		acl_supported = 1;
		acl_type = ACL_TYPE_NFS4;
	} else if (ret < 0 && errno != EINVAL) {
		warn("fpathconf(..., _PC_ACL_NFS4) failed for %s", to.p_path);
		return (1);
	}
	if (acl_supported == 0) {
		ret = fpathconf(source_fd, _PC_ACL_EXTENDED);
		if (ret > 0 ) {
			acl_supported = 1;
			acl_type = ACL_TYPE_ACCESS;
		} else if (ret < 0 && errno != EINVAL) {
			warn("fpathconf(..., _PC_ACL_EXTENDED) failed for %s",
			    to.p_path);
			return (1);
		}
	}
	if (acl_supported == 0)
		return (0);

	acl = acl_get_fd_np(source_fd, acl_type);
	if (acl == NULL) {
		warn("failed to get acl entries while setting %s", to.p_path);
		return (1);
	}
	if (acl_is_trivial_np(acl, &trivial)) {
		warn("acl_is_trivial() failed for %s", to.p_path);
		acl_free(acl);
		return (1);
	}
	if (trivial) {
		acl_free(acl);
		return (0);
	}
	if (acl_set_fd_np(dest_fd, acl, acl_type) < 0) {
		warn("failed to set acl entries for %s", to.p_path);
		acl_free(acl);
		return (1);
	}
	acl_free(acl);
	return (0);
}

int
preserve_dir_acls(struct stat *fs, char *source_dir, char *dest_dir)
{
	acl_t (*aclgetf)(const char *, acl_type_t);
	int (*aclsetf)(const char *, acl_type_t, acl_t);
	struct acl *aclp;
	acl_t acl;
	acl_type_t acl_type;
	int acl_supported = 0, ret, trivial;

	ret = pathconf(source_dir, _PC_ACL_NFS4);
	if (ret > 0) {
		acl_supported = 1;
		acl_type = ACL_TYPE_NFS4;
	} else if (ret < 0 && errno != EINVAL) {
		warn("fpathconf(..., _PC_ACL_NFS4) failed for %s", source_dir);
		return (1);
	}
	if (acl_supported == 0) {
		ret = pathconf(source_dir, _PC_ACL_EXTENDED);
		if (ret > 0) {
			acl_supported = 1;
			acl_type = ACL_TYPE_ACCESS;
		} else if (ret < 0 && errno != EINVAL) {
			warn("fpathconf(..., _PC_ACL_EXTENDED) failed for %s",
			    source_dir);
			return (1);
		}
	}
	if (acl_supported == 0)
		return (0);

	/*
	 * If the file is a link we will not follow it.
	 */
	if (S_ISLNK(fs->st_mode)) {
		aclgetf = acl_get_link_np;
		aclsetf = acl_set_link_np;
	} else {
		aclgetf = acl_get_file;
		aclsetf = acl_set_file;
	}
	if (acl_type == ACL_TYPE_ACCESS) {
		/*
		 * Even if there is no ACL_TYPE_DEFAULT entry here, a zero
		 * size ACL will be returned. So it is not safe to simply
		 * check the pointer to see if the default ACL is present.
		 */
		acl = aclgetf(source_dir, ACL_TYPE_DEFAULT);
		if (acl == NULL) {
			warn("failed to get default acl entries on %s",
			    source_dir);
			return (1);
		}
		aclp = &acl->ats_acl;
		if (aclp->acl_cnt != 0 && aclsetf(dest_dir,
		    ACL_TYPE_DEFAULT, acl) < 0) {
			warn("failed to set default acl entries on %s",
			    dest_dir);
			acl_free(acl);
			return (1);
		}
		acl_free(acl);
	}
	acl = aclgetf(source_dir, acl_type);
	if (acl == NULL) {
		warn("failed to get acl entries on %s", source_dir);
		return (1);
	}
	if (acl_is_trivial_np(acl, &trivial)) {
		warn("acl_is_trivial() failed on %s", source_dir);
		acl_free(acl);
		return (1);
	}
	if (trivial) {
		acl_free(acl);
		return (0);
	}
	if (aclsetf(dest_dir, acl_type, acl) < 0) {
		warn("failed to set acl entries on %s", dest_dir);
		acl_free(acl);
		return (1);
	}
	acl_free(acl);
	return (0);
}

void
usage(void)
{

	(void)fprintf(stderr, "%s\n%s\n",
	    "usage: cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file target_file",
	    "       cp [-R [-H | -L | -P]] [-f | -i | -n] [-alpsvx] "
	    "source_file ... "
	    "target_directory");
	exit(EX_USAGE);
}
