/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define WINOLDAP_NoPresent 1
#define WINOLDAP_ClpInUse  2
#define WINOLDAP_TooBig    3
#define WINOLDAP_Memory    4
#define WINOLDAP_WinErr    5

extern int WINOLDAP_Init(void);
extern char *WINOLDAP_GetClipboard(unsigned long *size);
extern char *WINOLDAP_GetError(void);
extern int WINOLDAP_SetClipboard(char *p, unsigned long len);
