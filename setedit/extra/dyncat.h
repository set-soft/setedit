/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifndef __DYNCAT_H__
#define __DYNCAT_H__
typedef struct
{
 int  len;
 char *str;
} DynStrCatStruct;

void DynStrCatInit(DynStrCatStruct *Struct, const char *str=NULL, int len=-1);
void DynStrCat(DynStrCatStruct *Struct, const char *str, int len=-1);

// Another useful stuff
char *newStrL(const char *start, int len);
int SplitStr(char *str, int copy, const char *delimiters,
             void (*adder)(const char *, void *), void *data);
#endif
