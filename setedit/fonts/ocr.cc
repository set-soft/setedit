/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include "common1.h"
#include "ocr0.000"

Font BoxRound16={0,NUM_FONTS,16,BR16};
Font BoxRound14={0,NUM_FONTS,14,BR14};

char *Name="OCR style";

#define PREFIX BR
#define PREFIXS "BR"
#define NAME_FONT "ocr.sft"
#define ARRAY1 BoxRound16
#define ARRAY2 BoxRound14
#define NAME_PART "ocr%d.%03d"

#include "common2.h"
