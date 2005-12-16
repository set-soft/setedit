/* Copyright (C) 1996-1998 Robert H”hne, see COPYING.RH for details */
/* This file is part of RHIDE. */
#define Uses_fixpath
#define Uses_string
#define Uses_stdio
#include <compatlayer.h>
#include <rhutils.h>

static char _static_buffer[2048]; // this should be enough

void FExpand(char * & name,int new_alloc)
{
  char *fname = _static_buffer;
  _fixpath(name,fname);
  if (new_alloc)
  {
    string_free(name);
    string_dup(name,fname);
  }
  else strcpy(name,fname);
}



