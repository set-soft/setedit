/**[txh]********************************************************************

  Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
  This program is covered by the GPL license you should get it with this
program.
  I based some routines (my_*) in Bjorn Reese <breese@mail1.stofanet.dk>
code, but to be honest that's standard UNIX process manipulation. But I
must give credit to Bjorn, your ideas helped me a lot in creating the code
that attaches gdb to setedit to retrieve more information.

  Description:
  That's a small tool to convert a raw stack trace into human readable
source points. The output format is the standard GNU errors one. It means
you can feed setedit with it and browse the crashing points.
  Is the equivalent to djgpp symify tool for Linux.
  Note that the stack traces I'm generating are incomplete because I can't
show the exact point of the signal, I should investigate how in the hell
gdb achieves it. Any help will be appreciated.

  Compilation:
  g++ -o symify -O2 symify.cc
  
***************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

const int maxLineL=512;
typedef unsigned long ulong;
typedef unsigned char uchar;
#define SYS_ERROR -1

/**[txh]********************************************************************

  Description:
  Runs the command as a child process redirecting the output, error and
input to a us a.
  By SET.
  
  Return:
  The file handle of a pipe connected to the child.
  
***************************************************************************/
static
int my_popen(const char *command, pid_t *pid, int *aWc)
{
 int rc,wc;
 int pipefd[2],pipefd2[2];

 /* To make it bidireccional we need 2 pipes */
 rc=pipe(pipefd);
 if (rc==SYS_ERROR)
    return rc;
 wc=pipe(pipefd2);
 if (wc==SYS_ERROR)
   {
    close(pipefd[0]);
    close(pipefd[1]);
    return wc;
   }
 if (rc!=SYS_ERROR && wc!=SYS_ERROR)
   {
    *pid=fork();
    switch (*pid)
      {
       case SYS_ERROR:
            wc = rc = SYS_ERROR;
            close(pipefd[0]);
            close(pipefd[1]);
            close(pipefd2[0]);
            close(pipefd2[1]);
            break;
         
       case 0: /* Child */
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            dup2(pipefd[1], STDERR_FILENO);
            close(pipefd[1]);

            close(pipefd2[1]);
            dup2(pipefd2[0], STDIN_FILENO);
            close(pipefd2[0]);
            /*
             * The System() call assumes that /bin/sh is
             * always available, and so will we.
             */
            execl("/bin/sh", "/bin/sh", "-c", command, NULL);
            _exit(EXIT_FAILURE);
            break;
         
       default: /* Parent */
            close(pipefd[1]);
            rc = pipefd[0];
            close(pipefd2[0]);
            wc = pipefd2[1];
            break;
      } /* switch */
   }
 *aWc=wc;
 return rc;
}

/**[txh]********************************************************************

  Description:
  Gets a line of text from the child.
  By breese.
  
  Return: 
  The ammount of bytes read.
  
***************************************************************************/
static
int my_getline(int fd, char *buffer, int max)
{
 char c;
 int i=0;
 
 do
   {
    if (read(fd,&c,1)<1)
       return 0;
    if (i<max)
       buffer[i++]=c;
   }
 while (c!='\n');
 buffer[i]=(char)0;
 return i;
}

/**[txh]********************************************************************

  Description:
  Closes the pipe and ensures the child dies.
  Note: We must use wait or our child will become a zombie.
  By SET.
  
***************************************************************************/
static
void my_pclose(int rc, int wc, int pid)
{
 int wstatus,ret;
 /* Close the pipe, it should kill the child with SIGPIPE */
 close(rc);
 close(wc);
 /* Is already dead? */
 ret=waitpid(pid,&wstatus,WNOHANG);
 if (ret==0)
   {/* Nope, wait a little bit */
    /* I choose to sleep and no hang, perhaps setting an alarm
       could be better */
    usleep(100000);
    /* Is dead? */
    ret=waitpid(pid,&wstatus,WNOHANG);
    if (ret==0)
      {/* Nope, a diehard, or just too slow. */
       /* Make sure the the child process has terminated */
       kill(pid,SIGKILL);
       /* The child can't block it so we can wait */
       waitpid(pid,&wstatus,0);
      }
   }
}

void Solve(const char *file, const char *outFile, const char *binary)
{
 FILE *f;
 if (strcmp(file,"-")==0)
    f=stdin;
 else
    f=fopen(file,"rt");
 if (!f)
   {
    perror("Error opening input file");
    return;
   }

 FILE *out;
 if (strcmp(outFile,"-")==0)
    out=stdout;
 else
    out=fopen(outFile,"wt");
 if (!out)
   {
    perror("Error creating output file");
    return;
   }

 int l=32+strlen(binary);
 if (l<maxLineL) l=maxLineL;
 char b[l];
 pid_t cp;
 int wc,rc;
 strcpy(b,"addr2line -Cfe ");
 strcat(b,binary);
 rc=my_popen(b,&cp,&wc);
 if (rc<0)
   {
    perror("Error creating pipe");
    return;
   }

 int count=0;
 l--;
 do
   {
    if (fgets(b,l,f))
      {
       fputs(b,out);
       if (isxdigit(b[0]))
         {
          char *s,*end;
          if (b[0]==0 && toupper((uchar)b[1])=='x') s+=2;
          for (s=b; *s && isxdigit((uchar)*s); s++);
          if (isspace((uchar)*s))
            {// We must use address-1 because address is the *return* point and
             // not the calling one
             ulong address=strtoul(b,&end,16);
             sprintf(b,"%X\n",address-1);
             write(wc,b,strlen(b));
             my_getline(rc,b,l);
             fputs("Function: ",out);
             fputs(b,out);
             my_getline(rc,b,l);
             int len=strlen(b);
             if (b[len-1]=='\n') b[len-1]=0;
             fputs(b,out);
             // If no line is provided is because no debug info is available for
             // it so avoid generating a valir error line
             if (len>3 && strcmp(b+len-3,":0")==0)
                fputc('\n',out);
             else
                fprintf(out,": frame %d\n",count);
             count++;
            }
         }
      }
   }
 while (!feof(f));
 if (f!=stdin)
    fclose(f);
 if (out!=stdout)
    fclose(out);
 my_pclose(rc,wc,cp);
}

int main(int argc, char *argv[])
{
 printf("Symbols translator Copyright (c) 2001 by Salvador E. Tropea\n");
 if (argc!=4)
   {
    printf("Use: %s input_file output_file executable\n",argv[0]);
    printf("You can replace input and output file by - to use the standard ones.\n");
    return 1;
   }
 Solve(argv[1],argv[2],argv[3]);
 return 0;
}
