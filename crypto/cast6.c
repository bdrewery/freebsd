/* Kernel cryptographic api.
 * cast6.c - Cast6 cipher algorithm [rfc2612].
 *
 * CAST-256 (*cast6*) is a DES like Substitution-Permutation Network (SPN)
 * cryptosystem built upon the CAST-128 (*cast5*) [rfc2144] encryption
 * algorithm.
 *
 * Copyright (C) 2003 Kartikey Mahendra Bhatt <kartik_me@hotmail.com>.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */


#include <linux/init.h>
#include <linux/crypto.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/string.h>

#define CAST6_BLOCK_SIZE 16
#define CAST6_MIN_KEY_SIZE 16
#define CAST6_MAX_KEY_SIZE 32

struct cast6_ctx {
	u32 Km[12][4];
	u8 Kr[12][4];
};

#define rol(n,x) ( ((x) << (n)) | ((x) >> (32-(n))) )

#define F1(D,r,m)  (  (I = ((m) + (D))), (I=rol((r),I)),   \
    (((s1[I >> 24] ^ s2[(I>>16)&0xff]) - s3[(I>>8)&0xff]) + s4[I&0xff]) )
#define F2(D,r,m)  (  (I = ((m) ^ (D))), (I=rol((r),I)),   \
    (((s1[I >> 24] - s2[(I>>16)&0xff]) + s3[(I>>8)&0xff]) ^ s4[I&0xff]) )
#define F3(D,r,m)  (  (I = ((m) - (D))), (I=rol((r),I)),   \
    (((s1[I >> 24] + s2[(I>>16)&0xff]) ^ s3[(I>>8)&0xff]) - s4[I&0xff]) )

static const u32 s1[256] = {
	0x30fb40d4, 0x9fa0ff0b, 0x6beccd2f, 0x3f258c7a, 0x1e213f2f,
	0x9c004dd3, 0x6003e540, 0xcf9fc949,
	0xbfd4af27, 0x88bbbdb5, 0xe2034090, 0x98d09675, 0x6e63a0e0,
	0x15c361d2, 0xc2e7661d, 0x22d4ff8e,
	0x28683b6f, 0xc07fd059, 0xff2379c8, 0x775f50e2, 0x43c340d3,
	0xdf2f8656, 0x887ca41a, 0xa2d2bd2d,
	0xa1c9e0d6, 0x346c4819, 0x61b76d87, 0x22540f2f, 0x2abe32e1,
	0xaa54166b, 0x22568e3a, 0xa2d341d0,
	0x66db40c8, 0xa784392f, 0x004dff2f, 0x2db9d2de, 0x97943fac,
	0x4a97c1d8, 0x527644b7, 0xb5f437a7,
	0xb82cbaef, 0xd751d159, 0x6ff7f0ed, 0x5a097a1f, 0x827b68d0,
	0x90ecf52e, 0x22b0c054, 0xbc8e5935,
	0x4b6d2f7f, 0x50bb64a2, 0xd2664910, 0xbee5812d, 0xb7332290,
	0xe93b159f, 0xb48ee411, 0x4bff345d,
	0xfd45c240, 0xad31973f, 0xc4f6d02e, 0x55fc8165, 0xd5b1caad,
	0xa1ac2dae, 0xa2d4b76d, 0xc19b0c50,
	0x882240f2, 0x0c6e4f38, 0xa4e4bfd7, 0x4f5ba272, 0x564c1d2f,
	0xc59c5319, 0xb949e354, 0xb04669fe,
	0xb1b6ab8a, 0xc71358dd, 0x6385c545, 0x110f935d, 0x57538ad5,
	0x6a390493, 0xe63d37e0, 0x2a54f6b3,
	0x3a787d5f, 0x6276a0b5, 0x19a6fcdf, 0x7a42206a, 0x29f9d4d5,
	0xf61b1891, 0xbb72275e, 0xaa508167,
	0x38901091, 0xc6b505eb, 0x84c7cb8c, 0x2ad75a0f, 0x874a1427,
	0xa2d1936b, 0x2ad286af, 0xaa56d291,
	0xd7894360, 0x425c750d, 0x93b39e26, 0x187184c9, 0x6c00b32d,
	0x73e2bb14, 0xa0bebc3c, 0x54623779,
	0x64459eab, 0x3f328b82, 0x7718cf82, 0x59a2cea6, 0x04ee002e,
	0x89fe78e6, 0x3fab0950, 0x325ff6c2,
	0x81383f05, 0x6963c5c8, 0x76cb5ad6, 0xd49974c9, 0xca180dcf,
	0x380782d5, 0xc7fa5cf6, 0x8ac31511,
	0x35e79e13, 0x47da91d0, 0xf40f9086, 0xa7e2419e, 0x31366241,
	0x051ef495, 0xaa573b04, 0x4a805d8d,
	0x548300d0, 0x00322a3c, 0xbf64cddf, 0xba57a68e, 0x75c6372b,
	0x50afd341, 0xa7c13275, 0x915a0bf5,
	0x6b54bfab, 0x2b0b1426, 0xab4cc9d7, 0x449ccd82, 0xf7fbf265,
	0xab85c5f3, 0x1b55db94, 0xaad4e324,
	0xcfa4bd3f, 0x2deaa3e2, 0x9e204d02, 0xc8bd25ac, 0xeadf55b3,
	0xd5bd9e98, 0xe31231b2, 0x2ad5ad6c,
	0x954329de, 0xadbe4528, 0xd8710f69, 0xaa51c90f, 0xaa786bf6,
	0x22513f1e, 0xaa51a79b, 0x2ad344cc,
	0x7b5a41f0, 0xd37cfbad, 0x1b069505, 0x41ece491, 0xb4c332e6,
	0x032268d4, 0xc9600acc, 0xce387e6d,
	0xbf6bb16c, 0x6a70fb78, 0x0d03d9c9, 0xd4df39de, 0xe01063da,
	0x4736f464, 0x5ad328d8, 0xb347cc96,
	0x75bb0fc3, 0x98511bfb, 0x4ffbcc35, 0xb58bcf6a, 0xe11f0abc,
	0xbfc5fe4a, 0xa70aec10, 0xac39570a,
	0x3f04442f, 0x6188b153, 0xe0397a2e, 0x5727cb79, 0x9ceb418f,
	0x1cacd68d, 0x2ad37c96, 0x0175cb9d,
	0xc69dff09, 0xc75b65f0, 0xd9db40d8, 0xec0e7779, 0x4744ead4,
	0xb11c3274, 0xdd24cb9e, 0x7e1c54bd,
	0xf01144f9, 0xd2240eb1, 0x9675b3fd, 0xa3ac3755, 0xd47c27af,
	0x51c85f4d, 0x56907596, 0xa5bb15e6,
	0x580304f0, 0xca042cf1, 0x011a37ea, 0x8dbfaadb, 0x35ba3e4a,
	0x3526ffa0, 0xc37b4d09, 0xbc306ed9,
	0x98a52666, 0x5648f725, 0xff5e569d, 0x0ced63d0, 0x7c63b2cf,
	0x700b45e1, 0xd5ea50f1, 0x85a92872,
	0xaf1fbda7, 0xd4234870, 0xa7870bf3, 0x2d3b4d79, 0x42e04198,
	0x0cd0ede7, 0x26470db8, 0xf881814c,
	0x474d6ad7, 0x7c0c5e5c, 0xd1231959, 0x381b7298, 0xf5d2f4db,
	0xab838653, 0x6e2f1e23, 0x83719c9e,
	0xbd91e046, 0x9a56456e, 0xdc39200c, 0x20c8c571, 0x962bda1c,
	0xe1e696ff, 0xb141ab08, 0x7cca89b9,
	0x1a69e783, 0x02cc4843, 0xa2f7c579, 0x429ef47d, 0x427b169c,
	0x5ac9f049, 0xdd8f0f00, 0x5c8165bf
};

static const u32 s2[256] = {
	0x1f201094, 0xef0ba75b, 0x69e3cf7e, 0x393f4380, 0xfe61cf7a,
	0xeec5207a, 0x55889c94, 0x72fc0651,
	0xada7ef79, 0x4e1d7235, 0xd55a63ce, 0xde0436ba, 0x99c430ef,
	0x5f0c0794, 0x18dcdb7d, 0xa1d6eff3,
	0xa0b52f7b, 0x59e83605, 0xee15b094, 0xe9ffd909, 0xdc440086,
	0xef944459, 0xba83ccb3, 0xe0c3cdfb,
	0xd1da4181, 0x3b092ab1, 0xf997f1c1, 0xa5e6cf7b, 0x01420ddb,
	0xe4e7ef5b, 0x25a1ff41, 0xe180f806,
	0x1fc41080, 0x179bee7a, 0xd37ac6a9, 0xfe5830a4, 0x98de8b7f,
	0x77e83f4e, 0x79929269, 0x24fa9f7b,
	0xe113c85b, 0xacc40083, 0xd7503525, 0xf7ea615f, 0x62143154,
	0x0d554b63, 0x5d681121, 0xc866c359,
	0x3d63cf73, 0xcee234c0, 0xd4d87e87, 0x5c672b21, 0x071f6181,
	0x39f7627f, 0x361e3084, 0xe4eb573b,
	0x602f64a4, 0xd63acd9c, 0x1bbc4635, 0x9e81032d, 0x2701f50c,
	0x99847ab4, 0xa0e3df79, 0xba6cf38c,
	0x10843094, 0x2537a95e, 0xf46f6ffe, 0xa1ff3b1f, 0x208cfb6a,
	0x8f458c74, 0xd9e0a227, 0x4ec73a34,
	0xfc884f69, 0x3e4de8df, 0xef0e0088, 0x3559648d, 0x8a45388c,
	0x1d804366, 0x721d9bfd, 0xa58684bb,
	0xe8256333, 0x844e8212, 0x128d8098, 0xfed33fb4, 0xce280ae1,
	0x27e19ba5, 0xd5a6c252, 0xe49754bd,
	0xc5d655dd, 0xeb667064, 0x77840b4d, 0xa1b6a801, 0x84db26a9,
	0xe0b56714, 0x21f043b7, 0xe5d05860,
	0x54f03084, 0x066ff472, 0xa31aa153, 0xdadc4755, 0xb5625dbf,
	0x68561be6, 0x83ca6b94, 0x2d6ed23b,
	0xeccf01db, 0xa6d3d0ba, 0xb6803d5c, 0xaf77a709, 0x33b4a34c,
	0x397bc8d6, 0x5ee22b95, 0x5f0e5304,
	0x81ed6f61, 0x20e74364, 0xb45e1378, 0xde18639b, 0x881ca122,
	0xb96726d1, 0x8049a7e8, 0x22b7da7b,
	0x5e552d25, 0x5272d237, 0x79d2951c, 0xc60d894c, 0x488cb402,
	0x1ba4fe5b, 0xa4b09f6b, 0x1ca815cf,
	0xa20c3005, 0x8871df63, 0xb9de2fcb, 0x0cc6c9e9, 0x0beeff53,
	0xe3214517, 0xb4542835, 0x9f63293c,
	0xee41e729, 0x6e1d2d7c, 0x50045286, 0x1e6685f3, 0xf33401c6,
	0x30a22c95, 0x31a70850, 0x60930f13,
	0x73f98417, 0xa1269859, 0xec645c44, 0x52c877a9, 0xcdff33a6,
	0xa02b1741, 0x7cbad9a2, 0x2180036f,
	0x50d99c08, 0xcb3f4861, 0xc26bd765, 0x64a3f6ab, 0x80342676,
	0x25a75e7b, 0xe4e6d1fc, 0x20c710e6,
	0xcdf0b680, 0x17844d3b, 0x31eef84d, 0x7e0824e4, 0x2ccb49eb,
	0x846a3bae, 0x8ff77888, 0xee5d60f6,
	0x7af75673, 0x2fdd5cdb, 0xa11631c1, 0x30f66f43, 0xb3faec54,
	0x157fd7fa, 0xef8579cc, 0xd152de58,
	0xdb2ffd5e, 0x8f32ce19, 0x306af97a, 0x02f03ef8, 0x99319ad5,
	0xc242fa0f, 0xa7e3ebb0, 0xc68e4906,
	0xb8da230c, 0x80823028, 0xdcdef3c8, 0xd35fb171, 0x088a1bc8,
	0xbec0c560, 0x61a3c9e8, 0xbca8f54d,
	0xc72feffa, 0x22822e99, 0x82c570b4, 0xd8d94e89, 0x8b1c34bc,
	0x301e16e6, 0x273be979, 0xb0ffeaa6,
	0x61d9b8c6, 0x00b24869, 0xb7ffce3f, 0x08dc283b, 0x43daf65a,
	0xf7e19798, 0x7619b72f, 0x8f1c9ba4,
	0xdc8637a0, 0x16a7d3b1, 0x9fc393b7, 0xa7136eeb, 0xc6bcc63e,
	0x1a513742, 0xef6828bc, 0x520365d6,
	0x2d6a77ab, 0x3527ed4b, 0x821fd216, 0x095c6e2e, 0xdb92f2fb,
	0x5eea29cb, 0x145892f5, 0x91584f7f,
	0x5483697b, 0x2667a8cc, 0x85196048, 0x8c4bacea, 0x833860d4,
	0x0d23e0f9, 0x6c387e8a, 0x0ae6d249,
	0xb284600c, 0xd835731d, 0xdcb1c647, 0xac4c56ea, 0x3ebd81b3,
	0x230eabb0, 0x6438bc87, 0xf0b5b1fa,
	0x8f5ea2b3, 0xfc184642, 0x0a036b7a, 0x4fb089bd, 0x649da589,
	0xa345415e, 0x5c038323, 0x3e5d3bb9,
	0x43d79572, 0x7e6dd07c, 0x06dfdf1e, 0x6c6cc4ef, 0x7160a539,
	0x73bfbe70, 0x83877605, 0x4523ecf1
};

static const u32 s3[256] = {
	0x8defc240, 0x25fa5d9f, 0xeb903dbf, 0xe810c907, 0x47607fff,
	0x369fe44b, 0x8c1fc644, 0xaececa90,
	0xbeb1f9bf, 0xeefbcaea, 0xe8cf1950, 0x51df07ae, 0x920e8806,
	0xf0ad0548, 0xe13c8d83, 0x927010d5,
	0x11107d9f, 0x07647db9, 0xb2e3e4d4, 0x3d4f285e, 0xb9afa820,
	0xfade82e0, 0xa067268b, 0x8272792e,
	0x553fb2c0, 0x489ae22b, 0xd4ef9794, 0x125e3fbc, 0x21fffcee,
	0x825b1bfd, 0x9255c5ed, 0x1257a240,
	0x4e1a8302, 0xbae07fff, 0x528246e7, 0x8e57140e, 0x3373f7bf,
	0x8c9f8188, 0xa6fc4ee8, 0xc982b5a5,
	0xa8c01db7, 0x579fc264, 0x67094f31, 0xf2bd3f5f, 0x40fff7c1,
	0x1fb78dfc, 0x8e6bd2c1, 0x437be59b,
	0x99b03dbf, 0xb5dbc64b, 0x638dc0e6, 0x55819d99, 0xa197c81c,
	0x4a012d6e, 0xc5884a28, 0xccc36f71,
	0xb843c213, 0x6c0743f1, 0x8309893c, 0x0feddd5f, 0x2f7fe850,
	0xd7c07f7e, 0x02507fbf, 0x5afb9a04,
	0xa747d2d0, 0x1651192e, 0xaf70bf3e, 0x58c31380, 0x5f98302e,
	0x727cc3c4, 0x0a0fb402, 0x0f7fef82,
	0x8c96fdad, 0x5d2c2aae, 0x8ee99a49, 0x50da88b8, 0x8427f4a0,
	0x1eac5790, 0x796fb449, 0x8252dc15,
	0xefbd7d9b, 0xa672597d, 0xada840d8, 0x45f54504, 0xfa5d7403,
	0xe83ec305, 0x4f91751a, 0x925669c2,
	0x23efe941, 0xa903f12e, 0x60270df2, 0x0276e4b6, 0x94fd6574,
	0x927985b2, 0x8276dbcb, 0x02778176,
	0xf8af918d, 0x4e48f79e, 0x8f616ddf, 0xe29d840e, 0x842f7d83,
	0x340ce5c8, 0x96bbb682, 0x93b4b148,
	0xef303cab, 0x984faf28, 0x779faf9b, 0x92dc560d, 0x224d1e20,
	0x8437aa88, 0x7d29dc96, 0x2756d3dc,
	0x8b907cee, 0xb51fd240, 0xe7c07ce3, 0xe566b4a1, 0xc3e9615e,
	0x3cf8209d, 0x6094d1e3, 0xcd9ca341,
	0x5c76460e, 0x00ea983b, 0xd4d67881, 0xfd47572c, 0xf76cedd9,
	0xbda8229c, 0x127dadaa, 0x438a074e,
	0x1f97c090, 0x081bdb8a, 0x93a07ebe, 0xb938ca15, 0x97b03cff,
	0x3dc2c0f8, 0x8d1ab2ec, 0x64380e51,
	0x68cc7bfb, 0xd90f2788, 0x12490181, 0x5de5ffd4, 0xdd7ef86a,
	0x76a2e214, 0xb9a40368, 0x925d958f,
	0x4b39fffa, 0xba39aee9, 0xa4ffd30b, 0xfaf7933b, 0x6d498623,
	0x193cbcfa, 0x27627545, 0x825cf47a,
	0x61bd8ba0, 0xd11e42d1, 0xcead04f4, 0x127ea392, 0x10428db7,
	0x8272a972, 0x9270c4a8, 0x127de50b,
	0x285ba1c8, 0x3c62f44f, 0x35c0eaa5, 0xe805d231, 0x428929fb,
	0xb4fcdf82, 0x4fb66a53, 0x0e7dc15b,
	0x1f081fab, 0x108618ae, 0xfcfd086d, 0xf9ff2889, 0x694bcc11,
	0x236a5cae, 0x12deca4d, 0x2c3f8cc5,
	0xd2d02dfe, 0xf8ef5896, 0xe4cf52da, 0x95155b67, 0x494a488c,
	0xb9b6a80c, 0x5c8f82bc, 0x89d36b45,
	0x3a609437, 0xec00c9a9, 0x44715253, 0x0a874b49, 0xd773bc40,
	0x7c34671c, 0x02717ef6, 0x4feb5536,
	0xa2d02fff, 0xd2bf60c4, 0xd43f03c0, 0x50b4ef6d, 0x07478cd1,
	0x006e1888, 0xa2e53f55, 0xb9e6d4bc,
	0xa2048016, 0x97573833, 0xd7207d67, 0xde0f8f3d, 0x72f87b33,
	0xabcc4f33, 0x7688c55d, 0x7b00a6b0,
	0x947b0001, 0x570075d2, 0xf9bb88f8, 0x8942019e, 0x4264a5ff,
	0x856302e0, 0x72dbd92b, 0xee971b69,
	0x6ea22fde, 0x5f08ae2b, 0xaf7a616d, 0xe5c98767, 0xcf1febd2,
	0x61efc8c2, 0xf1ac2571, 0xcc8239c2,
	0x67214cb8, 0xb1e583d1, 0xb7dc3e62, 0x7f10bdce, 0xf90a5c38,
	0x0ff0443d, 0x606e6dc6, 0x60543a49,
	0x5727c148, 0x2be98a1d, 0x8ab41738, 0x20e1be24, 0xaf96da0f,
	0x68458425, 0x99833be5, 0x600d457d,
	0x282f9350, 0x8334b362, 0xd91d1120, 0x2b6d8da0, 0x642b1e31,
	0x9c305a00, 0x52bce688, 0x1b03588a,
	0xf7baefd5, 0x4142ed9c, 0xa4315c11, 0x83323ec5, 0xdfef4636,
	0xa133c501, 0xe9d3531c, 0xee353783
};

static const u32 s4[256] = {
	0x9db30420, 0x1fb6e9de, 0xa7be7bef, 0xd273a298, 0x4a4f7bdb,
	0x64ad8c57, 0x85510443, 0xfa020ed1,
	0x7e287aff, 0xe60fb663, 0x095f35a1, 0x79ebf120, 0xfd059d43,
	0x6497b7b1, 0xf3641f63, 0x241e4adf,
	0x28147f5f, 0x4fa2b8cd, 0xc9430040, 0x0cc32220, 0xfdd30b30,
	0xc0a5374f, 0x1d2d00d9, 0x24147b15,
	0xee4d111a, 0x0fca5167, 0x71ff904c, 0x2d195ffe, 0x1a05645f,
	0x0c13fefe, 0x081b08ca, 0x05170121,
	0x80530100, 0xe83e5efe, 0xac9af4f8, 0x7fe72701, 0xd2b8ee5f,
	0x06df4261, 0xbb9e9b8a, 0x7293ea25,
	0xce84ffdf, 0xf5718801, 0x3dd64b04, 0xa26f263b, 0x7ed48400,
	0x547eebe6, 0x446d4ca0, 0x6cf3d6f5,
	0x2649abdf, 0xaea0c7f5, 0x36338cc1, 0x503f7e93, 0xd3772061,
	0x11b638e1, 0x72500e03, 0xf80eb2bb,
	0xabe0502e, 0xec8d77de, 0x57971e81, 0xe14f6746, 0xc9335400,
	0x6920318f, 0x081dbb99, 0xffc304a5,
	0x4d351805, 0x7f3d5ce3, 0xa6c866c6, 0x5d5bcca9, 0xdaec6fea,
	0x9f926f91, 0x9f46222f, 0x3991467d,
	0xa5bf6d8e, 0x1143c44f, 0x43958302, 0xd0214eeb, 0x022083b8,
	0x3fb6180c, 0x18f8931e, 0x281658e6,
	0x26486e3e, 0x8bd78a70, 0x7477e4c1, 0xb506e07c, 0xf32d0a25,
	0x79098b02, 0xe4eabb81, 0x28123b23,
	0x69dead38, 0x1574ca16, 0xdf871b62, 0x211c40b7, 0xa51a9ef9,
	0x0014377b, 0x041e8ac8, 0x09114003,
	0xbd59e4d2, 0xe3d156d5, 0x4fe876d5, 0x2f91a340, 0x557be8de,
	0x00eae4a7, 0x0ce5c2ec, 0x4db4bba6,
	0xe756bdff, 0xdd3369ac, 0xec17b035, 0x06572327, 0x99afc8b0,
	0x56c8c391, 0x6b65811c, 0x5e146119,
	0x6e85cb75, 0xbe07c002, 0xc2325577, 0x893ff4ec, 0x5bbfc92d,
	0xd0ec3b25, 0xb7801ab7, 0x8d6d3b24,
	0x20c763ef, 0xc366a5fc, 0x9c382880, 0x0ace3205, 0xaac9548a,
	0xeca1d7c7, 0x041afa32, 0x1d16625a,
	0x6701902c, 0x9b757a54, 0x31d477f7, 0x9126b031, 0x36cc6fdb,
	0xc70b8b46, 0xd9e66a48, 0x56e55a79,
	0x026a4ceb, 0x52437eff, 0x2f8f76b4, 0x0df980a5, 0x8674cde3,
	0xedda04eb, 0x17a9be04, 0x2c18f4df,
	0xb7747f9d, 0xab2af7b4, 0xefc34d20, 0x2e096b7c, 0x1741a254,
	0xe5b6a035, 0x213d42f6, 0x2c1c7c26,
	0x61c2f50f, 0x6552daf9, 0xd2c231f8, 0x25130f69, 0xd8167fa2,
	0x0418f2c8, 0x001a96a6, 0x0d1526ab,
	0x63315c21, 0x5e0a72ec, 0x49bafefd, 0x187908d9, 0x8d0dbd86,
	0x311170a7, 0x3e9b640c, 0xcc3e10d7,
	0xd5cad3b6, 0x0caec388, 0xf73001e1, 0x6c728aff, 0x71eae2a1,
	0x1f9af36e, 0xcfcbd12f, 0xc1de8417,
	0xac07be6b, 0xcb44a1d8, 0x8b9b0f56, 0x013988c3, 0xb1c52fca,
	0xb4be31cd, 0xd8782806, 0x12a3a4e2,
	0x6f7de532, 0x58fd7eb6, 0xd01ee900, 0x24adffc2, 0xf4990fc5,
	0x9711aac5, 0x001d7b95, 0x82e5e7d2,
	0x109873f6, 0x00613096, 0xc32d9521, 0xada121ff, 0x29908415,
	0x7fbb977f, 0xaf9eb3db, 0x29c9ed2a,
	0x5ce2a465, 0xa730f32c, 0xd0aa3fe8, 0x8a5cc091, 0xd49e2ce7,
	0x0ce454a9, 0xd60acd86, 0x015f1919,
	0x77079103, 0xdea03af6, 0x78a8565e, 0xdee356df, 0x21f05cbe,
	0x8b75e387, 0xb3c50651, 0xb8a5c3ef,
	0xd8eeb6d2, 0xe523be77, 0xc2154529, 0x2f69efdf, 0xafe67afb,
	0xf470c4b2, 0xf3e0eb5b, 0xd6cc9876,
	0x39e4460c, 0x1fda8538, 0x1987832f, 0xca007367, 0xa99144f8,
	0x296b299e, 0x492fc295, 0x9266beab,
	0xb5676e69, 0x9bd3ddda, 0xdf7e052f, 0xdb25701c, 0x1b5e51ee,
	0xf65324e6, 0x6afce36c, 0x0316cc04,
	0x8644213e, 0xb7dc59d0, 0x7965291f, 0xccd6fd43, 0x41823979,
	0x932bcdf6, 0xb657c34d, 0x4edfd282,
	0x7ae5290c, 0x3cb9536b, 0x851e20fe, 0x9833557e, 0x13ecf0b0,
	0xd3ffb372, 0x3f85c5c1, 0x0aef7ed2
};

static const u32 Tm[24][8] = {
	{ 0x5a827999, 0xc95c653a, 0x383650db, 0xa7103c7c, 0x15ea281d,
		0x84c413be, 0xf39dff5f, 0x6277eb00 } , 
	{ 0xd151d6a1, 0x402bc242, 0xaf05ade3, 0x1ddf9984, 0x8cb98525,
		0xfb9370c6, 0x6a6d5c67, 0xd9474808 } ,
	{ 0x482133a9, 0xb6fb1f4a, 0x25d50aeb, 0x94aef68c, 0x0388e22d,
		0x7262cdce, 0xe13cb96f, 0x5016a510 } ,
	{ 0xbef090b1, 0x2dca7c52, 0x9ca467f3, 0x0b7e5394, 0x7a583f35,
		0xe9322ad6, 0x580c1677, 0xc6e60218 } ,
	{ 0x35bfedb9, 0xa499d95a, 0x1373c4fb, 0x824db09c, 0xf1279c3d,
		0x600187de, 0xcedb737f, 0x3db55f20 } ,
	{ 0xac8f4ac1, 0x1b693662, 0x8a432203, 0xf91d0da4, 0x67f6f945,
		0xd6d0e4e6, 0x45aad087, 0xb484bc28 } ,
	{ 0x235ea7c9, 0x9238936a, 0x01127f0b, 0x6fec6aac, 0xdec6564d,
		0x4da041ee, 0xbc7a2d8f, 0x2b541930 } ,
	{ 0x9a2e04d1, 0x0907f072, 0x77e1dc13, 0xe6bbc7b4, 0x5595b355,
		0xc46f9ef6, 0x33498a97, 0xa2237638 } ,
	{ 0x10fd61d9, 0x7fd74d7a, 0xeeb1391b, 0x5d8b24bc, 0xcc65105d,
		0x3b3efbfe, 0xaa18e79f, 0x18f2d340 } ,
	{ 0x87ccbee1, 0xf6a6aa82, 0x65809623, 0xd45a81c4, 0x43346d65,
		0xb20e5906, 0x20e844a7, 0x8fc23048 } ,
	{ 0xfe9c1be9, 0x6d76078a, 0xdc4ff32b, 0x4b29decc, 0xba03ca6d,
		0x28ddb60e, 0x97b7a1af, 0x06918d50 } ,
	{ 0x756b78f1, 0xe4456492, 0x531f5033, 0xc1f93bd4, 0x30d32775,
		0x9fad1316, 0x0e86feb7, 0x7d60ea58 } ,
	{ 0xec3ad5f9, 0x5b14c19a, 0xc9eead3b, 0x38c898dc, 0xa7a2847d,
		0x167c701e, 0x85565bbf, 0xf4304760 } ,
	{ 0x630a3301, 0xd1e41ea2, 0x40be0a43, 0xaf97f5e4, 0x1e71e185,
		0x8d4bcd26, 0xfc25b8c7, 0x6affa468 } ,
	{ 0xd9d99009, 0x48b37baa, 0xb78d674b, 0x266752ec, 0x95413e8d,
		0x041b2a2e, 0x72f515cf, 0xe1cf0170 } ,
	{ 0x50a8ed11, 0xbf82d8b2, 0x2e5cc453, 0x9d36aff4, 0x0c109b95,
		0x7aea8736, 0xe9c472d7, 0x589e5e78 } ,
	{ 0xc7784a19, 0x365235ba, 0xa52c215b, 0x14060cfc, 0x82dff89d,
		0xf1b9e43e, 0x6093cfdf, 0xcf6dbb80 } ,
	{ 0x3e47a721, 0xad2192c2, 0x1bfb7e63, 0x8ad56a04, 0xf9af55a5,
		0x68894146, 0xd7632ce7, 0x463d1888 } ,
	{ 0xb5170429, 0x23f0efca, 0x92cadb6b, 0x01a4c70c, 0x707eb2ad,
		0xdf589e4e, 0x4e3289ef, 0xbd0c7590 } ,
	{ 0x2be66131, 0x9ac04cd2, 0x099a3873, 0x78742414, 0xe74e0fb5,
		0x5627fb56, 0xc501e6f7, 0x33dbd298 } ,
	{ 0xa2b5be39, 0x118fa9da, 0x8069957b, 0xef43811c, 0x5e1d6cbd,
		0xccf7585e, 0x3bd143ff, 0xaaab2fa0 } ,
	{ 0x19851b41, 0x885f06e2, 0xf738f283, 0x6612de24, 0xd4ecc9c5,
		0x43c6b566, 0xb2a0a107, 0x217a8ca8 } ,
	{ 0x90547849, 0xff2e63ea, 0x6e084f8b, 0xdce23b2c, 0x4bbc26cd,
		0xba96126e, 0x296ffe0f, 0x9849e9b0 } ,
	{ 0x0723d551, 0x75fdc0f2, 0xe4d7ac93, 0x53b19834, 0xc28b83d5,
		0x31656f76, 0xa03f5b17, 0x0f1946b8 }
};

static const u8 Tr[4][8] = {
	{ 0x13, 0x04, 0x15, 0x06, 0x17, 0x08, 0x19, 0x0a } ,
	{ 0x1b, 0x0c, 0x1d, 0x0e, 0x1f, 0x10, 0x01, 0x12 } ,
	{ 0x03, 0x14, 0x05, 0x16, 0x07, 0x18, 0x09, 0x1a } ,
	{ 0x0b, 0x1c, 0x0d, 0x1e, 0x0f, 0x00, 0x11, 0x02 }
};

/* forward octave */
static inline void W(u32 *key, unsigned int i) {
	u32 I;
	key[6] ^= F1(key[7], Tr[i % 4][0], Tm[i][0]);
	key[5] ^= F2(key[6], Tr[i % 4][1], Tm[i][1]);
	key[4] ^= F3(key[5], Tr[i % 4][2], Tm[i][2]);
	key[3] ^= F1(key[4], Tr[i % 4][3], Tm[i][3]);
	key[2] ^= F2(key[3], Tr[i % 4][4], Tm[i][4]);
	key[1] ^= F3(key[2], Tr[i % 4][5], Tm[i][5]);
	key[0] ^= F1(key[1], Tr[i % 4][6], Tm[i][6]);	
	key[7] ^= F2(key[0], Tr[i % 4][7], Tm[i][7]);
}

static int
cast6_setkey(void *ctx, const u8 * in_key, unsigned key_len, u32 * flags)
{
	int i;
	u32 key[8];
	u8 p_key[32]; /* padded key */
	struct cast6_ctx *c = (struct cast6_ctx *) ctx;

	if (key_len < 16 || key_len > 32 || key_len % 4 != 0) {
		*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
		return -EINVAL;
	}	

	memset (p_key, 0, 32);
	memcpy (p_key, in_key, key_len);
	
	key[0] = p_key[0] << 24 | p_key[1] << 16 | p_key[2] << 8 | p_key[3];		/* A */
	key[1] = p_key[4] << 24 | p_key[5] << 16 | p_key[6] << 8 | p_key[7];		/* B */
	key[2] = p_key[8] << 24 | p_key[9] << 16 | p_key[10] << 8 | p_key[11];		/* C */
	key[3] = p_key[12] << 24 | p_key[13] << 16 | p_key[14] << 8 | p_key[15];	/* D */
	key[4] = p_key[16] << 24 | p_key[17] << 16 | p_key[18] << 8 | p_key[19];	/* E */
	key[5] = p_key[20] << 24 | p_key[21] << 16 | p_key[22] << 8 | p_key[23];	/* F */
	key[6] = p_key[24] << 24 | p_key[25] << 16 | p_key[26] << 8 | p_key[27];	/* G */
	key[7] = p_key[28] << 24 | p_key[29] << 16 | p_key[30] << 8 | p_key[31];	/* H */
	


	for (i = 0; i < 12; i++) {
		W (key, 2 * i);
		W (key, 2 * i + 1);
		
		c->Kr[i][0] = key[0] & 0x1f;
		c->Kr[i][1] = key[2] & 0x1f;
		c->Kr[i][2] = key[4] & 0x1f;
		c->Kr[i][3] = key[6] & 0x1f;
		
		c->Km[i][0] = key[7];
		c->Km[i][1] = key[5];
		c->Km[i][2] = key[3];
		c->Km[i][3] = key[1];
	}

	return 0;
}

/*forward quad round*/
static inline void Q (u32 * block, u8 * Kr, u32 * Km) {
	u32 I;
	block[2] ^= F1(block[3], Kr[0], Km[0]);
	block[1] ^= F2(block[2], Kr[1], Km[1]);
	block[0] ^= F3(block[1], Kr[2], Km[2]);
	block[3] ^= F1(block[0], Kr[3], Km[3]);		
}

/*reverse quad round*/
static inline void QBAR (u32 * block, u8 * Kr, u32 * Km) {
	u32 I;
        block[3] ^= F1(block[0], Kr[3], Km[3]);
        block[0] ^= F3(block[1], Kr[2], Km[2]);
        block[1] ^= F2(block[2], Kr[1], Km[1]);
        block[2] ^= F1(block[3], Kr[0], Km[0]);
}

static void cast6_encrypt (void * ctx, u8 * outbuf, const u8 * inbuf) {
	struct cast6_ctx * c = (struct cast6_ctx *)ctx;
	u32 block[4];
	u32 * Km; 
	u8 * Kr;

	block[0] = inbuf[0] << 24 | inbuf[1] << 16 | inbuf[2] << 8 | inbuf[3];
	block[1] = inbuf[4] << 24 | inbuf[5] << 16 | inbuf[6] << 8 | inbuf[7];
	block[2] = inbuf[8] << 24 | inbuf[9] << 16 | inbuf[10] << 8 | inbuf[11];
	block[3] = inbuf[12] << 24 | inbuf[13] << 16 | inbuf[14] << 8 | inbuf[15];

	Km = c->Km[0]; Kr = c->Kr[0]; Q (block, Kr, Km);
	Km = c->Km[1]; Kr = c->Kr[1]; Q (block, Kr, Km);
	Km = c->Km[2]; Kr = c->Kr[2]; Q (block, Kr, Km);
	Km = c->Km[3]; Kr = c->Kr[3]; Q (block, Kr, Km);
	Km = c->Km[4]; Kr = c->Kr[4]; Q (block, Kr, Km);
	Km = c->Km[5]; Kr = c->Kr[5]; Q (block, Kr, Km);
	Km = c->Km[6]; Kr = c->Kr[6]; QBAR (block, Kr, Km);
	Km = c->Km[7]; Kr = c->Kr[7]; QBAR (block, Kr, Km);
	Km = c->Km[8]; Kr = c->Kr[8]; QBAR (block, Kr, Km);
	Km = c->Km[9]; Kr = c->Kr[9]; QBAR (block, Kr, Km);
	Km = c->Km[10]; Kr = c->Kr[10]; QBAR (block, Kr, Km);
	Km = c->Km[11]; Kr = c->Kr[11]; QBAR (block, Kr, Km);
	
	outbuf[0] = (block[0] >> 24) & 0xff;
	outbuf[1] = (block[0] >> 16) & 0xff;
	outbuf[2] = (block[0] >> 8) & 0xff;
	outbuf[3] = block[0] & 0xff;
	outbuf[4] = (block[1] >> 24) & 0xff;
	outbuf[5] = (block[1] >> 16) & 0xff;
	outbuf[6] = (block[1] >> 8) & 0xff;
	outbuf[7] = block[1] & 0xff;
	outbuf[8] = (block[2] >> 24) & 0xff;
	outbuf[9] = (block[2] >> 16) & 0xff;
	outbuf[10] = (block[2] >> 8) & 0xff;
	outbuf[11] = block[2] & 0xff;
	outbuf[12] = (block[3] >> 24) & 0xff;
	outbuf[13] = (block[3] >> 16) & 0xff;
	outbuf[14] = (block[3] >> 8) & 0xff;
	outbuf[15] = block[3] & 0xff;	
}	

static void cast6_decrypt (void * ctx, u8 * outbuf, const u8 * inbuf) {
	struct cast6_ctx * c = (struct cast6_ctx *)ctx;
	u32 block[4];
	u32 * Km; 
	u8 * Kr;

	block[0] = inbuf[0] << 24 | inbuf[1] << 16 | inbuf[2] << 8 | inbuf[3];
	block[1] = inbuf[4] << 24 | inbuf[5] << 16 | inbuf[6] << 8 | inbuf[7];
	block[2] = inbuf[8] << 24 | inbuf[9] << 16 | inbuf[10] << 8 | inbuf[11];
	block[3] = inbuf[12] << 24 | inbuf[13] << 16 | inbuf[14] << 8 | inbuf[15];

	Km = c->Km[11]; Kr = c->Kr[11]; Q (block, Kr, Km);
	Km = c->Km[10]; Kr = c->Kr[10]; Q (block, Kr, Km);
	Km = c->Km[9]; Kr = c->Kr[9]; Q (block, Kr, Km);
	Km = c->Km[8]; Kr = c->Kr[8]; Q (block, Kr, Km);
	Km = c->Km[7]; Kr = c->Kr[7]; Q (block, Kr, Km);
	Km = c->Km[6]; Kr = c->Kr[6]; Q (block, Kr, Km);
	Km = c->Km[5]; Kr = c->Kr[5]; QBAR (block, Kr, Km);
	Km = c->Km[4]; Kr = c->Kr[4]; QBAR (block, Kr, Km);
	Km = c->Km[3]; Kr = c->Kr[3]; QBAR (block, Kr, Km);
	Km = c->Km[2]; Kr = c->Kr[2]; QBAR (block, Kr, Km);
	Km = c->Km[1]; Kr = c->Kr[1]; QBAR (block, Kr, Km);
	Km = c->Km[0]; Kr = c->Kr[0]; QBAR (block, Kr, Km);
	
	outbuf[0] = (block[0] >> 24) & 0xff;
	outbuf[1] = (block[0] >> 16) & 0xff;
	outbuf[2] = (block[0] >> 8) & 0xff;
	outbuf[3] = block[0] & 0xff;
	outbuf[4] = (block[1] >> 24) & 0xff;
	outbuf[5] = (block[1] >> 16) & 0xff;
	outbuf[6] = (block[1] >> 8) & 0xff;
	outbuf[7] = block[1] & 0xff;
	outbuf[8] = (block[2] >> 24) & 0xff;
	outbuf[9] = (block[2] >> 16) & 0xff;
	outbuf[10] = (block[2] >> 8) & 0xff;
	outbuf[11] = block[2] & 0xff;
	outbuf[12] = (block[3] >> 24) & 0xff;
	outbuf[13] = (block[3] >> 16) & 0xff;
	outbuf[14] = (block[3] >> 8) & 0xff;
	outbuf[15] = block[3] & 0xff;	
}	

static struct crypto_alg alg = {
	.cra_name = "cast6",
	.cra_flags = CRYPTO_ALG_TYPE_CIPHER,
	.cra_blocksize = CAST6_BLOCK_SIZE,
	.cra_ctxsize = sizeof(struct cast6_ctx),
	.cra_module = THIS_MODULE,
	.cra_list = LIST_HEAD_INIT(alg.cra_list),
	.cra_u = {
		  .cipher = {
			     .cia_min_keysize = CAST6_MIN_KEY_SIZE,
			     .cia_max_keysize = CAST6_MAX_KEY_SIZE,
			     .cia_setkey = cast6_setkey,
			     .cia_encrypt = cast6_encrypt,
			     .cia_decrypt = cast6_decrypt}
		  }
};

static int __init init(void)
{
	return crypto_register_alg(&alg);
}

static void __exit fini(void)
{
	crypto_unregister_alg(&alg);
}

module_init(init);
module_exit(fini);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cast6 Cipher Algorithm");
