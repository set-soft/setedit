#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct
{
 char *file;
 char *chead;
 char *ctag;
 int rev;
 int type;
} stHeader;

const int maxLine=1024;
const int tfNorm=0, tfBin=1, tfMaskBinNorm=1, tfRemoved=2;

static
void ChopEOL(char *b)
{
 int l=strlen(b);
 if (b[l-1]=='\n') b[l-1]=0;
}

int GetHeader(FILE *f, const char *tag, stHeader *h)
{
 char b[maxLine],*rev;
 int tags=0;

 free(h->file);
 free(h->chead);
 free(h->ctag);
 h->file=h->chead=h->ctag=0;
 h->type=0;
 while (!feof(f))
   {
    if (fgets(b,maxLine-1,f))
      {
       ChopEOL(b);
       if (tags)
         {
          if (*b=='\t')
            {
             char *e=strchr(b,':');
             if (!e)
               {
                printf("Error, tag expected in:\n%s\n",b);
                return 0;
               }
             *e=0;
             if (strcmp(tag,b+1)==0)
               {
                free(h->ctag);
                h->ctag=strdup(e+2);
               }
             continue;
            }
          tags=0;
         }
       if (strncmp(b,"Working file",12)==0)
         {
          free(h->file);
          h->file=strdup(b+14);
         }
       else if (strncmp(b,"RCS file: ",10)==0)
         {
          if (strstr(b,"Attic"))
             h->type|=tfRemoved;
         }
       else if (strncmp(b,"head: ",6)==0)
         {
          free(h->chead);
          h->chead=strdup(b+6);
         }
       else if (strncmp(b,"symbolic names:",15)==0)
         {
          tags=1;
         }
       else if (strncmp(b,"keyword substitution: ",22)==0)
         {
          h->type|=*(b+22)=='b' ? tfBin : tfNorm;
         }
       else if ((rev=strstr(b,"selected revisions: "))!=0)
         {
          h->rev=atoi(rev+20);
         }
       else if (strncmp(b,"----------------------------",28)==0)
         return 1;
      }
   }
 return 0;
}

static
char *StrTok(char *str, char *from, int l)
{
 if (from-str>=l) return 0;
 char *s;
 for (s=from; *s && *s!='.'; s++);
 *s=0;
 return from;
}

static
int CompareRev(const char *r1, const char *r2)
{
 int l1=strlen(r1);
 char cr1[l1+1];
 strcpy(cr1,r1);
 int l2=strlen(r2);
 char cr2[l2+1];
 strcpy(cr2,r2);

 char *vr1=StrTok(cr1,cr1,l1);
 char *vr2=StrTok(cr2,cr2,l2);
 while (vr1 && vr2)
   {
    int ir1=atoi(vr1);
    int ir2=atoi(vr2);
    if (ir1>ir2)
       return 1;
    if (ir1<ir2)
       return -1;
    vr1=StrTok(cr1,vr1+strlen(vr1)+1,l1);
    vr2=StrTok(cr2,vr2+strlen(vr2)+1,l2);
   }
 if (vr1)
    return 1;
 if (vr2)
    return -1;
 return 0;
}

static
int CreateTmpFile(char *s)
{
 strcpy(s,"/tmp/csXXXXXX");
 return mkstemp(s);
}

typedef struct
{
 char *rev;
 long start,end;
} stLog;

const int maxLogs=64;
stLog Logs[maxLogs];
const int maxBinFiles=128;
char *BinFiles[maxBinFiles];
int cantBinFiles=0;

void DumpEntry(FILE *f, int index)
{
 fseek(f,Logs[index].start,SEEK_SET);
 long len=Logs[index].end-Logs[index].start;
 char buf[len+1];
 fread(buf,len,1,f);
 buf[len]=0;
 fputs(buf,stdout);
}

void ProcessFile(const char *file, const char *tag, const char *module,
                 const char *spFiles, int JustShow)
{
 FILE *f=fopen(file,"rt");
 if (!f)
   {
    printf("Error opening %s\n",file);
    return;
   }

 stHeader h;
 memset(&h,0,sizeof(h));
 char b[maxLine];
 int print,revsTot=0,newFiles=0;
 long pos;
 while (GetHeader(f,tag,&h))
   {
    int newFile=0;
    fprintf(stderr,"%s\n",h.file);
    if (!h.ctag)
      {
       if (h.type & tfRemoved)
         {
          //printf("Removed file? %s\n",h.file);
          h.ctag=strdup("999");
         }
       else
         {
          //printf("New file? %s\n",h.file);
          h.ctag=strdup("0");
          newFile=1;
          newFiles++;
         }
       fflush(stdout);
      }
    else
      {
       if (h.type & tfRemoved)
         {
          printf("Borrada %s\n",h.file);
          return;
         }
      }
    if (CompareRev(h.chead,h.ctag)==1)
      {
       printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
       printf("%sFile: \"%s\" (head: %s, tag==%s)\n",newFile ? "New " : "",h.file,
              h.chead,h.ctag);
       printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
      }
    int index=0,end=0;
    while (!end && !feof(f))
      {
       fgets(b,maxLine-1,f);
       ChopEOL(b);
       if (strncmp(b,"revision",8)!=0)
         {
          printf("Error revision expected in:\n%s\n",b);
          return;
         }
       print=0;
       if (CompareRev(b+9,h.ctag)>=0)
         {
          print=1;
          Logs[index].rev=strdup(b+9);
         }

       //if (print)
       //   fprintf(stderr,"  Revision %s:\n",Logs[index].rev);
       int start=0;
       while (!feof(f))
         {
          pos=ftell(f);
          if (fgets(b,maxLine-1,f))
            {
             ChopEOL(b);
             if (strncmp(b,"----------------------------",28)==0 && b[28]!='-')
                break;
             if (strncmp(b,"============================",28)==0)
               {
                end=1;
                break;
               }
             if (print && !start)
               {
                Logs[index].start=pos;
                start=1;
               }
            }
         }
       if (print)
         {
          Logs[index].end=pos;
          index++;
          if (index==maxLogs)
            {
             printf("Too much logs (%s)\n",index);
             return;
            }
         }
      }
    //if (CompareRev(h.chead,h.ctag)==1)
    //   printf("--------------------------\n");
    int minEntries=newFile ? 0 : 1;
    int orig=1,first=1,wasco;
    if (index>minEntries)
      {
       //printf("Volcando %d-1 entradas:\n",index);
       long pos=ftell(f);
       for (index-=minEntries+1; index>=0; --index)
          {
           if (JustShow)
              printf("* Log\n");
           else
              DumpEntry(f,index);
           if ((h.type & tfMaskBinNorm)!=tfBin)
             {
              printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
              if (newFile && orig)
                {
                 sprintf(b,"cvs -z 9 co -p -r %s %s/%s",Logs[index].rev,module,h.file);
                 orig=0;
                 wasco=1;
                }
              else
                {
                 sprintf(b,"cvs -z 9 diff -u -r %s -r %s %s",Logs[index+1].rev,Logs[index].rev,h.file);
                 revsTot++;
                 wasco=0;
                }
              if (JustShow)
                 puts(b);
              else
                {
                 fprintf(stderr,"%s\n",b);
                 FILE *d=popen(b,"r");
                 int start=wasco ? 1 : 0;
                 while (!feof(d))
                   {
                    if (fgets(b,maxLine-1,d))
                      {
                       if (!start && strncmp(b,"diff",4)==0)
                          start=1;
                       /*if (!start && strncmp(b,"***************",15)==0)
                         {
                          start=1;
                          continue;
                         }*/
                       if (start)
                          printf("%s",b);
                      }
                   }
                 pclose(d);
                }
              printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
             }
           if (newFile && first)
              first=0;
           else
              free(Logs[index+1].rev);
          }
       fseek(f,pos,SEEK_SET);
       if ((h.type & tfMaskBinNorm)==tfBin)
         {
          revsTot++;
          if (cantBinFiles==maxBinFiles)
            {
             printf("Too much binary files (%d)\n",cantBinFiles);
             return;
            }
          BinFiles[cantBinFiles++]=strdup(h.file);
         }
      }
    if (index)
       free(Logs[0].rev);
   }
 fprintf(stderr,"Total revisions: %d\nNew files: %d\n",revsTot,newFiles);
 // Take changed binaries and special files
 if (JustShow)
   {
    int i;
    for (i=0; i<cantBinFiles; i++)
        puts(BinFiles[i]);
   }
 else
   {
    char cft[16];
    int ft=CreateTmpFile(cft);
    if (cft>=0)
      {
       int i;
       for (i=0; i<cantBinFiles; i++)
          {
           write(ft,BinFiles[i],strlen(BinFiles[i]));
           write(ft," ",1);
          }
       write(ft,spFiles,strlen(spFiles));
       close(ft);
       sprintf(b,"tar zcf files.tar.gz `cat %s`",cft);
       fprintf(stderr,"%s\n",b);
       system(b);
       unlink(cft);
      }
   }
 fclose(f);
}


int main(int argc, char *argv[])
{
 if (argc!=4)
   {
    printf("cvssyncget Copyright (c) 2001 by Salvador E. Tropea\n");
    printf("Generates files needed to synchronize two cvs trunks\n\n");
    printf("Use: cvssyncget module tag special_files > dest_file\n\n");
    return 1;
   }
 char cft[16];
 int ft=CreateTmpFile(cft);
 if (ft==-1)
   {
    printf("Error creating temporal file\n");
    return 2;
   }
 char b[32];
 sprintf(b,"cvs log -b > %s",cft);
 system(b);

 //ProcessFile(cft,"i0447","setedit","makes/*.mak",1);
 //fprintf(stderr,"Tag: %s, proyecto: %s\n",argv[2],argv[1]);
 ProcessFile(cft,argv[2],argv[1],argv[3],0);

 unlink(cft);
 return 0;
}
