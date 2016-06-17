/* gt64240r.h - GT-64240 Internal registers definition file */

/* Copyright - Galileo technology. */

#ifndef __INCgt64240rh
#define __INCgt64240rh

#define GTREG(v)        (((v) & 0xff) << 24) | (((v) & 0xff00) << 8) | \
                        (((v) >> 24) & 0xff) | (((v) >> 8) & 0xff00)

#if 0
#define GTREG_SHORT(X)	(((X) << 8) | ((X) >> 8))

#define LONG_GTREG(X)	((l64) \
			(((X)&0x00000000000000ffULL) << 56) | \
			(((X)&0x000000000000ff00ULL) << 40) | \
			(((X)&0x0000000000ff0000ULL) << 24) | \
			(((X)&0x00000000ff000000ULL) << 8)  | \
			(((X)&0x000000ff00000000ULL) >> 8)  | \
			(((X)&0x0000ff0000000000ULL) >> 24) | \
			(((X)&0x00ff000000000000ULL) >> 40) | \
			(((X)&0xff00000000000000ULL) >> 56))
#endif

#include "gt64240_dep.h"

/****************************************/
/* CPU Control Registers		*/
/****************************************/

#define CPU_CONFIGURATION					0x000
#define CPU_MODE						0x120
#define CPU_READ_RESPONSE_CROSSBAR_LOW				0x170
#define CPU_READ_RESPONSE_CROSSBAR_HIGH				0x178

/****************************************/
/* Processor Address Space		*/
/****************************************/

/* Sdram's BAR'S */
#define SCS_0_LOW_DECODE_ADDRESS				0x008
#define SCS_0_HIGH_DECODE_ADDRESS				0x010
#define SCS_1_LOW_DECODE_ADDRESS				0x208
#define SCS_1_HIGH_DECODE_ADDRESS				0x210
#define SCS_2_LOW_DECODE_ADDRESS				0x018
#define SCS_2_HIGH_DECODE_ADDRESS				0x020
#define SCS_3_LOW_DECODE_ADDRESS				0x218
#define SCS_3_HIGH_DECODE_ADDRESS				0x220
/* Devices BAR'S */
#define CS_0_LOW_DECODE_ADDRESS					0x028
#define CS_0_HIGH_DECODE_ADDRESS				0x030
#define CS_1_LOW_DECODE_ADDRESS					0x228
#define CS_1_HIGH_DECODE_ADDRESS				0x230
#define CS_2_LOW_DECODE_ADDRESS					0x248
#define CS_2_HIGH_DECODE_ADDRESS				0x250
#define CS_3_LOW_DECODE_ADDRESS					0x038
#define CS_3_HIGH_DECODE_ADDRESS				0x040
#define BOOTCS_LOW_DECODE_ADDRESS				0x238
#define BOOTCS_HIGH_DECODE_ADDRESS				0x240

#define PCI_0I_O_LOW_DECODE_ADDRESS				0x048
#define PCI_0I_O_HIGH_DECODE_ADDRESS				0x050
#define PCI_0MEMORY0_LOW_DECODE_ADDRESS				0x058
#define PCI_0MEMORY0_HIGH_DECODE_ADDRESS			0x060
#define PCI_0MEMORY1_LOW_DECODE_ADDRESS				0x080
#define PCI_0MEMORY1_HIGH_DECODE_ADDRESS			0x088
#define PCI_0MEMORY2_LOW_DECODE_ADDRESS				0x258
#define PCI_0MEMORY2_HIGH_DECODE_ADDRESS			0x260
#define PCI_0MEMORY3_LOW_DECODE_ADDRESS				0x280
#define PCI_0MEMORY3_HIGH_DECODE_ADDRESS			0x288

#define PCI_1I_O_LOW_DECODE_ADDRESS				0x090
#define PCI_1I_O_HIGH_DECODE_ADDRESS				0x098
#define PCI_1MEMORY0_LOW_DECODE_ADDRESS				0x0a0
#define PCI_1MEMORY0_HIGH_DECODE_ADDRESS			0x0a8
#define PCI_1MEMORY1_LOW_DECODE_ADDRESS				0x0b0
#define PCI_1MEMORY1_HIGH_DECODE_ADDRESS			0x0b8
#define PCI_1MEMORY2_LOW_DECODE_ADDRESS				0x2a0
#define PCI_1MEMORY2_HIGH_DECODE_ADDRESS			0x2a8
#define PCI_1MEMORY3_LOW_DECODE_ADDRESS				0x2b0
#define PCI_1MEMORY3_HIGH_DECODE_ADDRESS			0x2b8

#define INTERNAL_SPACE_DECODE					0x068

#define CPU_0_LOW_DECODE_ADDRESS                            0x290
#define CPU_0_HIGH_DECODE_ADDRESS                           0x298
#define CPU_1_LOW_DECODE_ADDRESS                            0x2c0
#define CPU_1_HIGH_DECODE_ADDRESS                           0x2c8

#define PCI_0I_O_ADDRESS_REMAP					0x0f0
#define PCI_0MEMORY0_ADDRESS_REMAP  				0x0f8
#define PCI_0MEMORY0_HIGH_ADDRESS_REMAP				0x320
#define PCI_0MEMORY1_ADDRESS_REMAP  				0x100
#define PCI_0MEMORY1_HIGH_ADDRESS_REMAP				0x328
#define PCI_0MEMORY2_ADDRESS_REMAP  				0x2f8
#define PCI_0MEMORY2_HIGH_ADDRESS_REMAP				0x330
#define PCI_0MEMORY3_ADDRESS_REMAP  			  	0x300
#define PCI_0MEMORY3_HIGH_ADDRESS_REMAP			   	0x338

#define PCI_1I_O_ADDRESS_REMAP					0x108
#define PCI_1MEMORY0_ADDRESS_REMAP  				0x110
#define PCI_1MEMORY0_HIGH_ADDRESS_REMAP				0x340
#define PCI_1MEMORY1_ADDRESS_REMAP  				0x118
#define PCI_1MEMORY1_HIGH_ADDRESS_REMAP				0x348
#define PCI_1MEMORY2_ADDRESS_REMAP  				0x310
#define PCI_1MEMORY2_HIGH_ADDRESS_REMAP				0x350
#define PCI_1MEMORY3_ADDRESS_REMAP  				0x318
#define PCI_1MEMORY3_HIGH_ADDRESS_REMAP				0x358

/****************************************/
/* CPU Sync Barrier             		*/
/****************************************/

#define PCI_0SYNC_BARIER_VIRTUAL_REGISTER			0x0c0
#define PCI_1SYNC_BARIER_VIRTUAL_REGISTER			0x0c8


/****************************************/
/* CPU Access Protect             		*/
/****************************************/

#define CPU_LOW_PROTECT_ADDRESS_0                           0X180
#define CPU_HIGH_PROTECT_ADDRESS_0                          0X188
#define CPU_LOW_PROTECT_ADDRESS_1                           0X190
#define CPU_HIGH_PROTECT_ADDRESS_1                          0X198
#define CPU_LOW_PROTECT_ADDRESS_2                           0X1a0
#define CPU_HIGH_PROTECT_ADDRESS_2                          0X1a8
#define CPU_LOW_PROTECT_ADDRESS_3                           0X1b0
#define CPU_HIGH_PROTECT_ADDRESS_3                          0X1b8
#define CPU_LOW_PROTECT_ADDRESS_4                           0X1c0
#define CPU_HIGH_PROTECT_ADDRESS_4                          0X1c8
#define CPU_LOW_PROTECT_ADDRESS_5                           0X1d0
#define CPU_HIGH_PROTECT_ADDRESS_5                          0X1d8
#define CPU_LOW_PROTECT_ADDRESS_6                           0X1e0
#define CPU_HIGH_PROTECT_ADDRESS_6                          0X1e8
#define CPU_LOW_PROTECT_ADDRESS_7                           0X1f0
#define CPU_HIGH_PROTECT_ADDRESS_7                          0X1f8


/****************************************/
/*          Snoop Control          		*/
/****************************************/

#define SNOOP_BASE_ADDRESS_0                                0x380
#define SNOOP_TOP_ADDRESS_0                                 0x388
#define SNOOP_BASE_ADDRESS_1                                0x390
#define SNOOP_TOP_ADDRESS_1                                 0x398
#define SNOOP_BASE_ADDRESS_2                                0x3a0
#define SNOOP_TOP_ADDRESS_2                                 0x3a8
#define SNOOP_BASE_ADDRESS_3                                0x3b0
#define SNOOP_TOP_ADDRESS_3                                 0x3b8

/****************************************/
/*          CPU Error Report       		*/
/****************************************/

#define CPU_ERROR_ADDRESS_LOW 				    0x070
#define CPU_ERROR_ADDRESS_HIGH 				    0x078
#define CPU_ERROR_DATA_LOW                                  0x128
#define CPU_ERROR_DATA_HIGH                                 0x130
#define CPU_ERROR_PARITY                                    0x138
#define CPU_ERROR_CAUSE                                     0x140
#define CPU_ERROR_MASK                                      0x148

/****************************************/
/*          Pslave Debug           		*/
/****************************************/

#define X_0_ADDRESS                                         0x360
#define X_0_COMMAND_ID                                      0x368
#define X_1_ADDRESS                                         0x370
#define X_1_COMMAND_ID                                      0x378
#define WRITE_DATA_LOW                                      0x3c0
#define WRITE_DATA_HIGH                                     0x3c8
#define WRITE_BYTE_ENABLE                                   0X3e0
#define READ_DATA_LOW                                       0x3d0
#define READ_DATA_HIGH                                      0x3d8
#define READ_ID                                             0x3e8


/****************************************/
/* SDRAM and Device Address Space	*/
/****************************************/


/****************************************/
/* SDRAM Configuration			*/
/****************************************/

#define SDRAM_CONFIGURATION	 			0x448
#define SDRAM_OPERATION_MODE				0x474
#define SDRAM_ADDRESS_DECODE				0x47C
#define SDRAM_TIMING_PARAMETERS                         0x4b4
#define SDRAM_UMA_CONTROL                               0x4a4
#define SDRAM_CROSS_BAR_CONTROL_LOW                     0x4a8
#define SDRAM_CROSS_BAR_CONTROL_HIGH                    0x4ac
#define SDRAM_CROSS_BAR_TIMEOUT                         0x4b0


/****************************************/
/* SDRAM Parameters			*/
/****************************************/

#define SDRAM_BANK0PARAMETERS				0x44C
#define SDRAM_BANK1PARAMETERS				0x450
#define SDRAM_BANK2PARAMETERS				0x454
#define SDRAM_BANK3PARAMETERS				0x458


/****************************************/
/* SDRAM Error Report 			*/
/****************************************/

#define SDRAM_ERROR_DATA_LOW                            0x484
#define SDRAM_ERROR_DATA_HIGH                           0x480
#define SDRAM_AND_DEVICE_ERROR_ADDRESS                  0x490
#define SDRAM_RECEIVED_ECC                              0x488
#define SDRAM_CALCULATED_ECC                            0x48c
#define SDRAM_ECC_CONTROL                               0x494
#define SDRAM_ECC_ERROR_COUNTER                         0x498


/****************************************/
/* SDunit Debug (for internal use)	*/
/****************************************/

#define X0_ADDRESS                                      0x500
#define X0_COMMAND_AND_ID                               0x504
#define X0_WRITE_DATA_LOW                               0x508
#define X0_WRITE_DATA_HIGH                              0x50c
#define X0_WRITE_BYTE_ENABLE                            0x518
#define X0_READ_DATA_LOW                                0x510
#define X0_READ_DATA_HIGH                               0x514
#define X0_READ_ID                                      0x51c
#define X1_ADDRESS                                      0x520
#define X1_COMMAND_AND_ID                               0x524
#define X1_WRITE_DATA_LOW                               0x528
#define X1_WRITE_DATA_HIGH                              0x52c
#define X1_WRITE_BYTE_ENABLE                            0x538
#define X1_READ_DATA_LOW                                0x530
#define X1_READ_DATA_HIGH                               0x534
#define X1_READ_ID                                      0x53c
#define X0_SNOOP_ADDRESS                                0x540
#define X0_SNOOP_COMMAND                                0x544
#define X1_SNOOP_ADDRESS                                0x548
#define X1_SNOOP_COMMAND                                0x54c


/****************************************/
/* Device Parameters			*/
/****************************************/

#define DEVICE_BANK0PARAMETERS				0x45c
#define DEVICE_BANK1PARAMETERS				0x460
#define DEVICE_BANK2PARAMETERS				0x464
#define DEVICE_BANK3PARAMETERS				0x468
#define DEVICE_BOOT_BANK_PARAMETERS			0x46c
#define DEVICE_CONTROL                                  0x4c0
#define DEVICE_CROSS_BAR_CONTROL_LOW                    0x4c8
#define DEVICE_CROSS_BAR_CONTROL_HIGH                   0x4cc
#define DEVICE_CROSS_BAR_TIMEOUT                        0x4c4


/****************************************/
/* Device Interrupt 			*/
/****************************************/

#define DEVICE_INTERRUPT_CAUSE                              0x4d0
#define DEVICE_INTERRUPT_MASK                               0x4d4
#define DEVICE_ERROR_ADDRESS                                0x4d8

/****************************************/
/* DMA Record				*/
/****************************************/

#define CHANNEL0_DMA_BYTE_COUNT					0x800
#define CHANNEL1_DMA_BYTE_COUNT	 				0x804
#define CHANNEL2_DMA_BYTE_COUNT	 				0x808
#define CHANNEL3_DMA_BYTE_COUNT	 				0x80C
#define CHANNEL4_DMA_BYTE_COUNT					0x900
#define CHANNEL5_DMA_BYTE_COUNT	 				0x904
#define CHANNEL6_DMA_BYTE_COUNT	 				0x908
#define CHANNEL7_DMA_BYTE_COUNT	 				0x90C
#define CHANNEL0_DMA_SOURCE_ADDRESS				0x810
#define CHANNEL1_DMA_SOURCE_ADDRESS				0x814
#define CHANNEL2_DMA_SOURCE_ADDRESS				0x818
#define CHANNEL3_DMA_SOURCE_ADDRESS				0x81C
#define CHANNEL4_DMA_SOURCE_ADDRESS				0x910
#define CHANNEL5_DMA_SOURCE_ADDRESS				0x914
#define CHANNEL6_DMA_SOURCE_ADDRESS				0x918
#define CHANNEL7_DMA_SOURCE_ADDRESS				0x91C
#define CHANNEL0_DMA_DESTINATION_ADDRESS			0x820
#define CHANNEL1_DMA_DESTINATION_ADDRESS			0x824
#define CHANNEL2_DMA_DESTINATION_ADDRESS			0x828
#define CHANNEL3_DMA_DESTINATION_ADDRESS			0x82C
#define CHANNEL4_DMA_DESTINATION_ADDRESS			0x920
#define CHANNEL5_DMA_DESTINATION_ADDRESS			0x924
#define CHANNEL6_DMA_DESTINATION_ADDRESS			0x928
#define CHANNEL7_DMA_DESTINATION_ADDRESS			0x92C
#define CHANNEL0NEXT_RECORD_POINTER				0x830
#define CHANNEL1NEXT_RECORD_POINTER				0x834
#define CHANNEL2NEXT_RECORD_POINTER				0x838
#define CHANNEL3NEXT_RECORD_POINTER				0x83C
#define CHANNEL4NEXT_RECORD_POINTER				0x930
#define CHANNEL5NEXT_RECORD_POINTER				0x934
#define CHANNEL6NEXT_RECORD_POINTER				0x938
#define CHANNEL7NEXT_RECORD_POINTER				0x93C
#define CHANNEL0CURRENT_DESCRIPTOR_POINTER			0x870
#define CHANNEL1CURRENT_DESCRIPTOR_POINTER			0x874
#define CHANNEL2CURRENT_DESCRIPTOR_POINTER			0x878
#define CHANNEL3CURRENT_DESCRIPTOR_POINTER			0x87C
#define CHANNEL4CURRENT_DESCRIPTOR_POINTER			0x970
#define CHANNEL5CURRENT_DESCRIPTOR_POINTER			0x974
#define CHANNEL6CURRENT_DESCRIPTOR_POINTER			0x978
#define CHANNEL7CURRENT_DESCRIPTOR_POINTER			0x97C
#define CHANNEL0_DMA_SOURCE_HIGH_PCI_ADDRESS			0x890
#define CHANNEL1_DMA_SOURCE_HIGH_PCI_ADDRESS			0x894
#define CHANNEL2_DMA_SOURCE_HIGH_PCI_ADDRESS			0x898
#define CHANNEL3_DMA_SOURCE_HIGH_PCI_ADDRESS			0x89c
#define CHANNEL4_DMA_SOURCE_HIGH_PCI_ADDRESS			0x990
#define CHANNEL5_DMA_SOURCE_HIGH_PCI_ADDRESS			0x994
#define CHANNEL6_DMA_SOURCE_HIGH_PCI_ADDRESS			0x998
#define CHANNEL7_DMA_SOURCE_HIGH_PCI_ADDRESS			0x99c
#define CHANNEL0_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x8a0
#define CHANNEL1_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x8a4
#define CHANNEL2_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x8a8
#define CHANNEL3_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x8ac
#define CHANNEL4_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x9a0
#define CHANNEL5_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x9a4
#define CHANNEL6_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x9a8
#define CHANNEL7_DMA_DESTINATION_HIGH_PCI_ADDRESS		0x9ac
#define CHANNEL0_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x8b0
#define CHANNEL1_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x8b4
#define CHANNEL2_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x8b8
#define CHANNEL3_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x8bc
#define CHANNEL4_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x9b0
#define CHANNEL5_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x9b4
#define CHANNEL6_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x9b8
#define CHANNEL7_DMA_NEXT_RECORD_POINTER_HIGH_PCI_ADDRESS	0x9bc

/****************************************/
/* DMA Channel Control			*/
/****************************************/

#define CHANNEL0CONTROL 					0x840
#define CHANNEL0CONTROL_HIGH					0x880

#define CHANNEL1CONTROL 					0x844
#define CHANNEL1CONTROL_HIGH					0x884

#define CHANNEL2CONTROL 					0x848
#define CHANNEL2CONTROL_HIGH					0x888

#define CHANNEL3CONTROL 					0x84C
#define CHANNEL3CONTROL_HIGH					0x88C

#define CHANNEL4CONTROL 					0x940
#define CHANNEL4CONTROL_HIGH					0x980

#define CHANNEL5CONTROL 					0x944
#define CHANNEL5CONTROL_HIGH					0x984

#define CHANNEL6CONTROL 					0x948
#define CHANNEL6CONTROL_HIGH					0x988

#define CHANNEL7CONTROL 					0x94C
#define CHANNEL7CONTROL_HIGH					0x98C


/****************************************/
/* DMA Arbiter				*/
/****************************************/

#define ARBITER_CONTROL_0_3					0x860
#define ARBITER_CONTROL_4_7					0x960


/****************************************/
/* DMA Interrupt			*/
/****************************************/

#define CHANELS0_3_INTERRUPT_CAUSE				0x8c0
#define CHANELS0_3_INTERRUPT_MASK				0x8c4
#define CHANELS0_3_ERROR_ADDRESS				0x8c8
#define CHANELS0_3_ERROR_SELECT					0x8cc
#define CHANELS4_7_INTERRUPT_CAUSE				0x9c0
#define CHANELS4_7_INTERRUPT_MASK				0x9c4
#define CHANELS4_7_ERROR_ADDRESS				0x9c8
#define CHANELS4_7_ERROR_SELECT					0x9cc


/****************************************/
/* DMA Debug (for internal use)         */
/****************************************/

#define DMA_X0_ADDRESS                                      0x8e0
#define DMA_X0_COMMAND_AND_ID                               0x8e4
#define DMA_X0_WRITE_DATA_LOW                               0x8e8
#define DMA_X0_WRITE_DATA_HIGH                              0x8ec
#define DMA_X0_WRITE_BYTE_ENABLE                            0x8f8
#define DMA_X0_READ_DATA_LOW                                0x8f0
#define DMA_X0_READ_DATA_HIGH                               0x8f4
#define DMA_X0_READ_ID                                      0x8fc
#define DMA_X1_ADDRESS                                      0x9e0
#define DMA_X1_COMMAND_AND_ID                               0x9e4
#define DMA_X1_WRITE_DATA_LOW                               0x9e8
#define DMA_X1_WRITE_DATA_HIGH                              0x9ec
#define DMA_X1_WRITE_BYTE_ENABLE                            0x9f8
#define DMA_X1_READ_DATA_LOW                                0x9f0
#define DMA_X1_READ_DATA_HIGH                               0x9f4
#define DMA_X1_READ_ID                                      0x9fc

/****************************************/
/* Timer_Counter 						*/
/****************************************/

#define TIMER_COUNTER0						0x850
#define TIMER_COUNTER1						0x854
#define TIMER_COUNTER2						0x858
#define TIMER_COUNTER3						0x85C
#define TIMER_COUNTER_0_3_CONTROL				0x864
#define TIMER_COUNTER_0_3_INTERRUPT_CAUSE			0x868
#define TIMER_COUNTER_0_3_INTERRUPT_MASK      			0x86c
#define TIMER_COUNTER4						0x950
#define TIMER_COUNTER5						0x954
#define TIMER_COUNTER6						0x958
#define TIMER_COUNTER7						0x95C
#define TIMER_COUNTER_4_7_CONTROL				0x964
#define TIMER_COUNTER_4_7_INTERRUPT_CAUSE			0x968
#define TIMER_COUNTER_4_7_INTERRUPT_MASK      			0x96c

/****************************************/
/* PCI Slave Address Decoding           */
/****************************************/

#define PCI_0SCS_0_BANK_SIZE					0xc08
#define PCI_1SCS_0_BANK_SIZE					0xc88
#define PCI_0SCS_1_BANK_SIZE					0xd08
#define PCI_1SCS_1_BANK_SIZE					0xd88
#define PCI_0SCS_2_BANK_SIZE					0xc0c
#define PCI_1SCS_2_BANK_SIZE					0xc8c
#define PCI_0SCS_3_BANK_SIZE					0xd0c
#define PCI_1SCS_3_BANK_SIZE					0xd8c
#define PCI_0CS_0_BANK_SIZE				    	0xc10
#define PCI_1CS_0_BANK_SIZE				    	0xc90
#define PCI_0CS_1_BANK_SIZE				    	0xd10
#define PCI_1CS_1_BANK_SIZE				    	0xd90
#define PCI_0CS_2_BANK_SIZE				    	0xd18
#define PCI_1CS_2_BANK_SIZE				    	0xd98
#define PCI_0CS_3_BANK_SIZE			       		0xc14
#define PCI_1CS_3_BANK_SIZE			       		0xc94
#define PCI_0CS_BOOT_BANK_SIZE					0xd14
#define PCI_1CS_BOOT_BANK_SIZE					0xd94
#define PCI_0P2P_MEM0_BAR_SIZE                              0xd1c
#define PCI_1P2P_MEM0_BAR_SIZE                              0xd9c
#define PCI_0P2P_MEM1_BAR_SIZE                              0xd20
#define PCI_1P2P_MEM1_BAR_SIZE                              0xda0
#define PCI_0P2P_I_O_BAR_SIZE                               0xd24
#define PCI_1P2P_I_O_BAR_SIZE                               0xda4
#define PCI_0CPU_BAR_SIZE                                   0xd28
#define PCI_1CPU_BAR_SIZE                                   0xda8
#define PCI_0DAC_SCS_0_BANK_SIZE                            0xe00
#define PCI_1DAC_SCS_0_BANK_SIZE                            0xe80
#define PCI_0DAC_SCS_1_BANK_SIZE                            0xe04
#define PCI_1DAC_SCS_1_BANK_SIZE                            0xe84
#define PCI_0DAC_SCS_2_BANK_SIZE                            0xe08
#define PCI_1DAC_SCS_2_BANK_SIZE                            0xe88
#define PCI_0DAC_SCS_3_BANK_SIZE                            0xe0c
#define PCI_1DAC_SCS_3_BANK_SIZE                            0xe8c
#define PCI_0DAC_CS_0_BANK_SIZE                             0xe10
#define PCI_1DAC_CS_0_BANK_SIZE                             0xe90
#define PCI_0DAC_CS_1_BANK_SIZE                             0xe14
#define PCI_1DAC_CS_1_BANK_SIZE                             0xe94
#define PCI_0DAC_CS_2_BANK_SIZE                             0xe18
#define PCI_1DAC_CS_2_BANK_SIZE                             0xe98
#define PCI_0DAC_CS_3_BANK_SIZE                             0xe1c
#define PCI_1DAC_CS_3_BANK_SIZE                             0xe9c
#define PCI_0DAC_BOOTCS_BANK_SIZE                           0xe20
#define PCI_1DAC_BOOTCS_BANK_SIZE                           0xea0
#define PCI_0DAC_P2P_MEM0_BAR_SIZE                          0xe24
#define PCI_1DAC_P2P_MEM0_BAR_SIZE                          0xea4
#define PCI_0DAC_P2P_MEM1_BAR_SIZE                          0xe28
#define PCI_1DAC_P2P_MEM1_BAR_SIZE                          0xea8
#define PCI_0DAC_CPU_BAR_SIZE                               0xe2c
#define PCI_1DAC_CPU_BAR_SIZE                               0xeac
#define PCI_0EXPANSION_ROM_BAR_SIZE                         0xd2c
#define PCI_1EXPANSION_ROM_BAR_SIZE                         0xdac
#define PCI_0BASE_ADDRESS_REGISTERS_ENABLE 			0xc3c
#define PCI_1BASE_ADDRESS_REGISTERS_ENABLE 			0xcbc
#define PCI_0SCS_0_BASE_ADDRESS_REMAP				0xc48
#define PCI_1SCS_0_BASE_ADDRESS_REMAP				0xcc8
#define PCI_0SCS_1_BASE_ADDRESS_REMAP				0xd48
#define PCI_1SCS_1_BASE_ADDRESS_REMAP				0xdc8
#define PCI_0SCS_2_BASE_ADDRESS_REMAP				0xc4c
#define PCI_1SCS_2_BASE_ADDRESS_REMAP				0xccc
#define PCI_0SCS_3_BASE_ADDRESS_REMAP				0xd4c
#define PCI_1SCS_3_BASE_ADDRESS_REMAP				0xdcc
#define PCI_0CS_0_BASE_ADDRESS_REMAP				0xc50
#define PCI_1CS_0_BASE_ADDRESS_REMAP				0xcd0
#define PCI_0CS_1_BASE_ADDRESS_REMAP				0xd50
#define PCI_1CS_1_BASE_ADDRESS_REMAP				0xdd0
#define PCI_0CS_2_BASE_ADDRESS_REMAP				0xd58
#define PCI_1CS_2_BASE_ADDRESS_REMAP				0xdd8
#define PCI_0CS_3_BASE_ADDRESS_REMAP           			0xc54
#define PCI_1CS_3_BASE_ADDRESS_REMAP           			0xcd4
#define PCI_0CS_BOOTCS_BASE_ADDRESS_REMAP      			0xd54
#define PCI_1CS_BOOTCS_BASE_ADDRESS_REMAP      			0xdd4
#define PCI_0P2P_MEM0_BASE_ADDRESS_REMAP_LOW                0xd5c
#define PCI_1P2P_MEM0_BASE_ADDRESS_REMAP_LOW                0xddc
#define PCI_0P2P_MEM0_BASE_ADDRESS_REMAP_HIGH               0xd60
#define PCI_1P2P_MEM0_BASE_ADDRESS_REMAP_HIGH               0xde0
#define PCI_0P2P_MEM1_BASE_ADDRESS_REMAP_LOW                0xd64
#define PCI_1P2P_MEM1_BASE_ADDRESS_REMAP_LOW                0xde4
#define PCI_0P2P_MEM1_BASE_ADDRESS_REMAP_HIGH               0xd68
#define PCI_1P2P_MEM1_BASE_ADDRESS_REMAP_HIGH               0xde8
#define PCI_0P2P_I_O_BASE_ADDRESS_REMAP                     0xd6c
#define PCI_1P2P_I_O_BASE_ADDRESS_REMAP                     0xdec
#define PCI_0CPU_BASE_ADDRESS_REMAP                         0xd70
#define PCI_1CPU_BASE_ADDRESS_REMAP                         0xdf0
#define PCI_0DAC_SCS_0_BASE_ADDRESS_REMAP                   0xf00
#define PCI_1DAC_SCS_0_BASE_ADDRESS_REMAP                   0xff0
#define PCI_0DAC_SCS_1_BASE_ADDRESS_REMAP                   0xf04
#define PCI_1DAC_SCS_1_BASE_ADDRESS_REMAP                   0xf84
#define PCI_0DAC_SCS_2_BASE_ADDRESS_REMAP                   0xf08
#define PCI_1DAC_SCS_2_BASE_ADDRESS_REMAP                   0xf88
#define PCI_0DAC_SCS_3_BASE_ADDRESS_REMAP                   0xf0c
#define PCI_1DAC_SCS_3_BASE_ADDRESS_REMAP                   0xf8c
#define PCI_0DAC_CS_0_BASE_ADDRESS_REMAP                    0xf10
#define PCI_1DAC_CS_0_BASE_ADDRESS_REMAP                    0xf90
#define PCI_0DAC_CS_1_BASE_ADDRESS_REMAP                    0xf14
#define PCI_1DAC_CS_1_BASE_ADDRESS_REMAP                    0xf94
#define PCI_0DAC_CS_2_BASE_ADDRESS_REMAP                    0xf18
#define PCI_1DAC_CS_2_BASE_ADDRESS_REMAP                    0xf98
#define PCI_0DAC_CS_3_BASE_ADDRESS_REMAP                    0xf1c
#define PCI_1DAC_CS_3_BASE_ADDRESS_REMAP                    0xf9c
#define PCI_0DAC_BOOTCS_BASE_ADDRESS_REMAP                  0xf20
#define PCI_1DAC_BOOTCS_BASE_ADDRESS_REMAP                  0xfa0
#define PCI_0DAC_P2P_MEM0_BASE_ADDRESS_REMAP_LOW            0xf24
#define PCI_1DAC_P2P_MEM0_BASE_ADDRESS_REMAP_LOW            0xfa4
#define PCI_0DAC_P2P_MEM0_BASE_ADDRESS_REMAP_HIGH           0xf28
#define PCI_1DAC_P2P_MEM0_BASE_ADDRESS_REMAP_HIGH           0xfa8
#define PCI_0DAC_P2P_MEM1_BASE_ADDRESS_REMAP_LOW            0xf2c
#define PCI_1DAC_P2P_MEM1_BASE_ADDRESS_REMAP_LOW            0xfac
#define PCI_0DAC_P2P_MEM1_BASE_ADDRESS_REMAP_HIGH           0xf30
#define PCI_1DAC_P2P_MEM1_BASE_ADDRESS_REMAP_HIGH           0xfb0
#define PCI_0DAC_CPU_BASE_ADDRESS_REMAP                     0xf34
#define PCI_1DAC_CPU_BASE_ADDRESS_REMAP                     0xfb4
#define PCI_0EXPANSION_ROM_BASE_ADDRESS_REMAP               0xf38
#define PCI_1EXPANSION_ROM_BASE_ADDRESS_REMAP               0xfb8
#define PCI_0ADDRESS_DECODE_CONTROL                         0xd3c
#define PCI_1ADDRESS_DECODE_CONTROL                         0xdbc

/****************************************/
/* PCI Control                          */
/****************************************/

#define PCI_0COMMAND						0xc00
#define PCI_1COMMAND						0xc80
#define PCI_0MODE                                           0xd00
#define PCI_1MODE                                           0xd80
#define PCI_0TIMEOUT_RETRY					0xc04
#define PCI_1TIMEOUT_RETRY					0xc84
#define PCI_0READ_BUFFER_DISCARD_TIMER                      0xd04
#define PCI_1READ_BUFFER_DISCARD_TIMER                      0xd84
#define MSI_0TRIGGER_TIMER                                  0xc38
#define MSI_1TRIGGER_TIMER                                  0xcb8
#define PCI_0ARBITER_CONTROL                                0x1d00
#define PCI_1ARBITER_CONTROL                                0x1d80
/* changing untill here */
#define PCI_0CROSS_BAR_CONTROL_LOW                           0x1d08
#define PCI_0CROSS_BAR_CONTROL_HIGH                          0x1d0c
#define PCI_0CROSS_BAR_TIMEOUT                               0x1d04
#define PCI_0READ_RESPONSE_CROSS_BAR_CONTROL_LOW             0x1d18
#define PCI_0READ_RESPONSE_CROSS_BAR_CONTROL_HIGH            0x1d1c
#define PCI_0SYNC_BARRIER_VIRTUAL_REGISTER                   0x1d10
#define PCI_0P2P_CONFIGURATION                               0x1d14
#define PCI_0ACCESS_CONTROL_BASE_0_LOW                       0x1e00
#define PCI_0ACCESS_CONTROL_BASE_0_HIGH                      0x1e04
#define PCI_0ACCESS_CONTROL_TOP_0                            0x1e08
#define PCI_0ACCESS_CONTROL_BASE_1_LOW                       0c1e10
#define PCI_0ACCESS_CONTROL_BASE_1_HIGH                      0x1e14
#define PCI_0ACCESS_CONTROL_TOP_1                            0x1e18
#define PCI_0ACCESS_CONTROL_BASE_2_LOW                       0c1e20
#define PCI_0ACCESS_CONTROL_BASE_2_HIGH                      0x1e24
#define PCI_0ACCESS_CONTROL_TOP_2                            0x1e28
#define PCI_0ACCESS_CONTROL_BASE_3_LOW                       0c1e30
#define PCI_0ACCESS_CONTROL_BASE_3_HIGH                      0x1e34
#define PCI_0ACCESS_CONTROL_TOP_3                            0x1e38
#define PCI_0ACCESS_CONTROL_BASE_4_LOW                       0c1e40
#define PCI_0ACCESS_CONTROL_BASE_4_HIGH                      0x1e44
#define PCI_0ACCESS_CONTROL_TOP_4                            0x1e48
#define PCI_0ACCESS_CONTROL_BASE_5_LOW                       0c1e50
#define PCI_0ACCESS_CONTROL_BASE_5_HIGH                      0x1e54
#define PCI_0ACCESS_CONTROL_TOP_5                            0x1e58
#define PCI_0ACCESS_CONTROL_BASE_6_LOW                       0c1e60
#define PCI_0ACCESS_CONTROL_BASE_6_HIGH                      0x1e64
#define PCI_0ACCESS_CONTROL_TOP_6                            0x1e68
#define PCI_0ACCESS_CONTROL_BASE_7_LOW                       0c1e70
#define PCI_0ACCESS_CONTROL_BASE_7_HIGH                      0x1e74
#define PCI_0ACCESS_CONTROL_TOP_7                            0x1e78
#define PCI_1CROSS_BAR_CONTROL_LOW                           0x1d88
#define PCI_1CROSS_BAR_CONTROL_HIGH                          0x1d8c
#define PCI_1CROSS_BAR_TIMEOUT                               0x1d84
#define PCI_1READ_RESPONSE_CROSS_BAR_CONTROL_LOW             0x1d98
#define PCI_1READ_RESPONSE_CROSS_BAR_CONTROL_HIGH            0x1d9c
#define PCI_1SYNC_BARRIER_VIRTUAL_REGISTER                   0x1d90
#define PCI_1P2P_CONFIGURATION                               0x1d94
#define PCI_1ACCESS_CONTROL_BASE_0_LOW                       0x1e80
#define PCI_1ACCESS_CONTROL_BASE_0_HIGH                      0x1e84
#define PCI_1ACCESS_CONTROL_TOP_0                            0x1e88
#define PCI_1ACCESS_CONTROL_BASE_1_LOW                       0c1e90
#define PCI_1ACCESS_CONTROL_BASE_1_HIGH                      0x1e94
#define PCI_1ACCESS_CONTROL_TOP_1                            0x1e98
#define PCI_1ACCESS_CONTROL_BASE_2_LOW                       0c1ea0
#define PCI_1ACCESS_CONTROL_BASE_2_HIGH                      0x1ea4
#define PCI_1ACCESS_CONTROL_TOP_2                            0x1ea8
#define PCI_1ACCESS_CONTROL_BASE_3_LOW                       0c1eb0
#define PCI_1ACCESS_CONTROL_BASE_3_HIGH                      0x1eb4
#define PCI_1ACCESS_CONTROL_TOP_3                            0x1eb8
#define PCI_1ACCESS_CONTROL_BASE_4_LOW                       0c1ec0
#define PCI_1ACCESS_CONTROL_BASE_4_HIGH                      0x1ec4
#define PCI_1ACCESS_CONTROL_TOP_4                            0x1ec8
#define PCI_1ACCESS_CONTROL_BASE_5_LOW                       0c1ed0
#define PCI_1ACCESS_CONTROL_BASE_5_HIGH                      0x1ed4
#define PCI_1ACCESS_CONTROL_TOP_5                            0x1ed8
#define PCI_1ACCESS_CONTROL_BASE_6_LOW                       0c1ee0
#define PCI_1ACCESS_CONTROL_BASE_6_HIGH                      0x1ee4
#define PCI_1ACCESS_CONTROL_TOP_6                            0x1ee8
#define PCI_1ACCESS_CONTROL_BASE_7_LOW                       0c1ef0
#define PCI_1ACCESS_CONTROL_BASE_7_HIGH                      0x1ef4
#define PCI_1ACCESS_CONTROL_TOP_7                            0x1ef8

/****************************************/
/* PCI Snoop Control                    */
/****************************************/

#define PCI_0SNOOP_CONTROL_BASE_0_LOW                        0x1f00
#define PCI_0SNOOP_CONTROL_BASE_0_HIGH                       0x1f04
#define PCI_0SNOOP_CONTROL_TOP_0                             0x1f08
#define PCI_0SNOOP_CONTROL_BASE_1_0_LOW                      0x1f10
#define PCI_0SNOOP_CONTROL_BASE_1_0_HIGH                     0x1f14
#define PCI_0SNOOP_CONTROL_TOP_1                             0x1f18
#define PCI_0SNOOP_CONTROL_BASE_2_0_LOW                      0x1f20
#define PCI_0SNOOP_CONTROL_BASE_2_0_HIGH                     0x1f24
#define PCI_0SNOOP_CONTROL_TOP_2                             0x1f28
#define PCI_0SNOOP_CONTROL_BASE_3_0_LOW                      0x1f30
#define PCI_0SNOOP_CONTROL_BASE_3_0_HIGH                     0x1f34
#define PCI_0SNOOP_CONTROL_TOP_3                             0x1f38
#define PCI_1SNOOP_CONTROL_BASE_0_LOW                        0x1f80
#define PCI_1SNOOP_CONTROL_BASE_0_HIGH                       0x1f84
#define PCI_1SNOOP_CONTROL_TOP_0                             0x1f88
#define PCI_1SNOOP_CONTROL_BASE_1_0_LOW                      0x1f90
#define PCI_1SNOOP_CONTROL_BASE_1_0_HIGH                     0x1f94
#define PCI_1SNOOP_CONTROL_TOP_1                             0x1f98
#define PCI_1SNOOP_CONTROL_BASE_2_0_LOW                      0x1fa0
#define PCI_1SNOOP_CONTROL_BASE_2_0_HIGH                     0x1fa4
#define PCI_1SNOOP_CONTROL_TOP_2                             0x1fa8
#define PCI_1SNOOP_CONTROL_BASE_3_0_LOW                      0x1fb0
#define PCI_1SNOOP_CONTROL_BASE_3_0_HIGH                     0x1fb4
#define PCI_1SNOOP_CONTROL_TOP_3                             0x1fb8

/****************************************/
/* PCI Configuration Address            */
/****************************************/

#define PCI_0CONFIGURATION_ADDRESS 				0xcf8
#define PCI_0CONFIGURATION_DATA_VIRTUAL_REGISTER           	0xcfc
#define PCI_1CONFIGURATION_ADDRESS 				0xc78
#define PCI_1CONFIGURATION_DATA_VIRTUAL_REGISTER           	0xc7c
#define PCI_0INTERRUPT_ACKNOWLEDGE_VIRTUAL_REGISTER		0xc34
#define PCI_1INTERRUPT_ACKNOWLEDGE_VIRTUAL_REGISTER		0xcb4

/****************************************/
/* PCI Error Report                     */
/****************************************/

#define PCI_0SERR_MASK						 0xc28
#define PCI_0ERROR_ADDRESS_LOW                               0x1d40
#define PCI_0ERROR_ADDRESS_HIGH                              0x1d44
#define PCI_0ERROR_DATA_LOW                                  0x1d48
#define PCI_0ERROR_DATA_HIGH                                 0x1d4c
#define PCI_0ERROR_COMMAND                                   0x1d50
#define PCI_0ERROR_CAUSE                                     0x1d58
#define PCI_0ERROR_MASK                                      0x1d5c

#define PCI_1SERR_MASK						 0xca8
#define PCI_1ERROR_ADDRESS_LOW                               0x1dc0
#define PCI_1ERROR_ADDRESS_HIGH                              0x1dc4
#define PCI_1ERROR_DATA_LOW                                  0x1dc8
#define PCI_1ERROR_DATA_HIGH                                 0x1dcc
#define PCI_1ERROR_COMMAND                                   0x1dd0
#define PCI_1ERROR_CAUSE                                     0x1dd8
#define PCI_1ERROR_MASK                                      0x1ddc


/****************************************/
/* Lslave Debug  (for internal use)     */
/****************************************/

#define L_SLAVE_X0_ADDRESS                                  0x1d20
#define L_SLAVE_X0_COMMAND_AND_ID                           0x1d24
#define L_SLAVE_X1_ADDRESS                                  0x1d28
#define L_SLAVE_X1_COMMAND_AND_ID                           0x1d2c
#define L_SLAVE_WRITE_DATA_LOW                              0x1d30
#define L_SLAVE_WRITE_DATA_HIGH                             0x1d34
#define L_SLAVE_WRITE_BYTE_ENABLE                           0x1d60
#define L_SLAVE_READ_DATA_LOW                               0x1d38
#define L_SLAVE_READ_DATA_HIGH                              0x1d3c
#define L_SLAVE_READ_ID                                     0x1d64

/****************************************/
/* PCI Configuration Function 0         */
/****************************************/

#define PCI_DEVICE_AND_VENDOR_ID 				0x000
#define PCI_STATUS_AND_COMMAND					0x004
#define PCI_CLASS_CODE_AND_REVISION_ID			        0x008
#define PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE 		0x00C
#define PCI_SCS_0_BASE_ADDRESS	    				0x010
#define PCI_SCS_1_BASE_ADDRESS 					0x014
#define PCI_SCS_2_BASE_ADDRESS 					0x018
#define PCI_SCS_3_BASE_ADDRESS      				0x01C
#define PCI_INTERNAL_REGISTERS_MEMORY_MAPPED_BASE_ADDRESS	0x020
#define PCI_INTERNAL_REGISTERS_I_OMAPPED_BASE_ADDRESS		0x024
#define PCI_SUBSYSTEM_ID_AND_SUBSYSTEM_VENDOR_ID		0x02C
#define PCI_EXPANSION_ROM_BASE_ADDRESS_REGISTER			0x030
#define PCI_CAPABILTY_LIST_POINTER                          0x034
#define PCI_INTERRUPT_PIN_AND_LINE 			    0x03C
#define PCI_POWER_MANAGEMENT_CAPABILITY                     0x040
#define PCI_POWER_MANAGEMENT_STATUS_AND_CONTROL             0x044
#define PCI_VPD_ADDRESS                                     0x048
#define PCI_VPD_DATA                                        0X04c
#define PCI_MSI_MESSAGE_CONTROL                             0x050
#define PCI_MSI_MESSAGE_ADDRESS                             0x054
#define PCI_MSI_MESSAGE_UPPER_ADDRESS                       0x058
#define PCI_MSI_MESSAGE_DATA                                0x05c
#define PCI_COMPACT_PCI_HOT_SWAP_CAPABILITY                 0x058

/****************************************/
/* PCI Configuration Function 1         */
/****************************************/

#define PCI_CS_0_BASE_ADDRESS	    				0x110
#define PCI_CS_1_BASE_ADDRESS 					0x114
#define PCI_CS_2_BASE_ADDRESS 					0x118
#define PCI_CS_3_BASE_ADDRESS     				0x11c
#define PCI_BOOTCS_BASE_ADDRESS                     	    0x120

/****************************************/
/* PCI Configuration Function 2         */
/****************************************/

#define PCI_P2P_MEM0_BASE_ADDRESS	    			0x210
#define PCI_P2P_MEM1_BASE_ADDRESS 				0x214
#define PCI_P2P_I_O_BASE_ADDRESS 				0x218
#define PCI_CPU_BASE_ADDRESS      				0x21c

/****************************************/
/* PCI Configuration Function 4         */
/****************************************/

#define PCI_DAC_SCS_0_BASE_ADDRESS_LOW 				0x410
#define PCI_DAC_SCS_0_BASE_ADDRESS_HIGH			 	0x414
#define PCI_DAC_SCS_1_BASE_ADDRESS_LOW   			0x418
#define PCI_DAC_SCS_1_BASE_ADDRESS_HIGH  		    0x41c
#define PCI_DAC_P2P_MEM0_BASE_ADDRESS_LOW              	    0x420
#define PCI_DAC_P2P_MEM0_BASE_ADDRESS_HIGH             	    0x424


/****************************************/
/* PCI Configuration Function 5         */
/****************************************/

#define PCI_DAC_SCS_2_BASE_ADDRESS_LOW 				0x510
#define PCI_DAC_SCS_2_BASE_ADDRESS_HIGH				0x514
#define PCI_DAC_SCS_3_BASE_ADDRESS_LOW   		 	0x518
#define PCI_DAC_SCS_3_BASE_ADDRESS_HIGH  		 	0x51c
#define PCI_DAC_P2P_MEM1_BASE_ADDRESS_LOW              	    0x520
#define PCI_DAC_P2P_MEM1_BASE_ADDRESS_HIGH             	    0x524


/****************************************/
/* PCI Configuration Function 6         */
/****************************************/

#define PCI_DAC_CS_0_BASE_ADDRESS_LOW 				0x610
#define PCI_DAC_CS_0_BASE_ADDRESS_HIGH				0x614
#define PCI_DAC_CS_1_BASE_ADDRESS_LOW   			0x618
#define PCI_DAC_CS_1_BASE_ADDRESS_HIGH  			0x61c
#define PCI_DAC_CS_2_BASE_ADDRESS_LOW            	        0x620
#define PCI_DAC_CS_2_BASE_ADDRESS_HIGH           	        0x624

/****************************************/
/* PCI Configuration Function 7         */
/****************************************/

#define PCI_DAC_CS_3_BASE_ADDRESS_LOW 				0x710
#define PCI_DAC_CS_3_BASE_ADDRESS_HIGH			 	0x714
#define PCI_DAC_BOOTCS_BASE_ADDRESS_LOW   		 	0x718
#define PCI_DAC_BOOTCS_BASE_ADDRESS_HIGH  			0x71c
#define PCI_DAC_CPU_BASE_ADDRESS_LOW            	        0x720
#define PCI_DAC_CPU_BASE_ADDRESS_HIGH           	        0x724

/****************************************/
/* Interrupts	  			*/
/****************************************/

#define LOW_INTERRUPT_CAUSE_REGISTER   				0xc18
#define HIGH_INTERRUPT_CAUSE_REGISTER				0xc68
#define CPU_INTERRUPT_MASK_REGISTER_LOW				0xc1c
#define CPU_INTERRUPT_MASK_REGISTER_HIGH			0xc6c
#define CPU_SELECT_CAUSE_REGISTER				0xc70
#define PCI_0INTERRUPT_CAUSE_MASK_REGISTER_LOW			0xc24
#define PCI_0INTERRUPT_CAUSE_MASK_REGISTER_HIGH			0xc64
#define PCI_0SELECT_CAUSE                                   0xc74
#define PCI_1INTERRUPT_CAUSE_MASK_REGISTER_LOW			0xca4
#define PCI_1INTERRUPT_CAUSE_MASK_REGISTER_HIGH			0xce4
#define PCI_1SELECT_CAUSE                                   0xcf4
#define CPU_INT_0_MASK                                      0xe60
#define CPU_INT_1_MASK                                      0xe64
#define CPU_INT_2_MASK                                      0xe68
#define CPU_INT_3_MASK                                      0xe6c

/****************************************/
/* I20 Support registers		*/
/****************************************/

#define INBOUND_MESSAGE_REGISTER0_PCI0_SIDE			0x010
#define INBOUND_MESSAGE_REGISTER1_PCI0_SIDE  			0x014
#define OUTBOUND_MESSAGE_REGISTER0_PCI0_SIDE 			0x018
#define OUTBOUND_MESSAGE_REGISTER1_PCI0_SIDE  			0x01C
#define INBOUND_DOORBELL_REGISTER_PCI0_SIDE  			0x020
#define INBOUND_INTERRUPT_CAUSE_REGISTER_PCI0_SIDE  		0x024
#define INBOUND_INTERRUPT_MASK_REGISTER_PCI0_SIDE		0x028
#define OUTBOUND_DOORBELL_REGISTER_PCI0_SIDE 			0x02C
#define OUTBOUND_INTERRUPT_CAUSE_REGISTER_PCI0_SIDE   		0x030
#define OUTBOUND_INTERRUPT_MASK_REGISTER_PCI0_SIDE   		0x034
#define INBOUND_QUEUE_PORT_VIRTUAL_REGISTER_PCI0_SIDE  		0x040
#define OUTBOUND_QUEUE_PORT_VIRTUAL_REGISTER_PCI0_SIDE   	0x044
#define QUEUE_CONTROL_REGISTER_PCI0_SIDE 			0x050
#define QUEUE_BASE_ADDRESS_REGISTER_PCI0_SIDE 			0x054
#define INBOUND_FREE_HEAD_POINTER_REGISTER_PCI0_SIDE		0x060
#define INBOUND_FREE_TAIL_POINTER_REGISTER_PCI0_SIDE  		0x064
#define INBOUND_POST_HEAD_POINTER_REGISTER_PCI0_SIDE 		0x068
#define INBOUND_POST_TAIL_POINTER_REGISTER_PCI0_SIDE 		0x06C
#define OUTBOUND_FREE_HEAD_POINTER_REGISTER_PCI0_SIDE		0x070
#define OUTBOUND_FREE_TAIL_POINTER_REGISTER_PCI0_SIDE		0x074
#define OUTBOUND_POST_HEAD_POINTER_REGISTER_PCI0_SIDE		0x0F8
#define OUTBOUND_POST_TAIL_POINTER_REGISTER_PCI0_SIDE		0x0FC

#define INBOUND_MESSAGE_REGISTER0_PCI1_SIDE				0x090
#define INBOUND_MESSAGE_REGISTER1_PCI1_SIDE  				0x094
#define OUTBOUND_MESSAGE_REGISTER0_PCI1_SIDE 				0x098
#define OUTBOUND_MESSAGE_REGISTER1_PCI1_SIDE  				0x09C
#define INBOUND_DOORBELL_REGISTER_PCI1_SIDE  				0x0A0
#define INBOUND_INTERRUPT_CAUSE_REGISTER_PCI1_SIDE  		0x0A4
#define INBOUND_INTERRUPT_MASK_REGISTER_PCI1_SIDE			0x0A8
#define OUTBOUND_DOORBELL_REGISTER_PCI1_SIDE 				0x0AC
#define OUTBOUND_INTERRUPT_CAUSE_REGISTER_PCI1_SIDE   		0x0B0
#define OUTBOUND_INTERRUPT_MASK_REGISTER_PCI1_SIDE   		0x0B4
#define INBOUND_QUEUE_PORT_VIRTUAL_REGISTER_PCI1_SIDE  		0x0C0
#define OUTBOUND_QUEUE_PORT_VIRTUAL_REGISTER_PCI1_SIDE   	0x0C4
#define QUEUE_CONTROL_REGISTER_PCI1_SIDE 				0x0D0
#define QUEUE_BASE_ADDRESS_REGISTER_PCI1_SIDE 				0x0D4
#define INBOUND_FREE_HEAD_POINTER_REGISTER_PCI1_SIDE		0x0E0
#define INBOUND_FREE_TAIL_POINTER_REGISTER_PCI1_SIDE  		0x0E4
#define INBOUND_POST_HEAD_POINTER_REGISTER_PCI1_SIDE 		0x0E8
#define INBOUND_POST_TAIL_POINTER_REGISTER_PCI1_SIDE 		0x0EC
#define OUTBOUND_FREE_HEAD_POINTER_REGISTER_PCI1_SIDE		0x0F0
#define OUTBOUND_FREE_TAIL_POINTER_REGISTER_PCI1_SIDE		0x0F4
#define OUTBOUND_POST_HEAD_POINTER_REGISTER_PCI1_SIDE		0x078
#define OUTBOUND_POST_TAIL_POINTER_REGISTER_PCI1_SIDE		0x07C

#define INBOUND_MESSAGE_REGISTER0_CPU0_SIDE				0X1C10
#define INBOUND_MESSAGE_REGISTER1_CPU0_SIDE  				0X1C14
#define OUTBOUND_MESSAGE_REGISTER0_CPU0_SIDE 				0X1C18
#define OUTBOUND_MESSAGE_REGISTER1_CPU0_SIDE  				0X1C1C
#define INBOUND_DOORBELL_REGISTER_CPU0_SIDE  				0X1C20
#define INBOUND_INTERRUPT_CAUSE_REGISTER_CPU0_SIDE  		0X1C24
#define INBOUND_INTERRUPT_MASK_REGISTER_CPU0_SIDE			0X1C28
#define OUTBOUND_DOORBELL_REGISTER_CPU0_SIDE 				0X1C2C
#define OUTBOUND_INTERRUPT_CAUSE_REGISTER_CPU0_SIDE   		0X1C30
#define OUTBOUND_INTERRUPT_MASK_REGISTER_CPU0_SIDE   		0X1C34
#define INBOUND_QUEUE_PORT_VIRTUAL_REGISTER_CPU0_SIDE  		0X1C40
#define OUTBOUND_QUEUE_PORT_VIRTUAL_REGISTER_CPU0_SIDE   	0X1C44
#define QUEUE_CONTROL_REGISTER_CPU0_SIDE 				0X1C50
#define QUEUE_BASE_ADDRESS_REGISTER_CPU0_SIDE 				0X1C54
#define INBOUND_FREE_HEAD_POINTER_REGISTER_CPU0_SIDE		0X1C60
#define INBOUND_FREE_TAIL_POINTER_REGISTER_CPU0_SIDE  		0X1C64
#define INBOUND_POST_HEAD_POINTER_REGISTER_CPU0_SIDE 		0X1C68
#define INBOUND_POST_TAIL_POINTER_REGISTER_CPU0_SIDE 		0X1C6C
#define OUTBOUND_FREE_HEAD_POINTER_REGISTER_CPU0_SIDE		0X1C70
#define OUTBOUND_FREE_TAIL_POINTER_REGISTER_CPU0_SIDE		0X1C74
#define OUTBOUND_POST_HEAD_POINTER_REGISTER_CPU0_SIDE		0X1CF8
#define OUTBOUND_POST_TAIL_POINTER_REGISTER_CPU0_SIDE		0X1CFC

#define INBOUND_MESSAGE_REGISTER0_CPU1_SIDE				0X1C90
#define INBOUND_MESSAGE_REGISTER1_CPU1_SIDE  				0X1C94
#define OUTBOUND_MESSAGE_REGISTER0_CPU1_SIDE 				0X1C98
#define OUTBOUND_MESSAGE_REGISTER1_CPU1_SIDE  				0X1C9C
#define INBOUND_DOORBELL_REGISTER_CPU1_SIDE  				0X1CA0
#define INBOUND_INTERRUPT_CAUSE_REGISTER_CPU1_SIDE  		0X1CA4
#define INBOUND_INTERRUPT_MASK_REGISTER_CPU1_SIDE			0X1CA8
#define OUTBOUND_DOORBELL_REGISTER_CPU1_SIDE 				0X1CAC
#define OUTBOUND_INTERRUPT_CAUSE_REGISTER_CPU1_SIDE   		0X1CB0
#define OUTBOUND_INTERRUPT_MASK_REGISTER_CPU1_SIDE   		0X1CB4
#define INBOUND_QUEUE_PORT_VIRTUAL_REGISTER_CPU1_SIDE  		0X1CC0
#define OUTBOUND_QUEUE_PORT_VIRTUAL_REGISTER_CPU1_SIDE   	0X1CC4
#define QUEUE_CONTROL_REGISTER_CPU1_SIDE 				0X1CD0
#define QUEUE_BASE_ADDRESS_REGISTER_CPU1_SIDE 				0X1CD4
#define INBOUND_FREE_HEAD_POINTER_REGISTER_CPU1_SIDE		0X1CE0
#define INBOUND_FREE_TAIL_POINTER_REGISTER_CPU1_SIDE  		0X1CE4
#define INBOUND_POST_HEAD_POINTER_REGISTER_CPU1_SIDE 		0X1CE8
#define INBOUND_POST_TAIL_POINTER_REGISTER_CPU1_SIDE 		0X1CEC
#define OUTBOUND_FREE_HEAD_POINTER_REGISTER_CPU1_SIDE		0X1CF0
#define OUTBOUND_FREE_TAIL_POINTER_REGISTER_CPU1_SIDE		0X1CF4
#define OUTBOUND_POST_HEAD_POINTER_REGISTER_CPU1_SIDE		0X1C78
#define OUTBOUND_POST_TAIL_POINTER_REGISTER_CPU1_SIDE		0X1C7C

/****************************************/
/* Communication Unit Registers         */
/****************************************/

#define ETHERNET_0_ADDRESS_CONTROL_LOW
#define ETHERNET_0_ADDRESS_CONTROL_HIGH                     0xf204
#define ETHERNET_0_RECEIVE_BUFFER_PCI_HIGH_ADDRESS          0xf208
#define ETHERNET_0_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS         0xf20c
#define ETHERNET_0_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS      0xf210
#define ETHERNET_0_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS     0xf214
#define ETHERNET_0_HASH_TABLE_PCI_HIGH_ADDRESS              0xf218
#define ETHERNET_1_ADDRESS_CONTROL_LOW                      0xf220
#define ETHERNET_1_ADDRESS_CONTROL_HIGH                     0xf224
#define ETHERNET_1_RECEIVE_BUFFER_PCI_HIGH_ADDRESS          0xf228
#define ETHERNET_1_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS         0xf22c
#define ETHERNET_1_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS      0xf230
#define ETHERNET_1_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS     0xf234
#define ETHERNET_1_HASH_TABLE_PCI_HIGH_ADDRESS              0xf238
#define ETHERNET_2_ADDRESS_CONTROL_LOW                      0xf240
#define ETHERNET_2_ADDRESS_CONTROL_HIGH                     0xf244
#define ETHERNET_2_RECEIVE_BUFFER_PCI_HIGH_ADDRESS          0xf248
#define ETHERNET_2_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS         0xf24c
#define ETHERNET_2_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS      0xf250
#define ETHERNET_2_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS     0xf254
#define ETHERNET_2_HASH_TABLE_PCI_HIGH_ADDRESS              0xf258
#define MPSC_0_ADDRESS_CONTROL_LOW                          0xf280
#define MPSC_0_ADDRESS_CONTROL_HIGH                         0xf284
#define MPSC_0_RECEIVE_BUFFER_PCI_HIGH_ADDRESS              0xf288
#define MPSC_0_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS             0xf28c
#define MPSC_0_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS          0xf290
#define MPSC_0_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS         0xf294
#define MPSC_1_ADDRESS_CONTROL_LOW                          0xf2a0
#define MPSC_1_ADDRESS_CONTROL_HIGH                         0xf2a4
#define MPSC_1_RECEIVE_BUFFER_PCI_HIGH_ADDRESS              0xf2a8
#define MPSC_1_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS             0xf2ac
#define MPSC_1_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS          0xf2b0
#define MPSC_1_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS         0xf2b4
#define MPSC_2_ADDRESS_CONTROL_LOW                          0xf2c0
#define MPSC_2_ADDRESS_CONTROL_HIGH                         0xf2c4
#define MPSC_2_RECEIVE_BUFFER_PCI_HIGH_ADDRESS              0xf2c8
#define MPSC_2_TRANSMIT_BUFFER_PCI_HIGH_ADDRESS             0xf2cc
#define MPSC_2_RECEIVE_DESCRIPTOR_PCI_HIGH_ADDRESS          0xf2d0
#define MPSC_2_TRANSMIT_DESCRIPTOR_PCI_HIGH_ADDRESS         0xf2d4
#define SERIAL_INIT_PCI_HIGH_ADDRESS                        0xf320
#define SERIAL_INIT_LAST_DATA                               0xf324
#define SERIAL_INIT_STATUS_AND_CONTROL                      0xf328
#define COMM_UNIT_ARBITER_CONTROL                           0xf300
#define COMM_UNIT_CROSS_BAR_TIMEOUT                         0xf304
#define COMM_UNIT_INTERRUPT_CAUSE                           0xf310
#define COMM_UNIT_INTERRUPT_MASK                            0xf314
#define COMM_UNIT_ERROR_ADDRESS                             0xf314

/****************************************/
/* Cunit Debug  (for internal use)     */
/****************************************/

#define CUNIT_ADDRESS                                       0xf340
#define CUNIT_COMMAND_AND_ID                                0xf344
#define CUNIT_WRITE_DATA_LOW                                0xf348
#define CUNIT_WRITE_DATA_HIGH                               0xf34c
#define CUNIT_WRITE_BYTE_ENABLE                             0xf358
#define CUNIT_READ_DATA_LOW                                 0xf350
#define CUNIT_READ_DATA_HIGH                                0xf354
#define CUNIT_READ_ID                                       0xf35c

/****************************************/
/* Fast Ethernet Unit Registers         */
/****************************************/

/* Ethernet */

#define ETHERNET_PHY_ADDRESS_REGISTER                       0x2000
#define ETHERNET_SMI_REGISTER                               0x2010

/* Ethernet 0 */

#define ETHERNET0_PORT_CONFIGURATION_REGISTER               0x2400
#define ETHERNET0_PORT_CONFIGURATION_EXTEND_REGISTER        0x2408
#define ETHERNET0_PORT_COMMAND_REGISTER                     0x2410
#define ETHERNET0_PORT_STATUS_REGISTER                      0x2418
#define ETHERNET0_SERIAL_PARAMETRS_REGISTER                 0x2420
#define ETHERNET0_HASH_TABLE_POINTER_REGISTER               0x2428
#define ETHERNET0_FLOW_CONTROL_SOURCE_ADDRESS_LOW           0x2430
#define ETHERNET0_FLOW_CONTROL_SOURCE_ADDRESS_HIGH          0x2438
#define ETHERNET0_SDMA_CONFIGURATION_REGISTER               0x2440
#define ETHERNET0_SDMA_COMMAND_REGISTER                     0x2448
#define ETHERNET0_INTERRUPT_CAUSE_REGISTER                  0x2450
#define ETHERNET0_INTERRUPT_MASK_REGISTER                   0x2458
#define ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER0              0x2480
#define ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER1              0x2484
#define ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER2              0x2488
#define ETHERNET0_FIRST_RX_DESCRIPTOR_POINTER3              0x248c
#define ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER0            0x24a0
#define ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER1            0x24a4
#define ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER2            0x24a8
#define ETHERNET0_CURRENT_RX_DESCRIPTOR_POINTER3            0x24ac
#define ETHERNET0_CURRENT_TX_DESCRIPTOR_POINTER0            0x24e0
#define ETHERNET0_CURRENT_TX_DESCRIPTOR_POINTER1            0x24e4
#define ETHERNET0_MIB_COUNTER_BASE                          0x2500

/* Ethernet 1 */

#define ETHERNET1_PORT_CONFIGURATION_REGISTER               0x2800
#define ETHERNET1_PORT_CONFIGURATION_EXTEND_REGISTER        0x2808
#define ETHERNET1_PORT_COMMAND_REGISTER                     0x2810
#define ETHERNET1_PORT_STATUS_REGISTER                      0x2818
#define ETHERNET1_SERIAL_PARAMETRS_REGISTER                 0x2820
#define ETHERNET1_HASH_TABLE_POINTER_REGISTER               0x2828
#define ETHERNET1_FLOW_CONTROL_SOURCE_ADDRESS_LOW           0x2830
#define ETHERNET1_FLOW_CONTROL_SOURCE_ADDRESS_HIGH          0x2838
#define ETHERNET1_SDMA_CONFIGURATION_REGISTER               0x2840
#define ETHERNET1_SDMA_COMMAND_REGISTER                     0x2848
#define ETHERNET1_INTERRUPT_CAUSE_REGISTER                  0x2850
#define ETHERNET1_INTERRUPT_MASK_REGISTER                   0x2858
#define ETHERNET1_FIRST_RX_DESCRIPTOR_POINTER0              0x2880
#define ETHERNET1_FIRST_RX_DESCRIPTOR_POINTER1              0x2884
#define ETHERNET1_FIRST_RX_DESCRIPTOR_POINTER2              0x2888
#define ETHERNET1_FIRST_RX_DESCRIPTOR_POINTER3              0x288c
#define ETHERNET1_CURRENT_RX_DESCRIPTOR_POINTER0            0x28a0
#define ETHERNET1_CURRENT_RX_DESCRIPTOR_POINTER1            0x28a4
#define ETHERNET1_CURRENT_RX_DESCRIPTOR_POINTER2            0x28a8
#define ETHERNET1_CURRENT_RX_DESCRIPTOR_POINTER3            0x28ac
#define ETHERNET1_CURRENT_TX_DESCRIPTOR_POINTER0            0x28e0
#define ETHERNET1_CURRENT_TX_DESCRIPTOR_POINTER1            0x28e4
#define ETHERNET1_MIB_COUNTER_BASE                          0x2900

/* Ethernet 2 */

#define ETHERNET2_PORT_CONFIGURATION_REGISTER               0x2c00
#define ETHERNET2_PORT_CONFIGURATION_EXTEND_REGISTER        0x2c08
#define ETHERNET2_PORT_COMMAND_REGISTER                     0x2c10
#define ETHERNET2_PORT_STATUS_REGISTER                      0x2c18
#define ETHERNET2_SERIAL_PARAMETRS_REGISTER                 0x2c20
#define ETHERNET2_HASH_TABLE_POINTER_REGISTER               0x2c28
#define ETHERNET2_FLOW_CONTROL_SOURCE_ADDRESS_LOW           0x2c30
#define ETHERNET2_FLOW_CONTROL_SOURCE_ADDRESS_HIGH          0x2c38
#define ETHERNET2_SDMA_CONFIGURATION_REGISTER               0x2c40
#define ETHERNET2_SDMA_COMMAND_REGISTER                     0x2c48
#define ETHERNET2_INTERRUPT_CAUSE_REGISTER                  0x2c50
#define ETHERNET2_INTERRUPT_MASK_REGISTER                   0x2c58
#define ETHERNET2_FIRST_RX_DESCRIPTOR_POINTER0              0x2c80
#define ETHERNET2_FIRST_RX_DESCRIPTOR_POINTER1              0x2c84
#define ETHERNET2_FIRST_RX_DESCRIPTOR_POINTER2              0x2c88
#define ETHERNET2_FIRST_RX_DESCRIPTOR_POINTER3              0x2c8c
#define ETHERNET2_CURRENT_RX_DESCRIPTOR_POINTER0            0x2ca0
#define ETHERNET2_CURRENT_RX_DESCRIPTOR_POINTER1            0x2ca4
#define ETHERNET2_CURRENT_RX_DESCRIPTOR_POINTER2            0x2ca8
#define ETHERNET2_CURRENT_RX_DESCRIPTOR_POINTER3            0x2cac
#define ETHERNET2_CURRENT_TX_DESCRIPTOR_POINTER0            0x2ce0
#define ETHERNET2_CURRENT_TX_DESCRIPTOR_POINTER1            0x2ce4
#define ETHERNET2_MIB_COUNTER_BASE                          0x2d00

/****************************************/
/* SDMA Registers                       */
/****************************************/

#define SDMA_GROUP_CONFIGURATION_REGISTER                   0xb1f0
#define CHANNEL0_CONFIGURATION_REGISTER                     0x4000
#define CHANNEL0_COMMAND_REGISTER                           0x4008
#define CHANNEL0_RX_CMD_STATUS                              0x4800
#define CHANNEL0_RX_PACKET_AND_BUFFER_SIZES                 0x4804
#define CHANNEL0_RX_BUFFER_POINTER                          0x4808
#define CHANNEL0_RX_NEXT_POINTER                            0x480c
#define CHANNEL0_CURRENT_RX_DESCRIPTOR_POINTER              0x4810
#define CHANNEL0_TX_CMD_STATUS                              0x4C00
#define CHANNEL0_TX_PACKET_SIZE                             0x4C04
#define CHANNEL0_TX_BUFFER_POINTER                          0x4C08
#define CHANNEL0_TX_NEXT_POINTER                            0x4C0c
#define CHANNEL0_CURRENT_TX_DESCRIPTOR_POINTER              0x4c10
#define CHANNEL0_FIRST_TX_DESCRIPTOR_POINTER                0x4c14
#define CHANNEL1_CONFIGURATION_REGISTER                     0x6000
#define CHANNEL1_COMMAND_REGISTER                           0x6008
#define CHANNEL1_RX_CMD_STATUS                              0x6800
#define CHANNEL1_RX_PACKET_AND_BUFFER_SIZES                 0x6804
#define CHANNEL1_RX_BUFFER_POINTER                          0x6808
#define CHANNEL1_RX_NEXT_POINTER                            0x680c
#define CHANNEL1_CURRENT_RX_DESCRIPTOR_POINTER              0x6810
#define CHANNEL1_TX_CMD_STATUS                              0x6C00
#define CHANNEL1_TX_PACKET_SIZE                             0x6C04
#define CHANNEL1_TX_BUFFER_POINTER                          0x6C08
#define CHANNEL1_TX_NEXT_POINTER                            0x6C0c
#define CHANNEL1_CURRENT_RX_DESCRIPTOR_POINTER              0x6810
#define CHANNEL1_CURRENT_TX_DESCRIPTOR_POINTER              0x6c10
#define CHANNEL1_FIRST_TX_DESCRIPTOR_POINTER                0x6c14

/* SDMA Interrupt */

#define SDMA_CAUSE                                          0xb820
#define SDMA_MASK                                           0xb8a0


/****************************************/
/* Baude Rate Generators Registers      */
/****************************************/

/* BRG 0 */

#define BRG0_CONFIGURATION_REGISTER                         0xb200
#define BRG0_BAUDE_TUNING_REGISTER                          0xb204

/* BRG 1 */

#define BRG1_CONFIGURATION_REGISTER                         0xb208
#define BRG1_BAUDE_TUNING_REGISTER                          0xb20c

/* BRG 2 */

#define BRG2_CONFIGURATION_REGISTER                         0xb210
#define BRG2_BAUDE_TUNING_REGISTER                          0xb214

/* BRG Interrupts */

#define BRG_CAUSE_REGISTER                                  0xb834
#define BRG_MASK_REGISTER                                   0xb8b4

/* MISC */

#define MAIN_ROUTING_REGISTER                               0xb400
#define RECEIVE_CLOCK_ROUTING_REGISTER                      0xb404
#define TRANSMIT_CLOCK_ROUTING_REGISTER                     0xb408
#define COMM_UNIT_ARBITER_CONFIGURATION_REGISTER            0xb40c
#define WATCHDOG_CONFIGURATION_REGISTER                     0xb410
#define WATCHDOG_VALUE_REGISTER                             0xb414


/****************************************/
/* Flex TDM Registers                   */
/****************************************/

/* FTDM Port */

#define FLEXTDM_TRANSMIT_READ_POINTER                       0xa800
#define FLEXTDM_RECEIVE_READ_POINTER                        0xa804
#define FLEXTDM_CONFIGURATION_REGISTER                      0xa808
#define FLEXTDM_AUX_CHANNELA_TX_REGISTER                    0xa80c
#define FLEXTDM_AUX_CHANNELA_RX_REGISTER                    0xa810
#define FLEXTDM_AUX_CHANNELB_TX_REGISTER                    0xa814
#define FLEXTDM_AUX_CHANNELB_RX_REGISTER                    0xa818

/* FTDM Interrupts */

#define FTDM_CAUSE_REGISTER                                 0xb830
#define FTDM_MASK_REGISTER                                  0xb8b0


/****************************************/
/* GPP Interface Registers              */
/****************************************/

#define GPP_IO_CONTROL                                      0xf100
#define GPP_LEVEL_CONTROL                                   0xf110
#define GPP_VALUE                                           0xf104
#define GPP_INTERRUPT_CAUSE                                 0xf108
#define GPP_INTERRUPT_MASK                                  0xf10c

#define MPP_CONTROL0                                        0xf000
#define MPP_CONTROL1                                        0xf004
#define MPP_CONTROL2                                        0xf008
#define MPP_CONTROL3                                        0xf00c
#define DEBUG_PORT_MULTIPLEX                                0xf014
#define SERIAL_PORT_MULTIPLEX                               0xf010

/****************************************/
/* I2C Registers                        */
/****************************************/

#define I2C_SLAVE_ADDRESS                                   0xc000
#define I2C_EXTENDED_SLAVE_ADDRESS                          0xc040
#define I2C_DATA                                            0xc004
#define I2C_CONTROL                                         0xc008
#define I2C_STATUS_BAUDE_RATE                               0xc00C
#define I2C_SOFT_RESET                                      0xc01c

/****************************************/
/* MPSC Registers                       */
/****************************************/

/* MPSC0  */

#define MPSC0_MAIN_CONFIGURATION_LOW                        0x8000
#define MPSC0_MAIN_CONFIGURATION_HIGH                       0x8004
#define MPSC0_PROTOCOL_CONFIGURATION                        0x8008
#define CHANNEL0_REGISTER1                                  0x800c
#define CHANNEL0_REGISTER2                                  0x8010
#define CHANNEL0_REGISTER3                                  0x8014
#define CHANNEL0_REGISTER4                                  0x8018
#define CHANNEL0_REGISTER5                                  0x801c
#define CHANNEL0_REGISTER6                                  0x8020
#define CHANNEL0_REGISTER7                                  0x8024
#define CHANNEL0_REGISTER8                                  0x8028
#define CHANNEL0_REGISTER9                                  0x802c
#define CHANNEL0_REGISTER10                                 0x8030
#define CHANNEL0_REGISTER11                                 0x8034

/* MPSC1  */

#define MPSC1_MAIN_CONFIGURATION_LOW                        0x9000
#define MPSC1_MAIN_CONFIGURATION_HIGH                       0x9004
#define MPSC1_PROTOCOL_CONFIGURATION                        0x9008
#define CHANNEL1_REGISTER1                                  0x900c
#define CHANNEL1_REGISTER2                                  0x9010
#define CHANNEL1_REGISTER3                                  0x9014
#define CHANNEL1_REGISTER4                                  0x9018
#define CHANNEL1_REGISTER5                                  0x901c
#define CHANNEL1_REGISTER6                                  0x9020
#define CHANNEL1_REGISTER7                                  0x9024
#define CHANNEL1_REGISTER8                                  0x9028
#define CHANNEL1_REGISTER9                                  0x902c
#define CHANNEL1_REGISTER10                                 0x9030
#define CHANNEL1_REGISTER11                                 0x9034

/* MPSCs Interupts  */

#define MPSC0_CAUSE                                         0xb804
#define MPSC0_MASK                                          0xb884
#define MPSC1_CAUSE                                         0xb80c
#define MPSC1_MASK                                          0xb88c

#endif /* __INCgt64240rh */
