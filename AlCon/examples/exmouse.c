/*
  Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
  This code is covered by the GPL license. A copy of the license should
be provided in the same package.
*/
#define Uses_AlCon_conio
#include "AlCon.h"

int main(void)
{
 int x,y,buttons;
 int xa,ya,buttonsa;
 
 /* Create an 80x25 screen => 640x400 */
 AlCon_Init(80,25);

 /* Set background to blue and foreground to yellow */
 textattr(YELLOW | (BLUE<<4));
 
 while (!kbhit())
   {
    AlCon_GetMousePos(&x,&y,&buttons);
    if (x!=xa || y!=ya || buttons!=buttonsa)
      {
       gotoxy(5,5);
       cprintf("Mouse: X=%2d Y=%2d Buttons: %d",x,y,buttons);
       xa=x; ya=y; buttonsa=buttons;
      }
   }
 
 /* Deinitialize all */
 AlCon_Exit();
 return 0;
}

END_OF_MAIN();