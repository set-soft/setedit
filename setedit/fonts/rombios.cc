/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include "common1.h"
#include "rombios0.000"

Font RomBIOS16={0,NUM_FONTS,16,BR16};
Font RomBIOS14={0,NUM_FONTS,14,BR14};

char *Name="ROM BIOS";

#define PREFIX BR
#define PREFIXS "BR"
#define NAME_FONT "rombios.sft"
#define ARRAY1 RomBIOS16
#define ARRAY2 RomBIOS14
#define NAME_PART "rombios%d.%03d"

#include "common2.h"
