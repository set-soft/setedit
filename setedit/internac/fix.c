#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *ReadFile(const char *name, long *len)
{
 char *ret;
 FILE *f=fopen(name,"rb");
 if (!f)
   {
    printf("Error opening %s\n",name);
    exit(2);
   }
 fseek(f,0,SEEK_END);
 *len=ftell(f);
 if (!*len)
   {
    printf("Empty file! (%s)\n",name);
    exit(3);
   }
 rewind(f);
 ret=(char *)malloc(*len+1);
 fread(ret,*len,1,f);
 ret[*len]=0;
 fclose(f);

 return ret;
}

int main(int argc, char *argv[])
{
 long lenHeader,lenInput,lenVer;
 char *header,*input,*version,*s,*newHeader,*prox,*pos;
 char date[80];
 time_t t;
 struct tm *stm;
 FILE *f;

 if (argc!=4)
   {
    printf("Use: fix header input output\n");
    return 1;
   }
 header=ReadFile(argv[1],&lenHeader);
 input=ReadFile(argv[2],&lenInput);
 version=ReadFile("../version.txt",&lenVer);

 for (s=version; *s; s++)
     if (*s=='\n' || *s=='\r')
       {
        *s=0;
        break;
       }
 t=time(0);
 stm=localtime(&t);
 strftime(date,80,"%Y-%m-%d %H:%M%z",stm);

 newHeader=(char *)malloc(lenHeader+80);
 pos=header;
 newHeader[0]=0;
 do
   {
    prox=strstr(pos,"@@");
    if (!prox)
       strcat(newHeader,pos);
    else
      {
       *prox=0;
       strcat(newHeader,pos);
       prox+=2;
       pos=strstr(prox,"@@");
       if (!pos)
         {
          printf("Error: Unclosed tag\n");
          return 4;
         }
       *pos=0;
       printf("Tag: %s\n",prox);
       if (strcmp(prox,"VERSION")==0)
         {
          strcat(newHeader,version);
         }
       else if (strcmp(prox,"DATE")==0)
         {
          strcat(newHeader,date);
         }
       else
         {
          printf("Unknown tag: %s\n",prox);
          return 5;
         }
       pos+=2;
      }
   }
 while (prox);

 //printf("%s",newHeader);
 f=fopen(argv[3],"wb");
 if (!f)
   {
    printf("Error creating %s\n",argv[3]);
    return 6;
   }
 // Write new header
 fwrite(newHeader,strlen(newHeader),1,f);
 // Skip the old header
 pos=strstr(input,"# End of header");
 if (pos)
   {// Old header found
    for (; *pos && *pos!='\n'; pos++);
    if (*pos=='\n') pos++;
    lenInput-=pos-input;
   }
 else
    pos=input;
 // Write the rest
 fwrite(pos,lenInput,1,f);
 fclose(f);
 return 0;
}
