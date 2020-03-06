/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2017 Shunsuke Mie
 * Copyright (c) 2018 Leon Dang
 *
 * Function crc16 Copyright (c) 2017, Fedor Uporov 
 *     Obtained from function ext2_crc16() in sys/fs/ext2fs/ext2_csum.c
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * bhyve PCIe-NVMe device emulation.
 *
 * options:
 *  -s <n>,nvme,devpath,maxq=#,qsz=#,ioslots=#,sectsz=#,ser=A-Z,eui64=#
 *
 *  accepted devpath:
 *    /dev/blockdev
 *    /path/to/image
 *    ram=size_in_MiB
 *
 *  maxq    = max number of queues
 *  qsz     = max elements in each queue
 *  ioslots = max number of concurrent io requests
 *  sectsz  = sector size (defaults to blockif sector size)
 *  ser     = serial number (20-chars max)
 *  eui64   = IEEE Extended Unique Identifier (8 byte value)
 *
 */

/* TODO:
    - create async event for smart and log
    - intr coalesce
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>
#include <net/ieee_oui.h>

#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <machine/atomic.h>
#include <machine/vmm.h>
#include <vmmapi.h>

#include <dev/nvme/nvme.h>

#include "bhyverun.h"
#include "block_if.h"
#include "debug.h"
#include "pci_emul.h"


static int nvme_debug = 0;
#define	DPRINTF(params) if (nvme_debug) PRINTLN params
#define	WPRINTF(params) PRINTLN params

/* defaults; can be overridden */
#define	NVME_MSIX_BAR		4

#define	NVME_IOSLOTS		8

/* The NVMe spec defines bits 13:4 in BAR0 as reserved */
#define NVME_MMIO_SPACE_MIN	(1 << 14)

#define	NVME_QUEUES		16
#define	NVME_MAX_QENTRIES	2048

#define	NVME_PRP2_ITEMS		(PAGE_SIZE/sizeof(uint64_t))
#define	NVME_MAX_BLOCKIOVS	512

/* This is a synthetic status code to indicate there is no status */
#define NVME_NO_STATUS		0xffff
#define NVME_COMPLETION_VALID(c)	((c).status != NVME_NO_STATUS)

/* helpers */

/* Convert a zero-based value into a one-based value */
#define ONE_BASED(zero)		((zero) + 1)
/* Convert a one-based value into a zero-based value */
#define ZERO_BASED(one)		((one)  - 1)

/* Encode number of SQ's and CQ's for Set/Get Features */
#define NVME_FEATURE_NUM_QUEUES(sc) \
	(ZERO_BASED((sc)->num_squeues) & 0xffff) | \
	(ZERO_BASED((sc)->num_cqueues) & 0xffff) << 16;

#define	NVME_DOORBELL_OFFSET	offsetof(struct nvme_registers, doorbell)

enum nvme_controller_register_offsets {
	NVME_CR_CAP_LOW = 0x00,
	NVME_CR_CAP_HI  = 0x04,
	NVME_CR_VS      = 0x08,
	NVME_CR_INTMS   = 0x0c,
	NVME_CR_INTMC   = 0x10,
	NVME_CR_CC      = 0x14,
	NVME_CR_CSTS    = 0x1c,
	NVME_CR_NSSR    = 0x20,
	NVME_CR_AQA     = 0x24,
	NVME_CR_ASQ_LOW = 0x28,
	NVME_CR_ASQ_HI  = 0x2c,
	NVME_CR_ACQ_LOW = 0x30,
	NVME_CR_ACQ_HI  = 0x34,
};

enum nvme_cmd_cdw11 {
	NVME_CMD_CDW11_PC  = 0x0001,
	NVME_CMD_CDW11_IEN = 0x0002,
	NVME_CMD_CDW11_IV  = 0xFFFF0000,
};

#define	NVME_CQ_INTEN	0x01
#define	NVME_CQ_INTCOAL	0x02

struct nvme_completion_queue {
	struct nvme_completion *qbase;
	uint32_t	size;
	uint16_t	tail; /* nvme progress */
	uint16_t	head; /* guest progress */
	uint16_t	intr_vec;
	uint32_t	intr_en;
	pthread_mutex_t	mtx;
};

struct nvme_submission_queue {
	struct nvme_command *qbase;
	uint32_t	size;
	uint16_t	head; /* nvme progress */
	uint16_t	tail; /* guest progress */
	uint16_t	cqid; /* completion queue id */
	int		busy; /* queue is being processed */
	int		qpriority;
};

enum nvme_storage_type {
	NVME_STOR_BLOCKIF = 0,
	NVME_STOR_RAM = 1,
};

struct pci_nvme_blockstore {
	enum nvme_storage_type type;
	void		*ctx;
	uint64_t	size;
	uint32_t	sectsz;
	uint32_t	sectsz_bits;
	uint64_t	eui64;
};

struct pci_nvme_ioreq {
	struct pci_nvme_softc *sc;
	struct pci_nvme_ioreq *next;
	struct nvme_submission_queue *nvme_sq;
	uint16_t	sqid;

	/* command information */
	uint16_t	opc;
	uint16_t	cid;
	uint32_t	nsid;

	uint64_t	prev_gpaddr;
	size_t		prev_size;

	/*
	 * lock if all iovs consumed (big IO);
	 * complete transaction before continuing
	 */
	pthread_mutex_t	mtx;
	pthread_cond_t	cv;

	struct blockif_req io_req;

	/* pad to fit up to 512 page descriptors from guest IO request */
	struct iovec	iovpadding[NVME_MAX_BLOCKIOVS-BLOCKIF_IOV_MAX];
};

struct pci_nvme_softc {
	struct pci_devinst *nsc_pi;

	pthread_mutex_t	mtx;

	struct nvme_registers regs;

	struct nvme_namespace_data  nsdata;
	struct nvme_controller_data ctrldata;
	struct nvme_error_information_entry err_log;
	struct nvme_health_information_page health_log;
	struct nvme_firmware_page fw_log;

	struct pci_nvme_blockstore nvstore;

	uint16_t	max_qentries;	/* max entries per queue */
	uint32_t	max_queues;	/* max number of IO SQ's or CQ's */
	uint32_t	num_cqueues;
	uint32_t	num_squeues;

	struct pci_nvme_ioreq *ioreqs;
	struct pci_nvme_ioreq *ioreqs_free; /* free list of ioreqs */
	uint32_t	pending_ios;
	uint32_t	ioslots;
	sem_t		iosemlock;

	/*
	 * Memory mapped Submission and Completion queues
	 * Each array includes both Admin and IO queues
	 */
	struct nvme_completion_queue *compl_queues;
	struct nvme_submission_queue *submit_queues;

	/* controller features */
	uint32_t	intr_coales_aggr_time;   /* 0x08: uS to delay intr */
	uint32_t	intr_coales_aggr_thresh; /* 0x08: compl-Q entries */
	uint32_t	async_ev_config;         /* 0x0B: async event config */
};


static void pci_nvme_io_partial(struct blockif_req *br, int err);

/* Controller Configuration utils */
#define	NVME_CC_GET_EN(cc) \
	((cc) >> NVME_CC_REG_EN_SHIFT & NVME_CC_REG_EN_MASK)
#define	NVME_CC_GET_CSS(cc) \
	((cc) >> NVME_CC_REG_CSS_SHIFT & NVME_CC_REG_CSS_MASK)
#define	NVME_CC_GET_SHN(cc) \
	((cc) >> NVME_CC_REG_SHN_SHIFT & NVME_CC_REG_SHN_MASK)
#define	NVME_CC_GET_IOSQES(cc) \
	((cc) >> NVME_CC_REG_IOSQES_SHIFT & NVME_CC_REG_IOSQES_MASK)
#define	NVME_CC_GET_IOCQES(cc) \
	((cc) >> NVME_CC_REG_IOCQES_SHIFT & NVME_CC_REG_IOCQES_MASK)

#define	NVME_CC_WRITE_MASK \
	((NVME_CC_REG_EN_MASK << NVME_CC_REG_EN_SHIFT) | \
	 (NVME_CC_REG_IOSQES_MASK << NVME_CC_REG_IOSQES_SHIFT) | \
	 (NVME_CC_REG_IOCQES_MASK << NVME_CC_REG_IOCQES_SHIFT))

#define	NVME_CC_NEN_WRITE_MASK \
	((NVME_CC_REG_CSS_MASK << NVME_CC_REG_CSS_SHIFT) | \
	 (NVME_CC_REG_MPS_MASK << NVME_CC_REG_MPS_SHIFT) | \
	 (NVME_CC_REG_AMS_MASK << NVME_CC_REG_AMS_SHIFT))

/* Controller Status utils */
#define	NVME_CSTS_GET_RDY(sts) \
	((sts) >> NVME_CSTS_REG_RDY_SHIFT & NVME_CSTS_REG_RDY_MASK)

#define	NVME_CSTS_RDY	(1 << NVME_CSTS_REG_RDY_SHIFT)

/* Completion Queue status word utils */
#define	NVME_STATUS_P	(1 << NVME_STATUS_P_SHIFT)
#define	NVME_STATUS_MASK \
	((NVME_STATUS_SCT_MASK << NVME_STATUS_SCT_SHIFT) |\
	 (NVME_STATUS_SC_MASK << NVME_STATUS_SC_SHIFT))

static __inline void
cpywithpad(char *dst, size_t dst_size, const char *src, char pad)
{
	size_t len;

	len = strnlen(src, dst_size);
	memset(dst, pad, dst_size);
	memcpy(dst, src, len);
}

static __inline void
pci_nvme_status_tc(uint16_t *status, uint16_t type, uint16_t code)
{

	*status &= ~NVME_STATUS_MASK;
	*status |= (type & NVME_STATUS_SCT_MASK) << NVME_STATUS_SCT_SHIFT |
		(code & NVME_STATUS_SC_MASK) << NVME_STATUS_SC_SHIFT;
}

static __inline void
pci_nvme_status_genc(uint16_t *status, uint16_t code)
{

	pci_nvme_status_tc(status, NVME_SCT_GENERIC, code);
}

static __inline void
pci_nvme_toggle_phase(uint16_t *status, int prev)
{

	if (prev)
		*status &= ~NVME_STATUS_P;
	else
		*status |= NVME_STATUS_P;
}

static void
pci_nvme_init_ctrldata(struct pci_nvme_softc *sc)
{
	struct nvme_controller_data *cd = &sc->ctrldata;

	cd->vid = 0xFB5D;
	cd->ssvid = 0x0000;

	cpywithpad((char *)cd->mn, sizeof(cd->mn), "bhyve-NVMe", ' ');
	cpywithpad((char *)cd->fr, sizeof(cd->fr), "1.0", ' ');

	/* Num of submission commands that we can handle at a time (2^rab) */
	cd->rab   = 4;

	/* FreeBSD OUI */
	cd->ieee[0] = 0x58;
	cd->ieee[1] = 0x9c;
	cd->ieee[2] = 0xfc;

	cd->mic = 0;

	cd->mdts = 9;	/* max data transfer size (2^mdts * CAP.MPSMIN) */

	cd->ver = 0x00010300;

	cd->oacs = 1 << NVME_CTRLR_DATA_OACS_FORMAT_SHIFT;
	cd->acl = 2;
	cd->aerl = 4;

	cd->lpa = 0;	/* TODO: support some simple things like SMART */
	cd->elpe = 0;	/* max error log page entries */
	cd->npss = 1;	/* number of power states support */

	/* Warning Composite Temperature Threshold */
	cd->wctemp = 0x0157;

	cd->sqes = (6 << NVME_CTRLR_DATA_SQES_MAX_SHIFT) |
	    (6 << NVME_CTRLR_DATA_SQES_MIN_SHIFT);
	cd->cqes = (4 << NVME_CTRLR_DATA_CQES_MAX_SHIFT) |
	    (4 << NVME_CTRLR_DATA_CQES_MIN_SHIFT);
	cd->nn = 1;	/* number of namespaces */

	cd->fna = 0x03;

	cd->power_state[0].mp = 10;
}

/*
 * Calculate the CRC-16 of the given buffer
 * See copyright attribution at top of file
 */
static uint16_t
crc16(uint16_t crc, const void *buffer, unsigned int len)
{
	const unsigned char *cp = buffer;
	/* CRC table for the CRC-16. The poly is 0x8005 (x16 + x15 + x2 + 1). */
	static uint16_t const crc16_table[256] = {
		0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
		0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
		0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
		0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
		0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
		0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
		0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
		0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
		0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
		0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
		0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
		0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
		0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
		0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
		0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
		0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
		0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
		0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
		0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
		0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
		0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
		0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
		0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
		0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
		0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
		0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
		0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
		0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
		0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
		0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
		0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
		0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
	};

	while (len--)
		crc = (((crc >> 8) & 0xffU) ^
		    crc16_table[(crc ^ *cp++) & 0xffU]) & 0x0000ffffU;
	return crc;
}

static void
pci_nvme_init_nsdata(struct pci_nvme_softc *sc,
    struct nvme_namespace_data *nd, uint32_t nsid,
    uint64_t eui64)
{

	nd->nsze = sc->nvstore.size / sc->nvstore.sectsz;
	nd->ncap = nd->nsze;
	nd->nuse = nd->nsze;

	/* Get LBA and backstore information from backing store */
	nd->nlbaf = 0; /* NLBAF is a 0's based value (i.e. 1 LBA Format) */
	nd->flbas = 0;

	/* Create an EUI-64 if user did not provide one */
	if (eui64 == 0) {
		char *data = NULL;

		asprintf(&data, "%s%u%u%u", vmname, sc->nsc_pi->pi_bus,
		    sc->nsc_pi->pi_slot, sc->nsc_pi->pi_func);

		if (data != NULL) {
			eui64 = OUI_FREEBSD_NVME_LOW | crc16(0, data, strlen(data));
			free(data);
		}
		eui64 = (eui64 << 16) | (nsid & 0xffff);
	}
	be64enc(nd->eui64, eui64);

	/* LBA data-sz = 2^lbads */
	nd->lbaf[0] = sc->nvstore.sectsz_bits << NVME_NS_DATA_LBAF_LBADS_SHIFT;
}

static void
pci_nvme_init_logpages(struct pci_nvme_softc *sc)
{

	memset(&sc->err_log, 0, sizeof(sc->err_log));
	memset(&sc->health_log, 0, sizeof(sc->health_log));
	memset(&sc->fw_log, 0, sizeof(sc->fw_log));
}

static void
pci_nvme_reset_locked(struct pci_nvme_softc *sc)
{
	DPRINTF(("%s", __func__));

	sc->regs.cap_lo = (ZERO_BASED(sc->max_qentries) & NVME_CAP_LO_REG_MQES_MASK) |
	    (1 << NVME_CAP_LO_REG_CQR_SHIFT) |
	    (60 << NVME_CAP_LO_REG_TO_SHIFT);

	sc->regs.cap_hi = 1 << NVME_CAP_HI_REG_CSS_NVM_SHIFT;

	sc->regs.vs = 0x00010300;	/* NVMe v1.3 */

	sc->regs.cc = 0;
	sc->regs.csts = 0;

	sc->num_cqueues = sc->num_squeues = sc->max_queues;
	if (sc->submit_queues != NULL) {
		for (int i = 0; i < sc->num_squeues + 1; i++) {
			/*
			 * The Admin Submission Queue is at index 0.
			 * It must not be changed at reset otherwise the
			 * emulation will be out of sync with the guest.
			 */
			if (i != 0) {
				sc->submit_queues[i].qbase = NULL;
				sc->submit_queues[i].size = 0;
				sc->submit_queues[i].cqid = 0;
			}
			sc->submit_queues[i].tail = 0;
			sc->submit_queues[i].head = 0;
			sc->submit_queues[i].busy = 0;
		}
	} else
		sc->submit_queues = calloc(sc->num_squeues + 1,
		                        sizeof(struct nvme_submission_queue));

	if (sc->compl_queues != NULL) {
		for (int i = 0; i < sc->num_cqueues + 1; i++) {
			/* See Admin Submission Queue note above */
			if (i != 0) {
				sc->compl_queues[i].qbase = NULL;
				sc->compl_queues[i].size = 0;
			}

			sc->compl_queues[i].tail = 0;
			sc->compl_queues[i].head = 0;
		}
	} else {
		sc->compl_queues = calloc(sc->num_cqueues + 1,
		                        sizeof(struct nvme_completion_queue));

		for (int i = 0; i < sc->num_cqueues + 1; i++)
			pthread_mutex_init(&sc->compl_queues[i].mtx, NULL);
	}
}

static void
pci_nvme_reset(struct pci_nvme_softc *sc)
{
	pthread_mutex_lock(&sc->mtx);
	pci_nvme_reset_locked(sc);
	pthread_mutex_unlock(&sc->mtx);
}

static void
pci_nvme_init_controller(struct vmctx *ctx, struct pci_nvme_softc *sc)
{
	uint16_t acqs, asqs;

	DPRINTF(("%s", __func__));

	asqs = (sc->regs.aqa & NVME_AQA_REG_ASQS_MASK) + 1;
	sc->submit_queues[0].size = asqs;
	sc->submit_queues[0].qbase = vm_map_gpa(ctx, sc->regs.asq,
	            sizeof(struct nvme_command) * asqs);

	DPRINTF(("%s mapping Admin-SQ guest 0x%lx, host: %p",
	        __func__, sc->regs.asq, sc->submit_queues[0].qbase));

	acqs = ((sc->regs.aqa >> NVME_AQA_REG_ACQS_SHIFT) & 
	    NVME_AQA_REG_ACQS_MASK) + 1;
	sc->compl_queues[0].size = acqs;
	sc->compl_queues[0].qbase = vm_map_gpa(ctx, sc->regs.acq,
	         sizeof(struct nvme_completion) * acqs);
	DPRINTF(("%s mapping Admin-CQ guest 0x%lx, host: %p",
	        __func__, sc->regs.acq, sc->compl_queues[0].qbase));
}

static int
nvme_prp_memcpy(struct vmctx *ctx, uint64_t prp1, uint64_t prp2, uint8_t *src,
	size_t len)
{
	uint8_t *dst;
	size_t bytes;

	if (len > (8 * 1024)) {
		return (-1);
	}

	/* Copy from the start of prp1 to the end of the physical page */
	bytes = PAGE_SIZE - (prp1 & PAGE_MASK);
	bytes = MIN(bytes, len);

	dst = vm_map_gpa(ctx, prp1, bytes);
	if (dst == NULL) {
		return (-1);
	}

	memcpy(dst, src, bytes);

	src += bytes;

	len -= bytes;
	if (len == 0) {
		return (0);
	}

	len = MIN(len, PAGE_SIZE);

	dst = vm_map_gpa(ctx, prp2, len);
	if (dst == NULL) {
		return (-1);
	}

	memcpy(dst, src, len);

	return (0);
}

static int
nvme_opc_delete_io_sq(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	uint16_t qid = command->cdw10 & 0xffff;

	DPRINTF(("%s DELETE_IO_SQ %u", __func__, qid));
	if (qid == 0 || qid > sc->num_squeues) {
		WPRINTF(("%s NOT PERMITTED queue id %u / num_squeues %u",
		        __func__, qid, sc->num_squeues));
		pci_nvme_status_tc(&compl->status, NVME_SCT_COMMAND_SPECIFIC,
		    NVME_SC_INVALID_QUEUE_IDENTIFIER);
		return (1);
	}

	sc->submit_queues[qid].qbase = NULL;
	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_opc_create_io_sq(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	if (command->cdw11 & NVME_CMD_CDW11_PC) {
		uint16_t qid = command->cdw10 & 0xffff;
		struct nvme_submission_queue *nsq;

		if ((qid == 0) || (qid > sc->num_squeues)) {
			WPRINTF(("%s queue index %u > num_squeues %u",
			        __func__, qid, sc->num_squeues));
			pci_nvme_status_tc(&compl->status,
			    NVME_SCT_COMMAND_SPECIFIC,
			    NVME_SC_INVALID_QUEUE_IDENTIFIER);
			return (1);
		}

		nsq = &sc->submit_queues[qid];
		nsq->size = ONE_BASED((command->cdw10 >> 16) & 0xffff);

		nsq->qbase = vm_map_gpa(sc->nsc_pi->pi_vmctx, command->prp1,
		              sizeof(struct nvme_command) * (size_t)nsq->size);
		nsq->cqid = (command->cdw11 >> 16) & 0xffff;
		nsq->qpriority = (command->cdw11 >> 1) & 0x03;

		DPRINTF(("%s sq %u size %u gaddr %p cqid %u", __func__,
		        qid, nsq->size, nsq->qbase, nsq->cqid));

		pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);

		DPRINTF(("%s completed creating IOSQ qid %u",
		         __func__, qid));
	} else {
		/* 
		 * Guest sent non-cont submission queue request.
		 * This setting is unsupported by this emulation.
		 */
		WPRINTF(("%s unsupported non-contig (list-based) "
		         "create i/o submission queue", __func__));

		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
	}
	return (1);
}

static int
nvme_opc_delete_io_cq(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	uint16_t qid = command->cdw10 & 0xffff;

	DPRINTF(("%s DELETE_IO_CQ %u", __func__, qid));
	if (qid == 0 || qid > sc->num_cqueues) {
		WPRINTF(("%s queue index %u / num_cqueues %u",
		        __func__, qid, sc->num_cqueues));
		pci_nvme_status_tc(&compl->status, NVME_SCT_COMMAND_SPECIFIC,
		    NVME_SC_INVALID_QUEUE_IDENTIFIER);
		return (1);
	}

	sc->compl_queues[qid].qbase = NULL;
	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_opc_create_io_cq(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	if (command->cdw11 & NVME_CMD_CDW11_PC) {
		uint16_t qid = command->cdw10 & 0xffff;
		struct nvme_completion_queue *ncq;

		if ((qid == 0) || (qid > sc->num_cqueues)) {
			WPRINTF(("%s queue index %u > num_cqueues %u",
			        __func__, qid, sc->num_cqueues));
			pci_nvme_status_tc(&compl->status,
			    NVME_SCT_COMMAND_SPECIFIC,
			    NVME_SC_INVALID_QUEUE_IDENTIFIER);
			return (1);
		}

		ncq = &sc->compl_queues[qid];
		ncq->intr_en = (command->cdw11 & NVME_CMD_CDW11_IEN) >> 1;
		ncq->intr_vec = (command->cdw11 >> 16) & 0xffff;
		ncq->size = ONE_BASED((command->cdw10 >> 16) & 0xffff);

		ncq->qbase = vm_map_gpa(sc->nsc_pi->pi_vmctx,
		             command->prp1,
		             sizeof(struct nvme_command) * (size_t)ncq->size);

		pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	} else {
		/* 
		 * Non-contig completion queue unsupported.
		 */
		WPRINTF(("%s unsupported non-contig (list-based) "
		         "create i/o completion queue",
		         __func__));

		/* 0x12 = Invalid Use of Controller Memory Buffer */
		pci_nvme_status_genc(&compl->status, 0x12);
	}

	return (1);
}

static int
nvme_opc_get_log_page(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	uint32_t logsize = (1 + ((command->cdw10 >> 16) & 0xFFF)) * 2;
	uint8_t logpage = command->cdw10 & 0xFF;

	DPRINTF(("%s log page %u len %u", __func__, logpage, logsize));

	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);

	switch (logpage) {
	case NVME_LOG_ERROR:
		nvme_prp_memcpy(sc->nsc_pi->pi_vmctx, command->prp1,
		    command->prp2, (uint8_t *)&sc->err_log, logsize);
		break;
	case NVME_LOG_HEALTH_INFORMATION:
		/* TODO: present some smart info */
		nvme_prp_memcpy(sc->nsc_pi->pi_vmctx, command->prp1,
		    command->prp2, (uint8_t *)&sc->health_log, logsize);
		break;
	case NVME_LOG_FIRMWARE_SLOT:
		nvme_prp_memcpy(sc->nsc_pi->pi_vmctx, command->prp1,
		    command->prp2, (uint8_t *)&sc->fw_log, logsize);
		break;
	default:
		WPRINTF(("%s get log page %x command not supported",
		        __func__, logpage));

		pci_nvme_status_tc(&compl->status, NVME_SCT_COMMAND_SPECIFIC,
		    NVME_SC_INVALID_LOG_PAGE);
	}

	return (1);
}

static int
nvme_opc_identify(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	void *dest;

	DPRINTF(("%s identify 0x%x nsid 0x%x", __func__,
	        command->cdw10 & 0xFF, command->nsid));

	switch (command->cdw10 & 0xFF) {
	case 0x00: /* return Identify Namespace data structure */
		nvme_prp_memcpy(sc->nsc_pi->pi_vmctx, command->prp1,
		    command->prp2, (uint8_t *)&sc->nsdata, sizeof(sc->nsdata));
		break;
	case 0x01: /* return Identify Controller data structure */
		nvme_prp_memcpy(sc->nsc_pi->pi_vmctx, command->prp1,
		    command->prp2, (uint8_t *)&sc->ctrldata,
		    sizeof(sc->ctrldata));
		break;
	case 0x02: /* list of 1024 active NSIDs > CDW1.NSID */
		dest = vm_map_gpa(sc->nsc_pi->pi_vmctx, command->prp1,
		                  sizeof(uint32_t) * 1024);
		((uint32_t *)dest)[0] = 1;
		((uint32_t *)dest)[1] = 0;
		break;
	case 0x11:
		pci_nvme_status_genc(&compl->status,
		    NVME_SC_INVALID_NAMESPACE_OR_FORMAT);
		return (1);
	case 0x03: /* list of NSID structures in CDW1.NSID, 4096 bytes */
	case 0x10:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	default:
		DPRINTF(("%s unsupported identify command requested 0x%x",
		         __func__, command->cdw10 & 0xFF));
		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
		return (1);
	}

	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_set_feature_queues(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	uint16_t nqr;	/* Number of Queues Requested */

	nqr = command->cdw11 & 0xFFFF;
	if (nqr == 0xffff) {
		WPRINTF(("%s: Illegal NSQR value %#x", __func__, nqr));
		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
		return (-1);
	}

	sc->num_squeues = ONE_BASED(nqr);
	if (sc->num_squeues > sc->max_queues) {
		DPRINTF(("NSQR=%u is greater than max %u", sc->num_squeues,
					sc->max_queues));
		sc->num_squeues = sc->max_queues;
	}

	nqr = (command->cdw11 >> 16) & 0xFFFF;
	if (nqr == 0xffff) {
		WPRINTF(("%s: Illegal NCQR value %#x", __func__, nqr));
		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
		return (-1);
	}

	sc->num_cqueues = ONE_BASED(nqr);
	if (sc->num_cqueues > sc->max_queues) {
		DPRINTF(("NCQR=%u is greater than max %u", sc->num_cqueues,
					sc->max_queues));
		sc->num_cqueues = sc->max_queues;
	}

	compl->cdw0 = NVME_FEATURE_NUM_QUEUES(sc);

	return (0);
}

static int
nvme_opc_set_features(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	int feature = command->cdw10 & 0xFF;
	uint32_t iv;

	DPRINTF(("%s feature 0x%x", __func__, feature));
	compl->cdw0 = 0;

	switch (feature) {
	case NVME_FEAT_ARBITRATION:
		DPRINTF(("  arbitration 0x%x", command->cdw11));
		break;
	case NVME_FEAT_POWER_MANAGEMENT:
		DPRINTF(("  power management 0x%x", command->cdw11));
		break;
	case NVME_FEAT_LBA_RANGE_TYPE:
		DPRINTF(("  lba range 0x%x", command->cdw11));
		break;
	case NVME_FEAT_TEMPERATURE_THRESHOLD:
		DPRINTF(("  temperature threshold 0x%x", command->cdw11));
		break;
	case NVME_FEAT_ERROR_RECOVERY:
		DPRINTF(("  error recovery 0x%x", command->cdw11));
		break;
	case NVME_FEAT_VOLATILE_WRITE_CACHE:
		DPRINTF(("  volatile write cache 0x%x", command->cdw11));
		break;
	case NVME_FEAT_NUMBER_OF_QUEUES:
		nvme_set_feature_queues(sc, command, compl);
		break;
	case NVME_FEAT_INTERRUPT_COALESCING:
		DPRINTF(("  interrupt coalescing 0x%x", command->cdw11));

		/* in uS */
		sc->intr_coales_aggr_time = ((command->cdw11 >> 8) & 0xFF)*100;

		sc->intr_coales_aggr_thresh = command->cdw11 & 0xFF;
		break;
	case NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION:
		iv = command->cdw11 & 0xFFFF;

		DPRINTF(("  interrupt vector configuration 0x%x",
		        command->cdw11));

		for (uint32_t i = 0; i < sc->num_cqueues + 1; i++) {
			if (sc->compl_queues[i].intr_vec == iv) {
				if (command->cdw11 & (1 << 16))
					sc->compl_queues[i].intr_en |=
					                      NVME_CQ_INTCOAL;  
				else
					sc->compl_queues[i].intr_en &=
					                     ~NVME_CQ_INTCOAL;  
			}
		}
		break;
	case NVME_FEAT_WRITE_ATOMICITY:
		DPRINTF(("  write atomicity 0x%x", command->cdw11));
		break;
	case NVME_FEAT_ASYNC_EVENT_CONFIGURATION:
		DPRINTF(("  async event configuration 0x%x",
		        command->cdw11));
		sc->async_ev_config = command->cdw11;
		break;
	case NVME_FEAT_SOFTWARE_PROGRESS_MARKER:
		DPRINTF(("  software progress marker 0x%x",
		        command->cdw11));
		break;
	case 0x0C:
		DPRINTF(("  autonomous power state transition 0x%x",
		        command->cdw11));
		break;
	default:
		WPRINTF(("%s invalid feature", __func__));
		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
		return (1);
	}

	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_opc_get_features(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	int feature = command->cdw10 & 0xFF;

	DPRINTF(("%s feature 0x%x", __func__, feature));

	compl->cdw0 = 0;

	switch (feature) {
	case NVME_FEAT_ARBITRATION:
		DPRINTF(("  arbitration"));
		break;
	case NVME_FEAT_POWER_MANAGEMENT:
		DPRINTF(("  power management"));
		break;
	case NVME_FEAT_LBA_RANGE_TYPE:
		DPRINTF(("  lba range"));
		break;
	case NVME_FEAT_TEMPERATURE_THRESHOLD:
		DPRINTF(("  temperature threshold"));
		switch ((command->cdw11 >> 20) & 0x3) {
		case 0:
			/* Over temp threshold */
			compl->cdw0 = 0xFFFF;
			break;
		case 1:
			/* Under temp threshold */
			compl->cdw0 = 0;
			break;
		default:
			WPRINTF(("  invalid threshold type select"));
			pci_nvme_status_genc(&compl->status,
			    NVME_SC_INVALID_FIELD);
			return (1);
		}
		break;
	case NVME_FEAT_ERROR_RECOVERY:
		DPRINTF(("  error recovery"));
		break;
	case NVME_FEAT_VOLATILE_WRITE_CACHE:
		DPRINTF(("  volatile write cache"));
		break;
	case NVME_FEAT_NUMBER_OF_QUEUES:
		compl->cdw0 = NVME_FEATURE_NUM_QUEUES(sc);

		DPRINTF(("  number of queues (submit %u, completion %u)",
		        compl->cdw0 & 0xFFFF,
		        (compl->cdw0 >> 16) & 0xFFFF));

		break;
	case NVME_FEAT_INTERRUPT_COALESCING:
		DPRINTF(("  interrupt coalescing"));
		break;
	case NVME_FEAT_INTERRUPT_VECTOR_CONFIGURATION:
		DPRINTF(("  interrupt vector configuration"));
		break;
	case NVME_FEAT_WRITE_ATOMICITY:
		DPRINTF(("  write atomicity"));
		break;
	case NVME_FEAT_ASYNC_EVENT_CONFIGURATION:
		DPRINTF(("  async event configuration"));
		sc->async_ev_config = command->cdw11;
		break;
	case NVME_FEAT_SOFTWARE_PROGRESS_MARKER:
		DPRINTF(("  software progress marker"));
		break;
	case 0x0C:
		DPRINTF(("  autonomous power state transition"));
		break;
	default:
		WPRINTF(("%s invalid feature 0x%x", __func__, feature));
		pci_nvme_status_genc(&compl->status, NVME_SC_INVALID_FIELD);
		return (1);
	}

	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_opc_abort(struct pci_nvme_softc* sc, struct nvme_command* command,
	struct nvme_completion* compl)
{
	DPRINTF(("%s submission queue %u, command ID 0x%x", __func__,
	        command->cdw10 & 0xFFFF, (command->cdw10 >> 16) & 0xFFFF));

	/* TODO: search for the command ID and abort it */

	compl->cdw0 = 1;
	pci_nvme_status_genc(&compl->status, NVME_SC_SUCCESS);
	return (1);
}

static int
nvme_opc_async_event_req(struct pci_nvme_softc* sc,
	struct nvme_command* command, struct nvme_completion* compl)
{
	DPRINTF(("%s async event request 0x%x", __func__, command->cdw11));

	/*
	 * TODO: raise events when they happen based on the Set Features cmd.
	 * These events happen async, so only set completion successful if
	 * there is an event reflective of the request to get event.
	 */
	pci_nvme_status_tc(&compl->status, NVME_SCT_COMMAND_SPECIFIC,
	    NVME_SC_ASYNC_EVENT_REQUEST_LIMIT_EXCEEDED);
	return (0);
}

static void
pci_nvme_handle_admin_cmd(struct pci_nvme_softc* sc, uint64_t value)
{
	struct nvme_completion compl;
	struct nvme_command *cmd;
	struct nvme_submission_queue *sq;
	struct nvme_completion_queue *cq;
	int do_intr = 0;
	uint16_t sqhead;

	DPRINTF(("%s index %u", __func__, (uint32_t)value));

	sq = &sc->submit_queues[0];

	sqhead = atomic_load_acq_short(&sq->head);

	if (atomic_testandset_int(&sq->busy, 1)) {
		DPRINTF(("%s SQ busy, head %u, tail %u",
		        __func__, sqhead, sq->tail));
		return;
	}

	DPRINTF(("sqhead %u, tail %u", sqhead, sq->tail));
	
	while (sqhead != atomic_load_acq_short(&sq->tail)) {
		cmd = &(sq->qbase)[sqhead];
		compl.cdw0 = 0;
		compl.status = 0;

		switch (cmd->opc) {
		case NVME_OPC_DELETE_IO_SQ:
			DPRINTF(("%s command DELETE_IO_SQ", __func__));
			do_intr |= nvme_opc_delete_io_sq(sc, cmd, &compl);
			break;
		case NVME_OPC_CREATE_IO_SQ:
			DPRINTF(("%s command CREATE_IO_SQ", __func__));
			do_intr |= nvme_opc_create_io_sq(sc, cmd, &compl);
			break;
		case NVME_OPC_DELETE_IO_CQ:
			DPRINTF(("%s command DELETE_IO_CQ", __func__));
			do_intr |= nvme_opc_delete_io_cq(sc, cmd, &compl);
			break;
		case NVME_OPC_CREATE_IO_CQ:
			DPRINTF(("%s command CREATE_IO_CQ", __func__));
			do_intr |= nvme_opc_create_io_cq(sc, cmd, &compl);
			break;
		case NVME_OPC_GET_LOG_PAGE:
			DPRINTF(("%s command GET_LOG_PAGE", __func__));
			do_intr |= nvme_opc_get_log_page(sc, cmd, &compl);
			break;
		case NVME_OPC_IDENTIFY:
			DPRINTF(("%s command IDENTIFY", __func__));
			do_intr |= nvme_opc_identify(sc, cmd, &compl);
			break;
		case NVME_OPC_ABORT:
			DPRINTF(("%s command ABORT", __func__));
			do_intr |= nvme_opc_abort(sc, cmd, &compl);
			break;
		case NVME_OPC_SET_FEATURES:
			DPRINTF(("%s command SET_FEATURES", __func__));
			do_intr |= nvme_opc_set_features(sc, cmd, &compl);
			break;
		case NVME_OPC_GET_FEATURES:
			DPRINTF(("%s command GET_FEATURES", __func__));
			do_intr |= nvme_opc_get_features(sc, cmd, &compl);
			break;
		case NVME_OPC_ASYNC_EVENT_REQUEST:
			DPRINTF(("%s command ASYNC_EVENT_REQ", __func__));
			/* XXX dont care, unhandled for now
			do_intr |= nvme_opc_async_event_req(sc, cmd, &compl);
			*/
			compl.status = NVME_NO_STATUS;
			break;
		default:
			WPRINTF(("0x%x command is not implemented",
			    cmd->opc));
			pci_nvme_status_genc(&compl.status, NVME_SC_INVALID_OPCODE);
			do_intr |= 1;
		}
	
		if (NVME_COMPLETION_VALID(compl)) {
			struct nvme_completion *cp;
			int phase;

			cq = &sc->compl_queues[0];

			cp = &(cq->qbase)[cq->tail];
			cp->cdw0 = compl.cdw0;
			cp->sqid = 0;
			cp->sqhd = sqhead;
			cp->cid = cmd->cid;

			phase = NVME_STATUS_GET_P(cp->status);
			cp->status = compl.status;
			pci_nvme_toggle_phase(&cp->status, phase);

			cq->tail = (cq->tail + 1) % cq->size;
		}
		sqhead = (sqhead + 1) % sq->size;
	}

	DPRINTF(("setting sqhead %u", sqhead));
	atomic_store_short(&sq->head, sqhead);
	atomic_store_int(&sq->busy, 0);

	if (do_intr)
		pci_generate_msix(sc->nsc_pi, 0);

}

static int
pci_nvme_append_iov_req(struct pci_nvme_softc *sc, struct pci_nvme_ioreq *req,
	uint64_t gpaddr, size_t size, int do_write, uint64_t lba)
{
	int iovidx;

	if (req != NULL) {
		/* concatenate contig block-iovs to minimize number of iovs */
		if ((req->prev_gpaddr + req->prev_size) == gpaddr) {
			iovidx = req->io_req.br_iovcnt - 1;

			req->io_req.br_iov[iovidx].iov_base =
			    paddr_guest2host(req->sc->nsc_pi->pi_vmctx,
			                     req->prev_gpaddr, size);

			req->prev_size += size;
			req->io_req.br_resid += size;

			req->io_req.br_iov[iovidx].iov_len = req->prev_size;
		} else {
			pthread_mutex_lock(&req->mtx);

			iovidx = req->io_req.br_iovcnt;
			if (iovidx == NVME_MAX_BLOCKIOVS) {
				int err = 0;

				DPRINTF(("large I/O, doing partial req"));

				iovidx = 0;
				req->io_req.br_iovcnt = 0;

				req->io_req.br_callback = pci_nvme_io_partial;

				if (!do_write)
					err = blockif_read(sc->nvstore.ctx,
					                   &req->io_req);
				else
					err = blockif_write(sc->nvstore.ctx,
					                    &req->io_req);

				/* wait until req completes before cont */
				if (err == 0)
					pthread_cond_wait(&req->cv, &req->mtx);
			}
			if (iovidx == 0) {
				req->io_req.br_offset = lba;
				req->io_req.br_resid = 0;
				req->io_req.br_param = req;
			}

			req->io_req.br_iov[iovidx].iov_base =
			    paddr_guest2host(req->sc->nsc_pi->pi_vmctx,
			                     gpaddr, size);

			req->io_req.br_iov[iovidx].iov_len = size;

			req->prev_gpaddr = gpaddr;
			req->prev_size = size;
			req->io_req.br_resid += size;

			req->io_req.br_iovcnt++;

			pthread_mutex_unlock(&req->mtx);
		}
	} else {
		/* RAM buffer: read/write directly */
		void *p = sc->nvstore.ctx;
		void *gptr;

		if ((lba + size) > sc->nvstore.size) {
			WPRINTF(("%s write would overflow RAM", __func__));
			return (-1);
		}

		p = (void *)((uintptr_t)p + (uintptr_t)lba);
		gptr = paddr_guest2host(sc->nsc_pi->pi_vmctx, gpaddr, size);
		if (do_write) 
			memcpy(p, gptr, size);
		else
			memcpy(gptr, p, size);
	}
	return (0);
}

static void
pci_nvme_set_completion(struct pci_nvme_softc *sc,
	struct nvme_submission_queue *sq, int sqid, uint16_t cid,
	uint32_t cdw0, uint16_t status, int ignore_busy)
{
	struct nvme_completion_queue *cq = &sc->compl_queues[sq->cqid];
	struct nvme_completion *compl;
	int do_intr = 0;
	int phase;

	DPRINTF(("%s sqid %d cqid %u cid %u status: 0x%x 0x%x",
		 __func__, sqid, sq->cqid, cid, NVME_STATUS_GET_SCT(status),
		 NVME_STATUS_GET_SC(status)));

	pthread_mutex_lock(&cq->mtx);

	assert(cq->qbase != NULL);

	compl = &cq->qbase[cq->tail];

	compl->sqhd = atomic_load_acq_short(&sq->head);
	compl->sqid = sqid;
	compl->cid = cid;

	// toggle phase
	phase = NVME_STATUS_GET_P(compl->status);
	compl->status = status;
	pci_nvme_toggle_phase(&compl->status, phase);

	cq->tail = (cq->tail + 1) % cq->size;

	if (cq->intr_en & NVME_CQ_INTEN)
		do_intr = 1;

	pthread_mutex_unlock(&cq->mtx);

	if (ignore_busy || !atomic_load_acq_int(&sq->busy))
		if (do_intr)
			pci_generate_msix(sc->nsc_pi, cq->intr_vec);
}

static void
pci_nvme_release_ioreq(struct pci_nvme_softc *sc, struct pci_nvme_ioreq *req)
{
	req->sc = NULL;
	req->nvme_sq = NULL;
	req->sqid = 0;

	pthread_mutex_lock(&sc->mtx);

	req->next = sc->ioreqs_free;
	sc->ioreqs_free = req;
	sc->pending_ios--;

	/* when no more IO pending, can set to ready if device reset/enabled */
	if (sc->pending_ios == 0 &&
	    NVME_CC_GET_EN(sc->regs.cc) && !(NVME_CSTS_GET_RDY(sc->regs.csts)))
		sc->regs.csts |= NVME_CSTS_RDY;

	pthread_mutex_unlock(&sc->mtx);

	sem_post(&sc->iosemlock);
}

static struct pci_nvme_ioreq *
pci_nvme_get_ioreq(struct pci_nvme_softc *sc)
{
	struct pci_nvme_ioreq *req = NULL;;

	sem_wait(&sc->iosemlock);
	pthread_mutex_lock(&sc->mtx);

	req = sc->ioreqs_free;
	assert(req != NULL);

	sc->ioreqs_free = req->next;

	req->next = NULL;
	req->sc = sc;

	sc->pending_ios++;

	pthread_mutex_unlock(&sc->mtx);

	req->io_req.br_iovcnt = 0;
	req->io_req.br_offset = 0;
	req->io_req.br_resid = 0;
	req->io_req.br_param = req;
	req->prev_gpaddr = 0;
	req->prev_size = 0;

	return req;
}

static void
pci_nvme_io_done(struct blockif_req *br, int err)
{
	struct pci_nvme_ioreq *req = br->br_param;
	struct nvme_submission_queue *sq = req->nvme_sq;
	uint16_t code, status;

	DPRINTF(("%s error %d %s", __func__, err, strerror(err)));
	
	/* TODO return correct error */
	code = err ? NVME_SC_DATA_TRANSFER_ERROR : NVME_SC_SUCCESS;
	pci_nvme_status_genc(&status, code);

	pci_nvme_set_completion(req->sc, sq, req->sqid, req->cid, 0, status, 0);
	pci_nvme_release_ioreq(req->sc, req);
}

static void
pci_nvme_io_partial(struct blockif_req *br, int err)
{
	struct pci_nvme_ioreq *req = br->br_param;

	DPRINTF(("%s error %d %s", __func__, err, strerror(err)));

	pthread_cond_signal(&req->cv);
}


static void
pci_nvme_handle_io_cmd(struct pci_nvme_softc* sc, uint16_t idx)
{
	struct nvme_submission_queue *sq;
	uint16_t status;
	uint16_t sqhead;
	int err;

	/* handle all submissions up to sq->tail index */
	sq = &sc->submit_queues[idx];

	if (atomic_testandset_int(&sq->busy, 1)) {
		DPRINTF(("%s sqid %u busy", __func__, idx));
		return;
	}

	sqhead = atomic_load_acq_short(&sq->head);

	DPRINTF(("nvme_handle_io qid %u head %u tail %u cmdlist %p",
	         idx, sqhead, sq->tail, sq->qbase));

	while (sqhead != atomic_load_acq_short(&sq->tail)) {
		struct nvme_command *cmd;
		struct pci_nvme_ioreq *req = NULL;
		uint64_t lba;
		uint64_t nblocks, bytes, size, cpsz;

		/* TODO: support scatter gather list handling */

		cmd = &sq->qbase[sqhead];
		sqhead = (sqhead + 1) % sq->size;

		lba = ((uint64_t)cmd->cdw11 << 32) | cmd->cdw10;

		if (cmd->opc == NVME_OPC_FLUSH) {
			pci_nvme_status_genc(&status, NVME_SC_SUCCESS);
			pci_nvme_set_completion(sc, sq, idx, cmd->cid, 0,
			                        status, 1);

			continue;
		} else if (cmd->opc == 0x08) {
			/* TODO: write zeroes */
			WPRINTF(("%s write zeroes lba 0x%lx blocks %u",
			        __func__, lba, cmd->cdw12 & 0xFFFF));
			pci_nvme_status_genc(&status, NVME_SC_SUCCESS);
			pci_nvme_set_completion(sc, sq, idx, cmd->cid, 0,
			                        status, 1);

			continue;
		}

		nblocks = (cmd->cdw12 & 0xFFFF) + 1;

		bytes = nblocks * sc->nvstore.sectsz;

		if (sc->nvstore.type == NVME_STOR_BLOCKIF) {
			req = pci_nvme_get_ioreq(sc);
			req->nvme_sq = sq;
			req->sqid = idx;
		}

		/*
		 * If data starts mid-page and flows into the next page, then
		 * increase page count
		 */

		DPRINTF(("[h%u:t%u:n%u] %s starting LBA 0x%lx blocks %lu "
		         "(%lu-bytes)",
		         sqhead==0 ? sq->size-1 : sqhead-1, sq->tail, sq->size,
		         cmd->opc == NVME_OPC_WRITE ?
			     "WRITE" : "READ",
		         lba, nblocks, bytes));

		cmd->prp1 &= ~(0x03UL);
		cmd->prp2 &= ~(0x03UL);

		DPRINTF((" prp1 0x%lx prp2 0x%lx", cmd->prp1, cmd->prp2));

		size = bytes;
		lba *= sc->nvstore.sectsz;

		cpsz = PAGE_SIZE - (cmd->prp1 % PAGE_SIZE);

		if (cpsz > bytes)
			cpsz = bytes;

		if (req != NULL) {
			req->io_req.br_offset = ((uint64_t)cmd->cdw11 << 32) |
			                        cmd->cdw10;
			req->opc = cmd->opc;
			req->cid = cmd->cid;
			req->nsid = cmd->nsid;
		}

		err = pci_nvme_append_iov_req(sc, req, cmd->prp1, cpsz,
		    cmd->opc == NVME_OPC_WRITE, lba);
		lba += cpsz;
		size -= cpsz;

		if (size == 0)
			goto iodone;

		if (size <= PAGE_SIZE) {
			/* prp2 is second (and final) page in transfer */

			err = pci_nvme_append_iov_req(sc, req, cmd->prp2,
			    size,
			    cmd->opc == NVME_OPC_WRITE,
			    lba);
		} else {
			uint64_t *prp_list;
			int i;

			/* prp2 is pointer to a physical region page list */
			prp_list = paddr_guest2host(sc->nsc_pi->pi_vmctx,
			                            cmd->prp2, PAGE_SIZE);

			i = 0;
			while (size != 0) {
				cpsz = MIN(size, PAGE_SIZE);

				/*
				 * Move to linked physical region page list
				 * in last item.
				 */ 
				if (i == (NVME_PRP2_ITEMS-1) &&
				    size > PAGE_SIZE) {
					assert((prp_list[i] & (PAGE_SIZE-1)) == 0);
					prp_list = paddr_guest2host(
					              sc->nsc_pi->pi_vmctx,
					              prp_list[i], PAGE_SIZE);
					i = 0;
				}
				if (prp_list[i] == 0) {
					WPRINTF(("PRP2[%d] = 0 !!!", i));
					err = 1;
					break;
				}

				err = pci_nvme_append_iov_req(sc, req,
				    prp_list[i], cpsz,
				    cmd->opc == NVME_OPC_WRITE, lba);
				if (err)
					break;

				lba += cpsz;
				size -= cpsz;
				i++;
			}
		}

iodone:
		if (sc->nvstore.type == NVME_STOR_RAM) {
			uint16_t code, status;

			code = err ? NVME_SC_LBA_OUT_OF_RANGE :
			    NVME_SC_SUCCESS;
			pci_nvme_status_genc(&status, code);

			pci_nvme_set_completion(sc, sq, idx, cmd->cid, 0,
			                        status, 1);

			continue;
		}


		if (err)
			goto do_error;

		req->io_req.br_callback = pci_nvme_io_done;

		err = 0;
		switch (cmd->opc) {
		case NVME_OPC_READ:
			err = blockif_read(sc->nvstore.ctx, &req->io_req);
			break;
		case NVME_OPC_WRITE:
			err = blockif_write(sc->nvstore.ctx, &req->io_req);
			break;
		default:
			WPRINTF(("%s unhandled io command 0x%x",
				 __func__, cmd->opc));
			err = 1;
		}

do_error:
		if (err) {
			uint16_t status;

			pci_nvme_status_genc(&status,
			    NVME_SC_DATA_TRANSFER_ERROR);

			pci_nvme_set_completion(sc, sq, idx, cmd->cid, 0,
			                        status, 1);
			pci_nvme_release_ioreq(sc, req);
		}
	}

	atomic_store_short(&sq->head, sqhead);
	atomic_store_int(&sq->busy, 0);
}

static void
pci_nvme_handle_doorbell(struct vmctx *ctx, struct pci_nvme_softc* sc,
	uint64_t idx, int is_sq, uint64_t value)
{
	DPRINTF(("nvme doorbell %lu, %s, val 0x%lx",
	        idx, is_sq ? "SQ" : "CQ", value & 0xFFFF));

	if (is_sq) {
		atomic_store_short(&sc->submit_queues[idx].tail,
		                   (uint16_t)value);

		if (idx == 0) {
			pci_nvme_handle_admin_cmd(sc, value);
		} else {
			/* submission queue; handle new entries in SQ */
			if (idx > sc->num_squeues) {
				WPRINTF(("%s SQ index %lu overflow from "
				         "guest (max %u)",
				         __func__, idx, sc->num_squeues));
				return;
			}
			pci_nvme_handle_io_cmd(sc, (uint16_t)idx);
		}
	} else {
		if (idx > sc->num_cqueues) {
			WPRINTF(("%s queue index %lu overflow from "
			         "guest (max %u)",
			         __func__, idx, sc->num_cqueues));
			return;
		}

		sc->compl_queues[idx].head = (uint16_t)value;
	}
}

static void
pci_nvme_bar0_reg_dumps(const char *func, uint64_t offset, int iswrite)
{
	const char *s = iswrite ? "WRITE" : "READ";

	switch (offset) {
	case NVME_CR_CAP_LOW:
		DPRINTF(("%s %s NVME_CR_CAP_LOW", func, s));
		break;
	case NVME_CR_CAP_HI:
		DPRINTF(("%s %s NVME_CR_CAP_HI", func, s));
		break;
	case NVME_CR_VS:
		DPRINTF(("%s %s NVME_CR_VS", func, s));
		break;
	case NVME_CR_INTMS:
		DPRINTF(("%s %s NVME_CR_INTMS", func, s));
		break;
	case NVME_CR_INTMC:
		DPRINTF(("%s %s NVME_CR_INTMC", func, s));
		break;
	case NVME_CR_CC:
		DPRINTF(("%s %s NVME_CR_CC", func, s));
		break;
	case NVME_CR_CSTS:
		DPRINTF(("%s %s NVME_CR_CSTS", func, s));
		break;
	case NVME_CR_NSSR:
		DPRINTF(("%s %s NVME_CR_NSSR", func, s));
		break;
	case NVME_CR_AQA:
		DPRINTF(("%s %s NVME_CR_AQA", func, s));
		break;
	case NVME_CR_ASQ_LOW:
		DPRINTF(("%s %s NVME_CR_ASQ_LOW", func, s));
		break;
	case NVME_CR_ASQ_HI:
		DPRINTF(("%s %s NVME_CR_ASQ_HI", func, s));
		break;
	case NVME_CR_ACQ_LOW:
		DPRINTF(("%s %s NVME_CR_ACQ_LOW", func, s));
		break;
	case NVME_CR_ACQ_HI:
		DPRINTF(("%s %s NVME_CR_ACQ_HI", func, s));
		break;
	default:
		DPRINTF(("unknown nvme bar-0 offset 0x%lx", offset));
	}

}

static void
pci_nvme_write_bar_0(struct vmctx *ctx, struct pci_nvme_softc* sc,
	uint64_t offset, int size, uint64_t value)
{
	uint32_t ccreg;

	if (offset >= NVME_DOORBELL_OFFSET) {
		uint64_t belloffset = offset - NVME_DOORBELL_OFFSET;
		uint64_t idx = belloffset / 8; /* door bell size = 2*int */
		int is_sq = (belloffset % 8) < 4;

		if (belloffset > ((sc->max_queues+1) * 8 - 4)) {
			WPRINTF(("guest attempted an overflow write offset "
			         "0x%lx, val 0x%lx in %s",
			         offset, value, __func__));
			return;
		}

		pci_nvme_handle_doorbell(ctx, sc, idx, is_sq, value);
		return;
	}

	DPRINTF(("nvme-write offset 0x%lx, size %d, value 0x%lx",
	        offset, size, value));

	if (size != 4) {
		WPRINTF(("guest wrote invalid size %d (offset 0x%lx, "
		         "val 0x%lx) to bar0 in %s",
		         size, offset, value, __func__));
		/* TODO: shutdown device */
		return;
	}

	pci_nvme_bar0_reg_dumps(__func__, offset, 1);

	pthread_mutex_lock(&sc->mtx);

	switch (offset) {
	case NVME_CR_CAP_LOW:
	case NVME_CR_CAP_HI:
		/* readonly */
		break;
	case NVME_CR_VS:
		/* readonly */
		break;
	case NVME_CR_INTMS:
		/* MSI-X, so ignore */
		break;
	case NVME_CR_INTMC:
		/* MSI-X, so ignore */
		break;
	case NVME_CR_CC:
		ccreg = (uint32_t)value;

		DPRINTF(("%s NVME_CR_CC en %x css %x shn %x iosqes %u "
		         "iocqes %u",
		        __func__,
			 NVME_CC_GET_EN(ccreg), NVME_CC_GET_CSS(ccreg),
			 NVME_CC_GET_SHN(ccreg), NVME_CC_GET_IOSQES(ccreg),
			 NVME_CC_GET_IOCQES(ccreg)));

		if (NVME_CC_GET_SHN(ccreg)) {
			/* perform shutdown - flush out data to backend */
			sc->regs.csts &= ~(NVME_CSTS_REG_SHST_MASK <<
			    NVME_CSTS_REG_SHST_SHIFT);
			sc->regs.csts |= NVME_SHST_COMPLETE <<
			    NVME_CSTS_REG_SHST_SHIFT;
		}
		if (NVME_CC_GET_EN(ccreg) != NVME_CC_GET_EN(sc->regs.cc)) {
			if (NVME_CC_GET_EN(ccreg) == 0)
				/* transition 1-> causes controller reset */
				pci_nvme_reset_locked(sc);
			else
				pci_nvme_init_controller(ctx, sc);
		}

		/* Insert the iocqes, iosqes and en bits from the write */
		sc->regs.cc &= ~NVME_CC_WRITE_MASK;
		sc->regs.cc |= ccreg & NVME_CC_WRITE_MASK;
		if (NVME_CC_GET_EN(ccreg) == 0) {
			/* Insert the ams, mps and css bit fields */
			sc->regs.cc &= ~NVME_CC_NEN_WRITE_MASK;
			sc->regs.cc |= ccreg & NVME_CC_NEN_WRITE_MASK;
			sc->regs.csts &= ~NVME_CSTS_RDY;
		} else if (sc->pending_ios == 0) {
			sc->regs.csts |= NVME_CSTS_RDY;
		}
		break;
	case NVME_CR_CSTS:
		break;
	case NVME_CR_NSSR:
		/* ignore writes; don't support subsystem reset */
		break;
	case NVME_CR_AQA:
		sc->regs.aqa = (uint32_t)value;
		break;
	case NVME_CR_ASQ_LOW:
		sc->regs.asq = (sc->regs.asq & (0xFFFFFFFF00000000)) |
		               (0xFFFFF000 & value);
		break;
	case NVME_CR_ASQ_HI:
		sc->regs.asq = (sc->regs.asq & (0x00000000FFFFFFFF)) |
		               (value << 32);
		break;
	case NVME_CR_ACQ_LOW:
		sc->regs.acq = (sc->regs.acq & (0xFFFFFFFF00000000)) |
		               (0xFFFFF000 & value);
		break;
	case NVME_CR_ACQ_HI:
		sc->regs.acq = (sc->regs.acq & (0x00000000FFFFFFFF)) |
		               (value << 32);
		break;
	default:
		DPRINTF(("%s unknown offset 0x%lx, value 0x%lx size %d",
		         __func__, offset, value, size));
	}
	pthread_mutex_unlock(&sc->mtx);
}

static void
pci_nvme_write(struct vmctx *ctx, int vcpu, struct pci_devinst *pi,
                int baridx, uint64_t offset, int size, uint64_t value)
{
	struct pci_nvme_softc* sc = pi->pi_arg;

	if (baridx == pci_msix_table_bar(pi) ||
	    baridx == pci_msix_pba_bar(pi)) {
		DPRINTF(("nvme-write baridx %d, msix: off 0x%lx, size %d, "
		         " value 0x%lx", baridx, offset, size, value));

		pci_emul_msix_twrite(pi, offset, size, value);
		return;
	}

	switch (baridx) {
	case 0:
		pci_nvme_write_bar_0(ctx, sc, offset, size, value);
		break;

	default:
		DPRINTF(("%s unknown baridx %d, val 0x%lx",
		         __func__, baridx, value));
	}
}

static uint64_t pci_nvme_read_bar_0(struct pci_nvme_softc* sc,
	uint64_t offset, int size)
{
	uint64_t value;

	pci_nvme_bar0_reg_dumps(__func__, offset, 0);

	if (offset < NVME_DOORBELL_OFFSET) {
		void *p = &(sc->regs);
		pthread_mutex_lock(&sc->mtx);
		memcpy(&value, (void *)((uintptr_t)p + offset), size);
		pthread_mutex_unlock(&sc->mtx);
	} else {
		value = 0;
                WPRINTF(("pci_nvme: read invalid offset %ld", offset));
	}

	switch (size) {
	case 1:
		value &= 0xFF;
		break;
	case 2:
		value &= 0xFFFF;
		break;
	case 4:
		value &= 0xFFFFFFFF;
		break;
	}

	DPRINTF(("   nvme-read offset 0x%lx, size %d -> value 0x%x",
	         offset, size, (uint32_t)value));

	return (value);
}



static uint64_t
pci_nvme_read(struct vmctx *ctx, int vcpu, struct pci_devinst *pi, int baridx,
    uint64_t offset, int size)
{
	struct pci_nvme_softc* sc = pi->pi_arg;

	if (baridx == pci_msix_table_bar(pi) ||
	    baridx == pci_msix_pba_bar(pi)) {
		DPRINTF(("nvme-read bar: %d, msix: regoff 0x%lx, size %d",
		        baridx, offset, size));

		return pci_emul_msix_tread(pi, offset, size);
	}

	switch (baridx) {
	case 0:
       		return pci_nvme_read_bar_0(sc, offset, size);

	default:
		DPRINTF(("unknown bar %d, 0x%lx", baridx, offset));
	}

	return (0);
}


static int
pci_nvme_parse_opts(struct pci_nvme_softc *sc, char *opts)
{
	char bident[sizeof("XX:X:X")];
	char	*uopt, *xopts, *config;
	uint32_t sectsz;
	int optidx;

	sc->max_queues = NVME_QUEUES;
	sc->max_qentries = NVME_MAX_QENTRIES;
	sc->ioslots = NVME_IOSLOTS;
	sc->num_squeues = sc->max_queues;
	sc->num_cqueues = sc->max_queues;
	sectsz = 0;

	uopt = strdup(opts);
	optidx = 0;
	snprintf(sc->ctrldata.sn, sizeof(sc->ctrldata.sn),
	         "NVME-%d-%d", sc->nsc_pi->pi_slot, sc->nsc_pi->pi_func);
	for (xopts = strtok(uopt, ",");
	     xopts != NULL;
	     xopts = strtok(NULL, ",")) {

		if ((config = strchr(xopts, '=')) != NULL)
			*config++ = '\0';

		if (!strcmp("maxq", xopts)) {
			sc->max_queues = atoi(config);
		} else if (!strcmp("qsz", xopts)) {
			sc->max_qentries = atoi(config);
		} else if (!strcmp("ioslots", xopts)) {
			sc->ioslots = atoi(config);
		} else if (!strcmp("sectsz", xopts)) {
			sectsz = atoi(config);
		} else if (!strcmp("ser", xopts)) {
			/*
			 * This field indicates the Product Serial Number in
			 * 7-bit ASCII, unused bytes should be space characters.
			 * Ref: NVMe v1.3c.
			 */
			cpywithpad((char *)sc->ctrldata.sn,
			           sizeof(sc->ctrldata.sn), config, ' ');
		} else if (!strcmp("ram", xopts)) {
			uint64_t sz = strtoull(&xopts[4], NULL, 10);

			sc->nvstore.type = NVME_STOR_RAM;
			sc->nvstore.size = sz * 1024 * 1024;
			sc->nvstore.ctx = calloc(1, sc->nvstore.size);
			sc->nvstore.sectsz = 4096;
			sc->nvstore.sectsz_bits = 12;
			if (sc->nvstore.ctx == NULL) {
				perror("Unable to allocate RAM");
				free(uopt);
				return (-1);
			}
		} else if (!strcmp("eui64", xopts)) {
			sc->nvstore.eui64 = htobe64(strtoull(config, NULL, 0));
		} else if (optidx == 0) {
			snprintf(bident, sizeof(bident), "%d:%d",
			         sc->nsc_pi->pi_slot, sc->nsc_pi->pi_func);
			sc->nvstore.ctx = blockif_open(xopts, bident);
			if (sc->nvstore.ctx == NULL) {
				perror("Could not open backing file");
				free(uopt);
				return (-1);
			}
			sc->nvstore.type = NVME_STOR_BLOCKIF;
			sc->nvstore.size = blockif_size(sc->nvstore.ctx);
		} else {
			EPRINTLN("Invalid option %s", xopts);
			free(uopt);
			return (-1);
		}

		optidx++;
	}
	free(uopt);

	if (sc->nvstore.ctx == NULL || sc->nvstore.size == 0) {
		EPRINTLN("backing store not specified");
		return (-1);
	}
	if (sectsz == 512 || sectsz == 4096 || sectsz == 8192)
		sc->nvstore.sectsz = sectsz;
	else if (sc->nvstore.type != NVME_STOR_RAM)
		sc->nvstore.sectsz = blockif_sectsz(sc->nvstore.ctx);
	for (sc->nvstore.sectsz_bits = 9;
	     (1 << sc->nvstore.sectsz_bits) < sc->nvstore.sectsz;
	     sc->nvstore.sectsz_bits++);

	if (sc->max_queues <= 0 || sc->max_queues > NVME_QUEUES)
		sc->max_queues = NVME_QUEUES;

	if (sc->max_qentries <= 0) {
		EPRINTLN("Invalid qsz option");
		return (-1);
	}
	if (sc->ioslots <= 0) {
		EPRINTLN("Invalid ioslots option");
		return (-1);
	}

	return (0);
}

static int
pci_nvme_init(struct vmctx *ctx, struct pci_devinst *pi, char *opts)
{
	struct pci_nvme_softc *sc;
	uint32_t pci_membar_sz;
	int	error;

	error = 0;

	sc = calloc(1, sizeof(struct pci_nvme_softc));
	pi->pi_arg = sc;
	sc->nsc_pi = pi;

	error = pci_nvme_parse_opts(sc, opts);
	if (error < 0)
		goto done;
	else
		error = 0;

	sc->ioreqs = calloc(sc->ioslots, sizeof(struct pci_nvme_ioreq));
	for (int i = 0; i < sc->ioslots; i++) {
		if (i < (sc->ioslots-1))
			sc->ioreqs[i].next = &sc->ioreqs[i+1];
		pthread_mutex_init(&sc->ioreqs[i].mtx, NULL);
		pthread_cond_init(&sc->ioreqs[i].cv, NULL);
	}
	sc->ioreqs_free = sc->ioreqs;
	sc->intr_coales_aggr_thresh = 1;

	pci_set_cfgdata16(pi, PCIR_DEVICE, 0x0A0A);
	pci_set_cfgdata16(pi, PCIR_VENDOR, 0xFB5D);
	pci_set_cfgdata8(pi, PCIR_CLASS, PCIC_STORAGE);
	pci_set_cfgdata8(pi, PCIR_SUBCLASS, PCIS_STORAGE_NVM);
	pci_set_cfgdata8(pi, PCIR_PROGIF,
	                 PCIP_STORAGE_NVM_ENTERPRISE_NVMHCI_1_0);

	/*
	 * Allocate size of NVMe registers + doorbell space for all queues.
	 *
	 * The specification requires a minimum memory I/O window size of 16K.
	 * The Windows driver will refuse to start a device with a smaller
	 * window.
	 */
	pci_membar_sz = sizeof(struct nvme_registers) +
	    2 * sizeof(uint32_t) * (sc->max_queues + 1);
	pci_membar_sz = MAX(pci_membar_sz, NVME_MMIO_SPACE_MIN);

	DPRINTF(("nvme membar size: %u", pci_membar_sz));

	error = pci_emul_alloc_bar(pi, 0, PCIBAR_MEM64, pci_membar_sz);
	if (error) {
		WPRINTF(("%s pci alloc mem bar failed", __func__));
		goto done;
	}

	error = pci_emul_add_msixcap(pi, sc->max_queues + 1, NVME_MSIX_BAR);
	if (error) {
		WPRINTF(("%s pci add msixcap failed", __func__));
		goto done;
	}

	error = pci_emul_add_pciecap(pi, PCIEM_TYPE_ROOT_INT_EP);
	if (error) {
		WPRINTF(("%s pci add Express capability failed", __func__));
		goto done;
	}

	pthread_mutex_init(&sc->mtx, NULL);
	sem_init(&sc->iosemlock, 0, sc->ioslots);

	pci_nvme_reset(sc);
	pci_nvme_init_ctrldata(sc);
	pci_nvme_init_nsdata(sc, &sc->nsdata, 1, sc->nvstore.eui64);
	pci_nvme_init_logpages(sc);

	pci_lintr_request(pi);

done:
	return (error);
}


struct pci_devemu pci_de_nvme = {
	.pe_emu =	"nvme",
	.pe_init =	pci_nvme_init,
	.pe_barwrite =	pci_nvme_write,
	.pe_barread =	pci_nvme_read
};
PCI_EMUL_SET(pci_de_nvme);
