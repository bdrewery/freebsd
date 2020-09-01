/*-
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2016 Landon Fuller <landon@landonf.org>
 * Copyright (c) 2015 Broadcom Corporation
 * All rights reserved.
 *
 * This file is derived from the pcie_core.h and pcie2_core.h headers
 * from Broadcom's Linux driver sources as distributed by dd-wrt.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $FreeBSD$
 */

#ifndef _BHND_CORES_PCIE2_BHND_PCIE2_REG_H_
#define _BHND_CORES_PCIE2_BHND_PCIE2_REG_H_

/*
 * PCIe-Gen2 DMA Constants
 */

#define	BHND_PCIE2_DMA64_TRANSLATION	0x8000000000000000	/**< PCIe-Gen2 DMA64 address translation */
#define	BHND_PCIE2_DMA64_MASK		0xc000000000000000	/**< PCIe-Gen2 DMA64 translation mask */

/*
 * PCIe-Gen2 Core Registers
 */

#define	BHND_PCIE2_CLK_CONTROL		0x000

#define	BHND_PCIE2_RC_PM_CONTROL	0x004
#define	BHND_PCIE2_RC_PM_STATUS		0x008
#define	BHND_PCIE2_EP_PM_CONTROL	0x00C
#define	BHND_PCIE2_EP_PM_STATUS		0x010
#define	BHND_PCIE2_EP_LTR_CONTROL	0x014
#define	BHND_PCIE2_EP_LTR_STATUS	0x018
#define	BHND_PCIE2_EP_OBFF_STATUS	0x01C
#define	BHND_PCIE2_PCIE_ERR_STATUS	0x020
#define	BHND_PCIE2_RC_AXI_CONFIG	0x100
#define	BHND_PCIE2_EP_AXI_CONFIG	0x104
#define	BHND_PCIE2_RXDEBUG_STATUS0	0x108
#define	BHND_PCIE2_RXDEBUG_CONTROL0	0x10C

#define	BHND_PCIE2_CONFIGINDADDR	0x120
#define	BHND_PCIE2_CONFIGINDDATA	0x124

#define	BHND_PCIE2_CFG_ADDR		0x1F8
#define	BHND_PCIE2_CFG_DATA		0x1FC

#define	BHND_PCIE2_SYS_EQ_PAGE		0x200
#define	BHND_PCIE2_SYS_MSI_PAGE		0x204
#define	BHND_PCIE2_SYS_MSI_INTREN	0x208
#define	BHND_PCIE2_SYS_MSI_CTRL0	0x210
#define	BHND_PCIE2_SYS_MSI_CTRL1	0x214
#define	BHND_PCIE2_SYS_MSI_CTRL2	0x218
#define	BHND_PCIE2_SYS_MSI_CTRL3	0x21C
#define	BHND_PCIE2_SYS_MSI_CTRL4	0x220
#define	BHND_PCIE2_SYS_MSI_CTRL5	0x224

#define	BHND_PCIE2_SYS_EQ_HEAD0		0x250
#define	BHND_PCIE2_SYS_EQ_TAIL0		0x254
#define	BHND_PCIE2_SYS_EQ_HEAD1		0x258
#define	BHND_PCIE2_SYS_EQ_TAIL1		0x25C
#define	BHND_PCIE2_SYS_EQ_HEAD2		0x260
#define	BHND_PCIE2_SYS_EQ_TAIL2		0x264
#define	BHND_PCIE2_SYS_EQ_HEAD3		0x268
#define	BHND_PCIE2_SYS_EQ_TAIL3		0x26C
#define	BHND_PCIE2_SYS_EQ_HEAD4		0x270
#define	BHND_PCIE2_SYS_EQ_TAIL4		0x274
#define	BHND_PCIE2_SYS_EQ_HEAD5		0x278
#define	BHND_PCIE2_SYS_EQ_TAIL5		0x27C

#define	BHND_PCIE2_SYS_RC_INTX_EN	0x330
#define	BHND_PCIE2_SYS_RC_INTX_CSR	0x334
#define	BHND_PCIE2_SYS_MSI_REQ		0x340
#define	BHND_PCIE2_SYS_HOST_INTR_EN	0x344
#define	BHND_PCIE2_SYS_HOST_INTR_CSR	0x348
#define	BHND_PCIE2_SYS_HOST_INTR0	0x350
#define	BHND_PCIE2_SYS_HOST_INTR1	0x354
#define	BHND_PCIE2_SYS_HOST_INTR2	0x358
#define	BHND_PCIE2_SYS_HOST_INTR3	0x35C
#define	BHND_PCIE2_SYS_EP_INT_EN0	0x360
#define	BHND_PCIE2_SYS_EP_INT_EN1	0x364
#define	BHND_PCIE2_SYS_EP_INT_CSR0	0x370
#define	BHND_PCIE2_SYS_EP_INT_CSR1	0x374

#define	BHND_PCIE2_MDIO_CTL		0x128	/**< mdio control */
#define	BHND_PCIE2_MDIO_WRDATA		0x12C	/**< mdio data write */
#define	BHND_PCIE2_MDIO_RDDATA		0x130	/**< mdio data read */

/* DMA doorbell registers (>= rev5) */
#define	BHND_PCIE2_DB0_HOST2DEV0	0x140
#define	BHND_PCIE2_DB0_HOST2DEV1	0x144
#define	BHND_PCIE2_DB0_DEV2HOST0	0x148
#define	BHND_PCIE2_DB0_DEV2HOST1	0x14C

#define	BHND_PCIE2_DB1_HOST2DEV0	0x150
#define	BHND_PCIE2_DB1_HOST2DEV1	0x154
#define	BHND_PCIE2_DB1_DEV2HOST0	0x158
#define	BHND_PCIE2_DB1_DEV2HOST1	0x15C

#define	BHND_PCIE2_DB2_HOST2DEV0	0x160
#define	BHND_PCIE2_DB2_HOST2DEV1	0x164
#define	BHND_PCIE2_DB2_DEV2HOST0	0x168
#define	BHND_PCIE2_DB2_DEV2HOST1	0x16C

#define	BHND_PCIE2_DB3_HOST2DEV0	0x170
#define	BHND_PCIE2_DB3_HOST2DEV1	0x174
#define	BHND_PCIE2_DB3_DEV2HOST0	0x178
#define	BHND_PCIE2_DB3_DEV2HOST1	0x17C

#define	BHND_PCIE2_DATAINTF		0x180
#define	BHND_PCIE2_INTRLAZY0_DEV2HOST	0x188
#define	BHND_PCIE2_INTRLAZY0_HOST2DEV	0x18c
#define	BHND_PCIE2_INTSTAT0_HOST2DEV	0x190
#define	BHND_PCIE2_INTMASK0_HOST2DEV	0x194
#define	BHND_PCIE2_INTSTAT0_DEV2HOST	0x198
#define	BHND_PCIE2_INTMASK0_DEV2HOST	0x19c
#define	BHND_PCIE2_LTR_STATE		0x1A0
#define	BHND_PCIE2_PWR_INT_STATUS	0x1A4
#define	BHND_PCIE2_PWR_INT_MASK		0x1A8

/* DMA channel registers */
#define	BHND_PCIE2_DMA0_HOST2DEV_TX	0x200
#define	BHND_PCIE2_DMA0_HOST2DEV_RX	0x220
#define	BHND_PCIE2_DMA0_DEV2HOST_TX	0x240
#define	BHND_PCIE2_DMA0_DEV2HOST_RX	0x260

#define	BHND_PCIE2_DMA1_HOST2DEV_TX	0x280
#define	BHND_PCIE2_DMA1_HOST2DEV_RX	0x2A0
#define	BHND_PCIE2_DMA1_DEV2HOST_TX	0x2C0
#define	BHND_PCIE2_DMA1_DEV2HOST_RX	0x2E0

#define	BHND_PCIE2_DMA2_HOST2DEV_TX	0x300
#define	BHND_PCIE2_DMA2_HOST2DEV_RX	0x320
#define	BHND_PCIE2_DMA2_DEV2HOST_TX	0x340
#define	BHND_PCIE2_DMA2_DEV2HOST_RX	0x360

#define	BHND_PCIE2_DMA3_HOST2DEV_TX	0x380
#define	BHND_PCIE2_DMA3_HOST2DEV_RX	0x3A0
#define	BHND_PCIE2_DMA3_DEV2HOST_TX	0x3C0
#define	BHND_PCIE2_DMA3_DEV2HOST_RX	0x3E0

#define	BHND_PCIE2_PCIE_FUNC0_CFG	0x400	/**< PCIe function 0 config space */
#define	BHND_PCIE2_PCIE_FUNC1_CFG	0x500	/**< PCIe function 1 config space */
#define	BHND_PCIE2_PCIE_FUNC2_CFG	0x600	/**< PCIe function 2 config space */
#define	BHND_PCIE2_PCIE_FUNC3_CFG	0x700	/**< PCIe function 3 config space */
#define	BHND_PCIE2_SPROM		0x800	/**< SPROM shadow */

#define	BHND_PCIE2_FUNC0_IMAP0_0	0xC00
#define	BHND_PCIE2_FUNC0_IMAP0_1	0xC04
#define	BHND_PCIE2_FUNC0_IMAP0_2	0xC08
#define	BHND_PCIE2_FUNC0_IMAP0_3	0xC0C
#define	BHND_PCIE2_FUNC0_IMAP0_4	0xC10
#define	BHND_PCIE2_FUNC0_IMAP0_5	0xC14
#define	BHND_PCIE2_FUNC0_IMAP0_6	0xC18
#define	BHND_PCIE2_FUNC0_IMAP0_7	0xC1C

#define	BHND_PCIE2_FUNC1_IMAP0_0	0xC20
#define	BHND_PCIE2_FUNC1_IMAP0_1	0xC24
#define	BHND_PCIE2_FUNC1_IMAP0_2	0xC28
#define	BHND_PCIE2_FUNC1_IMAP0_3	0xC2C
#define	BHND_PCIE2_FUNC1_IMAP0_4	0xC30
#define	BHND_PCIE2_FUNC1_IMAP0_5	0xC34
#define	BHND_PCIE2_FUNC1_IMAP0_6	0xC38
#define	BHND_PCIE2_FUNC1_IMAP0_7	0xC3C

#define	BHND_PCIE2_FUNC0_IMAP1		0xC80
#define	BHND_PCIE2_FUNC1_IMAP1		0xC88
#define	BHND_PCIE2_FUNC0_IMAP2		0xCC0
#define	BHND_PCIE2_FUNC1_IMAP2		0xCC8

#define	BHND_PCIE2_IARR0_LOWER		0xD00
#define	BHND_PCIE2_IARR0_UPPER		0xD04
#define	BHND_PCIE2_IARR1_LOWER		0xD08
#define	BHND_PCIE2_IARR1_UPPER		0xD0C
#define	BHND_PCIE2_IARR2_LOWER		0xD10
#define	BHND_PCIE2_IARR2_UPPER		0xD14
#define	BHND_PCIE2_OARR0		0xD20
#define	BHND_PCIE2_OARR1		0xD28
#define	BHND_PCIE2_OARR2		0xD30
#define	BHND_PCIE2_OMAP0_LOWER		0xD40
#define	BHND_PCIE2_OMAP0_UPPER		0xD44
#define	BHND_PCIE2_OMAP1_LOWER		0xD48
#define	BHND_PCIE2_OMAP1_UPPER		0xD4C
#define	BHND_PCIE2_OMAP2_LOWER		0xD50
#define	BHND_PCIE2_OMAP2_UPPER		0xD54
#define	BHND_PCIE2_FUNC1_IARR1_SIZE	0xD58
#define	BHND_PCIE2_FUNC1_IARR2_SIZE	0xD5C
#define	BHND_PCIE2_MEM_CONTROL		0xF00
#define	BHND_PCIE2_MEM_ECC_ERRLOG0	0xF04
#define	BHND_PCIE2_MEM_ECC_ERRLOG1	0xF08
#define	BHND_PCIE2_LINK_STATUS		0xF0C
#define	BHND_PCIE2_STRAP_STATUS		0xF10
#define	BHND_PCIE2_RESET_STATUS		0xF14
#define	BHND_PCIE2_RESETEN_IN_LINKDOWN	0xF18
#define	BHND_PCIE2_MISC_INTR_EN		0xF1C
#define	BHND_PCIE2_TX_DEBUG_CFG		0xF20
#define	BHND_PCIE2_MISC_CONFIG		0xF24
#define	BHND_PCIE2_MISC_STATUS		0xF28
#define	BHND_PCIE2_INTR_EN		0xF30
#define	BHND_PCIE2_INTR_CLEAR		0xF34
#define	BHND_PCIE2_INTR_STATUS		0xF38

/* BHND_PCIE2_MDIO_CTL */
#define	BHND_PCIE2_MDIOCTL_DIVISOR_MASK		0x7f    /* clock to be used on MDIO */
#define	BHND_PCIE2_MDIOCTL_DIVISOR_VAL		0x2
#define	BHND_PCIE2_MDIOCTL_REGADDR_SHIFT	8               /* Regaddr shift */
#define	BHND_PCIE2_MDIOCTL_REGADDR_MASK		0x00FFFF00      /* Regaddr Mask */
#define	BHND_PCIE2_MDIOCTL_DEVADDR_SHIFT	24              /* Physmedia devaddr shift */
#define	BHND_PCIE2_MDIOCTL_DEVADDR_MASK		0x0f000000      /* Physmedia devaddr Mask */
#define	BHND_PCIE2_MDIOCTL_SLAVE_BYPASS		0x10000000      /* IP slave bypass */
#define	BHND_PCIE2_MDIOCTL_READ			0x20000000      /* IP slave bypass */

/* BHND_PCIE2_MDIO_DATA */
#define	BHND_PCIE2_MDIODATA_DONE		0x80000000      /* rd/wr transaction done */
#define	BHND_PCIE2_MDIODATA_MASK		0x7FFFFFFF      /* rd/wr transaction data */
#define	BHND_PCIE2_MDIODATA_DEVADDR_SHIFT	4               /* Physmedia devaddr shift */

/* BHND_PCIE2_DMA[0-4]_HOST2DEV_(TX|RX) per-channel register offsets */
#define	BHND_PCIE2_DMA_CTRL		0x0	/**< enable, et al */
#define	BHND_PCIE2_DMA_PTR		0x4	/**< last descriptor posted to chip */
#define	BHND_PCIE2_DMA_ADDRL		0x8	/**< descriptor ring base address low 32-bits (8K aligned) */
#define	BHND_PCIE2_DMA_ADDRH		0xC	/**< descriptor ring base address bits 63:32 (8K aligned) */
#define	BHND_PCIE2_DMA_STATUS0		0x10	/**< current descriptor, xmt state */
#define	BHND_PCIE2_DMA_STATUS1		0x10	/**< active descriptor, xmt error */

#endif /* _BHND_CORES_PCIE2_BHND_PCIE2_REG_H_ */
