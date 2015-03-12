/*-
 * Copyright (c) 2014 The FreeBSD Foundation
 * All rights reserved.
 *
 * This software was developed by Semihalf under
 * the sponsorship of the FreeBSD Foundation.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* PCIe root complex driver for Cavium Thunder SOC */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/malloc.h>
#include <sys/kernel.h>
#include <sys/rman.h>
#include <sys/module.h>
#include <sys/bus.h>
#include <sys/endian.h>
#include <sys/cpuset.h>
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#include <dev/pci/pcivar.h>
#include <dev/pci/pcireg.h>
#include <dev/pci/pcib_private.h>
#include <machine/cpu.h>
#include <machine/bus.h>
#include <machine/fdt.h>
#include <machine/intr.h>
#include <dev/fdt/fdt_common.h>

#include "pcib_if.h"

/* Assembling ECAM Configuration Address */
#define PCIE_BUS_SHIFT	20
#define PCIE_SLOT_SHIFT 15
#define PCIE_FUNC_SHIFT 12
#define PCIE_BUS_MASK	0xFF
#define PCIE_SLOT_MASK	0x1F
#define PCIE_FUNC_MASK	0x07
#define PCIE_REG_MASK	0xFFF

#define PCIE_ADDR_OFFSET(bus, slot, func, reg)			\
	((((bus) & PCIE_BUS_MASK) << PCIE_BUS_SHIFT)	|	\
	(((slot) & PCIE_SLOT_MASK) << PCIE_SLOT_SHIFT)	|	\
	(((func) & PCIE_FUNC_MASK) << PCIE_FUNC_SHIFT)	|	\
	((reg) & PCIE_REG_MASK))

#define MAX_RANGES_TUPLES	5
#define MIN_RANGES_TUPLES	2

#define THUNDER_ECAM0_CFG_BASE		0x848000000000UL
#define THUNDER_ECAM1_CFG_BASE		0x849000000000UL
#define THUNDER_ECAM2_CFG_BASE		0x84a000000000UL
#define THUNDER_ECAM3_CFG_BASE		0x84b000000000UL
#define THUNDER_ECAM4_CFG_BASE		0x948000000000UL
#define THUNDER_ECAM5_CFG_BASE		0x949000000000UL
#define THUNDER_ECAM6_CFG_BASE		0x94a000000000UL
#define THUNDER_ECAM7_CFG_BASE		0x94b000000000UL

#define THUNDER_PEMn_REG_BASE(unit)	(0x87e0c0000000UL | ((unit) << 24))

#define PCIERC_CFG002	0x08
#define PCIERC_CFG006	0x18
#define PCIERC_CFG032	0x80
#define SBNUM_OFFSET	8
#define SBNUM_MASK	0xFF
#define PEM_ON_REG	0x420
#define PEM_CTL_STATUS	0x0
#define PEM_LINK_ENABLE (1 << 4)
#define PEM_LINK_DLLA	(1 << 29)
#define PEM_LINK_LT	(1 << 27)

#define MEM_BAR_MIN_LOG2SIZE	4
#define IO_BAR_MIN_LOG2SIZE	2

#define SLIX_S2M_REGX_ACC		0x874001000000UL
#define SLIX_S2M_REGX_ACC_SIZE		0x1000

struct pcie_range {
	uint64_t	pci_base;
	uint64_t	phys_base;
	uint64_t	size;
};

enum pcie_type {
	THUNDER_ECAM,
	THUNDER_PEM,
};

struct thunder_pcie_softc {
	struct pcie_range	ranges[MAX_RANGES_TUPLES];
	struct rman		mem_rman;
	struct resource		*res;
	int			ecam;
	int			pem;
	bus_space_tag_t		bst;
	bus_space_handle_t      bsh;
	device_t		dev;
	enum pcie_type		type;
	bus_space_handle_t	pem_sli_base;
	uint64_t		pem_bar_freemempos;
	uint64_t		pem_bar_mem_bound;
};

/*
 * Early system init.
 * Register Thunder-specific function to acquire PCI device ID for ITS to use.
 */
static uint32_t thunder_its_get_pci_devid(device_t);

static void
thunder_pcie_set_devid_func(void *dummy __unused)
{

	its_set_devid_func(thunder_its_get_pci_devid);
}
SYSINIT(set_devid_func, SI_SUB_DRIVERS, SI_ORDER_ANY,
    thunder_pcie_set_devid_func, NULL);

/* Forward prototypes */

static int thunder_pcie_probe(device_t dev);
static int thunder_pcie_attach(device_t dev);
static int parse_pci_mem_ranges(struct thunder_pcie_softc *sc);
static uint64_t range_addr_pci_to_phys(struct thunder_pcie_softc *sc,
    uint64_t pci_addr);
static uint32_t thunder_pcie_read_config(device_t dev, u_int bus, u_int slot,
    u_int func, u_int reg, int bytes);
static void thunder_pcie_write_config(device_t dev, u_int bus, u_int slot,
    u_int func, u_int reg, uint32_t val, int bytes);
static int thunder_pcie_maxslots(device_t dev);
static int thunder_pcie_read_ivar(device_t dev, device_t child, int index,
    uintptr_t *result);
static int thunder_pcie_write_ivar(device_t dev, device_t child, int index,
    uintptr_t value);
static struct resource *thunder_pcie_alloc_resource(device_t dev,
    device_t child, int type, int *rid, u_long start, u_long end,
    u_long count, u_int flags);
static int thunder_pcie_release_resource(device_t dev, device_t child,
    int type, int rid, struct resource *res);
static int thunder_pcie_identify_pcib(device_t dev);
static int thunder_pcie_map_msi(device_t pcib, device_t child, int irq,
    uint64_t *addr, uint32_t *data);
static int thunder_pcie_alloc_msix(device_t pcib, device_t child, int *irq);
static int thunder_pcie_release_msix(device_t pcib, device_t child, int irq);
static int thunder_pcie_alloc_msi(device_t pcib, device_t child, int count,
    int maxcount, int *irqs);
static int thunder_pcie_release_msi(device_t pcib, device_t child, int count,
    int *irqs);
static void modify_slix_s2m_regx_acc(int sli, int reg);
static uint64_t thunder_pem_config_read(struct thunder_pcie_softc *sc, int reg);
static int thunder_pem_link_init(struct thunder_pcie_softc *sc);
static int thunder_pem_init(struct thunder_pcie_softc *sc);
static int thunder_pem_assign_resources(struct thunder_pcie_softc *sc, int bus);
static int thunder_pem_init_empty_bars(struct thunder_pcie_softc *sc,
    int bus, int slot, int func);
static int thunder_pem_init_bar(struct thunder_pcie_softc *sc,
    int bus, int slot, int func, int barno);
static void thunder_pem_read_bar(struct thunder_pcie_softc *sc, int bus,
    int slot, int func, int reg, pci_addr_t *mapp, pci_addr_t *testvalp,
    int *barlen);
static int get_pci_mapsize(uint64_t testval);

bus_space_handle_t sli0_s2m_regx_base = 0;
bus_space_handle_t sli1_s2m_regx_base = 0;

static int
thunder_pcie_probe(device_t dev)
{

	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	if (ofw_bus_is_compatible(dev, "cavium,thunder-pcie")) {
		device_set_desc(dev, "Cavium Integrated PCI/PCI-E Controller");
		return (BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

static int
thunder_pcie_attach(device_t dev)
{
	int rid;
	struct thunder_pcie_softc *sc;
	int error;
	int tuple;
	uint64_t base, size;

	sc = device_get_softc(dev);
	sc->dev = dev;

	/* Identify pcib domain */
	if (thunder_pcie_identify_pcib(dev))
		return (ENXIO);

	rid = 0;
	sc->res = bus_alloc_resource_any(dev, SYS_RES_MEMORY, &rid, RF_ACTIVE);
	if (sc->res == NULL) {
		device_printf(dev, "could not map memory.\n");
		return (ENXIO);
	}

	if (!sli0_s2m_regx_base)
		bus_space_map(fdtbus_bs_tag, SLIX_S2M_REGX_ACC,
		    SLIX_S2M_REGX_ACC_SIZE, 0, &sli0_s2m_regx_base);
	if (!sli1_s2m_regx_base)
		bus_space_map(fdtbus_bs_tag, SLIX_S2M_REGX_ACC | (1ul << 36),
		    SLIX_S2M_REGX_ACC_SIZE, 0, &sli1_s2m_regx_base);

	if (!sli0_s2m_regx_base || !sli1_s2m_regx_base) {
		device_printf(dev,
		    "bus_space_map failed to map slix_s2m_regx_base\n");
		return (ENXIO);
	}

	sc->bst = rman_get_bustag(sc->res);
	sc->bsh = rman_get_bushandle(sc->res);

	sc->mem_rman.rm_type = RMAN_ARRAY;
	sc->mem_rman.rm_descr = "PCIe Memory";

	/* Retrieve 'ranges' property from FDT */

	if (bootverbose) {
		if (sc->type == THUNDER_ECAM)
			device_printf(dev, "parsing FDT for ECAM%d:\n",
			    sc->ecam);
		else
			device_printf(dev, "parsing FDT for PEM%d:\n",
			    sc->pem);
	}
	if (parse_pci_mem_ranges(sc))
		return (ENXIO);

	/* Initialize rman and allocate memory regions */

	error = rman_init(&sc->mem_rman);
	if (error) {
		device_printf(dev, "rman_init() failed. error = %d\n", error);
		return (error);
	}

	for (tuple = 0; tuple < MAX_RANGES_TUPLES; tuple++) {
		base = sc->ranges[tuple].phys_base;
		size = sc->ranges[tuple].size;
		if (base == 0 || size == 0)
			continue; /* empty range element */

		error = rman_manage_region(&sc->mem_rman, base, base + size - 1);
		if (error) {
			device_printf(dev, "rman_manage_region() failed. error = %d\n", error);
			rman_fini(&sc->mem_rman);
			return (error);
		}
	}

	/* Initialize PEM */

	if (sc->type == THUNDER_PEM) {
		if (thunder_pem_init(sc)) {
			device_printf(dev, "Failure during PEM init\n");
			return (ENXIO);
		}

		/* Read PEM secondary bus number from PCIe RC conf register */
		int secondary_bus;
		secondary_bus = thunder_pem_config_read(sc, PCIERC_CFG006);
		secondary_bus = (secondary_bus >> SBNUM_OFFSET) & SBNUM_MASK;

		/* Allocate memory for devices on this bus, initialize BARs */
		error = thunder_pem_assign_resources(sc, secondary_bus);
		if (error) {
			return (error);
		}
	}

	device_add_child(dev, "pci", -1);

	return (bus_generic_attach(dev));
}

static void
modify_slix_s2m_regx_acc(int sli, int reg)
{
	uint64_t regval;
	bus_space_handle_t handle = 0;

	KASSERT(reg >= 0 && reg <= 255, ("Invalid SLI reg"));

	if (sli == 0) {
		handle = sli0_s2m_regx_base;
	}
	else if (sli == 1) {
		handle = sli1_s2m_regx_base;
	}
	else {
		printf("SLI id is not correct\n");
	}

	if (handle) {
		regval = bus_space_read_8(fdtbus_bs_tag, handle, reg << 4);
		regval &= ~(0xFFFFFFFFul);
		bus_space_write_8(fdtbus_bs_tag, handle, reg << 4, regval);
	}
}

#define PEM_CFG_RD	0x30

static uint64_t
thunder_pem_config_read(struct thunder_pcie_softc *sc, int reg)
{
	uint64_t data;

	/* Write to ADDR register */
	bus_space_write_8(sc->bst, sc->bsh, PEM_CFG_RD, reg & ~0x3);
	bus_space_barrier(sc->bst, sc->bsh, PEM_CFG_RD, 8,
	    BUS_SPACE_BARRIER_READ | BUS_SPACE_BARRIER_WRITE);
	/* Read from DATA register */
	data = bus_space_read_8(sc->bst, sc->bsh, PEM_CFG_RD) >> 32;

	return (data);
}

static int
thunder_pem_link_init(struct thunder_pcie_softc *sc)
{
	uint64_t regval;

	/* check whether PEM is safe to access. */
	regval = bus_space_read_8(sc->bst, sc->bsh, PEM_ON_REG);
	if ((regval & 0x3) != 0x3) {
		device_printf(sc->dev, "PEM%d is not ON\n", sc->pem);
		return (ENXIO);
	}

	regval = bus_space_read_8(sc->bst, sc->bsh, PEM_CTL_STATUS);
	regval |= PEM_LINK_ENABLE;
	bus_space_write_8(sc->bst, sc->bsh, PEM_CTL_STATUS, regval);

	DELAY(1000);
	regval = thunder_pem_config_read(sc, PCIERC_CFG032);

	if (((regval & PEM_LINK_DLLA) == 0) || (regval & PEM_LINK_LT)) {
		device_printf(sc->dev, "PCIe RC: Port %d Link Timeout\n", sc->pem);
		return (ENXIO);
	}

	return (0);
}

static int
thunder_pem_init(struct thunder_pcie_softc *sc)
{
	uint64_t pem_addr;
	uint64_t region;
	uint64_t sli_group;
	uint64_t sli;
	int i, retval = 0;

	switch (sc->pem) {
	case 0:
		sli =  0;
		sli_group = 0;
		break;

	case 1:
		sli =  0;
		sli_group = 1;
		break;
	case 2:
		sli =  0;
		sli_group = 2;
		break;
	case 3:
		sli =  1;
		sli_group = 0;
		break;
	case 4:
		sli =  1;
		sli_group = 1;
		break;
	case 5:
		sli =  1;
		sli_group = 2;
		break;
	default:
		return (ENXIO);
	}

	retval = thunder_pem_link_init(sc);
	if (retval) {
		device_printf(sc->dev, "%s failed\n", __func__);
		return retval;
	}

	/* To support 32-bit PCIe devices, set S2M_REGx_ACC[BA]=0x0 */
	for (i = 0; i < 255; i++) {
		modify_slix_s2m_regx_acc(sli, i);
	}

	/* PEM number and access type */
	region = ((sli_group << 6) | (0ul << 4)) << 32;
	pem_addr = (1ul << 47) | ((0x8 + sli) << 40) | region;

	retval = bus_space_map(fdtbus_bs_tag, pem_addr, (0xFFul << 24) - 1,
	    0, &sc->pem_sli_base);
	if (retval) {
		device_printf(sc->dev,
		    "Unable to map RC%d pem_addr base address", sc->pem);
		return (ENOMEM);
	}

	return (retval);
}

static int
thunder_pem_assign_resources(struct thunder_pcie_softc *sc, int bus)
{
	int slot, func, error;
	uint8_t hdrtype, command;

	/* Set initial free mem position */
	/* XXX ARM64TODO: Check which range should be used */
	sc->pem_bar_freemempos = sc->ranges[1].pci_base;
	sc->pem_bar_mem_bound = sc->ranges[1].pci_base + sc->ranges[1].size;

	/* Scan all devices and functions on given bus */
	for (slot = 0; slot <= PCI_SLOTMAX; slot++) {
		for (func = 0; func <= PCI_FUNCMAX; func++) {
			if (thunder_pcie_read_config(sc->dev, bus, slot, func,
			    PCIR_VENDOR, 4) == 0xFFFFFFFF)
				continue; /* no device */

			hdrtype = thunder_pcie_read_config(sc->dev, bus, slot,
			    func, PCIR_HDRTYPE, 1);
			if ((hdrtype & PCIM_HDRTYPE) != PCIM_HDRTYPE_NORMAL)
				continue; /* ignore bridge devices for now */

			/* Disable memory space access for device */
			command = thunder_pcie_read_config(sc->dev, bus, slot,
			    func, PCIR_COMMAND, 1);
			command &= ~(PCIM_CMD_MEMEN | PCIM_CMD_PORTEN);
			thunder_pcie_write_config(sc->dev, bus, slot, func,
			    PCIR_COMMAND, command, 1);

			/* Initialize all empty BARs for device */
			error = thunder_pem_init_empty_bars(sc, bus, slot,
			    func);

			if (error)
				return (error);

			/* Enable memory space access for device */
			command |= PCIM_CMD_BUSMASTEREN | PCIM_CMD_MEMEN;
			thunder_pcie_write_config(sc->dev, bus, slot, func,
			    PCIR_COMMAND, command, 1);
		}
	}

	return (0);
}

static int
thunder_pem_init_empty_bars(struct thunder_pcie_softc *sc, int bus, int slot,
    int func)
{
	int bar, i;

	bar = 0;

	/* Program the base address registers */
	while (bar < PCI_MAXMAPS_0) {
		i = thunder_pem_init_bar(sc, bus, slot, func, bar);
		bar += i;
		if (i < 0) {
			device_printf(sc->dev,
			    "BAR memory allocation error\n");
			return (ENOMEM);
		}
	}

	return (0);
}

static int
get_pci_mapsize(uint64_t testval)
{
        int ln2size;

	/* This function has been adapted based on 'pci_mapsize' in pci.c.
	 * Returns log2 of map size decoded for memory or port map.
	 */
	testval &= PCIM_BAR_MEM_BASE;
        ln2size = 0;
        if (testval != 0) {
                while ((testval & 1) == 0)
                {
                        ln2size++;
                        testval >>= 1;
                }
        }
        return (ln2size);
}

static int
thunder_pem_init_bar(struct thunder_pcie_softc *sc,
    int bus, int slot, int func, int barno)
{
	pci_addr_t base, map, testval, count, addr;
	int barlen, mapsize, reg;

	reg = PCIR_BAR(barno);

	thunder_pem_read_bar(sc, bus, slot, func, reg, &map, &testval, &barlen);

	/* Do not allocate I/O BARs */
	if (PCI_BAR_IO(map)) {
		if (bootverbose) {
			device_printf(sc->dev, "Device at bsf=(%d:%d:%d) "
			    "requested unsupported I/O port memory. "
			    "Skipping BAR%d.\n", bus, slot, func, barno);
		}
		return (barlen);
	}

	base = map & PCIM_BAR_MEM_BASE;
	if (base != 0) {
		/* BAR is already initialized (non-zero address) */
		device_printf(sc->dev,
		    "Error: device BAR%d is already initialized. Addr=0x%jx\n",
		    barno, base);
		return (-1);
	}

	mapsize = get_pci_mapsize(testval);
	count = (pci_addr_t)1 << mapsize;

	/*
	 * For I/O registers, if bottom bit is set, and the next bit up
	 * isn't clear, we know we have a BAR that doesn't conform to the
	 * spec, so ignore it.  Also, sanity check the size of the data
	 * areas to the type of memory involved.  Memory must be at least
	 * 16 bytes in size, while I/O ranges must be at least 4.
	 */
	if (PCI_BAR_IO(testval) && (testval & PCIM_BAR_IO_RESERVED) != 0)
		return (barlen);
	if ((PCI_BAR_MEM(map) && mapsize < MEM_BAR_MIN_LOG2SIZE) ||
	    (PCI_BAR_IO(map) && mapsize < IO_BAR_MIN_LOG2SIZE))
		return (barlen);

	/* Allocate BAR: align mem address to BAR size */
	sc->pem_bar_freemempos = roundup2(sc->pem_bar_freemempos, count);
	addr = sc->pem_bar_freemempos;
	/* Move free memory position pointer */
	sc->pem_bar_freemempos += count;

	if (!addr || (sc->pem_bar_freemempos > sc->pem_bar_mem_bound)) {
		/* Address is 0 or memory bound exceeded */
		device_printf(sc->dev,
		    "Error: address went outside bounds. "
		    "Addr = 0x%jx, bound = 0x%jx\n",
		    addr, sc->pem_bar_mem_bound);
		return (-1);
	}

	if (bootverbose) {
		device_printf(sc->dev, "Allocating BAR%d for device at: "
		    "bus=%d, slot=%d, func=%d\n"
		    "\tAddr: 0x%jx, Size: 0x%jx\n", barno, bus, slot, func,
		    addr, count);
	}

	/* Write address to BAR */
	thunder_pcie_write_config(sc->dev, bus, slot, func, reg,
	    (uint32_t)addr, 4);
	if (barlen == 2)
		thunder_pcie_write_config(sc->dev, bus, slot, func,
		    reg + 4, addr >> 32 , 4);

	return (barlen);
}

static void
thunder_pem_read_bar(struct thunder_pcie_softc *sc, int bus, int slot, int func,
    int reg, pci_addr_t *mapp, pci_addr_t *testvalp, int *barlen)
{
	pci_addr_t map, testval, len;

	/* This function has been adapted based on 'pci_read_bar' in pci.c */

	map = thunder_pcie_read_config(sc->dev, bus, slot, func, reg, 4);
	if ((map & PCIM_BAR_MEM_TYPE) == PCIM_BAR_MEM_64) {
		len = 2;
		map |= (pci_addr_t)thunder_pcie_read_config(sc->dev, bus, slot,
		    func, reg + 4, 4) << 32;
	}
	else
		len = 1;

	/*
	 * Determine the BAR's length by writing all 1's.  The bottom
	 * log_2(size) bits of the BAR will stick as 0 when we read
	 * the value back.
	 */
	thunder_pcie_write_config(sc->dev, bus, slot, func, reg, 0xffffffff, 4);
	testval = thunder_pcie_read_config(sc->dev, bus, slot, func, reg, 4);
	if (len == 2) {
		thunder_pcie_write_config(sc->dev, bus, slot, func,
		    reg + 4, 0xffffffff, 4);
		testval |= (pci_addr_t)thunder_pcie_read_config(sc->dev, bus,
		    slot, func, reg + 4, 4) << 32;
	}

	/* Restore the original value of the BAR. */
	thunder_pcie_write_config(sc->dev, bus, slot, func, reg, map, 4);
	if (len == 2)
		thunder_pcie_write_config(sc->dev, bus, slot, func,
		    reg + 4, map >> 32, 4);

	*mapp = map;
	*testvalp = testval;
	*barlen = len;
}

static uint64_t
range_addr_pci_to_phys(struct thunder_pcie_softc *sc, uint64_t pci_addr)
{
	struct pcie_range *r;
	uint64_t offset;
	int tuple;

	/* Find physical address corresponding to given bus address */
	for (tuple = 0; tuple < MAX_RANGES_TUPLES; tuple++) {
		r = &sc->ranges[tuple];
		if (pci_addr >= r->pci_base &&
		    pci_addr < (r->pci_base + r->size)) {
			/* Given pci addr is in this range.
			 * Translate bus addr to phys addr.
			 */
			offset = pci_addr - r->pci_base;
			return (r->phys_base + offset);
		}
	}
	return (0);
}

#define SPACE_CODE_SHIFT	24
#define SPACE_CODE_MASK		0x3
#define SPACE_CODE_IO_SPACE	0x1
#define PROPS_CELL_SIZE		1
#define PCI_ADDR_CELL_SIZE	2

static int
parse_pci_mem_ranges(struct thunder_pcie_softc *sc)
{
	phandle_t node;
	pcell_t pci_addr_cells, parent_addr_cells, size_cells;
	pcell_t attributes;
	pcell_t *ranges_buf, *cell_ptr;
	int cells_count, tuples_count;
	int tuple;
	int rv;

	node = ofw_bus_get_node(sc->dev);

	if (fdt_addrsize_cells(node, &pci_addr_cells, &size_cells))
		return (ENXIO);

	parent_addr_cells = fdt_parent_addr_cells(node);
	if (parent_addr_cells != 2 || pci_addr_cells != 3 || size_cells != 2) {
		device_printf(sc->dev,
		    "Unexpected number of address or size cells in FDT\n");
		return (ENXIO);
	}

	cells_count = OF_getprop_alloc(node, "ranges",
	    sizeof(pcell_t), (void **)&ranges_buf);
	if (cells_count == -1) {
		device_printf(sc->dev, "Error parsing FDT 'ranges' property\n");
		return (ENXIO);
	}

	tuples_count = cells_count /
	    (pci_addr_cells + parent_addr_cells + size_cells);
	if (tuples_count > MAX_RANGES_TUPLES || tuples_count < MIN_RANGES_TUPLES) {
		device_printf(sc->dev,
		    "Unexpected number of 'ranges' tuples in FDT\n");
		rv = ENXIO;
		goto out;
	}

	cell_ptr = ranges_buf;

	for (tuple = 0; tuple < tuples_count; tuple++) {
		attributes = fdt_data_get((void *)cell_ptr, PROPS_CELL_SIZE);
		attributes = (attributes >> SPACE_CODE_SHIFT) & SPACE_CODE_MASK;
		if (attributes == SPACE_CODE_IO_SPACE) {
			/* Ignore I/O space range, mark as empty */
			sc->ranges[tuple].phys_base = 0;
			sc->ranges[tuple].size = 0;
			cell_ptr +=
			    (pci_addr_cells + parent_addr_cells + size_cells);
			continue;
		}
		cell_ptr += PROPS_CELL_SIZE; /* move ptr to pci addr */
		sc->ranges[tuple].pci_base = fdt_data_get((void *)cell_ptr, 2);
		cell_ptr += PCI_ADDR_CELL_SIZE; /* move ptr to cpu addr */
		sc->ranges[tuple].phys_base = fdt_data_get((void *)cell_ptr, 2);
		cell_ptr += parent_addr_cells; /* move ptr to size cells*/
		sc->ranges[tuple].size = fdt_data_get((void *)cell_ptr, 2);
		cell_ptr += size_cells; /* move ptr to next tuple*/

		if (bootverbose) {
			device_printf(sc->dev,
			    "\tPCI addr: 0x%jx, CPU addr: 0x%jx, Size: 0x%jx\n",
			    sc->ranges[tuple].pci_base,
			    sc->ranges[tuple].phys_base,
			    sc->ranges[tuple].size);
		}

	}
	for (; tuple < MAX_RANGES_TUPLES; tuple++) {
		/* zero-fill remaining tuples to mark empty elements in array */
		sc->ranges[tuple].phys_base = 0;
		sc->ranges[tuple].size = 0;
	}

	rv = 0;
out:
	free(ranges_buf, M_OFWPROP);
	return (rv);
}

static uint32_t
thunder_pcie_read_config(device_t dev, u_int bus, u_int slot,
    u_int func, u_int reg, int bytes)
{
	uint64_t offset;
	uint32_t data;
	struct thunder_pcie_softc *sc;
	bus_space_tag_t	t;
	bus_space_handle_t h;

	if (bus > 255 || slot > 31 || func > 7 || reg > 4095)
		return (~0U);

	sc = device_get_softc(dev);

	switch (sc->type) {
	case THUNDER_ECAM:
		offset = PCIE_ADDR_OFFSET(bus, slot, func, reg);
		t = sc->bst;
		h = sc->bsh;
		break;
	case THUNDER_PEM:
		offset = (bus << 24) | (slot << 19) | (func << 16) | reg;
		t = fdtbus_bs_tag;
		h = sc->pem_sli_base;
		break;
	}

	switch (bytes) {
	case 1:
		data = bus_space_read_1(t, h, offset);
		break;
	case 2:
		data = le16toh(bus_space_read_2(t, h, offset));
		break;
	case 4:
		data = le32toh(bus_space_read_4(t, h, offset));
		break;
	default:
		return (~0U);
	}

	return (data);
}

static void
thunder_pcie_write_config(device_t dev, u_int bus, u_int slot,
    u_int func, u_int reg, uint32_t val, int bytes)
{
	uint64_t offset;
	struct thunder_pcie_softc *sc;
	bus_space_tag_t	t;
	bus_space_handle_t h;

	if (bus > 255 || slot > 31 || func > 7 || reg > 4095)
		return;

	sc = device_get_softc(dev);

	switch (sc->type) {
	case THUNDER_ECAM:
		offset = PCIE_ADDR_OFFSET(bus, slot, func, reg);
		t = sc->bst;
		h = sc->bsh;
		break;
	case THUNDER_PEM:
		offset = (bus << 24) | (slot << 19) | (func << 16) | reg;
		t = fdtbus_bs_tag;
		h = sc->pem_sli_base;
		break;
	}

	switch (bytes) {
	case 1:
		bus_space_write_1(t, h, offset, val);
		break;
	case 2:
		bus_space_write_2(t, h, offset, htole16(val));
		break;
	case 4:
		bus_space_write_4(t, h, offset, htole32(val));
		break;
	default:
		return;
	}

}

static int
thunder_pcie_maxslots(device_t dev)
{

	return 31; /* max slots per bus acc. to standard */
}

static int
thunder_pcie_read_ivar(device_t dev, device_t child, int index,
    uintptr_t *result)
{
	struct thunder_pcie_softc *sc;
	int secondary_bus = 0;

	sc = device_get_softc(dev);

	if (index == PCIB_IVAR_BUS) {
		if (sc->type == THUNDER_PEM) {
			secondary_bus = thunder_pem_config_read(sc,
			    PCIERC_CFG006);
			secondary_bus = (secondary_bus >> 8) & 0xFF;
		}
		else {
			/* this pcib adds only pci bus 0 as child */
			secondary_bus = 0;
		}

		*result = secondary_bus;
		return (0);
	}
	if (index == PCIB_IVAR_DOMAIN) {
		if (sc->type == THUNDER_PEM)
			*result = sc->pem;
		else
			*result = sc->ecam;
		return (0);
	}

	device_printf(dev, "ERROR: Unknown index.\n");
	return (ENOENT);
}

static int
thunder_pcie_write_ivar(device_t dev, device_t child, int index,
    uintptr_t value)
{

	return (ENOENT);
}

static int
thunder_pcie_release_resource(device_t dev, device_t child, int type, int rid,
    struct resource *res)
{

	if (type != SYS_RES_MEMORY)
		return (BUS_RELEASE_RESOURCE(device_get_parent(dev), child,
		    type, rid, res));

	return (rman_release_resource(res));
}

static struct resource *
thunder_pcie_alloc_resource(device_t dev, device_t child, int type, int *rid,
    u_long start, u_long end, u_long count, u_int flags)
{
	struct thunder_pcie_softc *sc = device_get_softc(dev);
	struct rman *rm = NULL;
	struct resource *res;

	switch (type) {
	case SYS_RES_IOPORT:
		goto fail;
		break;
	case SYS_RES_MEMORY:
		rm = &sc->mem_rman;
		break;
	default:
		return (BUS_ALLOC_RESOURCE(device_get_parent(dev), dev,
		    type, rid, start, end, count, flags));
	};

	if ((start == 0UL) && (end == ~0UL)) {
		device_printf(dev,
		    "Cannot allocate resource with unspecified range\n");
		goto fail;
	}

	/* XXX ARM64TODO: Find better way to check if addr needs to be translated */
	if (!(start & 0xF00000000000UL)) {
		start = range_addr_pci_to_phys(sc, start);
		end = start + count - 1;
		/* Check if address translation was successful */
		if (start == 0)
			goto fail;
	}

	if (bootverbose) {
		device_printf(dev,
		    "rman_reserve_resource: start=%#lx, end=%#lx, count=%#lx\n",
		    start, end, count);
	}

	res = rman_reserve_resource(rm, start, end, count, flags, child);
	if (res == NULL)
		goto fail;

	rman_set_rid(res, *rid);
	rman_set_bustag(res, fdtbus_bs_tag);
	rman_set_bushandle(res, start);

	if (flags & RF_ACTIVE)
		if (bus_activate_resource(child, type, *rid, res)) {
			rman_release_resource(res);
			goto fail;
		}

	return (res);

fail:
	if (bootverbose) {
		device_printf(dev, "%s FAIL: type=%d, rid=%d, "
		    "start=%016lx, end=%016lx, count=%016lx, flags=%x\n",
		    __func__, type, *rid, start, end, count, flags);
	}

	return (NULL);
}

static int
thunder_pcie_identify_pcib(device_t dev)
{
	struct thunder_pcie_softc *sc;
	u_long start;

	sc = device_get_softc(dev);
	start = bus_get_resource_start(dev, SYS_RES_MEMORY, 0);

	switch(start) {
	case THUNDER_ECAM0_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 0;
		break;
	case THUNDER_ECAM1_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 1;
		break;
	case THUNDER_ECAM2_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 2;
		break;
	case THUNDER_ECAM3_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 3;
		break;
	case THUNDER_ECAM4_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 4;
		break;
	case THUNDER_ECAM5_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 5;
		break;
	case THUNDER_ECAM6_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 6;
		break;
	case THUNDER_ECAM7_CFG_BASE:
		sc->type = THUNDER_ECAM;
		sc->ecam = 7;
		break;
	case THUNDER_PEMn_REG_BASE(0):
		sc->type = THUNDER_PEM;
		sc->pem = 0;
		break;
	case THUNDER_PEMn_REG_BASE(1):
		sc->type = THUNDER_PEM;
		sc->pem = 1;
		break;
	case THUNDER_PEMn_REG_BASE(2):
		sc->type = THUNDER_PEM;
		sc->pem = 2;
		break;
	case THUNDER_PEMn_REG_BASE(3):
		sc->type = THUNDER_PEM;
		sc->pem = 3;
		break;
	case THUNDER_PEMn_REG_BASE(4):
		sc->type = THUNDER_PEM;
		sc->pem = 4;
		break;
	case THUNDER_PEMn_REG_BASE(5):
		sc->type = THUNDER_PEM;
		sc->pem = 5;
		break;
	default:
		device_printf(dev,
		    "error: incorrect resource address=%#lx.\n", start);
		return (ENXIO);
	}
	return (0);
}

static int
thunder_pcie_map_msi(device_t pcib, device_t child, int irq,
    uint64_t *addr, uint32_t *data)
{
	int error;

	error = arm_map_msix(child, irq, addr, data);
	return (error);
}

static int
thunder_pcie_alloc_msix(device_t pcib, device_t child, int *irq)
{
	int error;

	error = arm_alloc_msix(child, irq);
	return (error);
}

static int
thunder_pcie_release_msix(device_t pcib, device_t child, int irq)
{
	int error;

	error = arm_release_msix(child, irq);
	return (error);
}

static int
thunder_pcie_alloc_msi(device_t pcib, device_t child, int count, int maxcount,
    int *irqs)
{
	int error;

	error = arm_alloc_msi(child, count, irqs);
	return (error);
}

static int
thunder_pcie_release_msi(device_t pcib, device_t child, int count, int *irqs)
{
	int error;

	error = arm_release_msi(child, count, irqs);
	return (error);
}

static uint32_t
thunder_its_get_pci_devid(device_t pci_dev)
{
	struct thunder_pcie_softc *sc;
	device_t pcib_dev;
	int bsf;

	pcib_dev = device_get_parent(device_get_parent(pci_dev));
	sc = device_get_softc(pcib_dev);

	bsf = (pci_get_bus(pci_dev) << PCI_RID_BUS_SHIFT) |
	    (pci_get_slot(pci_dev) << PCI_RID_SLOT_SHIFT) |
	    (pci_get_function(pci_dev) << PCI_RID_FUNC_SHIFT);

	if (sc->type == THUNDER_ECAM) {
		return (((pci_get_domain(pci_dev) >> 2) << 19) |
		    ((pci_get_domain(pci_dev) % 4) << 16) | bsf );
	}
	else {
		if(sc->pem < 3 )
			return ((1 << 16) | bsf );

		if(sc->pem < 6 )
			return ((3 << 16) | bsf );

		if(sc->pem < 9 )
			return ((1 << 19) | (1 << 16) | bsf );

		if(sc->pem < 12 )
			return ((1 << 19) | (3 << 16) | bsf);
	}

	return (0);
}

static device_method_t thunder_pcie_methods[] = {
	DEVMETHOD(device_probe,			thunder_pcie_probe),
	DEVMETHOD(device_attach,		thunder_pcie_attach),
	DEVMETHOD(pcib_maxslots,		thunder_pcie_maxslots),
	DEVMETHOD(pcib_read_config,		thunder_pcie_read_config),
	DEVMETHOD(pcib_write_config,		thunder_pcie_write_config),
	DEVMETHOD(bus_read_ivar,		thunder_pcie_read_ivar),
	DEVMETHOD(bus_write_ivar,		thunder_pcie_write_ivar),
	DEVMETHOD(bus_alloc_resource,		thunder_pcie_alloc_resource),
	DEVMETHOD(bus_release_resource,		thunder_pcie_release_resource),
	DEVMETHOD(bus_activate_resource,	bus_generic_activate_resource),
	DEVMETHOD(bus_deactivate_resource,	bus_generic_deactivate_resource),
	DEVMETHOD(bus_setup_intr,		bus_generic_setup_intr),
	DEVMETHOD(bus_teardown_intr,		bus_generic_teardown_intr),
	DEVMETHOD(pcib_map_msi,			thunder_pcie_map_msi),
	DEVMETHOD(pcib_alloc_msix,		thunder_pcie_alloc_msix),
	DEVMETHOD(pcib_release_msix,		thunder_pcie_release_msix),
	DEVMETHOD(pcib_alloc_msi,		thunder_pcie_alloc_msi),
	DEVMETHOD(pcib_release_msi,		thunder_pcie_release_msi),

	DEVMETHOD_END
};

static driver_t thunder_pcie_driver = {
	"pcib",
	thunder_pcie_methods,
	sizeof(struct thunder_pcie_softc),
};

static devclass_t thunder_pcie_devclass;

DRIVER_MODULE(pcib, simplebus, thunder_pcie_driver,
thunder_pcie_devclass, 0, 0);
DRIVER_MODULE(pcib, ofwbus, thunder_pcie_driver,
thunder_pcie_devclass, 0, 0);
