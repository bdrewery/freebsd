#-
# Copyright (c) 2019 The FreeBSD Foundation
#
# Portions of this software were developed by Björn Zeeb
# under sponsorship from the FreeBSD Foundation.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# $FreeBSD$
#

#include <sys/bus.h>
#include <machine/bus.h>

INTERFACE sdio;

#
# READ DIRECT (1byte)
#
METHOD int read_direct {
	device_t dev;
	uint8_t fn;
	uint32_t addr;
	uint8_t *val;
};

#
# WRITE DIRECT (1byte)
#
METHOD int write_direct {
	device_t dev;
	uint8_t fn;
	uint32_t addr;
	uint8_t val;
};

#
# READ EXTENDED
#
METHOD int read_extended {
	device_t dev;
	uint8_t fn;
	uint32_t addr;
	uint32_t size;
	uint8_t *buffer;
	bool incaddr;
};

#
# WRITE EXTENDED
#
METHOD int write_extended {
	device_t dev;
	uint8_t fn;
	uint32_t addr;
	uint32_t size;
	uint8_t *buffer;
	bool incaddr;
};

# end
