/* Copyright (C) 1996-1998 Robert H”hne, see COPYING.RH for details */
/* This file is part of RHIDE. */
#include <stdlib.h>
#include <stdio.h>
#include <rhutils.h>

void string_free(char * &dest)
{
  if (dest) free(dest);
  dest = NULL;
}


