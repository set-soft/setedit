/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/* The following macros are defined to avoid passing negative values to
   the ctype functions in the common case: isxxxx(char).
   They are suitable for x86 DOS and Linux, perhaps not for others */
#define ucisalnum(a)  isalnum((unsigned char)a)
#define ucisalpha(a)  isalpha((unsigned char)a)
#define ucisascii(a)  isascii((unsigned char)a)
#define uciscntrl(a)  iscntrl((unsigned char)a)
#define ucisdigit(a)  isdigit((unsigned char)a)
#define ucisgraph(a)  isgraph((unsigned char)a)
#define ucislower(a)  islower((unsigned char)a)
#define ucisprint(a)  isprint((unsigned char)a)
#define ucispunct(a)  ispunct((unsigned char)a)
#define ucisspace(a)  isspace((unsigned char)a)
#define ucisupper(a)  isupper((unsigned char)a)
#define ucisxdigit(a) isxdigit((unsigned char)a)
#define uctoascii(a)  toascii((unsigned char)a)
#define uctolower(a)  tolower((unsigned char)a)
#define uctoupper(a)  toupper((unsigned char)a)
