/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSNoStaticText
#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

/**[txh]********************************************************************

  Description:
  SView wrapper for the no static text object just pass the string.

***************************************************************************/

TSNoStaticText::TSNoStaticText(const char *aText) :
   TSView()
{
 int lines=1;
 int max_w=0,cur_w=0;
 stTVIntl *cache=NULL;
 const char *s=TVIntl::getText(aText,cache);

 while (*s)
   {
    if (*s==3)
      {
       s++;
       continue;
      }
    if (*s=='\n')
      {
       lines++;
       cur_w=0;
       s++;
       continue;
      }
    cur_w++;
    if (cur_w>max_w)
       max_w=cur_w;
    s++;
   }
 w=max_w;
 h=lines;
 view=new TNoStaticText(TRect(0,0,w,h),aText,cache);
}

