/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include "common1.h"
#include "thin0.000"

Font BoxRound16={0,NUM_FONTS,16,BR16};
Font BoxRound14={0,NUM_FONTS,14,BR14};

char *Name="Thin letters";

#define PREFIX BR
#define PREFIXS "BR"
#define NAME_FONT "thin.sft"
#define ARRAY1 BoxRound16
#define ARRAY2 BoxRound14
#define NAME_PART "thin%d.%03d"

#include "common2.h"
