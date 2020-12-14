/*-
 * Copyright (c) 2015 Ruslan Bukin <br@bsdpad.com>
 * All rights reserved.
 *
 * This software was developed by SRI International and the University of
 * Cambridge Computer Laboratory under DARPA/AFRL contract (FA8750-10-C-0237)
 * ("CTSRD"), as part of the DARPA CRASH research programme.
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
 * Performance Monitoring Unit
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include "opt_hwpmc_hooks.h"
#include "opt_platform.h"

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/module.h>
#include <sys/malloc.h>
#include <sys/rman.h>
#include <sys/timeet.h>
#include <sys/timetc.h>
#include <sys/pmc.h>
#include <sys/pmckern.h>

#ifdef FDT
#include <dev/ofw/openfirm.h>
#include <dev/ofw/ofw_bus.h>
#include <dev/ofw/ofw_bus_subr.h>
#endif

#include <machine/bus.h>
#include <machine/cpu.h>
#include <machine/intr.h>

#define	MAX_RLEN	8

struct pmu_intr {
	struct resource	*res;
	void		*ih;
	int		cpuid;
};

struct pmu_softc {
	device_t		dev;
	struct pmu_intr		irq[MAX_RLEN];
};

/* CCNT */
#if __ARM_ARCH > 6
int pmu_attched = 0;
uint32_t ccnt_hi[MAXCPU];
#endif

#define	PMU_OVSR_C		0x80000000	/* Cycle Counter */
#define	PMU_IESR_C		0x80000000	/* Cycle Counter */

static int
pmu_intr(void *arg)
{
#ifdef HWPMC_HOOKS
	struct trapframe *tf;
#endif
	uint32_t r;
#if defined(__arm__) && (__ARM_ARCH > 6)
	u_int cpu;

	cpu = PCPU_GET(cpuid);

	r = cp15_pmovsr_get();
	if (r & PMU_OVSR_C) {
		atomic_add_32(&ccnt_hi[cpu], 1);
		/* Clear the event. */
		r &= ~PMU_OVSR_C;
		cp15_pmovsr_set(PMU_OVSR_C);
	}
#else
	r = 1;
#endif

#ifdef HWPMC_HOOKS
	/* Only call into the HWPMC framework if we know there is work. */
	if (r != 0 && pmc_intr) {
		tf = arg;
		(*pmc_intr)(tf);
	}
#endif

	return (FILTER_HANDLED);
}

static int
pmu_parse_affinity(struct pmu_softc *sc, struct pmu_intr *irq, phandle_t xref,
    uint32_t mpidr)
{
	struct pcpu *pcpu;
	int i, err;


	if (xref  != 0) {
		err = OF_getencprop(OF_node_from_xref(xref), "reg", &mpidr,
		    sizeof(mpidr));
		if (err < 0) {
			device_printf(sc->dev, "missing 'reg' property\n");
				return (ENXIO);
		}
	}

	for (i = 0; i < MAXCPU; i++) {
		pcpu = pcpu_find(i);
		if (pcpu != NULL && pcpu->pc_mpidr == mpidr) {
			irq->cpuid = i;
			return (0);
		}
	}

	device_printf(sc->dev, "Cannot find CPU with MPIDR: 0x%08X\n", mpidr);
	return (ENXIO);
}

static int
pmu_parse_intr(struct pmu_softc *sc)
{
	bool has_affinity;
	phandle_t node, *cpus;
	int rid, err, ncpus, i;


	node = ofw_bus_get_node(sc->dev);
	has_affinity = OF_hasprop(node, "interrupt-affinity");

	for (i = 0; i < MAX_RLEN; i++)
		sc->irq[i].cpuid = -1;

	cpus = NULL;
	if (has_affinity) {
		ncpus = OF_getencprop_alloc_multi(node, "interrupt-affinity",
		    sizeof(*cpus), (void **)&cpus);
		if (ncpus < 0) {
			device_printf(sc->dev,
			   "Cannot read interrupt affinity property\n");
			return (ENXIO);
		}
	}

	/* Process first interrupt */
	rid = 0;
	sc->irq[0].res = bus_alloc_resource_any(sc->dev, SYS_RES_IRQ, &rid,
	    RF_ACTIVE | RF_SHAREABLE);

	if (sc->irq[0].res == NULL) {
		device_printf(sc->dev, "Cannot get interrupt\n");
		err = ENXIO;
		goto done;
	}

	/* Check if PMU have one per-CPU interrupt */
	if (intr_is_per_cpu(sc->irq[0].res)) {
		if (has_affinity) {
			device_printf(sc->dev,
			    "Per CPU interupt have declared affinity\n");
			err = ENXIO;
			goto done;
		}
		return (0);
	}

	/*
	 * PMU with set of generic interrupts (one per core)
	 * Each one must be binded to exact core.
	 */
	err = pmu_parse_affinity(sc, sc->irq + 0, has_affinity ? cpus[0]: 0,
	    0);
	if (err != 0) {
		device_printf(sc->dev, "Cannot parse affinity for CPUid: 0\n");
		goto done;
	}

	for (i = 1; i < MAX_RLEN; i++) {
		rid = i;
		sc->irq[i].res = bus_alloc_resource_any(sc->dev, SYS_RES_IRQ,
		    &rid, RF_ACTIVE | RF_SHAREABLE);
		if (sc->irq[i].res == NULL)
			break;

		if (intr_is_per_cpu(sc->irq[i].res))
		{
			device_printf(sc->dev, "Unexpected per CPU interupt\n");
			err = ENXIO;
			goto done;
		}

		if (has_affinity && i >= ncpus) {
			device_printf(sc->dev, "Missing value in interrupt "
			    "affinity property\n");
			err = ENXIO;
			goto done;
		}

		err = pmu_parse_affinity(sc, sc->irq + i,
		    has_affinity ? cpus[i]: 0, i);
		if (err != 0) {
			device_printf(sc->dev,
			   "Cannot parse affinity for CPUid: %d.\n", i);
			goto done;
		}
	}
	err = 0;
done:
	OF_prop_free(cpus);
	return (err);
}

static int
pmu_attach(device_t dev)
{
	struct pmu_softc *sc;
#if defined(__arm__) && (__ARM_ARCH > 6)
	uint32_t iesr;
#endif
	int err, i;

	sc = device_get_softc(dev);
	sc->dev = dev;

	err = pmu_parse_intr(sc);
	if (err != 0)
		return (err);

	for (i = 0; i < MAX_RLEN; i++) {
		if (sc->irq[i].res == NULL)
			break;
		err = bus_setup_intr(dev, sc->irq[i].res,
		    INTR_MPSAFE | INTR_TYPE_MISC, pmu_intr, NULL, NULL,
		    &sc->irq[i].ih);
		if (err != 0) {
			device_printf(dev,
			    "Unable to setup interrupt handler.\n");
			goto fail;
		}
		if (sc->irq[i].cpuid != -1) {
			err = bus_bind_intr(dev, sc->irq[i].res,
			    sc->irq[i].cpuid);
			if (err != 0) {
				device_printf(sc->dev,
				    "Unable to bind interrupt.\n");
				goto fail;
			}
		}
	}

#if defined(__arm__) && (__ARM_ARCH > 6)
	/* Initialize to 0. */
	for (i = 0; i < MAXCPU; i++)
		ccnt_hi[i] = 0;

	/* Enable the interrupt to fire on overflow. */
	iesr = cp15_pminten_get();
	iesr |= PMU_IESR_C;
	cp15_pminten_set(iesr);

	/* Need this for getcyclecount() fast path. */
	pmu_attched |= 1;
#endif

	return (0);

fail:
	for (i = 1; i < MAX_RLEN; i++) {
		if (sc->irq[i].ih != NULL)
			bus_teardown_intr(dev, sc->irq[i].res, sc->irq[i].ih);
		if (sc->irq[i].res != NULL)
			bus_release_resource(dev, SYS_RES_IRQ, i,
			    sc->irq[i].res);
	}
	return(err);
}

#ifdef FDT
static struct ofw_compat_data compat_data[] = {
	{"arm,armv8-pmuv3",	1},
	{"arm,cortex-a77-pmu",  1},
	{"arm,cortex-a76-pmu",  1},
	{"arm,cortex-a75-pmu",  1},
	{"arm,cortex-a73-pmu",  1},
	{"arm,cortex-a72-pmu",  1},
	{"arm,cortex-a65-pmu",  1},
	{"arm,cortex-a57-pmu",  1},
	{"arm,cortex-a55-pmu",  1},
	{"arm,cortex-a53-pmu",  1},
	{"arm,cortex-a34-pmu",  1},

	{"arm,cortex-a17-pmu",	1},
	{"arm,cortex-a15-pmu",	1},
	{"arm,cortex-a12-pmu",	1},
	{"arm,cortex-a9-pmu",	1},
	{"arm,cortex-a8-pmu",	1},
	{"arm,cortex-a7-pmu",	1},
	{"arm,cortex-a5-pmu",	1},
	{"arm,arm11mpcore-pmu",	1},
	{"arm,arm1176-pmu",	1},
	{"arm,arm1136-pmu",	1},
	{"qcom,krait-pmu",	1},
	{NULL,			0}
};

static int
pmu_fdt_probe(device_t dev)
{

	if (!ofw_bus_status_okay(dev))
		return (ENXIO);

	if (ofw_bus_search_compatible(dev, compat_data)->ocd_data != 0) {
		device_set_desc(dev, "Performance Monitoring Unit");
		return (BUS_PROBE_DEFAULT);
	}

	return (ENXIO);
}

static device_method_t pmu_fdt_methods[] = {
	DEVMETHOD(device_probe,		pmu_fdt_probe),
	DEVMETHOD(device_attach,	pmu_attach),
	{ 0, 0 }
};

static driver_t pmu_fdt_driver = {
	"pmu",
	pmu_fdt_methods,
	sizeof(struct pmu_softc),
};

static devclass_t pmu_fdt_devclass;

DRIVER_MODULE(pmu, simplebus, pmu_fdt_driver, pmu_fdt_devclass, 0, 0);
#endif
