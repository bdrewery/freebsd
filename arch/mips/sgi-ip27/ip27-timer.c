/*
 * Copytight (C) 1999, 2000, 2002 Ralf Baechle (ralf@gnu.org)
 * Copytight (C) 1999, 2000 Silicon Graphics, Inc.
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/param.h>
#include <linux/timex.h>
#include <linux/mm.h>

#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/sgialib.h>
#include <asm/sn/ioc3.h>
#include <asm/m48t35.h>
#include <asm/sn/klconfig.h>
#include <asm/sn/arch.h>
#include <asm/sn/addrs.h>
#include <asm/sn/sn_private.h>
#include <asm/sn/sn0/ip27.h>
#include <asm/sn/sn0/hub.h>

/*
 * This is a hack; we really need to figure these values out dynamically
 *
 * Since 800 ns works very well with various HUB frequencies, such as
 * 360, 380, 390 and 400 MHZ, we use 800 ns rtc cycle time.
 *
 * Ralf: which clock rate is used to feed the counter?
 */
#define NSEC_PER_CYCLE		800
#define NSEC_PER_SEC		1000000000
#define CYCLES_PER_SEC		(NSEC_PER_SEC/NSEC_PER_CYCLE)
#define CYCLES_PER_JIFFY	(CYCLES_PER_SEC/HZ)

static unsigned long ct_cur[NR_CPUS];	/* What counter should be at next timer irq */
static long last_rtc_update;		/* Last time the rtc clock got updated */

extern rwlock_t xtime_lock;
extern volatile unsigned long wall_jiffies;


static int set_rtc_mmss(unsigned long nowtime)
{
	int retval = 0;
	int real_seconds, real_minutes, cmos_minutes;
	struct m48t35_rtc *rtc;
	nasid_t nid;

	nid = get_nasid();
	rtc = (struct m48t35_rtc *)(KL_CONFIG_CH_CONS_INFO(nid)->memory_base +
							IOC3_BYTEBUS_DEV0);

	spin_lock(&rtc_lock);
	rtc->control |= M48T35_RTC_READ;
	cmos_minutes = rtc->min;
	BCD_TO_BIN(cmos_minutes);
	rtc->control &= ~M48T35_RTC_READ;

	/*
	 * Since we're only adjusting minutes and seconds, don't interfere with
	 * hour overflow. This avoids messing with unknown time zones but
	 * requires your RTC not to be off by more than 15 minutes
	 */
	real_seconds = nowtime % 60;
	real_minutes = nowtime / 60;
	if (((abs(real_minutes - cmos_minutes) + 15)/30) & 1)
		real_minutes += 30;	/* correct for half hour time zone */
	real_minutes %= 60;

	if (abs(real_minutes - cmos_minutes) < 30) {
		BIN_TO_BCD(real_seconds);
		BIN_TO_BCD(real_minutes);
		rtc->control |= M48T35_RTC_SET;
		rtc->sec = real_seconds;
		rtc->min = real_minutes;
		rtc->control &= ~M48T35_RTC_SET;
	} else {
		printk(KERN_WARNING
		       "set_rtc_mmss: can't update from %d to %d\n",
		       cmos_minutes, real_minutes);
		retval = -1;
	}
	spin_unlock(&rtc_lock);

	return retval;
}

#define IP27_TIMER_IRQ	9			/* XXX Assign number */

void rt_timer_interrupt(struct pt_regs *regs)
{
	int cpu = smp_processor_id();
	int cpuA = ((cputoslice(cpu)) == 0);
	int irq = IP27_TIMER_IRQ;

	irq_enter(cpu, irq);
	write_lock(&xtime_lock);

again:
	LOCAL_HUB_S(cpuA ? PI_RT_PEND_A : PI_RT_PEND_B, 0);	/* Ack  */
	ct_cur[cpu] += CYCLES_PER_JIFFY;
	LOCAL_HUB_S(cpuA ? PI_RT_COMPARE_A : PI_RT_COMPARE_B, ct_cur[cpu]);

	if (LOCAL_HUB_L(PI_RT_COUNT) >= ct_cur[cpu])
		goto again;

	kstat.irqs[cpu][irq]++;		/* kstat only for bootcpu? */

	if (cpu == 0)
		do_timer(regs);

#ifdef CONFIG_SMP
	update_process_times(user_mode(regs));
#endif /* CONFIG_SMP */

	/*
	 * If we have an externally synchronized Linux clock, then update
	 * RTC clock accordingly every ~11 minutes. Set_rtc_mmss() has to be
	 * called as close as possible to when a second starts.
	 */
	if ((time_status & STA_UNSYNC) == 0 &&
	    xtime.tv_sec > last_rtc_update + 660) {
		if (xtime.tv_usec >= 1000000 - ((unsigned) tick) / 2) {
			if (set_rtc_mmss(xtime.tv_sec + 1) == 0)
				last_rtc_update = xtime.tv_sec;
			else
				last_rtc_update = xtime.tv_sec - 600;
		} else if (xtime.tv_usec <= ((unsigned) tick) / 2) {
			if (set_rtc_mmss(xtime.tv_sec) == 0)
				last_rtc_update = xtime.tv_sec;
			else
				last_rtc_update = xtime.tv_sec - 600;
		}
        }

	write_unlock(&xtime_lock);
	irq_exit(cpu, irq);

	if (softirq_pending(cpu))
		do_softirq();
}

unsigned long ip27_do_gettimeoffset(void)
{
	unsigned long ct_cur1;
	ct_cur1 = REMOTE_HUB_L(cputonasid(0), PI_RT_COUNT) + CYCLES_PER_JIFFY;
	return (ct_cur1 - ct_cur[0]) * NSEC_PER_CYCLE / 1000;
}

/* Includes for ioc3_init().  */
#include <asm/sn/types.h>
#include <asm/sn/sn0/addrs.h>
#include <asm/sn/sn0/hubni.h>
#include <asm/sn/sn0/hubio.h>
#include <asm/pci/bridge.h>

static __init unsigned long get_m48t35_time(void)
{
	unsigned int year, month, date, hour, min, sec;
	struct m48t35_rtc *rtc;
	nasid_t nid;

	nid = get_nasid();
	rtc = (struct m48t35_rtc *)(KL_CONFIG_CH_CONS_INFO(nid)->memory_base +
							IOC3_BYTEBUS_DEV0);

	spin_lock(&rtc_lock);
	rtc->control |= M48T35_RTC_READ;
	sec = rtc->sec;
	min = rtc->min;
	hour = rtc->hour;
	date = rtc->date;
	month = rtc->month;
	year = rtc->year;
	rtc->control &= ~M48T35_RTC_READ;
	spin_unlock(&rtc_lock);

	BCD_TO_BIN(sec);
	BCD_TO_BIN(min);
	BCD_TO_BIN(hour);
	BCD_TO_BIN(date);
	BCD_TO_BIN(month);
	BCD_TO_BIN(year);

	year += 1970;

	return mktime(year, month, date, hour, min, sec);
}

static void ip27_timer_setup(struct irqaction *irq)
{
	/* over-write the handler, we use our own way */
	irq->handler = no_action;

	/* setup irqaction */
//	setup_irq(IP27_TIMER_IRQ, irq);		/* XXX Can't do this yet.  */
}

void __init ip27_time_init(void)
{
	xtime.tv_sec = get_m48t35_time();
	xtime.tv_usec = 0;

	do_gettimeoffset = ip27_do_gettimeoffset;

	// board_time_init = ip27_time_init;
	board_timer_setup = ip27_timer_setup;
}

void __init cpu_time_init(void)
{
	lboard_t *board;
	klcpu_t *cpu;
	int cpuid;

	/* Don't use ARCS.  ARCS is fragile.  Klconfig is simple and sane.  */
	board = find_lboard(KL_CONFIG_INFO(get_nasid()), KLTYPE_IP27);
	if (!board)
		panic("Can't find board info for myself.");

	cpuid = LOCAL_HUB_L(PI_CPU_NUM) ? IP27_CPU0_INDEX : IP27_CPU1_INDEX;
	cpu = (klcpu_t *) KLCF_COMP(board, cpuid);
	if (!cpu)
		panic("No information about myself?");

	printk("CPU %d clock is %dMHz.\n", smp_processor_id(), cpu->cpu_speed);

	set_c0_status(SRB_TIMOCLK);
}

void __init hub_rtc_init(cnodeid_t cnode)
{
	/*
	 * We only need to initialize the current node.
	 * If this is not the current node then it is a cpuless
	 * node and timeouts will not happen there.
	 */
	if (get_compact_nodeid() == cnode) {
		int cpu = smp_processor_id();
		LOCAL_HUB_S(PI_RT_EN_A, 1);
		LOCAL_HUB_S(PI_RT_EN_B, 1);
		LOCAL_HUB_S(PI_PROF_EN_A, 0);
		LOCAL_HUB_S(PI_PROF_EN_B, 0);
		ct_cur[cpu] = CYCLES_PER_JIFFY;
		LOCAL_HUB_S(PI_RT_COMPARE_A, ct_cur[cpu]);
		LOCAL_HUB_S(PI_RT_COUNT, 0);
		LOCAL_HUB_S(PI_RT_PEND_A, 0);
		LOCAL_HUB_S(PI_RT_COMPARE_B, ct_cur[cpu]);
		LOCAL_HUB_S(PI_RT_COUNT, 0);
		LOCAL_HUB_S(PI_RT_PEND_B, 0);
	}
}
