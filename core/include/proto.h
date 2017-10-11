#ifndef ___PROTO_H__
#define ___PROTO_H__

/* klib.asm */
public void	out_byte(u16 port, u8 value);
public u8	in_byte(u16 port);
public void	disp_str(char * info);
public void	disp_color_str(char * info, int color);
public void	init_prot();
public void	init_8259A();

#endif
