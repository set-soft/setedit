/* Copyright (C) 1996-1998 Robert H”hne, see COPYING.RH for details */
/* Copyright (C) 1998-2005 Salvador E. Tropea */
/* This file is part of RHIDE. */
#define Uses_string
#define Uses_unistd
#define Uses_stdio
#define Uses_stdlib
#define Uses_sys_stat
#define Uses_fcntl
#define Uses_free
#define Uses_mkstemp
#include <compatlayer.h>
#include <rhutils.h>

#define STDOUT 1
#define STDERR 2

static char *errname = NULL;
static char *outname = NULL;
static char *erroutname = NULL;
static int h_out,h_outbak;
static int h_err,h_errbak;

/* returns a malloced unique tempname in $TMPDIR */
char *unique_name(const char *before, char *retval)
{
 char *name;
 int h=unique_name(0,name,before,retval);
 if (h!=-1)
    close(h);
 return name;
}

/* returns a malloced unique tempname in $TMPDIR */
FILE *unique_name_f(char *&retname, const char *before, char *retval)
{
 char *name;
 int h=unique_name(1,name,before,retval);
 if (h!=-1)
   {
    retname=name;
    return fdopen(h,"w+");
   }
 return NULL;
}

/* returns a malloced unique tempname in $TMPDIR */
int unique_name(int remove, char *&retname, const char *before, char *retval)
{
  char *name;
  const char *tmp=getenv("TMPDIR");
  // SET: The next 2 fallbacks aren't usually needed because RHIDE and SETEdit
  // defines TMPDIR. SAA added them and I keep it because they make more
  // robust the code if anybody uses the library in another project.
  if (!tmp) tmp=getenv("TEMP");
  if (!tmp) tmp=getenv("TMP");
  if (!tmp) tmp=".";
  int l=strlen(tmp);
  if (retval)
  {
    strcpy(retval,tmp);
    if (!CLY_IsValidDirSep(tmp[l-1]))
       strcat(retval,DIRSEPARATOR_);
    strcat(retval,before);
    strcat(retval,"XXXXXX");
    name = string_dup(retval);
  }
  else
  {
    string_dup(name,tmp);
    if (!CLY_IsValidDirSep(tmp[l-1]))
       string_cat(name,DIRSEPARATOR_);
    string_cat(name,before);
    string_cat(name,"XXXXXX");
  }
  /* SET: Modified to use mkstemp because mktemp is dangerous */
  int handler=mkstemp(name);
  if (handler!=-1)
  {
    if (retval) strcpy(retval,name);
    retname=name;
    #ifdef TVOS_UNIX
    if (remove) unlink(name);
    #endif
    return handler;
  }
  free(name);
  retname=NULL;
  return -1; // What to do here?
}

char *open_stderr(int *nherr)
{
  if (errname) free(errname);
  errname = unique_name("er");
  h_err = open (errname,O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
                        S_IREAD | S_IWRITE);
  h_errbak = dup (STDERR);
  fflush(stderr);  /* so any buffered chars will be written out */
  dup2 (h_err, STDERR);
  if (nherr)
     *nherr=h_err;
  return errname;
}

char *open_stderr(void)
{
  return open_stderr(NULL);
}

void close_stderr(void)
{
  dup2 (h_errbak, STDERR);
  close (h_err);
  close (h_errbak);
}

char *open_stdout(void)
{
  if (outname) free(outname);
  outname = unique_name("ou");
  h_out = open (outname,O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
                        S_IREAD | S_IWRITE);
  h_outbak = dup (STDOUT);
  fflush(stdout);  /* so any buffered chars will be written out */
  dup2 (h_out, STDOUT);
  return outname;
}

void close_stdout(void)
{
  dup2 (h_outbak, STDOUT);
  close (h_out);
  close (h_outbak);
}

char *open_stderr_out(int *nherr)
{
 if (erroutname) free(erroutname);
 erroutname = unique_name("eo");
 h_err = open (erroutname,O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
                       S_IREAD | S_IWRITE);
 h_errbak = dup(STDERR);
 h_outbak = dup(STDOUT);
 fflush(stderr);  /* so any buffered chars will be written out */
 fflush(stdout);  /* so any buffered chars will be written out */
 if (nherr)
    *nherr=h_err;
 return erroutname;
}

char *open_stderr_out(void)
{
 open_stderr_out(NULL);
 dup2(h_err,STDERR);
 dup2(h_err,STDOUT);
 return erroutname;
}

void close_stderr_out(void)
{
 dup2(h_errbak,STDERR);
 dup2(h_outbak,STDOUT);
 close (h_err);
 close (h_errbak);
 close (h_outbak);
}

void op_cl_std_clean_up(void)
{
 free(errname);
 free(outname);
 free(erroutname);
}

