/*
  Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
  This code is covered by the GPL license. A copy of the license should
be provided in the same package.
*/
#define Uses_AlCon_conio
#include "AlCon.h"

int main(void)
{
 uchar buf[32];
 unsigned off;
 
 /* Create an 80x25 screen => 640x400 */
 AlCon_Init(80,25);

 /* Set background to blue and foreground to yellow */
 textattr(YELLOW | (BLUE<<4));
 
 gotoxy(34,12);
 cputs("Hello, world!");
 
 gotoxy(77,1);
 cprintf("Hola");
 
 cprintf("funka?");
 gotoxy(1,25);
 
 cprintf("Last line");
 getch();
 
 cprintf("\nScrollg");
 gotoxy(7,25);
 
 for (off=0; off<16; off++)
    {
     buf[off*2+charPos]=off+'A';
     buf[off*2+attrPos]=(off<<4) | (15-off);
    }
 AlCon_PutBuf(20*2,(uint16 *)buf,16);
 AlCon_PutChar((3*ScreenCols()+20)*2,*((uint16 *)(buf+2)));
 AlCon_GetScrChars(20*2,(uint16 *)buf,16);
 AlCon_PutBuf((2*ScreenCols()+20)*2,(uint16 *)buf,16);
 getch();
 
 /* Deinitialize all */
 AlCon_Exit();
 return 0;
}

END_OF_MAIN();