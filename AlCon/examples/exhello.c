/*
  Copyright (c) 2001 by Salvador E. Tropea (SET) <set@ieee.org>
  This code is covered by the GPL license. A copy of the license should
be provided in the same package.
*/
#define Uses_AlCon_conio
#include "AlCon.h"

int main(void)
{
 /* Create an 80x25 screen => 640x400 */
 AlCon_Init(80,25);

 /* Set background to blue and foreground to yellow */
 textattr(YELLOW | (BLUE<<4));

 /* Jump to column 34, row 12 (1,1 is top left) */
 gotoxy(34,12);

 /* Print a message */
 cputs("Hello, world!");

 /* Wait for a key */
 getch();

 /* Deinitialize all */
 AlCon_Exit();
 return 0;
}

END_OF_MAIN();