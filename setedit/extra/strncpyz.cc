/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
char *strncpyZ(char *dest, char *orig, int size)
{
 if (size)
    size--;
 for (;*orig && size; size--,dest++,orig++)
     *dest=*orig;
 *dest=0;
 return dest;
}

