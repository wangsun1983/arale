/******************************************************************************
 *  Broken libc implementation ;)
 *  Just the functions I need with some custom stuff included.
 *
 *****************************************************************************/

#include "klibc.h"

static int _cursor_loc = 0;
static char _color = 0;

static int _cursor_save = 0;
static char _color_save = 0;

static struct frame_t *cur_frame = NULL;

static int cursor_move(int cnt);
static int cursor_move_line(int cnt);
static int _puts(const char *text);
static inline void put_tab();

#define CHAR_TO_MEMVAL(chr) \
        ((chr) | (_color << 8))

/*
 * Clears the screen with background colors
 */
void clear_screen()
{
    int i;
    int max_offset = MAX_CRS_X * MAX_CRS_Y;

    for (i = 0; i < max_offset; i++)
    {
        short mem_val = CHAR_TO_MEMVAL(' ');
        ((short *)VIDEO_MEMORY)[i] = mem_val;
    }
}

/*
 * Sets foreground and background colors according to table defined
 * in libc.h
 */
void set_color(unsigned char backgrnd, unsigned char forgrnd)
{
    _color = (forgrnd | (backgrnd << 4));
}

/*
 * Moves cursor position to provided `x` and `y`.
 */
void goto_xy(unsigned x, unsigned y)
{
    _cursor_loc = MAX_CRS_X * y + x;
}

void cursor_save()
{
    _cursor_save = _cursor_loc;
    _color_save = _color;
}

void cursor_load()
{
    _cursor_loc = _cursor_save;
    _color = _color_save;
}

void color_save()
{
    _color_save = _color;
}

void color_load()
{
    _color = _color_save;
}

/*
 * Sets memory location at `dest` to `val` for `count` bytes.
 * Returns `dest`
 */
void *kmemset(void *dest, int val, size_t count)
{
    int index = 0;
    while (index < count)
    {
        ((unsigned char *) dest)[index] = val;
        index++;
    }
    return dest;
}

/*
 * Copyes `num` bytes of values from `src` to `dest`.
 * Does not do any error checking - pure binary copy!
 * Returns `dest`
 */
void *memcpy(char *dest, const char *src, size_t num)
{
    size_t i;

    for (i = 0; i < num; i++) {
        dest[i] = src[i];
        //*(char *)dest = *(const char *)src;
    }
    return dest;
}

/*
 * Returns the size of provided null terminated char array
 */
size_t strlen(const char* str)
{
    int i = 0;

    while (str[i])
        i++;

    return i;
}

/*
 * Prints a given character on the screen.
 */
int kputchar(int c)
{
    bool mov_forw = true;
    bool put_char = true;

    switch (c) {
    case (0x8):     /* backspace */
        cursor_move(-1);    
        c = ' ';
        mov_forw = false;
        break;
    case ('\r'):    /* carriage return */
    case ('\n'):    /* new line */
        cursor_move_line(1);
        mov_forw = false;
        put_char = false;
        break;
    case ('\t'):    /* tab */
        cursor_move(TAB_SIZE);
        mov_forw = false;
        put_char = false;
    }

    if (put_char)
        ((short *)VIDEO_MEMORY)[_cursor_loc] = CHAR_TO_MEMVAL(c);
    if (mov_forw)
        cursor_move(1);

    return c;
}

/*
 * Prints a null terminated char array on the screen
 * with added new line character at the end.
 */
int kputs(const char *text)
{
    _puts(text);
    kputchar('\n');
    return 0;
}

/*
 * Prints a null terminated char array on the screen without '\n'
 */
static int _puts(const char *text)
{
    while (*text)
        if (kputchar(*text++) == EOF)
            return EOF;
    return 0;
}

/*
 * Prints tab space
 */
static inline void put_tab()
{
    int i;

    for (i = 0; i < TAB_SIZE; i++)
        kputchar(' ');
}

/*
 * All known `printf` ;)
 */
int kprintf(const char *format, ...)
{
    int i;
    va_list list;
    va_start(list, format);
    
    for (i = 0; format[i]; i++)
    {
        /* handle tab */
        if (format[i] == '\t')
        {
            put_tab();
            i++;
            continue;
        }

        /* any non-special character just print out */
        if (format[i] != '%')
        {
            kputchar(format[i]);
            continue;
        }

        switch (format[i+1]) {
        /* double-% */
        case ('%'):
            kputchar('%');
            break;
        /* integral */
        case ('i'):
        case ('d'): {
            int val = va_arg(list, int);
            char str[64];
            kitoa(val, str, 10);
            _puts(str);
            break;
        }
        /* character */
        case ('c'): {
            char val = va_arg(list, char);
            kputchar(val);
            break;
        }
        /* string */
        case ('s'): {
            char *val = va_arg(list, char *);
            _puts(val);
            break;
        }
        /* hex */
        case ('x'):
        case ('X'): {
            int val = va_arg(list, int);
            char str[64];
            kitoa(val, str, 16);
            _puts(str);
            break;
        }
        default:
            va_end(list);
            return -1;
        }
        // double increment since "%d" are double-chars
        i++;
    }

    va_end(list);
    return i;
}

/*
 * Sets a frame to the screen, so that when printing text, the
 * text going to scroll based on current frame.
 */
int activate_frame(struct frame_t *frame)
{
    cur_frame = frame;

    return 0;
}

/*
 * Disables a frame. Generally speaking this disables scrolling.
 */
int disable_frame()
{
    cur_frame = NULL;

    return 0;
}

/*
 * Gets current frame.
 */
struct frame_t *get_cur_frame()
{
    return cur_frame;
}

/*
 * Scrolls the text by `ln_cnt` using the provided frame.
 */
static int do_scroll(struct frame_t *frm, size_t ln_cnt)
{
    size_t crs_diff, i;
    unsigned int frame_loc_start, frame_loc_end;
    unsigned int new_frame_start;
    short *dest, *src;
    size_t sz;

    /* calculate frame cursor zone */
    frame_loc_start = frm->top * MAX_CRS_X;
    frame_loc_end = frm->bottom * MAX_CRS_X;
    new_frame_start = frame_loc_start + (ln_cnt * MAX_CRS_X);
    crs_diff = new_frame_start - frame_loc_start;

    /* move everything up by `ln_cnt` */
    dest = &((short *) VIDEO_MEMORY)[frame_loc_start];
    src = &((short *) VIDEO_MEMORY)[new_frame_start];
    sz = (frame_loc_end - crs_diff) * sizeof(short);
    memcpy((void *) dest, (void *) src, sz);

    /* clean the visible space which got shifted up */
    for (i = frame_loc_end - (ln_cnt * MAX_CRS_X); i < frame_loc_end; i++)
        ((short *) VIDEO_MEMORY)[i] = CHAR_TO_MEMVAL(' ');

    _cursor_loc -= _cursor_loc % MAX_CRS_X;

    return 0;
}

/*
 * Moves the cursor by `cnt`.
 * Returns 0 if moved by all `cnt`
 */
static int cursor_move(int cnt)
{
    int new_loc = _cursor_loc + cnt;
    if (new_loc < 0)
    {
        _cursor_loc = 0;
        return ABS(new_loc);
    }

    int max_loc = MAX_CRS_X * cur_frame->bottom;
    if (new_loc >= max_loc)
    {
        size_t ln_cnt = (new_loc - max_loc) / MAX_CRS_X + 1;
        do_scroll(cur_frame, ln_cnt);
    }
    else
        _cursor_loc = new_loc;

    return 0;
}

/*
 * Moves the cursor by `cnt` lines.
 * Returns 0 if moved by all
 */
static int cursor_move_line(int cnt)
{
    int move_by;

    if (cnt > 0)
        move_by = MAX_CRS_X - (_cursor_loc % MAX_CRS_X) + (cnt - 1) * MAX_CRS_X;
    else
        move_by = _cursor_loc % MAX_CRS_X + (cnt + 1) * MAX_CRS_X;
    return cursor_move(move_by);
}

/* 
 * Returns a pointer to the first occurance of 'c'
 */
char *kstrchr(char *str, char c)
{
	while (*str)
	{
		if (*str == c)
			return str;
		str++;
	}

	return NULL;
}

/*
 * Compares 2 null terminated char arrays.
 * Returns 0 if they are equal, 1 if the first is great, -1 otherwise.
 */
int kstrcmp(const char* str1, const char* str2)
{
    while (*str1 == *str2 && (*str1 != '\0' || *str2 != '\0'))
    {
        str1++;
        str2++;
    }

    if (*str1 == '\0' && *str2 == '\0')
        return 0;
    if (*str1)
        return 1;
    return -1;
}

/*
 * Concatinates `src` to `dest` and returns `dest`.
 * The caller is is responsible that `dest` has enough
 * space allocated to have `src` appended.
 * NOTE: the arguments are null-terminated char arrays.
 */
char *kstrcat(char *dest, const char *src)
{
    size_t i;
    size_t start_point;

    if (!dest || !src)
        return NULL;
    
    start_point = strlen(dest);

    for (i = start_point; *src; i++, src++)
        dest[i] = *src;
    dest[i] = '\0';

    return dest;
}

/*
 * Copies a string pointed by `src` to `dest`.
 * Caller is responsible for allocating enough space in `dest`.
 * Same is valid to free it.
 * Returns `dest`.
 */
char *kstrcpy(char *dest, const char *src)
{
    if (!dest || !src)
        return NULL;

    while (*src)
    {
        *dest = *src;
        dest++;
        src++;
    }
    *dest = '\0';

    return dest;
}

/*
 * Converts string to int.
 */
int katoi(const char *str)
{
    int digit = 0;
    int i, j;
    bool neg = false;

    if (*str == '-')
    {
        str++;
        neg = true;
    }

    for (i = 0, j = strlen(str) - 1; j >= 0; i++, j--)
        digit += (str[i] - 0x30) * kpow(10, j);

    return (neg ? -digit : digit);
}

/*
 * Converts int to null terminated string.
 * Returns `str` or EOF on error
 */
char *kitoa(int value, char *str, int base)
{
    static const char *tokens = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    bool neg = false;
    int i, n;

    if (base < 2 || base > 32)
        return str;

    /* TODO: 16-base should never be treated as negative */
    if (value < 0)
    {
        if (base == 10)
            neg = true;
        neg = true;
        value = -value;
    }

    i = 0;
    do {
        str[i++] = tokens[value % base];
        value /= base;
    } while (value);
    if (neg)
        str[i++] = '-';
    str[i--] = '\0';

    for (n = 0; n < i; n++, i--)
        SWAP(str[n], str[i]);

    return str;
}

/*
 * Returns base raised to the power of exponent
 */
int kpow(int base, int exp)
{
    int digit = base;

    if (exp == 0)
        return 1;

    while (exp-- > 1)
        digit *= base;
    return digit;
}


static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

static char *number(char *str, long num, int base, int size, int precision,
		    int type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	char tmp[66];
	char c, sign, locase;
	int i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = (digits[__do_div(num, base)] | locase);
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base == 8)
			*str++ = '0';
		else if (base == 16) {
			*str++ = '0';
			*str++ = ('X' | locase);
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}


static int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
	      repeat:
		++fmt;		/* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char)va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			//len = strnlen(s, precision);
                        len = strlen(s);
                        len = len > precision?precision:len;
                          
			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
				     (unsigned long)va_arg(args, void *), 16,
				     field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (short)num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str - buf;
}

int ksprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsprintf(buf, fmt, args);
	va_end(args);
	return i;
}

