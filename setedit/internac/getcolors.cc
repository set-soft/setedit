/**[txh]********************************************************************

  Copyright (c) 2003 by Salvador E. Tropea
  Part of setedit package.
  Covered by the GPL license.

  Description:
  Generates a fake C source containing the names of the colors and groups
of colors for gettext.
  
***************************************************************************/

#include <stdio.h>
#include <pal.h>
#undef S
#undef SS
#define S(index,foreground,background,name) \
  puts("__(\"" #name "\")");
#define SS(index,foreground,background,name,_group) \
  puts("__(\"" #name "\")"); \
  puts("\nGroup: __(\"" #_group "\")");

int main()
{
 SE_cpColor
 return 0;
}

