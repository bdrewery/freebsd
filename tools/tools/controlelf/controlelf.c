/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2019 The FreeBSD Foundation.
 *
 * This software was developed by Bora Ozarslan under sponsorship from
 * the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/param.h>
#include <sys/elf_common.h>
#include <sys/stat.h>

#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <getopt.h>
#include <libelf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_elftc.h"

__FBSDID("$FreeBSD$");

static bool convert_to_feature_val(char*, u_int32_t*);
static bool edit_file_features(Elf *, int, int, char*);
static bool get_file_features(Elf *, int, int, u_int32_t *, u_int64_t *);
static void print_features(void);
static bool print_file_features(Elf *, int, int, char*);
static void usage(void);

struct ControlFeatures {
	const char *alias;
	unsigned long featureVal;
	const char *desc;
};

static struct ControlFeatures featurelist[] = {
	{ "aslr",	NT_FREEBSD_FCTL_ASLR_DISABLE,			"Disable ASLR" }
};

static struct option controlelf_longopts[] = {
	{ "help",	no_argument,	NULL,   'h' },
	{ NULL,		0,		NULL,	0   }
};

int
main(int argc, char **argv)
{
	GElf_Ehdr ehdr;
	Elf *elf;
	Elf_Kind kind;
	int ch, fd, listed, editfeatures, retval;
	char *features;

	listed = 0;
	editfeatures = 0;
	retval = 0;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EXIT_FAILURE, "elf_version error");

	while ((ch = getopt_long(argc, argv, "hle:", controlelf_longopts,
	    NULL)) != -1) {
		switch (ch) {
		case 'l':
			print_features();
			listed = 1;
			break;
		case 'e':
			features = optarg;
			editfeatures = 1;
			break;
		case 'h':
			usage();
			break;
		default:
			usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (!argc) {
		if (listed)
			exit(0);
		else {
			warnx("no file(s) specified");
			usage();
		}
	}

	while (argc) {
		elf = NULL;

		if ((fd = open(argv[0], editfeatures ? O_RDWR :
		    O_RDONLY, 0)) < 0) {
			warn("error opening file %s", argv[0]);
			retval = 1;
			goto fail;
		}

		if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
			warnx("elf_begin failed: %s", elf_errmsg(-1));
			retval = 1;
			goto fail;
		}

		if ((kind = elf_kind(elf)) != ELF_K_ELF) {
			if (kind == ELF_K_AR)
				warnx("file '%s' is an archive.", argv[0]);
			else
				warnx("file '%s' is not an ELF file.",
				    argv[0]);
			retval = 1;
			goto fail;
		}

		if (gelf_getehdr(elf, &ehdr) == NULL) {
			warnx("gelf_getehdr: %s", elf_errmsg(-1));
			retval = 1;
			goto fail;
		}

		if (!editfeatures) {
			if (!print_file_features(elf, ehdr.e_phnum, fd, argv[0])) {
				retval = 1;
				goto fail;
			}
		} else if (!edit_file_features(elf, ehdr.e_phnum, fd, features)) {
			retval = 1;
			goto fail;
		}
fail:
		if (elf != NULL)
			elf_end(elf);

		if (fd >= 0)
			close(fd);

		argc--;
		argv++;
	}

	return (retval);
}

#define	USAGE_MESSAGE	"\
Usage: %s [options] file...\n\
  Set or display the control features for an ELF object.\n\n\
  Supported options are:\n\
  -l                        List known control features.\n\
  -e [+-=]feature,list      Edit features from a comma separated list.\n\
  -h | --help               Print a usage message and exit.\n"

static void
usage(void)
{

	fprintf(stderr, USAGE_MESSAGE, ELFTC_GETPROGNAME());
	exit(1);
}

static bool
convert_to_feature_val(char* feature_str, u_int32_t* feature_val)
{
	char *feature_input, *feature;
	int i, len;
	u_int32_t input;
	bool add, set;

	add = set = false;
	input = 0;

	if (feature_str[0] == '+')
		add = true;
	else if (feature_str[0] == '=')
		set = true;
	else if (feature_str[0] != '-') {
		warnx("'%c' is not an operator. Use instead '+', '-', '='.",
		    feature_str[0]);
		return (false);
	}

	feature_input = feature_str + 1;
	len = nitems(featurelist);
	while ((feature = strsep(&feature_input, ",")) != NULL) {
		for (i = 0; i < len; ++i) {
			if (strcmp(featurelist[i].alias, feature) == 0) {
				input |= featurelist[i].featureVal;
				break;
			}
		}
		if (i == len) {
			warnx("%s is not a valid feature.", feature);
			return (false);
		}
	}

	if (add) {
		*feature_val |= input;
	} else if (set) {
		*feature_val = input;
	} else {
		*feature_val -= (*feature_val) & input;
	}
	return (true);
}

static bool
edit_file_features(Elf *elf, int phcount, int fd, char *val)
{
	u_int32_t features;
	u_int64_t off;

	if (!get_file_features(elf, phcount, fd, &features, &off)) {
		warnx("No control features note on the file.\n");
		return (false);
	}

	if (!convert_to_feature_val(val, &features))
		return (false);

	lseek(fd, off, SEEK_SET);
	write(fd, &features, sizeof(u_int32_t));
	return (true);
}

static void
print_features(void)
{
	size_t i;

	printf("Known features are:\n");
	for (i = 0; i < nitems(featurelist); ++i)
		printf("%s\t\t %s\n", featurelist[i].alias,
		    featurelist[i].desc);
}

static bool
print_file_features(Elf *elf, int phcount, int fd, char *filename) {
	u_int32_t features;
	unsigned long i;

	if (!get_file_features(elf, phcount, fd, &features, NULL)) {
		return (false);
	}

	printf("File '%s' features:\n", filename);
	for (i = 0; i < nitems(featurelist); ++i) {
		printf("%s\t\t'%s' is ", featurelist[i].alias,
		    featurelist[i].desc);

		if ((featurelist[i].featureVal & features) == 0)
			printf("un");

		printf("set.\n");
	}
	return (true);
}

static bool
get_file_features(Elf *elf, int phcount, int fd, u_int32_t *features, u_int64_t *off)
{
	GElf_Phdr phdr;
	Elf_Note note;
	unsigned long read_total;
	int namesz, descsz, i;
	char *name;
	ssize_t size;

	/*
	 * Go through each program header to find one that is of type PT_NOTE
	 * and has a note for feature control.
	 */
	for (i = 0; i < phcount; ++i) {
		if (gelf_getphdr(elf, i, &phdr) == NULL) {
			warnx("gelf_getphdr failed: %s", elf_errmsg(-1));
			return (false);
		}

		if (phdr.p_type != PT_NOTE)
			continue;

		if (lseek(fd, phdr.p_offset, SEEK_SET) < 0) {
			warn("lseek() failed:");
			return (false);
		}

		read_total = 0;
		while (read_total < phdr.p_filesz) {
			size = read(fd, &note, sizeof(note));
			if (size < (ssize_t)sizeof(note)) {
				warn("read() failed:");
				return (false);
			}
			read_total += sizeof(note);

			/*
			 * XXX: Name and descriptor are 4 byte aligned, however,
			 * 	the size given doesn't include the padding.
			 */
			namesz = roundup2(note.n_namesz, 4);
			name = malloc(namesz);
			if (name == NULL) {
				warn("malloc() failed.\n");
				return (false);
			}
			descsz = roundup2(note.n_descsz, 4);
			size = read(fd, name, namesz);
			read_total += namesz;

			if (note.n_namesz != 8 ||
			    strncmp("FreeBSD", name, 7) != 0 ||
			    note.n_type != NT_FREEBSD_FEATURE_CTL) {
				/* Not the right note. Skip the description */
				if (lseek(fd, descsz, SEEK_CUR) < 0) {
					warn("lseek() failed.\n");
					free(name);
					return (false);
				}
				read_total += descsz;
				free(name);
				continue;
			}

			if (note.n_descsz < sizeof(u_int32_t)) {
				warnx("Feature descriptor can't "
				    "be less than 4 bytes");
				free(name);
				return (false);
			}

			/*
			 * XXX: For now we look at only 4 bytes of the
			 * 	descriptor. This should respect descsz.
			 */
			if (note.n_descsz > sizeof(u_int32_t))
				warnx("Feature note is bigger than expected.");
			read(fd, features, sizeof(u_int32_t));
			if (off != NULL)
				*off = phdr.p_offset + read_total;
			free(name);
			return (true);
		}
	}

	warnx("Couldn't find a note header with control feature note.");
	return (false);
}
