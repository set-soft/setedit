/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/* This file is an adaptation of idespecs.cc from Robert Hoehne to make the editor */
/* more coherent with RHIDE and more easy to configure (?)                         */

#define Uses_string
#include <stdlib.h>
#include <dir.h>
#include <stdio.h>
#include <compatlayer.h>
#ifndef TVComp_BCPP
extern char **environ;
#endif

static char *default_variables[] =
{
#if defined(TVCompf_djgpp) || defined(TVComp_BCPP)
 // In DOS is common to have C:
 "SET_FILES",
 "c:/etc",
#else
 // For UNIX the right place to put these files is the etc dir
 "SET_FILES",
 "/etc",
#endif

 0,
 0
};

static char **vars;
static int var_count = 0;

/**[txh]**********************************************************************

 Function: add_variable
 Module:   Editor Specs
 Include: edspecs.h
 Prototype: void add_variable(char *variable,char *contents)
 Description:
   Adds a variable to the variables list.

*****************************************************************************/

static
void add_variable(char *variable,char *contents)
{
 var_count++;
 vars = (char **)realloc(vars,var_count*2*sizeof(char *));
 vars[var_count*2-2]=strdup(variable);
 vars[var_count*2-1]=strdup(contents);
}

static
void insert_variable(char *variable,char *contents)
{
 int i;
 for (i=0;i<var_count;i++)
 {
   if (strcmp(vars[i*2],variable) == 0)
   {
     free(vars[2*i+1]);
     vars[2*i+1]=strdup(contents);
     return;
   }
 }
 add_variable(variable,contents);
}

/**[txh]**********************************************************************

 Function: GetVariable
 Prototype: GetVariable(const char *variable)
 Description:
   Gets the contents of a variable.

 Return: a pointer to the variable contents.

*****************************************************************************/

//static
const char * GetVariable(const char *variable)
{
 int i;
 for (i=0;i<var_count;i++)
    {
     if (strcmp(variable,vars[i*2]) == 0)
        return vars[i*2+1];
    }
 return getenv(variable);
}

// The following function is defined as a structure initializer to be called
// as a constructor by BC++, but it gives a warning about the fact that even
// when declared inline won't be inlined. The following pragma disables it.
#pragma option push -w-inl

struct init_edspecs_t
{
init_edspecs_t()
{
  char *variable,*contents;
  int i=0;
  while (default_variables[i])
  {
    variable = default_variables[i];
    contents = getenv(variable);
    if (!contents) contents = default_variables[i+1];
    add_variable(variable,contents);
    i += 2;
  }
  // Now check the env for any SET_ variable
  for (i=0;environ[i];i++)
  {
    if (strncmp(environ[i],"SET_",6) == 0)
    {
      contents = strchr(environ[i],'=');
      if (!contents) continue;
      contents++;
      char var[256];
      memcpy(var,environ[i],(int)(contents-environ[i])-1);
      var[(int)(contents-environ[i])-1] = 0;
      insert_variable(var,contents);
    }
  }
}
} init_edspecs;

#pragma option pop
