/* Copyright (C) 1996-2007 by Salvador E. Tropea (SET),
   see copyrigh file for details */
//#define DEBUG
#define Uses_string
#define Uses_stdlib
#define Uses_alloca
#define Uses_ctype
#define Uses_TVCodePage
#define Uses_snprintf
#define Uses_TScreen // App helpers
#define Uses_MsgBox
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


static TScreen::appHelperHandler hPDF=-1;
static TScreen::appHelperHandler hImg=-1;

/**[txh]********************************************************************

  Description:
  Opens an image viewer.
  
***************************************************************************/

static
void openImage(const char *fileName)
{
 if (hImg==-1)
   {
    hImg=TScreen::openHelperApp(TScreen::ImageViewer);
    if (hImg==-1)
      {
       messageBox(TScreen::getHelperAppError(),mfError | mfOKButton);
       return;
      }
   }
 if (!TScreen::sendFileToHelper(hImg,fileName,NULL))
   {
    messageBox(TScreen::getHelperAppError(),mfError | mfOKButton);
    return;
   }
}

/**[txh]********************************************************************

  Description:
  Opens a PDF viewer.
  
***************************************************************************/

static
void openPDF(const char *fileName, int page)
{
 if (hPDF==-1)
   {
    hPDF=TScreen::openHelperApp(TScreen::PDFViewer);
    if (hPDF==-1)
      {
       messageBox(TScreen::getHelperAppError(),mfError | mfOKButton);
       return;
      }
   }
 if (!TScreen::sendFileToHelper(hPDF,fileName,&page))
   {
    messageBox(TScreen::getHelperAppError(),mfError | mfOKButton);
    return;
   }
}

/**[txh]********************************************************************

  Description:
  Closes all opened viewers.
  
***************************************************************************/

void CloseViewers()
{
 if (hImg!=-1)
    TScreen::closeHelperApp(hImg);
 if (hPDF!=-1)
    TScreen::closeHelperApp(hPDF);
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
 // Is an image?
 Boolean isImage=False;
 if (lname>6 && strncmp(startWord,"image:",6)==0)
   {
    lname-=6;
    startWord+=6;
    isImage=True;
   }
 // Is a PDF?
 Boolean isPDF=False;
 int page=-1;
 if (!isImage && lname>4 && strncmp(startWord,"pdf:",4)==0)
   {
    lname-=4;
    startWord+=4;
    isPDF=True;
    if (lname>2 && ucisdigit(startWord[0]))
      {
       char *s=startWord+1;
       int n=lname-1;
       for (;n && ucisdigit(*s); n--, s++);
       if (n>1 && *s==':')
         {
          page=strtol(startWord,NULL,10);
          lname=n-1;
          startWord=s+1;
         }
      }
   }
 name=(char *)alloca(lname+1);
 strncpy(name,startWord,lname);
 name[lname]=0;
 #ifdef DEBUG
 messageBox(name,mfOKButton);
 #endif

 int result=FindFile(name,fullName,reference);
 if (!result)
   {// We failed, try changing slashes
    char *s=name;
    int changed=0;
    for (; *s; s++)
        if (*s=='\\')
          {
           *s='/';
           changed++;
          }
    if (changed)
       result=FindFile(name,fullName,reference);
   }
 while (!result && lname && !TVCodePage::isAlNum(name[lname-1]))
   {// We failed and the last char isn't alphanumeric
    name[--lname]=0;
    result=FindFile(name,fullName,reference);
   }
 if (result)
   {
    if (isImage)
       openImage(fullName);
    else if (isPDF)
       openPDF(fullName,page);
    else
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

#define badForFile(b) \
 (ucisspace(b) || !ucisprint(b) || b=='\"' || b=='>' || b=='<' || b=='|')

static
int isImage(char *c, char *start)
{
 if (*c==':' && c-start>=5 && strncmp(c-5,"image",5)==0)
   {
    if (c-start==5) return 1;
    uchar b=(uchar)c[-6];
    return badForFile(b);
   }
 return 0;
}

static
int isPDF(char *c, char *start)
{
 if (*c==':' && c-start>=4 && ucisdigit(c[-1]))
   {// Skip page number
    for (c-=2; c>=start && ucisdigit(*c); c--);
    if (c<start || *c!=':')
       return 0;
    // Is it "pdf:"?
    if (!(c-start>=3 && strncmp(c-3,"pdf",3)==0))
       return 0;
    if (c-start==3) return 1;
    uchar b=(uchar)c[-4];
    return badForFile(b);
   }
 if (*c==':' && c-start>=3 && strncmp(c-3,"pdf",3)==0)
   {
    if (c-start==3) return 1;
    uchar b=(uchar)c[-4];
    return badForFile(b);
   }
 return 0;
}

int isValidForFile(char *c, char *start)
{
 if (badForFile(*c))
    return 0;
 #ifdef CLY_HaveDriveLetters
 if (*c==':')
   {
    if (c==start) return 0;
    if (!TVCodePage::isAlpha(c[-1])) return 0;
    if (c==start-1) return 1;
    uchar b=(uchar)c[-2];
    if (badForFile(b))
       return 1;
    if (isImage(c,start))
       return 1;
    if (isPDF(c,start))
       return 1;
    return 0;
   }
 return 1;
 #else
 // Over : in "image:filename"
 if (isImage(c,start))
    return 1;
 // Over : in "pdf:NN:filename"
 if (isPDF(c,start))
    return 1;
 return *c!=':';
 #endif
}

