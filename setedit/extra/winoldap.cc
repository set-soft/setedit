/**[txh]********************************************************************

  Copyright (c) 1997 by Salvador Eduardo Tropea.
  That's the interface with the Windows Clipboard.

***************************************************************************/

#ifdef __DJGPP__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <go32.h>
#include <dpmi.h>
#include <sys/movedata.h>
#include <winoldap.h>

#define USE_TB

char *ErrorName[]=
{
 "No error",
 "Windows not present",
 "Clipboard in use by other application",
 "Clipboard too big for the transfer buffer",
 "Not enough memory",
 "Windows reports error"
};

static int isValid=0;
static int Version;
static int Error=0;

int WINOLDAP_Init(void)
{
 __dpmi_regs r;

 r.x.ax=0x1700;
 __dpmi_int(0x2F,&r);
 Version=r.x.ax;
 isValid=r.x.ax!=0x1700;
 if (!isValid)
    Error=WINOLDAP_NoPresent;
 return isValid;
}

static int AllocateDOSMem(unsigned long size,unsigned long *BaseAddress)
{
 __dpmi_regs r;
#ifdef USE_TB
 unsigned long tbsize=_go32_info_block.size_of_transfer_buffer;

 if (size<=tbsize)
   {
    *BaseAddress=__tb;
    return 1;
   }
#endif
 if (size>0x100000)
   {
    Error=WINOLDAP_TooBig;
    return 0;
   }
 r.h.ah=0x48;
 r.x.bx=(size>>4)+(size & 0xF ? 1 : 0);
 __dpmi_int(0x21,&r);
 if (r.x.flags & 1)
   {
    Error=WINOLDAP_TooBig;
    return 0;
   }
 *BaseAddress=r.x.ax<<4;
 return 1;
}

static void FreeDOSMem(unsigned long Address)
{
 __dpmi_regs r;
#ifdef USE_TB
 if (Address==__tb)
    return;
#endif
 r.h.ah=0x49;
 r.x.es=Address>>4;
 __dpmi_int(0x21,&r);
}

char *WINOLDAP_GetClipboard(unsigned long *size)
{
 char *p=NULL;
 unsigned long BaseAddress;
 __dpmi_regs r;

 if (!isValid)
    return NULL;
 r.x.ax=0x1701;
 __dpmi_int(0x2F,&r);
 if (r.x.ax==0)
   {
    Error=WINOLDAP_ClpInUse;
    return NULL;
   }
 r.x.ax=0x1704;
 r.x.dx=1;
 __dpmi_int(0x2F,&r);
 *size=r.x.ax+(r.x.dx<<16);
 if (*size)
   {
    if (AllocateDOSMem(*size,&BaseAddress))
      {
       p=new char[*size];
       if (p)
         {
          r.x.dx=1;
          r.x.bx=BaseAddress & 0x0f;
          r.x.es=(BaseAddress>>4) & 0xffff;
          r.x.ax=0x1705;
          __dpmi_int(0x2F,&r);
          dosmemget(BaseAddress,*size,p);
          *size=strlen(p);
         }
       else
          Error=WINOLDAP_Memory;
       FreeDOSMem(BaseAddress);
      }
   }
 else
   {
    p=new char[1];
    *p=0;
   }
 r.x.ax=0x1708;
 __dpmi_int(0x2F,&r);
 return p;
}

char *WINOLDAP_GetError(void)
{
 return ErrorName[Error];
}

int WINOLDAP_SetClipboard(char *p, unsigned long len)
{
 __dpmi_regs r;
 unsigned long dataoff;
 char cleaner[32];
 int winLen;

 if (!isValid)
    return 0;
 r.x.ax=0x1701;
 __dpmi_int(0x2F,&r);
 if (r.x.ax==0)
   {
    Error=WINOLDAP_ClpInUse;
    return 0;
   }
 // Erase the current contents of the clipboard
 r.x.ax=0x1702;
 __dpmi_int(0x2F,&r);
 winLen=((len+1)+0x1F) & ~0x1F;
 memset(cleaner,0,32);
 if (AllocateDOSMem(winLen,&dataoff))
   {
    dosmemput(p,len,dataoff);
    dosmemput(cleaner,winLen-len,dataoff+len);
    r.x.ax=0x1703;
    r.x.dx=7; // OEM text
    r.x.bx=dataoff & 0x0f;
    r.x.es=(dataoff>>4) & 0xffff;
    r.x.si=winLen>>16;
    r.x.cx=winLen & 0xffff;
    __dpmi_int(0x2F,&r);
    FreeDOSMem(dataoff);
    if (r.x.ax==0)
      {
       Error=WINOLDAP_WinErr;
       r.x.ax=0x1708;
       __dpmi_int(0x2F,&r);
       return 0;
      }
    r.x.ax=0x1708;
    __dpmi_int(0x2F,&r);
   }
 return 1;
}

#ifdef TEST
int main(void)
{
 unsigned long size;
 char *p;
 char *d="Hola como estas";

 if (!WINOLDAP_Init())
   {
    printf("No soportado\n");
    return 1;
   }
 printf("version: %d.%d\n",Version & 0xFF,Version>>8);

 printf("Tomando el clipboard:\n");
 if ((p=WINOLDAP_GetClipboard(&size))==NULL)
   {
    printf("Error: %s\n",WINOLDAP_GetError());
    return 1;
   }
 printf("Tama¤o: %lu\n",size);
 printf("%s",p);
 if (WINOLDAP_SetClipboard(d,strlen(d)))
   {
    printf("Data transfered to Windows OK\n");
   }
 return 0;
}
#endif // TEST
#else  // !__DJGPP__
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>

int WINOLDAP_Init(void)
{
 return 1;
}

char *WINOLDAP_GetError(void)
{
 return "Not enough memory";
}

char *WINOLDAP_GetClipboard(unsigned long *size)
{
 char* p = NULL;
 HGLOBAL hMem;
 if (OpenClipboard(NULL))
   {
    hMem = GetClipboardData(CF_TEXT);
    if (hMem)
      {
       char* d = (char*)GlobalLock(hMem);
       if (d)
         {
          *size = GlobalSize(hMem) - 1;
          p = new char[*size];
          if (p)
            memcpy(p, d, *size);
          GlobalUnlock(hMem);
         }
      }
    CloseClipboard();
   }
 return p;
}

int WINOLDAP_SetClipboard(char *p, unsigned long len)
{
 if (!len || !p)
   return 1;
 HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE, len + 1);
 if (!hMem)
   return 0;
 char* d = (char*)GlobalLock(hMem);
 if (d)
   {
    memcpy(d, p, len);
    d[len] = 0;
   }
 GlobalUnlock(hMem);
 if (OpenClipboard(NULL))
   {
    EmptyClipboard();
    if (SetClipboardData(CF_TEXT, hMem))
      {
       CloseClipboard();
       return 1;
      }
    CloseClipboard();
   }
 GlobalFree(hMem);
 return 0;
}

#else // _WIN32

#include <stdlib.h>
// Linux version. By now does nothing. Perhaps it can use gpm or other
// thing.
int WINOLDAP_Init(void)
{
 return 0;
}

char *WINOLDAP_GetClipboard(unsigned long *)
{
 return NULL;
}

char *WINOLDAP_GetError(void)
{
 return "Windows not present";
}

int WINOLDAP_SetClipboard(char *, unsigned long )
{
 return 0;
}
#endif // _WIN32
#endif // __DJGPP__

