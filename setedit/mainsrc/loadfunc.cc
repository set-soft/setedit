/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
//#define DEBUG
#define Uses_string
#define Uses_stdlib
#define Uses_alloca
#define Uses_ctype
#define Uses_TVCodePage
#define Uses_snprintf
#ifdef DEBUG
 #define Uses_MsgBox
#endif
#include <tv.h>

#include <pathtool.h>

int isValidForFile(char *c, char *start);
extern void OpenFileFromEditor(char *fullName);

#define NoFile() NoFileNameUnderCursor(Message,data)

static
void NoFileNameUnderCursor(void (*Message)(const char *msg, void *data),
                           void *data)
{
 const char *msg=TVIntl::getTextNew(__("No file name under cursor"));
 Message(msg,data);
 DeleteArray(msg);
}

/**[txh]********************************************************************

  Description:
  Try to separate the name of the file under the cursor and pass it to the
editor to load this file.
  
  Return: True if loaded.
  
***************************************************************************/

Boolean LoadFileUnderCursor(char *lineStart, char *cursor, unsigned l,
                            void (*Message)(const char *msg, void *data),
                            void *data, const char *reference)
{
 char *end=lineStart+l;
 char *startWord,*endWord,*name,*fullName;
 int lname;

 // eat all the white spaces at the start
 for (;lineStart<end && ucisspace(*lineStart); lineStart++);
 if (lineStart>=end)
   {
    NoFile();
    return False;
   }

 if (strncmp(lineStart,"#include",8)==0)
   { // Is #include, so is easy to parse
    lineStart+=8;
    // Search for " or <
    for (;lineStart<end && *lineStart!='\"' && *lineStart!='<'; lineStart++);
    for (lineStart++;lineStart<end && ucisspace(*lineStart); lineStart++);
    if (lineStart>=end)
      {
       NoFile();
       return False;
      }
    startWord=lineStart;
    // Search for " or < or space
    for (;lineStart<end && *lineStart!='\"' && *lineStart!='>' && !ucisspace(*lineStart); lineStart++);
    if (lineStart>=end)
      {
       NoFile();
       return False;
      }
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
         {
          NoFile();
          return False;
         }
      }
    startWord=cursor;
    // Now forward to the end
    for (;cursor<end && isValidForFile(cursor,lineStart); ++cursor);
    endWord=cursor;
   }
 // Now we have a file name enclosed
 lname=endWord-startWord;
 if (!lname)
   {
    NoFile();
    return False;
   }
 name=(char *)alloca(lname+1);
 strncpy(name,startWord,lname);
 name[lname]=0;
 #ifdef DEBUG
 messageBox(name,mfOKButton);
 #endif

 if (FindFile(name,fullName,reference))
   {
    OpenFileFromEditor(fullName);
    free(fullName);
    return True;
   }

 // Generate a message indicating we couldn't load it
 const char *msg=TVIntl::getTextNew(__("Can't find \"%s\" file"));
 int len=strlen(msg)+lname+1;
 char *msgF=(char *)alloca(len);
 CLY_snprintf(msgF,len,msg,name);
 Message(msgF,data);
 DeleteArray(msg);

 return False;
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

