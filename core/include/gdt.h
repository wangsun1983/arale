#ifndef _GDT_H_
#define _GDT_H_

#define GDT_START 3


struct segment_descriptor {
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

void set_segmdesc(struct segment_descriptor *sd, unsigned int limit, int base, int ar);

#endif
