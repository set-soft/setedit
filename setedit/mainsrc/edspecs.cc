/* Copyright (C) 2001-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/* This file is an adaptation of idespecs.cc from Robert Hoehne to make the editor */
/* more coherent with RHIDE and more easy to configure (?)                         */

#define Uses_string
#define Uses_stdlib
#define Uses_stdio
#define Uses_limits
#include <pathtool.h>
#include <tv.h>

#include <ced_inte.h>
#include <edspecs.h>

#ifndef TVComp_BCPP
// BC++ defines it in stdlib.h
extern char **environ;
#endif

static char *default_variables[]=
{
 "SET_CONFQUIT","0",
 "SET_CREATE_DST","1",
 0,0
};

static char **vars;
static int var_count = 0;

static
void add_variable(const char *variable, const char *contents)
{
 var_count++;
 vars = (char **)realloc(vars,var_count*2*sizeof(char *));
 vars[var_count*2-2]=strdup(variable);
 vars[var_count*2-1]=contents ? strdup(contents) : NULL;
}

void InsertEnvironmentVar(const char *variable, const char *contents)
{
 int i;
 for (i=0;i<var_count;i++)
 {
   if (strcmp(vars[i*2],variable)==0)
   {
     free(vars[2*i+1]);
     vars[2*i+1]=contents ? strdup(contents) : NULL;
     return;
   }
 }
 add_variable(variable,contents);
}

const char *GetVariable(const char *variable, const char *def)
{
 int i;
 for (i=0;i<var_count;i++)
    {
     if (strcmp(variable,vars[i*2])==0 && vars[i*2+1])
        return vars[i*2+1];
    }
 const char *env=getenv(variable);
 return env ? env : def;
}

/**[txh]********************************************************************

  Description:
  Returns the settings for the .dst files: create only one, create it
hidden, etc. Called by edprj.cc

  Return:
  The flags as defined in edspecs.h.

***************************************************************************/

unsigned GetDSTOptions()
{
 return EnvirGetIntVar("SET_CREATE_DST",dstCreate);
}

unsigned EnvirGetIntVar(const char *name, unsigned aDefault)
{
 const char *s=GetVariable(name);
 if (!s) return aDefault;
 char *end;
 return (unsigned)strtol(s,&end,0);
}

unsigned EnvirGetBits(const char *name, unsigned Value, unsigned aDefault)
{
 return EnvirGetIntVar(name,aDefault) & Value;
}

void EnvirSetIntVar(const char *name, unsigned Value)
{
 char buf[32];
 sprintf(buf,"0x%X",Value);
 InsertEnvironmentVar(name,buf);
}

unsigned EnvirSetBits(const char *name, unsigned Value)
{
 unsigned v=EnvirGetIntVar(name);
 v|=Value;
 EnvirSetIntVar(name,v);
 return v;
}

unsigned EnvirResetBits(const char *name, unsigned Value)
{
 unsigned v=EnvirGetIntVar(name);
 v&= ~Value;
 EnvirSetIntVar(name,v);
 return v;
}

static char Signature[]="SET's editor enviroment\x1A";

static
void fGetStr(char *s, FILE *f)
{
 ushort len;
 fread(&len,sizeof(ushort),1,f);
 if (!feof(f))
   {
    fread(s,len,1,f);
    s[len]=0;
   }
 else
    *s=0;
}

static
void ReadEnviromentFile(void)
{// ExpandHome so it also tries all locations (~/.setenvir.dat)
 char *s=ExpandHome("setenvir.dat");

 if (s)
   {
    char Name[PATH_MAX];
    char Val[PATH_MAX];
    FILE *f=fopen(s,"rb");
    if (f)
      {
       fread(Name,sizeof(Signature),1,f);
       if (strcmp(Name,Signature)==0)
         {
          do
            {
             fGetStr(Name,f);
             fGetStr(Val,f);
             if (!feof(f))
                InsertEnvironmentVar(Name,Val);
            }
          while (!feof(f));
         }
       fclose(f);
      }
   }
}

static
void fPutVar(char *var,FILE *f)
{
 char *c=(char *)GetVariable(var);
 if (c && c[0])
   {
    ushort len=strlen(var);
    fwrite(&len,sizeof(ushort),1,f);
    fwrite(var,len,1,f);
    len=strlen(c);
    fwrite(&len,sizeof(ushort),1,f);
    fwrite(c,len,1,f);
   }
}

void SaveEnviromentFile(void)
{
 char *s=ExpandHomeSave("setenvir.dat");
 if (s)
   {
    FILE *f=fopen(s,"wb");
    if (f)
      {
       fwrite(Signature,sizeof(Signature),1,f);
       fPutVar("SET_CONFQUIT",f);
       fPutVar("SET_VERUSED",f);
       fPutVar("SET_CREATE_DST",f);
       fPutVar("SET_TIP_INFO",f);
       fPutVar("SET_README_SHOWN",f);
       fPutVar("SET_TIPS1",f);
       fPutVar("SET_VARIOUS1",f);
       fPutVar("SET_FORCED_LANG",f);
       fPutVar("SET_GDB_TIME_OUT",f);
       fPutVar("SET_GDB_MSG_LINES",f);
       fPutVar("SET_GDB_MISC",f);
       fPutVar("SET_GDB_EXE",f);
       fPutVar("SET_XTERM_EXE",f);
       fclose(f);
      }
   }
}

//static __attribute__ ((__constructor__))
void InitEnvirVariables(void)
{
 char *variable,*contents;
 int i=0;
 // The lower priority is the one of the hardcoded
 while (default_variables[i])
   {
    variable = default_variables[i];
    contents = default_variables[i+1];
    add_variable(variable,contents);
    i += 2;
   }
 // Now read the binary enviroment file
 ReadEnviromentFile();
 // Now check the env for any SET_ variable (higher priority)
 for (i=0;environ[i];i++)
    {
     if (strncmp(environ[i],"SET_",4)==0)
       {
        contents = strchr(environ[i],'=');
        if (!contents) continue;
        contents++;
        char var[256];
        memcpy(var,environ[i],(int)(contents-environ[i])-1);
        var[(int)(contents-environ[i])-1] = 0;
        InsertEnvironmentVar(var,contents);
       }
    }
 // Now fix old version flags to new version
 // First try to figure out what version wrote the file
 unsigned version=EnvirGetIntVar("SET_VERUSED");
 // First time?
 //if (!version) return;
 // Is this version or newer?
 if (version>=TCEDITOR_VERSION) return;
 // An old version, mark the new one
 EnvirSetIntVar("SET_VERUSED",TCEDITOR_VERSION);
 // Force to see the start about
 EnvirResetBits("SET_VARIOUS1",svr1DontShowAbout);
 // In 0.4.47 I introduced a better control for what is saved/restored
 // to/from the dst file/s.
 if (version<0x447)
   {
    unsigned opsDst=GetDSTOptions();
    // Have dstOpenSpec enabled?
    if (opsDst & 4)
      {// Remove the old flag that now have another meaning
       opsDst&= ~4;
       // Set the equivalent options
       opsDst|=dstEdOnlyIfNoCL | dstOwOnlyIfNoCL | dstCfEver;
       EnvirSetIntVar("SET_CREATE_DST",opsDst);
      }
   }
}

// Is ever good to have a way to destroy all the allocated memory and helps to
// find leaks.
void DeInitEnvirVariables(void)
{
 int i;
 for (i=0;i<var_count;i++)
    {
     free(vars[i*2]);
     free(vars[i*2+1]);
    }
 free(vars);
 vars=0;
 var_count=0;
}
