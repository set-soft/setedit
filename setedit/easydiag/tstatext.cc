/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_TSStaticText
#include <string.h>
#include <easydia1.h>
#include <tv.h>
#include <easydiag.h>

/**[txh]********************************************************************

  Description:
  SView wrapper for the static text object just pass the string.

***************************************************************************/

TSStaticText::TSStaticText(const char *aText) :
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
 view=new TStaticText(TRect(0,0,w,h),aText,cache);
}

TSStaticText::TSStaticText(const char *aText, int wrapCol) :
   TSView()
{
 int l,p,y,i,j;
 stTVIntl *cache=NULL;
 const char *s=TVIntl::getText(aText,cache);
 w=wrapCol;

 // That's the code used by TStaticText to wrap the words, here is used to meassure
 // the height.
 l=strlen(s);
 p=0;
 y=0;
 while (p<l)
   {
    if (s[p]==3) ++p;
    i=p;
    do
      {
       j=p;
       while ((p<l) && (s[p]==' ')) ++p;
       while ((p<l) && (s[p]!=' ') && (s[p] != '\n')) ++p;
      }
    while ((p<l) && (p<i+wrapCol) && (s[p]!='\n'));
    if (p>i+wrapCol)
      {
       if (j>i)
          p=j;
       else
          p=i+wrapCol;
      }
    while ((p<l) && (s[p]==' ')) p++;
    if ((p<l) && (s[p]=='\n'))
      {
       p++;
       if ((p<l) && (s[p]=='\n'))
          p++;
      }
    y++;
   }
 h=y;
 view=new TStaticText(TRect(0,0,w,h),aText,cache);
}

