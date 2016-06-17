/* keyspan_usa19qi_fw.h
 
	The firmware contained herein as keyspn_usa19qi_fw.h is

		Copyright (C) 1999-2001
		Keyspan, A division of InnoSys Incorporated ("Keyspan")
		
	as an unpublished work. This notice does not imply unrestricted or
	public access to the source code from which this firmware image is
	derived.  Except as noted below this firmware image may not be 
	reproduced, used, sold or transferred to any third party without 
	Keyspan's prior written consent.  All Rights Reserved.

	Permission is hereby granted for the distribution of this firmware 
	image as part of a Linux or other Open Source operating system kernel 
	in text or binary form as required. 

	This firmware may not be modified and may only be used with  
	Keyspan hardware.  Distribution and/or Modification of the 
	keyspan.c driver which includes this firmware, in whole or in 
	part, requires the inclusion of this statement."

*/

static const struct ezusb_hex_record keyspan_usa19qi_firmware[] = {
 {0x0033,  3, { 0x02, 0x00, 0x1a}},
 {0x001a,  4, { 0x53, 0xd8, 0xef, 0x32}},
 {0x0003, 16, { 0x8e, 0x11, 0x8f, 0x12, 0xe5, 0x12, 0x15, 0x12, 0xae, 0x11, 0x70, 0x02, 0x15, 0x11, 0x4e, 0x60}},
 {0x0013,  7, { 0x05, 0x12, 0x0f, 0x84, 0x80, 0xee, 0x22}},
 {0x0023,  3, { 0x02, 0x00, 0x46}},
 {0x0046, 16, { 0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x86, 0x75, 0x86, 0x00, 0xc0, 0xd0, 0x75, 0xd0, 0x08}},
 {0x0056, 16, { 0x30, 0x99, 0x0e, 0x30, 0x0b, 0x07, 0xa2, 0x0e, 0x92, 0x9b, 0x85, 0x36, 0x99, 0xc2, 0x99, 0xd2}},
 {0x0066, 16, { 0x12, 0x20, 0x12, 0x03, 0x02, 0x04, 0x1e, 0xc2, 0x12, 0x20, 0x03, 0x03, 0x02, 0x02, 0x4e, 0x20}},
 {0x0076, 16, { 0x0b, 0x03, 0x02, 0x01, 0x26, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x50, 0x3c, 0x20, 0x0c, 0x34, 0x20}},
 {0x0086, 16, { 0x09, 0x31, 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x70, 0x29, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05}},
 {0x0096, 16, { 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf}},
 {0x00a6, 16, { 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0xf5, 0x36}},
 {0x00b6, 16, { 0x02, 0x04, 0x1c, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0x90, 0x7f, 0xc7, 0xe4, 0xf0, 0xc2, 0x03, 0x30}},
 {0x00c6, 16, { 0x0d, 0x0c, 0xc2, 0x0d, 0x90, 0x7f, 0xbb, 0x04, 0xf0, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0x90, 0x7f}},
 {0x00d6, 16, { 0xc8, 0xe0, 0x30, 0xe1, 0x05, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0x90, 0x7f, 0xc9, 0xe0, 0xf5, 0x53}},
 {0x00e6, 16, { 0x90, 0x7d, 0xc0, 0xe0, 0x13, 0x92, 0x0d, 0x75, 0x16, 0xff, 0x20, 0x0c, 0x2b, 0x20, 0x09, 0x28}},
 {0x00f6, 16, { 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x70, 0x20, 0x30, 0x10, 0x11, 0x90, 0x7d, 0xc1, 0xe0, 0x13}},
 {0x0043,  3, { 0x02, 0x0e, 0x00}},
 {0x0000,  3, { 0x02, 0x00, 0x26}},
 {0x0026, 12, { 0x78, 0x7f, 0xe4, 0xf6, 0xd8, 0xfd, 0x75, 0x81, 0x54, 0x02, 0x0b, 0x28}},
 {0x0106, 64, { 0x92, 0x0e, 0xa3, 0xe0, 0xf5, 0x36, 0x75, 0x3a, 0x03, 0x02, 0x04, 0x1c, 0x75, 0x3a, 0x02, 0x90, 0x7d,
  0xc1, 0xe0, 0xf5, 0x36, 0x02, 0x04, 0x1c, 0x75, 0x3a, 0x01, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0xe5,
  0x3a, 0xc3, 0x95, 0x53, 0x50, 0x03, 0x02, 0x01, 0xc9, 0x90, 0x7f, 0xc6, 0xe0, 0x30, 0xe1, 0x07,
  0xc2, 0x14, 0xc2, 0x05, 0x02, 0x04, 0x1c, 0x90, 0x7f, 0xc7, 0xe0, 0xf5, 0x53, 0x90, 0x7e}},
 {0x0146, 64, { 0x40, 0xe0, 0x13, 0x92, 0x0d, 0x75, 0x16, 0xff, 0x20, 0x0c, 0x70, 0x20, 0x09, 0x6d, 0x90, 0x7f, 0x9b,
  0xe0, 0x55, 0x38, 0x70, 0x65, 0x30, 0x10, 0x10, 0x90, 0x7e, 0x41, 0xe0, 0x13, 0x92, 0x9b, 0xa3,
  0xe0, 0xf5, 0x99, 0x75, 0x3a, 0x03, 0x80, 0x09, 0x90, 0x7e, 0x41, 0xe0, 0xf5, 0x99, 0x75, 0x3a,
  0x02, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x40, 0x17, 0x90, 0x7f, 0xc7, 0xe4, 0xf0, 0xc2, 0x03}},
 {0x0186, 64, { 0x20, 0x0d, 0x03, 0x02, 0x04, 0x1c, 0xc2, 0x0d, 0x90, 0x7f, 0xbb, 0x04, 0xf0, 0x02, 0x04, 0x1c, 0x30,
  0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83,
  0xe0, 0x13, 0x92, 0x0e, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e,
  0xf5, 0x83, 0xe0, 0xf5, 0x36, 0xd2, 0x0b, 0x02, 0x04, 0x1c, 0x75, 0x3a, 0x01, 0xc2, 0x14}},
 {0x01c6, 64, { 0x02, 0x04, 0x1c, 0x30, 0x0c, 0x03, 0x02, 0x02, 0x49, 0x20, 0x09, 0x77, 0x90, 0x7f, 0x9b, 0xe0, 0x55,
  0x38, 0x70, 0x6f, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4,
  0x34, 0x7e, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x9b, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5,
  0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0xf5, 0x99, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x40}},
 {0x0206, 64, { 0x17, 0x90, 0x7f, 0xc7, 0xe4, 0xf0, 0xc2, 0x03, 0x20, 0x0d, 0x03, 0x02, 0x04, 0x1c, 0xc2, 0x0d, 0x90,
  0x7f, 0xbb, 0x04, 0xf0, 0x02, 0x04, 0x1c, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40,
  0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf, 0x3a, 0x05, 0x3a,
  0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0xf5, 0x36, 0xd2, 0x0b}},
 {0x0246, 64, { 0x02, 0x04, 0x1c, 0xc2, 0x14, 0x02, 0x04, 0x1c, 0x20, 0x0b, 0x03, 0x02, 0x02, 0xff, 0xe5, 0x3a, 0xc3,
  0x95, 0x53, 0x50, 0x3c, 0x20, 0x0c, 0x34, 0x20, 0x09, 0x31, 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38,
  0x70, 0x29, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34,
  0x7d, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5}},
 {0x0286, 64, { 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xf5, 0x36, 0x02, 0x04, 0x1c, 0xc2, 0x0b, 0x02, 0x04, 0x1c,
  0x90, 0x7f, 0xc9, 0xe4, 0xf0, 0xd2, 0x03, 0x30, 0x0d, 0x0c, 0xc2, 0x0d, 0x90, 0x7f, 0xbb, 0x04,
  0xf0, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0x90, 0x7f, 0xc6, 0xe0, 0x30, 0xe1, 0x05, 0xc2, 0x0b, 0x02,
  0x04, 0x1c, 0x90, 0x7f, 0xc7, 0xe0, 0xf5, 0x53, 0x90, 0x7e, 0x40, 0xe0, 0x13, 0x92, 0x0d}},
 {0x02c6, 64, { 0x75, 0x16, 0xff, 0x20, 0x0c, 0x2b, 0x20, 0x09, 0x28, 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x70, 0x20,
  0x30, 0x10, 0x11, 0x90, 0x7e, 0x41, 0xe0, 0x13, 0x92, 0x0e, 0xa3, 0xe0, 0xf5, 0x36, 0x75, 0x3a,
  0x03, 0x02, 0x04, 0x1c, 0x75, 0x3a, 0x02, 0x90, 0x7e, 0x41, 0xe0, 0xf5, 0x36, 0x02, 0x04, 0x1c,
  0x75, 0x3a, 0x01, 0xc2, 0x0b, 0x02, 0x04, 0x1c, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x50, 0x03}},
 {0x0306, 64, { 0x02, 0x03, 0xa2, 0x90, 0x7f, 0xc8, 0xe0, 0x30, 0xe1, 0x07, 0xc2, 0x14, 0xc2, 0x05, 0x02, 0x04, 0x1c,
  0x90, 0x7f, 0xc9, 0xe0, 0xf5, 0x53, 0x90, 0x7d, 0xc0, 0xe0, 0x13, 0x92, 0x0d, 0x75, 0x16, 0xff,
  0x20, 0x0c, 0x70, 0x20, 0x09, 0x6d, 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x70, 0x65, 0x30, 0x10,
  0x10, 0x90, 0x7d, 0xc1, 0xe0, 0x13, 0x92, 0x9b, 0xa3, 0xe0, 0xf5, 0x99, 0x75, 0x3a, 0x03}},
 {0x0346, 64, { 0x80, 0x09, 0x90, 0x7d, 0xc1, 0xe0, 0xf5, 0x99, 0x75, 0x3a, 0x02, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x40,
  0x17, 0x90, 0x7f, 0xc9, 0xe4, 0xf0, 0xd2, 0x03, 0x20, 0x0d, 0x03, 0x02, 0x04, 0x1c, 0xc2, 0x0d,
  0x90, 0x7f, 0xbb, 0x04, 0xf0, 0x02, 0x04, 0x1c, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74,
  0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf, 0x3a}},
 {0x0386, 64, { 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xf5, 0x36, 0xd2, 0x0b,
  0x02, 0x04, 0x1c, 0x75, 0x3a, 0x01, 0xc2, 0x14, 0x02, 0x04, 0x1c, 0x20, 0x0c, 0x75, 0x20, 0x09,
  0x72, 0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x70, 0x6a, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a,
  0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x9b, 0xaf}},
 {0x03c6, 64, { 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xf5, 0x99, 0xe5,
  0x3a, 0xc3, 0x95, 0x53, 0x40, 0x13, 0x90, 0x7f, 0xc9, 0xe4, 0xf0, 0xd2, 0x03, 0x30, 0x0d, 0x35,
  0xc2, 0x0d, 0x90, 0x7f, 0xbb, 0x04, 0xf0, 0x80, 0x2c, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a,
  0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf}},
 {0x0406, 64, { 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xf5, 0x36, 0xd2,
  0x0b, 0x80, 0x02, 0xc2, 0x14, 0xd2, 0x01, 0x20, 0x98, 0x03, 0x02, 0x05, 0x5a, 0xc2, 0x98, 0x20,
  0x02, 0x03, 0x02, 0x04, 0xc7, 0x20, 0x16, 0x27, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x80, 0x2f, 0xf5,
  0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe5, 0x99, 0xf0, 0x30, 0x10, 0x4d, 0xaf, 0x39, 0x05}},
 {0x0446, 64, { 0x39, 0x74, 0x80, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe5, 0x98, 0xf0, 0x80, 0x3a, 0x85,
  0x99, 0x10, 0xe5, 0x10, 0xb5, 0x47, 0x04, 0xd2, 0x09, 0x80, 0x2e, 0xe5, 0x10, 0xb5, 0x46, 0x04,
  0xc2, 0x09, 0x80, 0x25, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x80, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e,
  0xf5, 0x83, 0xe5, 0x10, 0xf0, 0x30, 0x10, 0x11, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x80, 0x2f}},
 {0x0486, 64, { 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe5, 0x98, 0xf0, 0xd2, 0x0f, 0xe5, 0x39, 0xc3, 0x95, 0x43,
  0x50, 0x03, 0x02, 0x05, 0x58, 0x90, 0x7f, 0xb8, 0xe0, 0x30, 0xe1, 0x16, 0xe5, 0x39, 0xc3, 0x94,
  0x40, 0x50, 0x03, 0x02, 0x05, 0x58, 0x15, 0x39, 0x15, 0x39, 0x05, 0x2b, 0x43, 0x34, 0x01, 0x02,
  0x05, 0x58, 0x90, 0x7f, 0xb7, 0xe5, 0x39, 0xf0, 0x75, 0x39, 0x00, 0xc2, 0x02, 0x02, 0x05}},
 {0x04c6, 64, { 0x58, 0x20, 0x16, 0x27, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x00, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5,
  0x83, 0xe5, 0x99, 0xf0, 0x30, 0x10, 0x4d, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x00, 0x2f, 0xf5, 0x82,
  0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe5, 0x98, 0xf0, 0x80, 0x3a, 0x85, 0x99, 0x10, 0xe5, 0x10, 0xb5,
  0x47, 0x04, 0xd2, 0x09, 0x80, 0x2e, 0xe5, 0x10, 0xb5, 0x46, 0x04, 0xc2, 0x09, 0x80, 0x25}},
 {0x0506, 64, { 0xaf, 0x39, 0x05, 0x39, 0x74, 0x00, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe5, 0x10, 0xf0,
  0x30, 0x10, 0x11, 0xaf, 0x39, 0x05, 0x39, 0x74, 0x00, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5,
  0x83, 0xe5, 0x98, 0xf0, 0xd2, 0x0f, 0xe5, 0x39, 0xc3, 0x95, 0x43, 0x40, 0x24, 0x90, 0x7f, 0xb6,
  0xe0, 0x30, 0xe1, 0x12, 0xe5, 0x39, 0xc3, 0x94, 0x40, 0x40, 0x16, 0x15, 0x39, 0x15, 0x39}},
 {0x0546, 64, { 0x05, 0x2b, 0x43, 0x34, 0x01, 0x80, 0x0b, 0x90, 0x7f, 0xb9, 0xe5, 0x39, 0xf0, 0x75, 0x39, 0x00, 0xd2,
  0x02, 0xd2, 0x01, 0x30, 0x01, 0x05, 0xc2, 0x01, 0x02, 0x00, 0x56, 0xd0, 0xd0, 0xd0, 0x86, 0xd0,
  0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0x90, 0x7f, 0xbc, 0xe0, 0x20, 0xe1, 0x51, 0xe5, 0x34, 0x60,
  0x4d, 0xe5, 0x31, 0x70, 0x49, 0xe5, 0x34, 0x30, 0xe1, 0x08, 0xe4, 0xf5, 0x2f, 0x75, 0x34}},
 {0x0586, 64, { 0x01, 0x80, 0x0b, 0xa2, 0x08, 0xe4, 0x33, 0xf5, 0x2f, 0xc2, 0x08, 0xe4, 0xf5, 0x34, 0xe4, 0xf5, 0x11,
  0x7e, 0x00, 0x7b, 0x00, 0x74, 0x24, 0x25, 0x11, 0xf9, 0xee, 0x34, 0x00, 0xfa, 0x12, 0x0d, 0x06,
  0xff, 0x74, 0x00, 0x25, 0x11, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xef, 0xf0, 0x05, 0x11,
  0xe5, 0x11, 0xb4, 0x0c, 0xdb, 0x90, 0x7f, 0xbd, 0x74, 0x0c, 0xf0, 0x75, 0x31, 0x10, 0x90}},
 {0x05c6, 64, { 0x7f, 0xca, 0xe0, 0x30, 0xe1, 0x03, 0x02, 0x06, 0xf3, 0xe4, 0xf5, 0x11, 0x74, 0x40, 0x25, 0x11, 0xf5,
  0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xff, 0xe5, 0x11, 0x7c, 0x00, 0x7b, 0x00, 0x24, 0x3b,
  0xf9, 0xec, 0x34, 0x00, 0xfa, 0xef, 0x12, 0x0d, 0x1f, 0x05, 0x11, 0xe5, 0x11, 0xb4, 0x18, 0xdb,
  0xe5, 0x3b, 0x60, 0x11, 0x75, 0xc9, 0x20, 0x75, 0xc8, 0x36, 0x85, 0x3c, 0xca, 0x85, 0x3d}},
 {0x0606, 64, { 0xcb, 0xe4, 0x90, 0x7f, 0x9f, 0xf0, 0xe5, 0x3e, 0x13, 0x92, 0x10, 0x92, 0x9f, 0x85, 0x3f, 0x38, 0xe5,
  0x40, 0x13, 0x92, 0x16, 0xe5, 0x41, 0x60, 0x09, 0x90, 0x7f, 0x98, 0xe0, 0x54, 0xfb, 0xf0, 0x80,
  0x07, 0x90, 0x7f, 0x98, 0xe0, 0x44, 0x04, 0xf0, 0xe5, 0x42, 0x60, 0x09, 0x90, 0x7f, 0x98, 0xe0,
  0x54, 0x7f, 0xf0, 0x80, 0x07, 0x90, 0x7f, 0x98, 0xe0, 0x44, 0x80, 0xf0, 0xe5, 0x48, 0x60}},
 {0x0646, 64, { 0x0b, 0xc2, 0x0c, 0xc2, 0x09, 0x90, 0x7f, 0x95, 0xe0, 0x44, 0x02, 0xf0, 0xe5, 0x49, 0x60, 0x0c, 0xd2,
  0x09, 0x43, 0x34, 0x01, 0x90, 0x7f, 0x95, 0xe0, 0x44, 0x02, 0xf0, 0xe5, 0x4a, 0x60, 0x0d, 0xc2,
  0xaf, 0xc2, 0x0b, 0xd2, 0x00, 0xe4, 0xf5, 0x53, 0xf5, 0x3a, 0xd2, 0xaf, 0xe5, 0x4b, 0x60, 0x05,
  0x30, 0x16, 0x02, 0xd2, 0x09, 0xe5, 0x4c, 0x60, 0x15, 0x90, 0x7f, 0x95, 0xe0, 0x54, 0xfd}},
 {0x0686, 64, { 0xf0, 0x90, 0x7f, 0x9e, 0xe0, 0x44, 0x02, 0xf0, 0x90, 0x7f, 0x98, 0xe0, 0x54, 0xfd, 0xf0, 0xe5, 0x4d,
  0x60, 0x0a, 0xd2, 0x9c, 0xc2, 0x98, 0x75, 0x2c, 0x01, 0x75, 0x31, 0x1e, 0xe5, 0x4e, 0x60, 0x07,
  0xc2, 0x9c, 0xe4, 0xf5, 0x39, 0xf5, 0x2c, 0xe5, 0x4f, 0x60, 0x03, 0xe4, 0xf5, 0x39, 0xe5, 0x50,
  0x60, 0x02, 0xd2, 0x07, 0xe5, 0x51, 0x60, 0x0a, 0xe5, 0x4d, 0x70, 0x02, 0xf5, 0x31, 0xe5}},
 {0x06c6, 64, { 0x51, 0x42, 0x34, 0xe5, 0x52, 0x60, 0x1f, 0x90, 0x7f, 0xd7, 0x74, 0x11, 0xf0, 0x74, 0x31, 0xf0, 0x74,
  0x12, 0xf0, 0x74, 0x32, 0xf0, 0x74, 0x13, 0xf0, 0x74, 0x33, 0xf0, 0x74, 0x14, 0xf0, 0x74, 0x34,
  0xf0, 0xd2, 0x03, 0xd2, 0x02, 0xd2, 0x08, 0xe4, 0x90, 0x7f, 0xcb, 0xf0, 0xa2, 0x0c, 0xe4, 0x33,
  0xff, 0x65, 0x29, 0x60, 0x05, 0x8f, 0x29, 0x43, 0x34, 0x01, 0xa2, 0x09, 0xe4, 0x33, 0xff}},
 {0x0706, 64, { 0x65, 0x2a, 0x60, 0x05, 0x8f, 0x2a, 0x43, 0x34, 0x01, 0x90, 0x7f, 0x9b, 0xe0, 0xff, 0x54, 0x08, 0x64,
  0x08, 0xfe, 0x65, 0x25, 0x60, 0x05, 0x8e, 0x25, 0x43, 0x34, 0x01, 0xef, 0x54, 0x10, 0x64, 0x10,
  0xfe, 0x65, 0x26, 0x60, 0x05, 0x8e, 0x26, 0x43, 0x34, 0x01, 0xef, 0x54, 0x40, 0x64, 0x40, 0xfe,
  0x65, 0x27, 0x60, 0x05, 0x8e, 0x27, 0x43, 0x34, 0x01, 0xef, 0x54, 0x20, 0x64, 0x20, 0xfe}},
 {0x0746, 64, { 0x65, 0x28, 0x60, 0x05, 0x8e, 0x28, 0x43, 0x34, 0x01, 0x90, 0x7f, 0x9a, 0xe0, 0x54, 0x40, 0x64, 0x40,
  0xfe, 0x65, 0x2e, 0x60, 0x05, 0x8e, 0x2e, 0x43, 0x34, 0x01, 0x30, 0x07, 0x35, 0xc2, 0xaf, 0x30,
  0x02, 0x18, 0x90, 0x7f, 0xb8, 0xe0, 0x20, 0xe1, 0x27, 0xe5, 0x39, 0x60, 0x09, 0x90, 0x7f, 0xb7,
  0xf0, 0xe4, 0xf5, 0x39, 0xc2, 0x02, 0xc2, 0x07, 0x80, 0x16, 0x90, 0x7f, 0xb6, 0xe0, 0x20}},
 {0x0786, 64, { 0xe1, 0x0f, 0xe5, 0x39, 0x60, 0x09, 0x90, 0x7f, 0xb9, 0xf0, 0xe4, 0xf5, 0x39, 0xd2, 0x02, 0xc2, 0x07,
  0xd2, 0xaf, 0x20, 0x05, 0x3d, 0x30, 0x03, 0x1e, 0x90, 0x7f, 0xc6, 0xe0, 0x20, 0xe1, 0x33, 0x90,
  0x7e, 0x40, 0xe0, 0x13, 0x92, 0x0d, 0x75, 0x3a, 0x01, 0x90, 0x7f, 0xc7, 0xe0, 0xf5, 0x53, 0xd2,
  0x05, 0x75, 0x16, 0xff, 0x80, 0x1c, 0x90, 0x7f, 0xc8, 0xe0, 0x20, 0xe1, 0x15, 0x90, 0x7d}},
 {0x07c6, 64, { 0xc0, 0xe0, 0x13, 0x92, 0x0d, 0x75, 0x3a, 0x01, 0x90, 0x7f, 0xc9, 0xe0, 0xf5, 0x53, 0xd2, 0x05, 0x75,
  0x16, 0xff, 0x20, 0x14, 0x33, 0x20, 0x00, 0x06, 0xe5, 0x3a, 0x65, 0x53, 0x70, 0x2a, 0x30, 0x05,
  0x1a, 0x30, 0x03, 0x09, 0xe4, 0x90, 0x7f, 0xc7, 0xf0, 0xc2, 0x03, 0x80, 0x07, 0xe4, 0x90, 0x7f,
  0xc9, 0xf0, 0xd2, 0x03, 0xc2, 0x05, 0xe4, 0xf5, 0x53, 0xf5, 0x3a, 0x30, 0x0d, 0x0a, 0xc2}},
 {0x0806, 64, { 0x0d, 0xc2, 0x00, 0x90, 0x7f, 0xbb, 0x74, 0x01, 0xf0, 0x30, 0x14, 0x03, 0x02, 0x09, 0x14, 0x20, 0x05,
  0x03, 0x02, 0x09, 0x14, 0x30, 0x0c, 0x03, 0x02, 0x09, 0x14, 0x30, 0x09, 0x03, 0x02, 0x09, 0x14,
  0x90, 0x7f, 0x9b, 0xe0, 0x55, 0x38, 0x60, 0x03, 0x02, 0x09, 0x14, 0x30, 0x03, 0x61, 0x30, 0x10,
  0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83}},
 {0x0846, 64, { 0xe0, 0x13, 0x92, 0x1b, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5,
  0x83, 0xe0, 0xfe, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x50, 0x2a, 0x30, 0x10, 0x12, 0xaf, 0x3a, 0x05,
  0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e, 0xaf,
  0x3a, 0x05, 0x3a, 0x74, 0x40, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7e, 0xf5, 0x83, 0xe0, 0xf5}},
 {0x0886, 64, { 0x36, 0xd2, 0x0b, 0x80, 0x6a, 0xc2, 0x0b, 0xe4, 0x90, 0x7f, 0xc7, 0xf0, 0xc2, 0x03, 0x80, 0x5f, 0x30,
  0x10, 0x12, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83,
  0xe0, 0x13, 0x92, 0x1b, 0xaf, 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d,
  0xf5, 0x83, 0xe0, 0xfe, 0xe5, 0x3a, 0xc3, 0x95, 0x53, 0x50, 0x2a, 0x30, 0x10, 0x12, 0xaf}},
 {0x08c6, 64, { 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0x13, 0x92, 0x0e,
  0xaf, 0x3a, 0x05, 0x3a, 0x74, 0xc0, 0x2f, 0xf5, 0x82, 0xe4, 0x34, 0x7d, 0xf5, 0x83, 0xe0, 0xf5,
  0x36, 0xd2, 0x0b, 0x80, 0x09, 0xc2, 0x0b, 0xe4, 0x90, 0x7f, 0xc9, 0xf0, 0xd2, 0x03, 0x30, 0x10,
  0x04, 0xa2, 0x1b, 0x92, 0x9b, 0xd2, 0x14, 0xc2, 0xaf, 0x8e, 0x99, 0x20, 0x0b, 0x0d, 0x30}},
 {0x0906, 64, { 0x0d, 0x0a, 0xc2, 0x0d, 0xc2, 0x00, 0x90, 0x7f, 0xbb, 0x74, 0x01, 0xf0, 0xd2, 0xaf, 0x22, 0x90, 0x7f,
  0xe9, 0xe0, 0x12, 0x0d, 0x31, 0x0a, 0x11, 0x00, 0x0a, 0x7e, 0x01, 0x0a, 0xdb, 0x03, 0x09, 0x38,
  0x06, 0x0a, 0x02, 0x08, 0x09, 0xf6, 0x09, 0x09, 0xde, 0x0a, 0x09, 0xed, 0x0b, 0x00, 0x00, 0x0b,
  0x19, 0x90, 0x7f, 0xeb, 0xe0, 0x24, 0xfe, 0x60, 0x19, 0x14, 0x60, 0x5a, 0x24, 0x02, 0x60}},
 {0x0946, 64, { 0x03, 0x02, 0x0b, 0x19, 0x74, 0x0d, 0x90, 0x7f, 0xd4, 0xf0, 0x74, 0x87, 0x90, 0x7f, 0xd5, 0xf0, 0x02,
  0x0b, 0x20, 0x90, 0x7f, 0xea, 0xe0, 0x70, 0x04, 0x7f, 0x02, 0x80, 0x02, 0x7f, 0x03, 0x75, 0x82,
  0xd8, 0x75, 0x83, 0x0d, 0xef, 0xf0, 0x75, 0x82, 0xd1, 0x75, 0x83, 0x0d, 0xf0, 0x75, 0x82, 0xca,
  0x75, 0x83, 0x0d, 0xf0, 0x75, 0x82, 0xc3, 0x75, 0x83, 0x0d, 0xf0, 0x90, 0x7f, 0xea, 0xe0}},
 {0x0986, 64, { 0x04, 0x75, 0x82, 0x9e, 0x75, 0x83, 0x0d, 0xf0, 0x74, 0x0d, 0x90, 0x7f, 0xd4, 0xf0, 0x74, 0x99, 0x90,
  0x7f, 0xd5, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xea, 0xe0, 0x70, 0x0b, 0x75, 0x11, 0xff, 0x75,
  0x12, 0x0d, 0x75, 0x13, 0xdc, 0x80, 0x1b, 0x90, 0x7f, 0xea, 0xe0, 0xb4, 0x01, 0x0b, 0x75, 0x11,
  0xff, 0x75, 0x12, 0x0d, 0x75, 0x13, 0xe0, 0x80, 0x09, 0x75, 0x11, 0xff, 0x75, 0x12, 0x0d}},
 {0x09c6, 64, { 0x75, 0x13, 0xf0, 0xaa, 0x12, 0xa9, 0x13, 0xae, 0x02, 0xee, 0x90, 0x7f, 0xd4, 0xf0, 0xaf, 0x01, 0xef,
  0x90, 0x7f, 0xd5, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0x00, 0xe5, 0x15, 0xf0, 0x90, 0x7f, 0xb5,
  0x74, 0x01, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xea, 0xe0, 0xf5, 0x15, 0x02, 0x0b, 0x20, 0x12,
  0x0c, 0xb1, 0x90, 0x7f, 0xea, 0xe0, 0xf5, 0x14, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0x00, 0xe5}},
 {0x0a06, 64, { 0x14, 0xf0, 0x90, 0x7f, 0xb5, 0x74, 0x01, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xe8, 0xe0, 0x24, 0x7f,
  0x60, 0x27, 0x14, 0x60, 0x34, 0x24, 0x02, 0x60, 0x03, 0x02, 0x0b, 0x19, 0xa2, 0x17, 0xe4, 0x33,
  0xff, 0x25, 0xe0, 0xff, 0xa2, 0x19, 0xe4, 0x33, 0x4f, 0x90, 0x7f, 0x00, 0xf0, 0xe4, 0xa3, 0xf0,
  0x90, 0x7f, 0xb5, 0x74, 0x02, 0xf0, 0x02, 0x0b, 0x20, 0xe4, 0x90, 0x7f, 0x00, 0xf0, 0xa3}},
 {0x0a46, 64, { 0xf0, 0x90, 0x7f, 0xb5, 0x74, 0x02, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xec, 0xe0, 0xf4, 0x54, 0x80,
  0xff, 0xc4, 0x54, 0x0f, 0xff, 0xe0, 0x54, 0x07, 0x2f, 0x25, 0xe0, 0x24, 0xb4, 0xf5, 0x82, 0xe4,
  0x34, 0x7f, 0xf5, 0x83, 0xe0, 0x54, 0xfd, 0x90, 0x7f, 0x00, 0xf0, 0xe4, 0xa3, 0xf0, 0x90, 0x7f,
  0xb5, 0x74, 0x02, 0xf0, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xe8, 0xe0, 0x24, 0xfe, 0x60, 0x17}},
 {0x0a86, 64, { 0x24, 0x02, 0x60, 0x03, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xea, 0xe0, 0x64, 0x01, 0x60, 0x03, 0x02, 0x0b,
  0x19, 0xc2, 0x17, 0x02, 0x0b, 0x20, 0x90, 0x7f, 0xea, 0xe0, 0x70, 0x76, 0x90, 0x7f, 0xec, 0xe0,
  0xf4, 0x54, 0x80, 0xff, 0xc4, 0x54, 0x0f, 0xff, 0xe0, 0x54, 0x07, 0x2f, 0x25, 0xe0, 0x24, 0xb4,
  0xf5, 0x82, 0xe4, 0x34, 0x7f, 0xf5, 0x83, 0xe4, 0xf0, 0x90, 0x7f, 0xec, 0xe0, 0x54, 0x80}},
 {0x0ac6, 64, { 0xff, 0x13, 0x13, 0x13, 0x54, 0x1f, 0xff, 0xe0, 0x54, 0x07, 0x2f, 0x90, 0x7f, 0xd7, 0xf0, 0xe0, 0x44,
  0x20, 0xf0, 0x80, 0x45, 0x90, 0x7f, 0xe8, 0xe0, 0x24, 0xfe, 0x60, 0x10, 0x24, 0x02, 0x70, 0x39,
  0x90, 0x7f, 0xea, 0xe0, 0x64, 0x01, 0x70, 0x2a, 0xd2, 0x17, 0x80, 0x2d, 0x90, 0x7f, 0xea, 0xe0,
  0x70, 0x20, 0x90, 0x7f, 0xec, 0xe0, 0xf4, 0x54, 0x80, 0xff, 0xc4, 0x54, 0x0f, 0xff, 0xe0}},
 {0x0b06, 64, { 0x54, 0x07, 0x2f, 0x25, 0xe0, 0x24, 0xb4, 0xf5, 0x82, 0xe4, 0x34, 0x7f, 0xf5, 0x83, 0x74, 0x01, 0xf0,
  0x80, 0x07, 0x90, 0x7f, 0xb4, 0xe0, 0x44, 0x01, 0xf0, 0x90, 0x7f, 0xb4, 0xe0, 0x44, 0x02, 0xf0,
  0x22, 0xc2, 0x10, 0xe4, 0xf5, 0x14, 0xf5, 0x34, 0xc2, 0x09, 0xc2, 0x0c, 0xc2, 0x0b, 0xc2, 0x14,
  0xc2, 0x0d, 0xc2, 0x16, 0xc2, 0x11, 0xc2, 0x07, 0xc2, 0x12, 0xc2, 0x0f, 0xc2, 0x08, 0xf5}},
 {0x0b46, 64, { 0x35, 0xf5, 0x39, 0xf5, 0x53, 0xf5, 0x3a, 0xf5, 0x33, 0xf5, 0x30, 0xf5, 0x2f, 0xf5, 0x2e, 0xf5, 0x2d,
  0xf5, 0x2c, 0xf5, 0x2b, 0xf5, 0x2a, 0xf5, 0x29, 0xf5, 0x28, 0xf5, 0x27, 0xf5, 0x26, 0xf5, 0x25,
  0xf5, 0x24, 0xc2, 0x05, 0xc2, 0x18, 0xc2, 0x1a, 0xc2, 0x17, 0xc2, 0x19, 0xc2, 0x15, 0xc2, 0x04,
  0xd2, 0x13, 0xc2, 0x06, 0xc2, 0x01, 0x90, 0x7f, 0x92, 0xe0, 0x54, 0xfd, 0xf0, 0xd2, 0xe8}},
 {0x0b86, 64, { 0x43, 0xd8, 0x20, 0x90, 0x7f, 0xde, 0x74, 0x01, 0xf0, 0x90, 0x7f, 0xdf, 0xf0, 0x90, 0x7f, 0xab, 0x74,
  0xff, 0xf0, 0x90, 0x7f, 0xa9, 0xf0, 0x90, 0x7f, 0xaa, 0xf0, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xaf,
  0xe0, 0x44, 0x01, 0xf0, 0x90, 0x7f, 0xae, 0xe0, 0x44, 0x0f, 0xf0, 0x90, 0x7f, 0xac, 0x74, 0x0e,
  0xf0, 0xd2, 0xaf, 0xd2, 0xbc, 0xd2, 0x1b, 0x12, 0x0f, 0x5f, 0xc2, 0x18, 0x30, 0x04, 0x03}},
 {0x0bc6, 64, { 0x12, 0x05, 0x6d, 0x30, 0x04, 0x2a, 0x30, 0x06, 0x27, 0xc2, 0x06, 0xe5, 0x16, 0x60, 0x16, 0x15, 0x16,
  0x90, 0x7f, 0xd8, 0xe0, 0x30, 0xe6, 0x04, 0x7f, 0x00, 0x80, 0x02, 0x7f, 0x20, 0x90, 0x7f, 0x96,
  0xef, 0xf0, 0x80, 0x06, 0x90, 0x7f, 0x96, 0x74, 0x20, 0xf0, 0x12, 0x0c, 0x0b, 0x80, 0xcd, 0x30,
  0x18, 0x07, 0xc2, 0x18, 0x12, 0x09, 0x15, 0x80, 0xc3, 0x30, 0x1a, 0xc0, 0xc2, 0x1a, 0x12}},
 {0x0c06, 64, { 0x0f, 0xbb, 0x80, 0xb9, 0x22, 0xe5, 0x31, 0x60, 0x02, 0x15, 0x31, 0xe5, 0x39, 0x60, 0x55, 0x65, 0x35,
  0x70, 0x4b, 0xe5, 0x33, 0xf4, 0x60, 0x02, 0x05, 0x33, 0xe5, 0x33, 0xc3, 0x95, 0x44, 0x40, 0x43,
  0xc2, 0xaf, 0x30, 0x02, 0x1b, 0x90, 0x7f, 0xb8, 0xe0, 0x20, 0xe1, 0x2d, 0x90, 0x7f, 0xb7, 0xe5,
  0x39, 0xf0, 0xc2, 0x02, 0xe4, 0xf5, 0x39, 0xf5, 0x33, 0xf5, 0x35, 0x75, 0x16, 0xff, 0x80}},
 {0x0c46, 64, { 0x19, 0x90, 0x7f, 0xb6, 0xe0, 0x20, 0xe1, 0x12, 0x90, 0x7f, 0xb9, 0xe5, 0x39, 0xf0, 0xd2, 0x02, 0xe4,
  0xf5, 0x39, 0xf5, 0x33, 0xf5, 0x35, 0x75, 0x16, 0xff, 0xd2, 0xaf, 0x80, 0x06, 0x85, 0x39, 0x35,
  0xe4, 0xf5, 0x33, 0xe5, 0x2c, 0x60, 0x30, 0x20, 0x0f, 0x07, 0x90, 0x7f, 0x9b, 0xe0, 0x30, 0xe0,
  0x0f, 0xe5, 0x2d, 0x60, 0x06, 0xe4, 0xf5, 0x2d, 0x43, 0x34, 0x01, 0xe4, 0xf5, 0x30, 0x80}},
 {0x0c86, 64, { 0x14, 0xe5, 0x30, 0xd3, 0x95, 0x45, 0x50, 0x0d, 0xe5, 0x30, 0xb5, 0x45, 0x06, 0x75, 0x2d, 0x01, 0x43,
  0x34, 0x01, 0x05, 0x30, 0xc2, 0x0f, 0x22, 0x90, 0x7f, 0xd9, 0xe0, 0x30, 0xe2, 0x04, 0x7f, 0x00,
  0x80, 0x02, 0x7f, 0x20, 0x90, 0x7f, 0x96, 0xef, 0xf0, 0x22, 0xe4, 0x90, 0x7f, 0x93, 0xf0, 0x90,
  0x7f, 0x9c, 0x74, 0x30, 0xf0, 0x90, 0x7f, 0x96, 0x74, 0x20, 0xf0, 0x90, 0x7f, 0x94, 0x74}},
 {0x0cc6, 64, { 0x01, 0xf0, 0x90, 0x7f, 0x9d, 0x74, 0xbf, 0xf0, 0x90, 0x7f, 0x97, 0x74, 0x86, 0xf0, 0x90, 0x7f, 0x95,
  0x74, 0x03, 0xf0, 0x90, 0x7f, 0x9e, 0x74, 0x84, 0xf0, 0x90, 0x7f, 0x98, 0xf0, 0xe4, 0x90, 0x7f,
  0xc7, 0xf0, 0x90, 0x7f, 0xc9, 0xf0, 0x90, 0x7f, 0xcb, 0xf0, 0x75, 0x98, 0x40, 0x43, 0xa8, 0x10,
  0x90, 0x7f, 0xde, 0x74, 0x1f, 0xf0, 0x90, 0x7f, 0xdf, 0x74, 0x0f, 0xf0, 0xd2, 0x04, 0x22}},
 {0x0d06, 64, { 0xbb, 0x01, 0x06, 0x89, 0x82, 0x8a, 0x83, 0xe0, 0x22, 0x50, 0x02, 0xe7, 0x22, 0xbb, 0xfe, 0x02, 0xe3,
  0x22, 0x89, 0x82, 0x8a, 0x83, 0xe4, 0x93, 0x22, 0xbb, 0x01, 0x06, 0x89, 0x82, 0x8a, 0x83, 0xf0,
  0x22, 0x50, 0x02, 0xf7, 0x22, 0xbb, 0xfe, 0x01, 0xf3, 0x22, 0xd0, 0x83, 0xd0, 0x82, 0xf8, 0xe4,
  0x93, 0x70, 0x12, 0x74, 0x01, 0x93, 0x70, 0x0d, 0xa3, 0xa3, 0x93, 0xf8, 0x74, 0x01, 0x93}},
 {0x0d46, 64, { 0xf5, 0x82, 0x88, 0x83, 0xe4, 0x73, 0x74, 0x02, 0x93, 0x68, 0x60, 0xef, 0xa3, 0xa3, 0xa3, 0x80, 0xdf,
  0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86, 0x00, 0x90,
  0x7f, 0xc4, 0xe4, 0xf0, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xab, 0x74, 0x04, 0xf0, 0xd0, 0x86, 0xd0,
  0x84, 0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00}},
 {0x0d86, 64, { 0x00, 0x12, 0x01, 0x10, 0x01, 0xff, 0x00, 0x00, 0x40, 0xcd, 0x06, 0x0c, 0x01, 0x01, 0x00, 0x01, 0x02,
  0x00, 0x02, 0x09, 0x02, 0x43, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, 0x09, 0x04, 0x00, 0x00, 0x07,
  0xff, 0x00, 0x00, 0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x02, 0x02, 0x40,
  0x00, 0x00, 0x07, 0x05, 0x03, 0x02, 0x40, 0x00, 0x00, 0x07, 0x05, 0x81, 0x02, 0x40, 0x00}},
 {0x0dc6, 64, { 0x01, 0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x01, 0x07, 0x05, 0x83, 0x02, 0x40, 0x00, 0x01, 0x07, 0x05,
  0x84, 0x02, 0x40, 0x00, 0x01, 0x04, 0x03, 0x09, 0x04, 0x10, 0x03, 0x4b, 0x00, 0x65, 0x00, 0x79,
  0x00, 0x73, 0x00, 0x70, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x0e, 0x03, 0x53, 0x00, 0x65, 0x00, 0x72,
  0x00, 0x69, 0x00, 0x61, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x02, 0x0e, 0xa2, 0x00, 0x02, 0x0e}},
 {0x0e06, 64, { 0x7b, 0x00, 0x02, 0x0d, 0x57, 0x00, 0x02, 0x0e, 0xc9, 0x00, 0x02, 0x0e, 0x10, 0x00, 0x02, 0x0e, 0x14,
  0x00, 0x02, 0x0e, 0x18, 0x00, 0x02, 0x0e, 0x1c, 0x00, 0x02, 0x0e, 0xf0, 0x00, 0x02, 0x0e, 0x24,
  0x00, 0x02, 0x0f, 0x15, 0x00, 0x02, 0x0e, 0x2c, 0x00, 0x02, 0x0f, 0x3a, 0xe4, 0x90, 0x7f, 0x95,
  0xf0, 0x90, 0x7f, 0x94, 0xf0, 0x90, 0x7f, 0x93, 0xf0, 0x90, 0x7f, 0x9d, 0xe0, 0x44, 0x02}},
 {0x0e46, 64, { 0xf0, 0x90, 0x7f, 0x97, 0xe0, 0x44, 0x02, 0xf0, 0x90, 0x7f, 0x9c, 0x74, 0x10, 0xf0, 0xe4, 0x90, 0x7f,
  0x96, 0xf0, 0x90, 0x7f, 0x9d, 0x74, 0xfe, 0xf0, 0x30, 0x17, 0x04, 0x7f, 0x80, 0x80, 0x02, 0x7f,
  0x00, 0x90, 0x7f, 0x97, 0xef, 0xf0, 0xe4, 0x90, 0x7f, 0x95, 0xf0, 0x90, 0x7f, 0x9e, 0xf0, 0x90,
  0x7f, 0x98, 0xf0, 0x22, 0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0}},
 {0x0e86, 64, { 0x86, 0x75, 0x86, 0x00, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xab, 0x74, 0x02, 0xf0, 0xd2, 0x06, 0xd0, 0x86,
  0xd0, 0x84, 0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0xc0, 0xe0, 0xc0, 0x83, 0xc0,
  0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86, 0x00, 0xd2, 0x18, 0x53, 0x91, 0xef, 0x90,
  0x7f, 0xab, 0x74, 0x01, 0xf0, 0xd0, 0x86, 0xd0, 0x84, 0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83}},
 {0x0ec6, 64, { 0xd0, 0xe0, 0x32, 0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86,
  0x00, 0xd2, 0x1a, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xab, 0x74, 0x08, 0xf0, 0xd0, 0x86, 0xd0, 0x84,
  0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0,
  0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86, 0x00, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xa9, 0x74}},
 {0x0f06, 64, { 0x02, 0xf0, 0xd0, 0x86, 0xd0, 0x84, 0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0xc0, 0xe0,
  0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86, 0x75, 0x86, 0x00, 0x53, 0x91, 0xef,
  0x90, 0x7f, 0xa9, 0x74, 0x04, 0xf0, 0xd0, 0x86, 0xd0, 0x84, 0xd0, 0x85, 0xd0, 0x82, 0xd0, 0x83,
  0xd0, 0xe0, 0x32, 0xc0, 0xe0, 0xc0, 0x83, 0xc0, 0x82, 0xc0, 0x85, 0xc0, 0x84, 0xc0, 0x86}},
 {0x0f46, 64, { 0x75, 0x86, 0x00, 0x53, 0x91, 0xef, 0x90, 0x7f, 0xa9, 0x74, 0x08, 0xf0, 0xd0, 0x86, 0xd0, 0x84, 0xd0,
  0x85, 0xd0, 0x82, 0xd0, 0x83, 0xd0, 0xe0, 0x32, 0x90, 0x7f, 0xd6, 0xe0, 0x54, 0xfb, 0xf0, 0xe0,
  0x44, 0x08, 0xf0, 0x30, 0x1b, 0x04, 0xe0, 0x44, 0x02, 0xf0, 0x7f, 0xf4, 0x7e, 0x01, 0x12, 0x00,
  0x03, 0x90, 0x7f, 0xd6, 0xe0, 0x54, 0xf7, 0xf0, 0xe0, 0x44, 0x04, 0xf0, 0x22, 0x74, 0x00}},
 {0x0f86, 64, { 0xf5, 0x86, 0x90, 0xfd, 0xa5, 0x7c, 0x05, 0xa3, 0xe5, 0x82, 0x45, 0x83, 0x70, 0xf9, 0x22, 0x90, 0x7f,
  0xd6, 0xe0, 0x44, 0x80, 0xf0, 0x43, 0x87, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x90, 0x7f,
  0xd6, 0xe0, 0x44, 0x01, 0xf0, 0x7f, 0x0d, 0x7e, 0x00, 0x12, 0x00, 0x03, 0x90, 0x7f, 0xd6, 0xe0,
  0x54, 0xfe, 0xf0, 0x22, 0x12, 0x0e, 0x33, 0x12, 0x0f, 0x95, 0x90, 0x7f, 0xd6, 0xe0, 0x30}},
 {0x0fc6,  9, { 0xe7, 0x03, 0x12, 0x0f, 0xa5, 0x12, 0x0c, 0xb1, 0x22}},
 {0xffff,	0,	{0x00}}
};
