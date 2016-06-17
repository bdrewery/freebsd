/*
 *  linux/arch/m68k/hp300/hil.c
 *
 *  Copyright (C) 1998 Philip Blundell <philb@gnu.org>
 *
 *  HP300 Human Interface Loop driver.  This handles the keyboard and mouse.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/keyboard.h>
#include <linux/kbd_ll.h>
#include <asm/io.h>
#include <asm/hwtest.h>
#include <asm/ptrace.h>
#include <asm/irq.h>
#include <asm/system.h>

#define HILBASE			0xf0428000
#define HIL_DATA			0x1
#define HIL_CMD			0x3

#define	HIL_BUSY		0x02
#define	HIL_DATA_RDY		0x01

#define hil_busy()		(in_8(HILBASE + HIL_CMD) & HIL_BUSY)
#define hil_data_available()	(in_8(HILBASE + HIL_CMD) & HIL_DATA_RDY)
#define hil_status()		(in_8(HILBASE + HIL_CMD))
#define hil_command(x)		out_8(HILBASE + HIL_CMD, (x))
#define hil_read_data()		(in_8(HILBASE + HIL_DATA))
#define hil_write_data(x)	out_8(HILBASE + HIL_DATA, (x))

#define	HIL_SETARD		0xA0		/* set auto-repeat delay */
#define	HIL_SETARR		0xA2		/* set auto-repeat rate */
#define	HIL_SETTONE		0xA3		/* set tone generator */
#define	HIL_CNMT		0xB2		/* clear nmi */
#define	HIL_INTON		0x5C		/* Turn on interrupts. */
#define	HIL_INTOFF		0x5D		/* Turn off interrupts. */
#define	HIL_TRIGGER		0xC5		/* trigger command */
#define	HIL_STARTCMD		0xE0		/* start loop command */
#define	HIL_TIMEOUT		0xFE		/* timeout */
#define	HIL_READTIME		0x13		/* Read real time register */

#define	HIL_READBUSY		0x02		/* internal "busy" register */
#define	HIL_READKBDLANG		0x12		/* read keyboard language code */
#define	HIL_READKBDSADR	 	0xF9
#define	HIL_WRITEKBDSADR 	0xE9
#define	HIL_READLPSTAT  	0xFA
#define	HIL_WRITELPSTAT 	0xEA
#define	HIL_READLPCTRL  	0xFB
#define	HIL_WRITELPCTRL 	0xEB

#define HIL_IRQ			1

#define plain_map		hp_plain_map
#define shift_map		hp_shift_map
#define altgr_map		hp_altgr_map
#define ctrl_map		hp_ctrl_map
#define shift_ctrl_map		hp_shift_ctrl_map
#define alt_map			hp_alt_map
#define ctrl_alt_map		hp_ctrl_alt_map

u_short plain_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb62,	0xfb76,	0xfb63,	0xfb78,	0xfb7a,	0xf200,	0xf200,	0xf01b,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb68,	0xfb67,	0xfb66,	0xfb64,	0xfb73,	0xfb61,	0xf200,	0xf207,
	0xfb75,	0xfb79,	0xfb74,	0xfb72,	0xfb65,	0xfb77,	0xfb71,	0xf009,
	0xf037,	0xf036,	0xf035,	0xf034,	0xf033,	0xf032,	0xf031,	0xf060,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf038,	0xf039,	0xf030,	0xf02d,	0xf03d,	0xf008,	0xf200,	0xf200,
	0xfb69,	0xfb6f,	0xfb70,	0xf05b,	0xf05d,	0xf05c,	0xf200,	0xf200,
	0xfb6a,	0xfb6b,	0xfb6c,	0xf03b,	0xf027,	0xf201,	0xf200,	0xf200,
	0xfb6d,	0xf02c,	0xf02e,	0xf02f,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb6e,	0xf020,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short shift_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb42,	0xfb56,	0xfb43,	0xfb58,	0xfb5a,	0xf200,	0xf200,	0xf07f,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb48,	0xfb47,	0xfb46,	0xfb44,	0xfb53,	0xfb41,	0xf200,	0xf207,
	0xfb55,	0xfb59,	0xfb54,	0xfb52,	0xfb45,	0xfb57,	0xfb51,	0xf009,
	0xf026,	0xf05e,	0xf025,	0xf024,	0xf023,	0xf040,	0xf021,	0xf07e,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf02a,	0xf028,	0xf029,	0xf05f,	0xf02b,	0xf200,	0xf200,	0xf200,
	0xfb49,	0xfb4f,	0xfb50,	0xf07b,	0xf07d,	0xf07c,	0xf200,	0xf200,
	0xfb4a,	0xfb4b,	0xfb4c,	0xf03a,	0xf022,	0xf201,	0xf200,	0xf200,
	0xfb4d,	0xf03c,	0xf03e,	0xf03f,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb4e,	0xf020,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short altgr_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb62,	0xfb76,	0xfb63,	0xfb78,	0xfb7a,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb68,	0xfb67,	0xfb66,	0xfb64,	0xfb73,	0xfb61,	0xf200,	0xf207,
	0xfb75,	0xfb79,	0xfb74,	0xfb72,	0xfb65,	0xfb77,	0xfb71,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf040,	0xf021,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf02a,	0xf05b,	0xf05d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb69,	0xfb6f,	0xfb70,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb6a,	0xfb6b,	0xfb6c,	0xf200,	0xf200,	0xf201,	0xf200,	0xf200,
	0xfb6d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xfb6e,	0xf200,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short ctrl_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf002,	0xf016,	0xf003,	0xf018,	0xf01a,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf008,	0xf007,	0xf006,	0xf004,	0xf013,	0xf001,	0xf200,	0xf207,
	0xf015,	0xf019,	0xf014,	0xf012,	0xf005,	0xf017,	0xf011,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf000,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf503,	0xf502,	0xf501,	0xf500,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf504,	0xf505,	0xf506,	0xf507,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf009,	0xf00f,	0xf010,	0xf200,	0xf200,	0xf01c,	0xf200,	0xf200,
	0xf00a,	0xf00b,	0xf00c,	0xf200,	0xf007,	0xf201,	0xf200,	0xf200,
	0xf00d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf00e,	0xf200,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short shift_ctrl_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf002,	0xf016,	0xf003,	0xf018,	0xf01a,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf008,	0xf007,	0xf006,	0xf004,	0xf013,	0xf001,	0xf200,	0xf207,
	0xf015,	0xf019,	0xf014,	0xf012,	0xf005,	0xf017,	0xf011,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf009,	0xf00f,	0xf010,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf00a,	0xf00b,	0xf00c,	0xf200,	0xf200,	0xf201,	0xf200,	0xf200,
	0xf00d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf00e,	0xf200,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short alt_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf862,	0xf876,	0xf863,	0xf878,	0xf87a,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf868,	0xf867,	0xf866,	0xf864,	0xf873,	0xf861,	0xf200,	0xf207,
	0xf875,	0xf879,	0xf874,	0xf872,	0xf865,	0xf877,	0xf871,	0xf809,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf860,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf869,	0xf86f,	0xf870,	0xf200,	0xf200,	0xf85c,	0xf200,	0xf200,
	0xf86a,	0xf86b,	0xf86c,	0xf83b,	0xf827,	0xf201,	0xf200,	0xf200,
	0xf86d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf86e,	0xf200,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

u_short ctrl_alt_map[NR_KEYS] = {
	0xf200,	0xf200,	0xf703,	0xf703,	0xf700,	0xf700,	0xf702,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf802,	0xf816,	0xf803,	0xf818,	0xf81a,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf808,	0xf807,	0xf806,	0xf804,	0xf813,	0xf801,	0xf200,	0xf207,
	0xf815,	0xf819,	0xf814,	0xf812,	0xf805,	0xf817,	0xf811,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf103,	0xf102,	0xf101,	0xf100,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf104,	0xf105,	0xf106,	0xf107,	0xf200,	0xf200,	0xf200,
	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf809,	0xf80f,	0xf810,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf80a,	0xf80b,	0xf80c,	0xf200,	0xf200,	0xf201,	0xf200,	0xf200,
	0xf80d,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,	0xf200,
	0xf80e,	0xf200,	0xf200,	0xf200,	0xf601,	0xf600,	0xf603,	0xf602,
};

#undef plain_map
#undef ctrl_alt_map
#undef shift_map
#undef altgr_map
#undef ctrl_map
#undef shift_ctrl_map
#undef alt_map

struct {
  unsigned char s, c;
  int valid;
} hil_last;

#define hil_getlast(s,c)  do { s = hil_last.s; c = hil_last.c; hil_last.valid = 0; } while (0)

struct {
  unsigned char data[16];
  unsigned int ptr;
} poll;

unsigned char curdev = 0;

static void poll_finished(void)
{
  switch (poll.data[0])
  {
  case 0x40:
    {
      int down = (poll.data[1] & 1) == 0;
      unsigned char scode = poll.data[1] >> 1;
#if 0
      if (down)
	printk("[%02x]", scode);
#endif
      handle_scancode(scode, down);
    }
    break;
  }
  curdev = 0;
}

static inline void handle_status(unsigned char s, unsigned char c)
{
  if (c & 0x8) {
    /* End of block */
    if (c & 0x10)
      poll_finished();
  } else {
    if (c & 0x10) {
      if (curdev)
	poll_finished();		/* just in case */
      curdev = c & 7;
      poll.ptr = 0;
    }
  }
}

static inline void handle_data(unsigned char s, unsigned char c)
{
  if (curdev)
    poll.data[poll.ptr++] = c;
}

/* 
 * Handle HIL interrupts.
 */

static void hil_interrupt(int irq, void *handle, struct pt_regs *regs)
{
  unsigned char s, c;
  s = hil_status(); c = hil_read_data();
  switch (s >> 4)
  {
  case 0x5:
    handle_status(s, c);
    break;
  case 0x6:
    handle_data(s, c);
    break;
  case 0x4:
    hil_last.s = s;
    hil_last.c = c;
    mb();
    hil_last.valid = 1;
    break;
  }
}

/*
 * Send a command to the HIL
 */

static void hil_do(unsigned char cmd, unsigned char *data, unsigned int len)
{
  unsigned long flags;
  save_flags(flags); cli();
  while (hil_busy());
  hil_command(cmd);
  while (len--) {
    while (hil_busy());
    hil_write_data(*(data++));
  }
  restore_flags(flags);
}

/*
 * Initialise HIL. 
 */

int __init hp300_keyb_init(void)
{
  unsigned char s, c, kbid;
  unsigned int n = 0;

  memcpy(key_maps[0], hp_plain_map, sizeof(plain_map));
  memcpy(key_maps[1], hp_shift_map, sizeof(plain_map));
  memcpy(key_maps[4], hp_ctrl_map, sizeof(plain_map));

  if (!hwreg_present((void *)(HILBASE + HIL_DATA)))
    return 1;		/* maybe this can happen */

  request_irq(HIL_IRQ, hil_interrupt, 0, "HIL", NULL);

  /* Turn on interrupts */
  hil_do(HIL_INTON, NULL, 0);

  /* Look for keyboards */
  hil_do(HIL_READKBDSADR, NULL, 0);
  while (!hil_last.valid) {
    if (n++ > 100000) {
      printk("HIL: timed out, assuming no keyboard present.\n");
      return 1;
    }
    mb();
  }
  hil_getlast(s, c);
  if (c == 0) {
    printk("HIL: no keyboard present.\n");
    return 1;
  }
  for (kbid = 0; (kbid < 8) && ((c & (1<<kbid)) == 0); kbid++);
  printk("HIL: keyboard found at id %d\n", kbid);
  /* set it to raw mode */
  c = 0;
  hil_do(HIL_WRITEKBDSADR, &c, 1);
  return 0;
}
