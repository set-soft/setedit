/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_string
#define Uses_stdlib
#define Uses_alloca
#define Uses_ctype
#include <stdio.h>
#include <tv.h>

#include <pathtool.h>

int isValidForFile(int c);
extern void OpenFileFromEditor(char *fullName);

/**********************************************************************************

   Function: void LoadFileUnderCursor(char *lineStart,char *cursor,unsigned l)

   Type: Normal.

   Objetive: Try to separate the name of the file under the cursor and pass it
             to the editor to load this file.

   by SET.

**********************************************************************************/

void LoadFileUnderCursor(char *lineStart,char *cursor,unsigned l)
{
 char *end=lineStart+l;
 char *startWord,*endWord,*name,*fullName;
 int lname;

 // eat all the white spaces at the start
 for (;lineStart<end && ucisspace(*lineStart); lineStart++);
 if (lineStart>=end)
    return;

 if (strncmp(lineStart,"#include",8)==0)
   { // Is #include, so is easy to parse
    lineStart+=8;
    // Search for " or <
    for (;lineStart<end && *lineStart!='\"' && *lineStart!='<'; lineStart++);
    for (lineStart++;lineStart<end && ucisspace(*lineStart); lineStart++);
    if (lineStart>=end)
       return;
    startWord=lineStart;
    // Search for " or < or space
    for (;lineStart<end && *lineStart!='\"' && *lineStart!='>' && !ucisspace(*lineStart); lineStart++);
    if (lineStart>=end)
       return;
    endWord=lineStart;
   }
 else
   { // Nope, so make some thing like WordUnderCursor
    if (isValidForFile(*cursor))
      {
       // Walk backward to the start of the name
       for (;cursor>lineStart && isValidForFile(*cursor); --cursor);
       if (cursor!=lineStart) cursor++;
      }
    else
      {
       // If isn't in a word walk forward
       for (;cursor<end && !isValidForFile(*cursor); ++cursor);
       if (!isValidForFile(*cursor))
          return;
      }
    startWord=cursor;
    // Now forward to the end
    for (;cursor<end && isValidForFile(*cursor); ++cursor);
    endWord=cursor;
   }
 // Now we have a file name enclosed
 lname=endWord-startWord;
 if (!lname)
    return;
 name=(char *)alloca(lname+1);
 strncpy(name,startWord,lname);
 name[lname]=0;
 #ifdef DEBUG
 fprintf(stderr,"\"%s\" %d\n",name,lname);
 #endif

 if (FindFile(name,fullName))
   {
    OpenFileFromEditor(fullName);
    free(fullName);
   }
}

int isValidForFile(int c)
{
 return (!ucisspace(c) && ucisprint(c) && c!='\"' && c!='>' && c!='<');
}

