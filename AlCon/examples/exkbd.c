/*
  Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
  This code is covered by the GPL license. A copy of the license should
be provided in the same package.
*/
#define Uses_AlCon_conio
#include "AlCon.h"

void InterpretAbstract(unsigned Abstract)
{
 int key=Abstract & kbKeyMask;
 cprintf("Abstract: kb%s",AlCon_KeyNames[key]);
 if (Abstract & kbShiftCode)
    cprintf(" SHIFT");
 if (Abstract & kbCtrlCode)
    cprintf(" CTRL");
 if (Abstract & kbAltRCode)
    cprintf(" ALT-R");
 if (Abstract & kbAltLCode)
    cprintf(" ALT-L");
 cprintf("\n");
}

int main(void)
{
 unsigned symbol;
 uchar ucscan;
 unsigned flags;
 int key;
 
 /* Create an 80x25 screen => 640x400 */
 AlCon_Init(80,25);

 /* Set background to blue and foreground to yellow */
 textattr(YELLOW | (BLUE<<4));

 cprintf("Press any key, ESC to exit\n");
 do
   {
    key=AlCon_GetKey(&symbol,&ucscan,&flags);
    InterpretAbstract(key);
    cprintf("ASCII: %c Scan: %d Flags: %d\n",symbol ? symbol : '?',ucscan,flags);
   }
 while ((key & kbKeyMask)!=kbEsc);
 
 /* Deinitialize all */
 AlCon_Exit();
 return 0;
}

END_OF_MAIN();