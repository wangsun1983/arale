#include "sys_call.h"
#include "common.h"

private int _puts(const char *text);
private void put_tab();

public int logdisp(const char *format, ...)
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
              sys_putchar(format[i]);
              continue;
          }

          switch (format[i+1]) {
          /* double-% */
          case ('%'):
              sys_putchar('%');
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
              sys_putchar(val);
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

private int _puts(const char *text)
{
    while (*text)
        if (kputchar(*text++) == EOF)
            return EOF;
    return 0;
}

/*
 * Prints tab space
 */
private void put_tab()
{
    int i;

    for (i = 0; i < TAB_SIZE; i++)
        kputchar(' ');
}
