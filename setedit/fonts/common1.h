/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#include <string.h>
#include <limits.h>
//#include <conio.h>

// 0 - NUM_FONTS!
#define NUM_FONTS 632
typedef struct
{
 int first, last, lines;
 unsigned char *font;
} Font;

char *Signature="SET's editor font\x1A";

