/* Copyright (C) 1996-2017 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdio.h>
#define Uses_string
#define Uses_ctype
#define Uses_ftell
#define Uses_TPMCollection
#define Uses_MsgBox
#define Uses_snprintf
#define Uses_AllocLocal
#include <ceditor.h>
#include <dyncat.h>

static
char *FGets(char *s, int n, FILE *f)
{
 char *ret=fgets(s,n,f);
 if (ret)
   {
    int l=strlen(s);
    if (l>1 && s[l-2]=='\r' && s[l-1]=='\n')
      {
       s[l-2]='\n';
       s[l-1]=0;
      }
   }
 return ret;
}

/****************************************************************************

   Pseudo Macros

****************************************************************************/


ccIndex TPMCollection::insert(void *item)
{
 // Fill the vars field
 PMacroStr *pm=(PMacroStr *)item;
 char *s=pm->str;
 for (; *s; s++)
    {
     if (*s=='@' && s[1]=='{' && s[2]!='(')
       {
        s+=2;
        char *start=s;
        for (; *s && *s!='}'; s++);
        if (*s=='}')
          {
           if (!pm->vars)
             {
              pm->vars=new TNSCollection(2,2);
              pm->defaults=new TNSCollection(2,2);
              pm->mLenVar=0;
             }
           // Extract the variable
           char *aux=newStrL(start,s-start);
           char *val=strtok(aux,";");
           unsigned lenVal=strlen(val);
           pm->vars->insert(newStrL(val,lenVal));
           if (lenVal>pm->mLenVar)
              pm->mLenVar=lenVal;
           //printf("Var: %s %d\n",val,lenVal);
           // Default value?
           val=strtok(NULL,";");
           pm->defaults->insert(newStr(val));
           //printf("Default: %s\n",val);
           // Free the temporal
           delete[] aux;
          }
       }
    }
 // Insert it
 return TStringCollection::insert(item);
}

void TPMCollection::freeItem(void *p)
{
 PMacroStr *s=(PMacroStr *)p;
 delete[] s->str;
 delete[] s->name;
 destroy(s->vars);
 destroy(s->defaults);
 delete s;
}

void *TPMCollection::keyOf(void *p)
{
 return ((PMacroStr *)p)->trigger;
}

void TPMCollection::getText(char *dest, unsigned item, int maxLen)
{
 PMacroStr *s=(PMacroStr *)(at(item));
 char *aux=NULL;
 if (!s->name && !s->str)
    aux=TVIntl::getTextNew(__("Unknown"));
 strncpyZ(dest,s->name ? s->name : (s->str ? s->str : aux),maxLen);
 DeleteArray(aux);
}

PMacroStr *TPMCollection::searchByNamePointer(void *name)
{
 ccIndex i,c=getCount();

 for (i=0; i<c; i++)
    {
     PMacroStr *p=(PMacroStr *)at(i);
     if (p->name==name)
        return p;
    }
 return NULL;
}

static
int MeassureTriLine(char *b,unsigned &s,unsigned &e)
{
 int l;
 char *p=b;

 for (; *b && *b!='"'; b++);
 if (*b==0) return -1;
 s=(unsigned)(b-p);
 b++;
 for (l=0; *b && *b!='"'; b++)
    {
     if (*b=='\\')
       {
        if (*(b+1)==0)
           return -2;
        b++;
       }
     l++;
    }
 if (*b==0) return -2;
 e=(unsigned)(b-p);
 return l;
}

Boolean LoadPseudoMacroFile(char *name, TPMCollection *coll)
{
 FILE *f;
 char buf[256],*s;
 const char *sep="\"";
 int Trs=0,partial,loaded;
 unsigned l,start=0,end=0,total=0;
 unsigned mode,bit;
 PMacroStr *nDef;
 long startDef;

 if (!coll || (f=fopen(name,"rb"))==NULL)
    return False;

 FGets(buf,250,f);
 do
  {
   if (buf[0]==0 || buf[0]==';' || buf[0]=='\n')
     {
      FGets(buf,250,f);
      continue;
     }
   if (strncmp(buf,"Trigger:",8)!=0)
     {
      messageBox(__("Macro definition doesn't start with Trigger"),mfError | mfOKButton);
      fclose(f);
      return False;
     }
   strtok(buf,sep);
   s=strtok(NULL,sep);
   if (s==NULL || strlen(s)!=2)
     {
      messageBox(__("Missing Trigger sequence or too short"),mfError | mfOKButton);
      fclose(f);
      return False;
     }
   nDef=new PMacroStr;
   if (!nDef)
     {
      fclose(f);
      return False;
     }
   nDef->trigger[0]=s[0];
   nDef->trigger[1]=s[1];
   nDef->trigger[2]=0;
   nDef->vars=NULL;
   nDef->defaults=NULL;

   // Process the mode keyword
   FGets(buf,250,f);
   if (strncmp(buf,"Mode:",5)!=0)
     {
      messageBox(__("Macro definition without mode keyword"),mfError | mfOKButton);
      fclose(f);
      return False;
     }
   for (s=buf+5; *s!=0 && ucisspace(*s); s++);
   for (l=0,mode=0,bit=1; l<32; l++,bit<<=1)
      {
       for (; *s && ucisspace(*s); s++);
       if (!*s) break;
       if (*s!='0' && *s!='1')
         {
          messageBox(__("Wrong mode definition in pseudo macro."),mfError | mfOKButton);
          fclose(f);
          return False;
         }
       if (*s=='1')
          mode|=bit;
       for (s++; *s && ucisspace(*s); s++);
       if (!*s) break;
       if (*s!=',')
         {
          messageBox(__("Wrong mode definition in pseudo macro."),mfError | mfOKButton);
          fclose(f);
          return False;
         }
       s++;
      }
   nDef->flags=mode;
   nDef->str=0;  // Is loaded later
   nDef->name=0; // idem

   startDef=ftell(f);
   // Check if it have a name
   loaded=1;
   FGets(buf,250,f);
   if (strncmp(buf,"Name:",5)==0)
     {
      loaded=0;
      for (s=buf+5; *s && ucisspace(*s); s++);
      char *aux;
      for (aux=s; *aux && *aux!='\n'; aux++);
      *aux=0;
      int len=aux-s;
      if (len)
        {
         aux=nDef->name=new char[len+1+5];
         strcpy(aux,s);
         aux[len+0]=' ';
         aux[len+1]='[';
         aux[len+2]=nDef->trigger[0];
         aux[len+3]=nDef->trigger[1];
         aux[len+4]=']';
         aux[len+5]=0;
        }
      startDef=ftell(f);
     }
  // End of optional name

   l=0;
   while (!feof(f))
     {
      if (loaded)
         loaded=0;
      else
         FGets(buf,250,f);
      if (feof(f)) break;
      if (buf[0]==0 || buf[0]==';' || buf[0]=='\n')
         continue;
      if (buf[0]=='T') break;
      partial=MeassureTriLine(buf,start,end);
      switch (partial)
        {
         case -1:
              messageBox(__("Missing start in pseudo macro"),mfError | mfOKButton);
              fclose(f);
              return False;

         case -2:
              messageBox(__("Missing end in pseudo macro"),mfError | mfOKButton);
              fclose(f);
              return False;

         default: l+=partial;
        }
     }
   if (l==0)
     {
      messageBox(__("Empty pseudo macro"),mfError | mfOKButton);
      fclose(f);
      return False;
     }
   fseek(f,startDef,SEEK_SET);
   s=nDef->str=new char[l+1];
   while (!feof(f))
     {
      FGets(buf,250,f);
      if (feof(f)) break;
      if (buf[0]==0 || buf[0]==';' || buf[0]=='\n')
         continue;
      if (buf[0]=='T') break;
      MeassureTriLine(buf,start,end);
      int in_slash;
      for (in_slash=0,l=start+1; l<end; l++)
         {
          if (!in_slash && buf[l]=='\\')
             in_slash=1;
          else
            {
             if (in_slash)
               {
                in_slash=0;
                switch (buf[l])
                  {
                   case 'a': *s='\a';
                             break;
                   case 'f': *s='\f';
                             break;
                   case 'v': *s='\v';
                             break;
                   case '\\': *s='\\';
                             break;
                   case 'n': *s='\n';
                             break;
                   case 'b': *s='\b';
                             break;
                   case 't': *s='\t';
                             break;
                   default: *s=buf[l];
                  }
               }
             else
                *s=buf[l];
             s++;
            }
         }
     }
   *s=0;
   if (!nDef->name)
     {// Use a default name is none is provided
      size_t len=CLY_snprintf(NULL,(size_t)0,"%s [%c%c]",__("No name"),nDef->trigger[0],
                              nDef->trigger[1]);
      char *name=new char[len+1];
      CLY_snprintf(name,len+1,"%s [%c%c]",__("No name"),nDef->trigger[0],nDef->trigger[1]);
      nDef->name=name;
     }
   coll->insert(nDef);
   Trs++;
   total+=l+1;
  }
 while (!feof(f));
 fclose(f);
 return True;
}


