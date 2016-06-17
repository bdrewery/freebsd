/*
 *  linux/arch/arm/mach-integrator/leds.c
 *
 *  Integrator LED control routines
 *
 *  Copyright (C) 1999 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/kernel.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/leds.h>
#include <asm/system.h>
#include <asm/mach-types.h>

static int saved_leds;

static void integrator_leds_event(led_event_t ledevt)
{
	unsigned long flags;
	const unsigned int dbg_base = IO_ADDRESS(INTEGRATOR_DBG_BASE);
	const unsigned int hdr_ctrl = IO_ADDRESS(INTEGRATOR_HDR_BASE) +
					INTEGRATOR_HDR_CTRL_OFFSET;
	unsigned int ctrl;
	unsigned int update_alpha_leds;
	
	// yup, change the LEDs
	local_irq_save(flags);
	update_alpha_leds = 0;

	switch(ledevt) {
	case led_idle_start:
		ctrl = __raw_readl(hdr_ctrl);
		ctrl &= ~INTEGRATOR_HDR_CTRL_LED;
		__raw_writel(ctrl, hdr_ctrl);
		break;

	case led_idle_end:
		ctrl = __raw_readl(hdr_ctrl);
		ctrl |= INTEGRATOR_HDR_CTRL_LED;
		__raw_writel(ctrl, hdr_ctrl);
		break;

	case led_timer:
		saved_leds ^= GREEN_LED;
		update_alpha_leds = 1;
		break;

	case led_red_on:
		saved_leds |= RED_LED;
		update_alpha_leds = 1;
		break;

	case led_red_off:
		saved_leds &= ~RED_LED;
		update_alpha_leds = 1;
		break;

	default:
		break;
	}

	if (update_alpha_leds) {
		while (__raw_readl(dbg_base + INTEGRATOR_DBG_ALPHA_OFFSET) & 1);
		__raw_writel(saved_leds, dbg_base + INTEGRATOR_DBG_LEDS_OFFSET);
	}
	local_irq_restore(flags);
}

static int __init leds_init(void)
{
	if (machine_is_integrator())
		leds_event = integrator_leds_event;

	return 0;
}

__initcall(leds_init);
