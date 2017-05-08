#ifndef _GDT_H_
#define _GDT_H_

#include "ctype.h"

#define GDT_START 3

#define	_KERNEL_CS_	0x08 	// kernel code
#define	_KERNEL_DS_	0x10
#define	_USER_CS_	0x18
#define	_USER_DS_	0x20	// user data
#define	_TSS0_		0x28	// Task segment selector for CPU 0

#define	_TEMP_CS_	0x08 	
#define	_TEMP_DS_	0x10
#define	_NORMAL_CS_	0x18
#define	_NORMAL_DS_	0x20	
#define	_TEMP_GS_	0x28

#define STA_X		0x8	    // Executable segment
#define STA_E		0x4	    // Expand down (non-executable segments)
#define STA_C		0x4	    // Conforming code segment (executable only)
#define STA_W		0x2	    // Writeable (non-executable segments)
#define STA_R		0x2	    // Readable (executable segments)
#define STA_A		0x1	    // Accessed


typedef struct _segment_descriptor_ {
	unsigned lim_15_0 : 16;  // Low bits of segment limit
	unsigned base_15_0 : 16; // Low bits of segment base address
	unsigned base_23_16 : 8; // Middle bits of segment base address
	unsigned type : 4;       // Segment type (see STS_ constants)
	unsigned s : 1;          // 0 = system, 1 = application
	unsigned dpl : 2;        // Descriptor Privilege Level
	unsigned p : 1;          // Present
	unsigned lim_19_16 : 4;  // High bits of segment limit
	unsigned avl : 1;        // Unused (available for software use)
	unsigned rsv1 : 1;       // Reserved
	unsigned db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
	unsigned g : 1;          // Granularity: limit scaled by 4K when set
	unsigned base_31_24 : 8; // High bits of segment base address
}seg_descriptor;

typedef struct descriptor_addr {
	uint16_t lim;		// Limit
	uint32_t base;		// Base address
} __attribute__ ((packed));

#define set_seg(type, base, limt, dpl) (seg_descriptor)\
	{(limt) & 0xFFFF,(base) & 0xFFFF,((base) >> 16) & 0xFF,type, 1, dpl , 1, ((limt) >> 16) & 0xF, 0, 0,1,1,((base)>>24) & 0xFF}

#define set_seg_real_mode(type, base, limt, dpl) (seg_descriptor)\
	{(limt) & 0xFFFF,(base) & 0xFFFF,((base) >> 16) & 0xFF,type, 1, dpl , 1, ((limt) >> 16) & 0xF, 0, 0,0,0,((base)>>24) & 0xFF}

#define set_seg_null	(seg_descriptor){ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

void init_gdt();

#endif
