/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
//#define DEBUG
#define Uses_string
#define Uses_stdlib
#define Uses_alloca
#define Uses_ctype
#define Uses_TVCodePage
#ifdef DEBUG
 #define Uses_MsgBox
#endif
#include <tv.h>

#include <pathtool.h>

int isValidForFile(char *c, char *start);
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
    if (isValidForFile(cursor,lineStart))
      {
       // Walk backward to the start of the name
       for (;cursor>lineStart && isValidForFile(cursor,lineStart); --cursor);
       if (cursor!=lineStart) cursor++;
      }
    else
      {
       // If isn't in a word walk forward
       for (;cursor<end && !isValidForFile(cursor,lineStart); ++cursor);
       if (!isValidForFile(cursor,lineStart))
          return;
      }
    startWord=cursor;
    // Now forward to the end
    for (;cursor<end && isValidForFile(cursor,lineStart); ++cursor);
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
 messageBox(name,mfOKButton);
 #endif

 if (FindFile(name,fullName))
   {
    OpenFileFromEditor(fullName);
    free(fullName);
   }
}

int isValidForFile(char *c, char *start)
{
 if (ucisspace(*c) || !ucisprint(*c) || *c=='\"' || *c=='>' || *c=='<' || *c=='|')
    return 0;
 #ifdef CLY_HaveDriveLetters
 if (*c==':')
   {
    if (c==start) return 0;
    if (!TVCodePage::isAlpha(c[-1])) return 0;
    if (c==start-1) return 1;
    uchar b=(uchar)c[-2];
    return ucisspace(b) || !ucisprint(b) || b=='\"' || b=='>' || b=='<' || b=='|';
   }
 return 1;
 #else
 return *c!=':';
 #endif
}

