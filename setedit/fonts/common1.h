/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <conio.h>

#define NUM_FONTS 587
typedef struct
{
 int first, last, lines;
 unsigned char *font;
} Font;

char *Signature="SET's editor font\x1A";

