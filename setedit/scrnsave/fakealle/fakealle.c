/* Copyright (C) 1999,2000 by Salvador E. Tropea (SET),
   see copying file for details */
#include <fakealle.h>
#include <stdio.h>
#include <stdlib.h>
#include <pc.h>
#include <sys/movedata.h>
#include <dpmi.h>
#include <conio.h>

void allegro_exit()
{
 asm ("movl $0x3,%eax  \n\
       int  $0x10");
}

int mouse_x,mouse_y,mouse_b,key_pressed=-1;

static
void getMouseStatus(int *x, int *y, int *b)
{
 __dpmi_regs r;
 r.x.ax=3;
 __dpmi_int(0x33,&r);
 *x=r.x.cx;
 *y=r.x.dx;
 *b=r.x.bx;
}

void allegro_init()
{
 getMouseStatus(&mouse_x,&mouse_y,&mouse_b);
}

/* _vsync_out_v:
 *  Waits until the VGA is not in a vertical retrace.
 */
inline void _vsync_out_v()
{
 do {
 } while (inportb(0x3DA) & 8);
}

/* _vsync_in:
 *  Waits until the VGA is in the vertical retrace period.
 */
inline void _vsync_in()
{
 do {
 } while (!(inportb(0x3DA) & 8));
}

void vsync()
{
 _vsync_out_v();
 _vsync_in();
}

void set_gfx_mode(int driver,int xres, int yres, int x, int y)
{
 if (xres!=320 || yres!=200)
   {
    printf("Error setting mode\n");
    exit(1);
   }
 asm ("movl $0x13,%eax; int $0x10");
}

BITMAP *create_bitmap(int x, int y)
{
 int size=sizeof(BITMAP)+y*sizeof(unsigned char *);
 BITMAP *ret;
 int i;
 unsigned char *dat;

 ret=(BITMAP *)malloc(size);
 dat=(unsigned char *)ret->dat=malloc(x*y);
 for (i=0; i<y; i++)
    {
     ret->line[i]=dat;
     dat+=x;
    }
 ret->w=x; ret->h=y;
 return ret;
}

void blit(BITMAP *bmp)
{
 _dosmemputl(bmp->dat,320*200/4,0xA0000);
}

int poll_input()
{
 int x,y,b;

 if (key_pressed>=0)
    return KEY_EVENT;
 if (kbhit())
   {
    key_pressed=getch();
    return KEY_EVENT;
   }
 getMouseStatus(&x,&y,&b);
 if (x!=mouse_x || y!=mouse_y || b!=mouse_b)
   {
    mouse_x=x;
    mouse_y=y;
    mouse_b=b;
    return MOUSE_EVENT;
   }
 return 0;
}

void clear(BITMAP *bmp)
{
 memset(bmp->dat,0,bmp->w*bmp->h);
}
