#include "common.h"

public int atoi(const char *str)
{
    int digit = 0;
    int i, j;
    bool neg = false;

    if (*str == '-')
    {
        str++;
        neg = true;
    }

    for (i = 0, j = kstrlen(str) - 1; j >= 0; i++, j--)
        digit += (str[i] - 0x30) * kpow(10, j);

    return (neg ? -digit : digit);
}

/*
 * Converts int to null terminated string.
 * Returns `str` or EOF on error
 */
 public char *itoa(int value, char *str, int base)
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

public int skip_atoi(const char **s)
{
 	  int i = 0;

 	  while (ISDIGIT(**s))
 		    i = i * 10 + *((*s)++) - '0';
 	  return i;
}
