/* Copyright (C) 2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Description:
  This program reads information from a linker map (-Map name for ld or
-Wl,-M,name for gcc) and prints information about the size of the .text,
.data, .bss, .text+.data and rest of sections.
  Is very useful to know how much code comes from each static library.
  
***************************************************************************/

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

const int mxLen=PATH_MAX+80;
const int szCode=0, szData=1, szBss=2, szOther=3, szExe=4;
const int szCant=5;

const char *typeNames[szCant]=
{
 "code (text)",
 "data (data)",
 "unitialized data (bss)",
 "other (mostly debug)",
 "code+data (stripped exe size)"
};

struct node;

struct node
{
 char *name;
 int size[szCant];
 node *next;
};

static node *base=NULL;
static node *last;
static int nodes;
static int whichSort;

static
node *SearchModule(const char *mod)
{
 node *p=base;
 while (p)
   {
    if (strcmp(mod,p->name)==0)
       return p;
    p=p->next;
   }
 return p;
}

static
void AddSizeNode(node *p, const char *section, int size)
{
 if (strcmp(section,"text")==0)
   {
    p->size[szCode]+=size;
    p->size[szExe]+=size;
   }
 else if (strcmp(section,"data")==0)
   {
    p->size[szData]+=size;
    p->size[szExe]+=size;
   }
 else if (strcmp(section,"bss")==0)
    p->size[szBss]+=size;
 else
    p->size[szOther]+=size;
}

static
void AddToModule(char *mod, const char *section, int size)
{
 int l=strlen(mod);
 if (mod[l-2]=='.' && mod[l-1]=='o')
    strcpy(mod,"object files");
 if (!base)
   {
    base=new node;
    memset(base,0,sizeof(node));
    base->next=NULL;
    base->name=strdup(mod);
    AddSizeNode(base,section,size);
    last=base;
    nodes=1;
    return;
   }
 node *p=SearchModule(mod);
 if (p)
    AddSizeNode(p,section,size);
 else
   {
    last->next=new node;
    last=last->next;
    memset(last,0,sizeof(node));
    last->next=NULL;
    last->name=strdup(mod);
    AddSizeNode(last,section,size);
    nodes++;
   }
}

static
void ParseAndAdd(char *s, char *section)
{
 for (;*s && *s==' '; s++);
 if (*s)
   {
    int modStart,modSize;
    char mod[256];
    sscanf(s,"0x%x      0x%x %256[^(\n]",&modStart,&modSize,mod);
    //printf("Start: 0x%X Size: 0x%X module: %s\n",modStart,modSize,mod);
    AddToModule(mod,section,modSize);
   }
}

static
int Compare(const void *e1, const void *e2)
{
 const node **p1=(const node **)e1;
 const node **p2=(const node **)e2;
 return (*p2)->size[whichSort]-(*p1)->size[whichSort];
}

static
node **SortList(int which)
{
 node **table=new node *[nodes], *p;
 int i;
 for (p=base, i=0; i<nodes; p=p->next, i++)
     table[i]=p;
 whichSort=which;
 qsort(table,nodes,sizeof(node *),Compare);
 return table;
}

static
void PrintList(int which)
{
 node **p=SortList(which);
 int total=0, i;
 for (i=0; i<nodes; i++)
     total+=p[i]->size[which];
 printf("\n\nValues for section: %s (size=%d)\n\n",typeNames[which],total);
 for (i=0; i<nodes; i++)
     printf("%-40s     %8d (%6.2f %%)\n",p[i]->name,p[i]->size[which],
            p[i]->size[which]/(double)total*100.0);
 printf("----------------------------------------------------------------\n");
 printf("%-40s     %8d (%6.2f %%)\n","Total",total,100.0);
 delete[] p;
}

int main(int argc, char *argv[])
{
 FILE *f;
 char b[mxLen];
 char secName[20];
 f=fopen(argv[1],"rt");
 if (!f)
   {
    printf("Can't open %s\n",argv[1]);
    return 1;
   }
 while (!feof(f))
   {
    if (fgets(b,mxLen,f) && b[0]=='.' && b[1]!='.' && b[1]!='/')
       break;
   }
 if (feof(f))
   {
    printf("Can't find any section\n");
    return 2;
   }
 int secStart,secSize;
 do
   {
    int re=sscanf(b,".%20s           0x%x   0x%x",secName,&secStart,&secSize);
    int skip=0;
    if (re!=3)
      {
       if (re==1)
         {
          fgets(b,mxLen,f);
          char *s=b;
          for (;*s && *s==' '; s++);
          re=sscanf(s,"0x%x      0x%x",&secStart,&secSize);
          if (re!=2)
            {
             skip=1;
             printf("Skipping %s\n",secName);
            }
         }
       else
         {
          printf("Wrong section format (1)\n");
          return 3;
         }
      }
    if (!skip)
       printf("Section: %s Start: 0x%X Size: %d\n",secName,secStart,secSize);
    int useNext=0;
    do
      {
       if (fgets(b,mxLen,f))
         {
          if (useNext)
            {
             useNext=0;
             ParseAndAdd(b,secName);
            }
          else if (b[0]==' ' && b[1]=='.')
            {
             char *s=b+2;
             for (;*s && *s!=' '; s++);
             if (*s)
               {
                for (;*s && *s==' '; s++);
                if (*s)
                   ParseAndAdd(s,secName);
               }
             else
                useNext=1;
            }
          else if (b[0]=='.')
            break;
         }
      }
    while (!feof(f));
   }
 while (!feof(f));
 int i;
 for (i=0; i<szCant; i++)
     PrintList(i);
 return 0;
}

