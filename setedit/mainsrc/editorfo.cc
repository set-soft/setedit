/* Copyright (C) 1996-2002 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/******************************************************************************

  This file is part of the TCEditor class by SET.
  
  Made:

   Added the table for comments and preprocessor.
   Search enhanced by my own fast engine 2.75 to 25 times faster depending on
  the case.
   Better floats highlight.
   Eliminated the in_string, in_char, in_cpp_comment and in_prep test.
   is_symbol uses a look up table.
   Eliminated the temporal copy of name (faster and safter).
   Modified the SetChar for a (char *)DrawBuffer[bufptr*2]=c much better
  option.

26/9
  Patch from Robert for:
  1) Kill a bug with the garbage at the end of a line, a test for <=0 was
     added.
  2) Get the colors instead of use fixed values, that's includes the assign
     for EDITOR.

28/9
  Added Pascal highlight, is very green by now.
  Added Clipper highlight, is even more green.

30/9
  Applied the second patch of Robert to support strings crossing lines.

17/10
  Added a new method to avoid unused warnings (by Robert).
  Corrected to support reserved words with '_'

28/10
  Corrected some stuff about the number of bytes filled in the drawbuffer.

******************************************************************************/

// That's the first include because is used to configure the editor.
#include "ceditint.h"
#include <stdio.h>
#define Uses_string
#define Uses_AllocLocal
#define Uses_TCEditor
#define Uses_TCEditor_Internal
#define Uses_ctype
#define Uses_TVCodePage
#include <ceditor.h>

#define Uses_SOStack
#define Uses_TSOSStringCollection
#include <settvuti.h>

inline
static int in_range(uint32 start,uint32 end,uint32 pos)
{
 //if (start >= end) return 0;
 if (pos >= start && pos < end) return 1;
 return 0;
}

void SyntaxFormatLine(TCEditor * editor,char *DrawBuf,uint32 LinePtr,int Width,
                      uint32 Attr,unsigned lineLength,int seeTabs);
void SyntaxFormatLinePascal(TCEditor * editor,char *DrawBuf,uint32 LinePtr,int Width,
                            int check_comment_1,int check_comment_2,unsigned lineLength,
                            int seeTabs);
void SyntaxFormatLineClipper(TCEditor * editor,char *DrawBuf,uint32 LinePtr,int Width,
                             int check_c_comment, unsigned,int seeTabs);
void SyntaxFormatLineGeneric(TCEditor * editor,char *DrawBuf,uint32 LinePtr,
                             int Width, uint32 Attr, unsigned lineLength,
                             int seeTabs);

#define CALL(a)  if (!call10(buffer,(char *)DrawBuf,color,count,offset,LinePtr,Width,tabSize,SeeTabs,a)) goto paint
//#define CALL2 if (!call10(buffer,(char *)DrawBuf,color,count,offset,LinePtr,Width,tabSize,SeeTabs)) goto paint
#define CALL3 call30(buffer,(char *)DrawBuf,bufLen,LinePtr,Width,tabSize,color,normalColor,SeeTabs,TabChar)
#define CALL4 call20(buffer,(char *)DrawBuf,bufLen,LinePtr,Width,tabSize,color)

static unsigned fl_bufptr;
// Colored tabs stuff
//   Globals because they must be shared between functions
static char tabColor;
static int  tabFF;

static char *BUFFER;
static TCEditor *EDITOR;
static uint32 lineptr;

/*
by Robert:

 Enable the following code if you know, what it does.

1. It caches the color attributes for the editor and brings
   a big speedup especially for the syntax highlight, because
   TView::getColor is slow and called very often.

2. The cached colors are valid only, if the final application
   does not change the colors. If it changes the colors, the
   application should set 'colors_cached' to zero.
   One possiblity for this is to use that global variable. An
   other thing would be to change the handleEvent method of the
   editor to handle to an colors_changed broadcast or something
   like that.

3. Enabling the cache brings a speedup of about 15%
   without caching:  about  87 lines/second
   with caching:     about 100 lines/second
*/

uchar TCEditor::cachedColors[cNumColors];

void TCEditor::CacheColors()
{
 int i;
 for (i=1; i<cNumColors; i++)
     cachedColors[i]=getColor(i);
}

/**[txh]********************************************************************

  Description:
  Fills the colors cache with correlative numbers. This can be used to get
SHL id values instead of colors so you can analyze according to the data
type instead of the color.
  
***************************************************************************/

void TCEditor::ColorsCacheToIDs()
{
 int i;
 for (i=1; i<cNumColors; i++)
     cachedColors[i]=i;
}

#define GetColor(a) TCEditor::cachedColors[a]

#define NormalColor  GetColor(cNormal)
#define CommentColor GetColor(cComment)
#define PreproColor  GetColor(cPre)
#define StringColor  GetColor(cString)
#define CharColor    GetColor(cChar)
#define IlegalColor  GetColor(cIllegal)
#define ResColor     GetColor(cReserved)
#define IdentColor   GetColor(cIdent)
#define UserColor    GetColor(cUser)
#define OctalColor   GetColor(cOctal)
#define IntColor     GetColor(cInteger)
#define FloatColor   GetColor(cFloat)
#define SymbolColor  GetColor(cSymbol)
#define Symbol2Color GetColor(cSymbol2)
#define HexColor     GetColor(cHex)
#define OddTabColor  GetColor(cOddTab)
#define EvenTabColor GetColor(cEvenTab)
#define ColMarkColor GetColor(cColMark)

inline
static void ToggleTabColor()
{
 tabFF=~tabFF;
 tabColor=tabFF ? EvenTabColor : OddTabColor;
}

#define fill_line(flag)\
{\
  int count = Width-offset;\
  if (count<=0) return flag;\
  offset+=count;\
  while (count--)\
    {\
     drawBuf[fl_bufptr] = ' ';\
     drawBuf[fl_bufptr+1] = color;  \
     fl_bufptr+=2;\
    }\
  return flag;\
}

#define SETCHAR(c)\
 {\
  drawBuf[fl_bufptr]=c;\
  drawBuf[fl_bufptr+1]=color;\
  fl_bufptr+=2;\
  offset++;\
 }

#define SETCHARTAB(c)\
 {\
  drawBuf[fl_bufptr]=c;\
  drawBuf[fl_bufptr+1]=tabColor;\
  fl_bufptr+=2;\
  offset++;\
 }


static Boolean call10(char *buffer,char *drawBuf,char color,int cx,
                      unsigned &offset, unsigned &lineptr,unsigned Width,
                      unsigned tabSize,int seeTabs,char tabChar)
{
  uchar c;
  long count = (long)cx - (long)lineptr;
  if (count<=0)
     return True;

  // Colored tabs stuff
  /*tabFF=0;              No here!
  tabColor=OddTabColor;*/
 
  do {
    c = buffer[lineptr++];
    if ( !c || c == '\n' || c == '\r' || c == '\t')
    {
      if (c=='\t')
      {
       if (seeTabs)
         {
          do
           {
            SETCHARTAB(tabChar);
           }
          while ((offset % tabSize)!=0);
          ToggleTabColor();
         }
       else
         {
          do
          {
            SETCHAR(' ');
          } while ((offset % tabSize)!=0);
         }
      }
      else
      {
        fill_line(False);
      }
    }
    else SETCHAR(c);
    if (offset >= Width)
    {
      return False;
    }
    count--;
  } while (count>0);
  return True;
}

static unsigned call20(char *buffer,char *drawBuf,int bytesAvail, unsigned lineptr,
                       unsigned Width,unsigned tabSize,char color)
{
  uchar c;
  int count = bytesAvail - (int)lineptr;

  if (count<=0) return 0;
  unsigned offset=0;
  buffer+=lineptr;

  do
    {
     c = *buffer;
     buffer++;
     if (c=='\r' || !c || c=='\n') // !c is for the lines under edition where 0 is the end
       {
        int count = Width-offset;
        if (count<=0) return offset;
        while (count--)
          {
           *drawBuf=' ';
           *(drawBuf+1)=color;
           drawBuf+=2;
          }
        return Width;
       }
     if (c=='\t')
       {
        int tabCount=tabSize-(offset % tabSize);
        while (tabCount--)
         {
          *drawBuf=' ';
          drawBuf+=2;
          offset++;
         }
       }
     else
       {
        *drawBuf=c;
        drawBuf+=2;
        offset++;
       }
    if (offset >= Width)
       return offset;
    count--;
   }
 while (count>0);
 return offset;
}

static unsigned call30(char *buffer,char *drawBuf,int bytesAvail, unsigned lineptr,
                       unsigned Width,unsigned tabSize,char color,char normalCol,
                       int seeTabs, char tabChar)
{
  uchar c;
  int count = bytesAvail - (int)lineptr;

  // Colored tabs stuff
  tabFF=0;
  tabColor=OddTabColor;
 
  if (count<=0) return 0;
  unsigned offset=0;
  buffer+=lineptr;
 
  do
    {
     c = *buffer;
     buffer++;
     if (c=='\r' || !c || c=='\n') // !c is for the lines under edition where 0 is the end
       {
        int count = Width-offset;
        if (count<=0) return offset;
        while (count--)
          {
           *drawBuf=' ';
           *(drawBuf+1)=normalCol;
           drawBuf+=2;
          }
        return Width;
       }
     if (c=='\t')
       {
        int tabCount=tabSize-(offset % tabSize);
        if (seeTabs)
          { // It paints the tabs to be visible
           while (tabCount--)
             {
              *drawBuf=' ';
              *(drawBuf+1)=tabColor;
              drawBuf+=2;
              offset++;
             }
           ToggleTabColor();
          }
        else
          {
           while (tabCount--)
             {
              *drawBuf=' ';
              *(drawBuf+1)=color;
              drawBuf+=2;
              offset++;
             }
          }
       }
     else
       {
        *drawBuf=c;
        *(drawBuf+1)=color;
        drawBuf+=2;
        offset++;
       }
    if (offset >= Width)
       return offset;
    count--;
   }
 while (count>0);
 return offset;
}

static
void PaintMarkers(char *db, char Color, unsigned Width, uint32 *colMarkers)
{
 int marker=0;
 unsigned nextCol=colMarkers[marker]-1;
 char color=Color & 0xF0;   // Only the background

 while (nextCol<Width)
   {
    db[nextCol*2+1]=(db[nextCol*2+1] & 0xF) | color;
    nextCol=colMarkers[++marker]-1;
   }
}

#define normalColor (Colors & 0xff)
#define selectColor (Colors >> 8)

/* Old function
void TCEditor::formatLine( void *DrawBuf,
			  unsigned LinePtr,
			  int Width,
			  unsigned short Colors
			)
{
  unsigned count,offset;
  char color;
  fl_bufptr = 0;
  offset = 0;


  if (selHided)
    {
     color = normalColor;
     count = bufLen;
     CALL;
    }

  if (selStart > LinePtr)
    {
     color = normalColor;
     count = selStart;
     CALL;
    }
  color = selectColor;
  count = selEnd;
  CALL;

  color = normalColor;
  count = bufLen;
  CALL;
  count = Width-offset;
  //if (count<=offset) return;
  //while (count--) ((ushort far *)(DrawBuf))[bufptr++] = (color << 8) | ' ';
  while (count--)
     {
      ((char *)(DrawBuf))[fl_bufptr]=' ';
      ((char *)(DrawBuf))[fl_bufptr+1]=color;
      fl_bufptr+=2;
     }
}
*/

void TCEditor::formatLine(void *DrawBuf, unsigned LinePtr, int Width, ushort Colors,
       unsigned , // lineLen
       uint32 ,   // shl attributes
       unsigned , // LineNo needed for RHIDE
       uint32 *colMarkers)
{
 unsigned offset;
 int count; // must be signed
 char color;
 fl_bufptr = 0;
 offset = 0;

 // Colored tabs stuff
 tabFF=0;
 tabColor=OddTabColor;

 if (selHided)
   {
    color = normalColor;
    count = bufLen;
    CALL(' ');
   }
 if (selStart > LinePtr)
   {
    color = normalColor;
    count = selStart>bufLen ? bufLen : selStart;
    CALL(' ');
   }
 color = selectColor;
 count = selEnd;
 CALL(TabChar);

 color = normalColor;
 count = bufLen;
 CALL(' ');

 count = Width-offset;
 // Bug corrected by Robert:
 if (count<=0) return; // This is the fix for the bug with the garbage at
                       // the end of the line
 while (count--)
    {
     ((char *)(DrawBuf))[fl_bufptr]=' ';
     ((char *)(DrawBuf))[fl_bufptr+1]=color;
     fl_bufptr+=2;
    }

paint:
 // Column markers over the selection
 if (colMarkers)
    PaintMarkers((char *)DrawBuf,ColMarkColor,Width,colMarkers);
}

static void PaintPreproNoSel(unsigned offset,char *db,char *buffer,unsigned lineLen,
                             unsigned VisibleWidth,int tabSize,int extCom,
                             int seeTabs, char tabChar)
{
 char col=PreproColor,c;
 int in_com,in_cpp_com,x=0;

 // Colored tabs stuff
 tabFF=0;
 tabColor=OddTabColor;

 in_com=extCom;
 if (in_com)
    col=CommentColor;
 in_cpp_com=0;
 VisibleWidth<<=1;
 // adjust lineLen relative to offset
 lineLen += offset;
 do
  {
   c=buffer[offset++];
   x++;
   if (CLY_IsEOL(c)) break;
   if (in_com)
     {
      if (c=='*' && buffer[offset]=='/')
        {
         db[fl_bufptr++]=c;
         db[fl_bufptr++]=col;
         c=buffer[offset++];
         x++;
         db[fl_bufptr++]=c;
         db[fl_bufptr++]=col;
         in_com=0;
         col=PreproColor;
         continue;
        }
     }
   else
     {
      if (!in_cpp_com)
        {
         if (c=='/')
           {
            if (buffer[offset]=='/')
              {
               in_cpp_com=1;
               in_com=0;
               col=CommentColor;
              }
            else
              if (buffer[offset]=='*')
                {
                 in_com=1;
                 col=CommentColor;
                 db[fl_bufptr++]=c;
                 db[fl_bufptr++]=col;
                 c=buffer[offset++];
                 x++;
                }
           }
        }
     }
   if (c=='\t')
     {
      x--;
      if (seeTabs)
        { // It paints the tabs to be visible
         do
           {
            db[fl_bufptr++]=' ';
            db[fl_bufptr++]=tabColor;
            x++;
           }
         while ((x%tabSize)!=0);
         ToggleTabColor();
        }
      else
        {
         do
           {
            db[fl_bufptr++]=' ';
            db[fl_bufptr++]=col;
            x++;
           }
         while ((x%tabSize)!=0);
        }
     }
   else
     {
      db[fl_bufptr++]=c;
      db[fl_bufptr++]=col;
     }
  }
 while (offset<lineLen && fl_bufptr<VisibleWidth);
}


static void PaintSelection(unsigned selStart,unsigned selEnd,unsigned lineStart,
                           char *db,char oldColor,unsigned lineLen,char *buffer,
                           unsigned tabSize,unsigned Width,Boolean TransparentSel,
                           int seeTabs, char tabChar)
{
 unsigned offset;
 int count;
 char color;

 char *s=buffer+lineStart;  // Is a pointer to the char in the buffer
 char *ori=s;
 offset=0;                  // Is the X coordinate

 // Colored tabs stuff
 tabFF=0;
 tabColor=OddTabColor;

 // Move s and offset to the start of the selection, calculate the length of
 // the selected area.
 count=selStart-lineStart;
 if (count>0)
   { // The start is inside the line
    while (count--)
      {
       AdvanceWithTab(*s,offset);
       ToggleTabColor();
       s++;
      }
    count=selEnd-selStart;
   }
 else
    count=selEnd-lineStart;

 // Prepare the colors
 color = oldColor & 0xF0;   // Only the background
 char colFor = ((unsigned char)color) >> 4;

 // Prepare a pointer in the buffer
 char *db2=db+((offset<<1)+1);

 // Adjust count
 int PaintAll;
 unsigned skiped=s-ori;
 if ((unsigned)count+skiped>lineLen)
   { // If the selection ends outside the line
    count=lineLen-skiped;
    PaintAll=1;
   }
 else
    PaintAll=0;

 if (TransparentSel)
   {
    // Paint the line in the transparent fashion
    while (count-- && offset<Width)
      {
       if (*s=='\t')
         {
          int tabcount = tabSize-(offset % tabSize);
          if (seeTabs)
            { // It paints the tabs to be visible
             while (tabcount--)
               {
                db2[0]=tabColor;
                db2[-1]=tabChar;
                db2+=2;
                offset++;
               }
             ToggleTabColor();
            }
          else
            {
             while (tabcount--)
               {
                *db2=oldColor;
                db2+=2;
                offset++;
               }
            }
         }
       else
         {
          char colOri=(*db2 & 0x0F);
          if (colOri==colFor)
            *db2=((colOri+1) & 0x0F) | color; // Ops!, the same color!
          else
            *db2=colOri | color;
          db2+=2;
          offset++;
         }
       s++;
      }
   }
 else
   {
    // Paint the line normally
    while (count-- && offset<Width)
      {
       if (*s=='\t')
         {
          int tabcount = tabSize-(offset % tabSize);
          if (seeTabs)
            { // It paints the tabs to be visible
             while (tabcount--)
               {
                db2[0]=tabColor;
                db2[-1]=tabChar;
                db2+=2;
                offset++;
               }
             ToggleTabColor();
            }
          else
            {
             while (tabcount--)
               {
                *db2=oldColor;
                db2+=2;
                offset++;
               }
            }
         }
       else
         {
          *db2=oldColor;
          db2+=2;
          offset++;
         }
       s++;
      }
   }

 // Paint outside it, if needed.
 if (PaintAll)
    while (offset<(unsigned)Width)
      {
       *db2=oldColor;
       db2+=2;
       offset++;
      }
}

#define FillRestOfLine() \
 { \
  count = Width-offset;  \
  if (count>0)           \
    {                    \
     while (count--)     \
        {                \
         ((char *)(DrawBuf))[fl_bufptr]=' ';      \
         ((char *)(DrawBuf))[fl_bufptr+1]=color;  \
         fl_bufptr+=2;   \
        }                \
    }                    \
 }

void TCEditor::formatLineHighLight(void *DrawBuf, unsigned LinePtr, int Width,
        ushort Colors, unsigned lineLen, uint32 Attr, unsigned , uint32 *colMarkers)
{
  unsigned offset;
  int count;
  char color;
  unsigned lineStart=LinePtr;
  char *db=(char *)DrawBuf;
  EDITOR = this; // for the two functions above
  fl_bufptr = 0;
  offset = 0;

  if (!lineLen)
    { // If empty just fill it with spaces
     color = normalColor;
    }
  else
  if (Attr & InsideCom)
    { // All comented
     color = CommentColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
  if ((Attr & PartialCom) && (Attr & Prepro))
    { // Partially comented
     PaintPreproNoSel(LinePtr,db,buffer,lineLen,Width,tabSize,Attr & StartInCom,
                      SeeTabs,TabChar);
     offset=fl_bufptr>>1;
     color = normalColor;
    }
  else
  if ((Attr & Prepro) && !(Attr & PartialCom))
    { // Preprocesor
     color = PreproColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
    {
     color = normalColor;
     offset=CALL4;
     fl_bufptr=offset<<1;
     SyntaxFormatLine( this,(char *)DrawBuf,lineStart,Width,Attr,lineLen,SeeTabs);
    }

  FillRestOfLine();

  // Now paint the selection OVER the buffer
  if (!selHided && selStart<selEnd && selStart<=lineLen+lineStart && selEnd>lineStart)
     PaintSelection(selStart,selEnd,lineStart,db,selectColor,
                    lineLen,buffer,tabSize,Width,TransparentSel,SeeTabs,TabChar);

  // Column markers over the selection
  if (colMarkers)
     PaintMarkers(db,ColMarkColor,Width,colMarkers);
}



void TCEditor::formatLineHighLightPascal(void *DrawBuf, unsigned LinePtr, int Width,
       ushort Colors, unsigned lineLen, uint32 Attr, unsigned , uint32 *colMarkers)
{
  unsigned offset;
  int count;
  char color;
  unsigned lineStart=LinePtr;
  char *db=(char *)DrawBuf;
  EDITOR = this; // for the two functions above
  fl_bufptr = 0;
  offset = 0;

  if (!lineLen)
    { // If empty just fill it with spaces
     color = normalColor;
    }
  else
  if (Attr & InsidePascalCom)
    { // All comented
     color = CommentColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
  if ((Attr & PartialCom) && (Attr & Prepro))
    { // Partially comented
     PaintPreproNoSel(LinePtr,db,buffer,lineLen,Width,tabSize,Attr & StartInCom,
                      SeeTabs,TabChar);
     offset=fl_bufptr>>1;
     color = normalColor;
    }
  else
  if ((Attr & Prepro) && !(Attr & PartialCom))
    { // Preprocesor
     color = PreproColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
    {
     color = normalColor;
     offset=CALL4;
     fl_bufptr=offset<<1;
     SyntaxFormatLinePascal( this,(char *)DrawBuf,lineStart,Width,
                                  Attr & StartInCom,Attr & StartInCom2,lineLen,SeeTabs);
    }

  FillRestOfLine();

  // Now paint the selection OVER the buffer
  if (!selHided && selStart<selEnd && selStart<=lineLen+lineStart && selEnd>lineStart)
     PaintSelection(selStart,selEnd,lineStart,db,selectColor,
                    lineLen,buffer,tabSize,Width,TransparentSel,SeeTabs,TabChar);

  // Column markers over the selection
  if (colMarkers)
     PaintMarkers(db,ColMarkColor,Width,colMarkers);
}



void TCEditor::formatLineHighLightClipper(void *DrawBuf, unsigned LinePtr, int Width,
       ushort Colors, unsigned lineLen, uint32 Attr, unsigned, uint32 *colMarkers)
{
  unsigned offset;
  int count;
  char color;
  unsigned lineStart=LinePtr;
  char *db=(char *)DrawBuf;
  EDITOR = this; // for the two functions above
  fl_bufptr = 0;
  offset = 0;

  if (!lineLen)
    { // If empty just fill it with spaces
     color = normalColor;
    }
  else
  if (Attr & InsideCom)
    { // All comented
     color = CommentColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
  if ((Attr & PartialCom) && (Attr & Prepro))
    { // Partially comented
     PaintPreproNoSel(LinePtr,db,buffer,lineLen,Width,tabSize,Attr & StartInCom,
                      SeeTabs,TabChar);
     offset=fl_bufptr>>1;
     color = normalColor;
    }
  else
  if ((Attr & Prepro) && !(Attr & PartialCom))
    { // Preprocesor
     color = PreproColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
    {
     color = normalColor;
     offset=CALL4;
     fl_bufptr=offset<<1;
     SyntaxFormatLineClipper( this,(char *)DrawBuf,lineStart,Width,Attr & StartInCom,
                              lineLen,SeeTabs);
    }

  FillRestOfLine();

  // Now paint the selection OVER the buffer
  if (!selHided && selStart<selEnd && selStart<=lineLen+lineStart && selEnd>lineStart)
     PaintSelection(selStart,selEnd,lineStart,db,selectColor,
                    lineLen,buffer,tabSize,Width,TransparentSel,SeeTabs,TabChar);

  // Column markers over the selection
  if (colMarkers)
     PaintMarkers(db,ColMarkColor,Width,colMarkers);
}


void TCEditor::formatLineHighLightGeneric(void *DrawBuf, unsigned LinePtr, int Width,
       ushort Colors, unsigned lineLen, uint32 Attr, unsigned, uint32 *colMarkers)
{
  unsigned offset;
  int count;
  char color;
  unsigned lineStart=LinePtr;
  char *db=(char *)DrawBuf;
  EDITOR = this; // for the two functions above
  fl_bufptr = 0;
  offset = 0;

  if (!lineLen)
    { // If empty just fill it with spaces
     color = normalColor;
    }
  else
  if (Attr & InsideGenericCom)
    { // All comented
     color = CommentColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
  if ((Attr & PartialCom) && (Attr & Prepro))
    { // Partially comented
     PaintPreproNoSel(LinePtr,db,buffer,lineLen,Width,tabSize,Attr & StartInGenericCom,
                      SeeTabs,TabChar);
     offset=fl_bufptr>>1;
     color = normalColor;
    }
  else
  if ((Attr & Prepro) && !(Attr & PartialCom))
    { // Preprocesor
     color = PreproColor;
     offset= CALL3;
     fl_bufptr=offset<<1;
    }
  else
    {
     color = normalColor;
     offset=CALL4;
     fl_bufptr=offset<<1;
     SyntaxFormatLineGeneric( this,(char *)DrawBuf,lineStart,Width,Attr,lineLen,
                              SeeTabs);
    }

  FillRestOfLine();

  // Now paint the selection OVER the buffer
  if (!selHided && selStart<selEnd && selStart<=lineLen+lineStart && selEnd>lineStart)
     PaintSelection(selStart,selEnd,lineStart,db,selectColor,
                    lineLen,buffer,tabSize,Width,TransparentSel,SeeTabs,TabChar);

  // Column markers over the selection
  if (colMarkers)
     PaintMarkers(db,ColMarkColor,Width,colMarkers);
}

// For Reserved words
static strSETSE stRW={NULL,NULL,NULL,NULL,NULL,0};
// For User words
static strSETSE stUW={NULL,NULL,NULL,NULL,NULL,0};
// For Pascal Reserved words
static strSETSE stPR={NULL,NULL,NULL,NULL,NULL,0};
// For Clipper Reserved words
static strSETSE stCR={NULL,NULL,NULL,NULL,NULL,0};

#if 0
/*****************************************************************************

  Function: SETSECreateTables

  Create the necesary tables for the search

*****************************************************************************/

static Boolean SETSECreateTables(char *&firstLetters,
                                 int *&lenTable,
                                 int *&firstWithLength,
                                 char **&mainTable,
                                 int *&equalCharsInNext,
                                 int &maxLen,
                                 TStrCol *TSC)
{
 int i;
 int total=TSC->getCount();
 int thisLen;
 maxLen=0;

 for (i=0; i<total; i++)
    {
     if ((thisLen=strlen((char *)TSC->at(i)))>maxLen)
        maxLen=thisLen;
    }

 // The following is nice but missaling data, some platforms doesn't support it.
 // Linux/PPC logs it as a warning at kernel level and Linux/SPARC64 just raise
 // a SIGBUS signal.
 // *:-P  One allocation for the five tables.
 //firstLetters=new char[total*(sizeof(char)+sizeof(char *)+sizeof(int))+(2*sizeof(int))*(maxLen+1)];
 firstLetters    =new char[total];
 lenTable        =new int[maxLen+1];
 firstWithLength =new int[maxLen+1];
 mainTable       =new char *[total];
 equalCharsInNext=new int[total];

 // Fill the length table
 for (i=0; i<=maxLen; i++)
    {
     firstWithLength[i]=0;
     lenTable[i]=0;
    }

 for (i=0; i<total; i++)
     lenTable[strlen((char *)TSC->at(i))]++;

 // Search & Clasify
 int pos=0;
 for (i=0; i<=maxLen; i++)
    {
     int j=0,n=0;
     while (j<lenTable[i])
       {
        char *s=(char *)TSC->at(n);
        if ((int)strlen(s)==i)
          {
           if (j==0)
              firstWithLength[i]=pos;
           mainTable[pos++]=s;
           j++;
          }
        n++;
       }
    }

 // Make the first letters
 for (i=0; i<total; i++)
     firstLetters[i]=mainTable[i][0];

 // Make the equality table
 for (pos=0,i=0; i<=maxLen; i++)
    {
     int j=lenTable[i];
     while (j)
       {
        if (j>1)
          {
           char *este=mainTable[pos];
           char *prox=mainTable[pos+1];
           int c;
           for (c=0; *este==*prox && *este; este++, prox++, c++);
           equalCharsInNext[pos]=c;
          }
        else
          equalCharsInNext[pos]=0;
        pos++;
        j--;
       }
    }
 return True;
}
#endif

Boolean SETSECreateTables(strSETSE &st, int CaseSens,
                          TStringCollection *TSC)
{
 int i;
 int total=TSC->getCount();
 int thisLen;
 st.maxLen=0;

 for (i=0; i<total; i++)
    {
     char *elem=(char *)TSC->at(i);
     if ((thisLen=strlen(elem))>st.maxLen)
        st.maxLen=thisLen;
     if (!CaseSens)
        strlwr(elem);
    }

 st.firstLetters    =new char[total];
 st.lenTable        =new int[st.maxLen+1];
 st.firstWithLength =new int[st.maxLen+1];
 st.mainTable       =new char *[total];
 st.equalCharsInNext=new int[total];

 // Fill the length table
 for (i=0; i<=st.maxLen; i++)
    {
     st.firstWithLength[i]=0;
     st.lenTable[i]=0;
    }

 for (i=0; i<total; i++)
     st.lenTable[strlen((char *)TSC->at(i))]++;

 // Search & Clasify
 int pos=0;
 for (i=0; i<=st.maxLen; i++)
    {
     int j=0,n=0;
     while (j<st.lenTable[i])
       {
        char *s=(char *)TSC->at(n);
        if ((int)strlen(s)==i)
          {
           if (j==0)
              st.firstWithLength[i]=pos;
           st.mainTable[pos++]=s;
           j++;
          }
        n++;
       }
    }

 // Make the first letters
 for (i=0; i<total; i++)
     st.firstLetters[i]=st.mainTable[i][0];

 // Make the equality table
 for (pos=0,i=0; i<=st.maxLen; i++)
    {
     int j=st.lenTable[i];
     while (j)
       {
        if (j>1)
          {
           char *este=st.mainTable[pos];
           char *prox=st.mainTable[pos+1];
           int c;
           for (c=0; *este==*prox && *este; este++, prox++, c++);
           st.equalCharsInNext[pos]=c;
          }
        else
          st.equalCharsInNext[pos]=0;
        pos++;
        j--;
       }
    }
 return True;
}

/*****************************************************************************

  Function: SETSE_RW_Search

  Is the search engine of the SETSE algorithm.
  The tables are sorted 1st by string length and then by alphabetic order.
  When we need to search a string first we check if the length is valid, if
not the search ends, if the length is OK we go directly to the first element
with this length, then we use a table with the first letters of the list and
we search in this table until we found the letter or we exced the range of
word with the desired length ... puf ..., if we found a match we start to
compare the whole string, if the any letter fails we ask if the following
string have enough equal letters (there is a table for this) if not we return,
if yes we continue the comparison in the next string, not from the start the
start because we know that this string match the first letters.
  The goal of the system is: We never retrace in the string index we ever
advance or still in the same index.
  If you don't understand what I'm saying don't worry my english is a
disaster and the algorithm is a little tricky.

*****************************************************************************/

#define is_reserved(a,b)  SETSE_RW_Search(a,b)
static int SETSE_RW_Search(char *s, int l)
{
 // Is in the length range?
 if (l<=stRW.maxLen)
   {
    int cant=stRW.lenTable[l];
    // Exist any of this length?
    if (cant)
      {
       // Take the pos of the first string of this length
       int pos=stRW.firstWithLength[l];
       do
         {
          // Search a string of this length that starts with the same character
          if (stRW.firstLetters[pos]==*s)
            {
             int equals=1;
             // Compare the string
             char *aux=stRW.mainTable[pos]+1;
             s++;
RWProxSSearch1:
             if (--l==0) // Eureka!
                return pos+1; // +1 because the position 0 exist
RWProxSSearch2:
             if (*aux==*s)
               {
                // Next char
                equals++;
                s++;
                aux++;
                goto RWProxSSearch1;
               }
             // Is usefull the next string?
             if (stRW.equalCharsInNext[pos]>=equals)
               {
                // Yes!, use it, but don't waste the work already done
                aux=stRW.mainTable[++pos]+equals;
                goto RWProxSSearch2;
               }
             return 0;
            }
          pos++;
         }
       while (--cant);
      }
   }
 return 0;
}

static
int SETSE_UW_Search(char *s, int l)
{
 // Is in the length range?
 if (l<=stUW.maxLen)
   {
    int cant=stUW.lenTable[l];
    // Exist any of this length?
    if (cant)
      {
       // Take the pos of the first string of this length
       int pos=stUW.firstWithLength[l];
       do
         {
          // Search a string of this length that starts with the same character
          if (stUW.firstLetters[pos]==*s)
            {
             int equals=1;
             // Compare the string
             char *aux=stUW.mainTable[pos]+1;
             s++;
UWProxSSearch1:
             if (--l==0) // Eureka!
                return pos+1;
UWProxSSearch2:
             if (*aux==*s)
               {
                // Next char
                equals++;
                s++;
                aux++;
                goto UWProxSSearch1;
               }
             // Is usefull the next string?
             if (stUW.equalCharsInNext[pos]>=equals)
               {
                // Yes!, use it, but don't waste the work already done
                aux=stUW.mainTable[++pos]+equals;
                goto UWProxSSearch2;
               }
             return 0;
            }
          pos++;
         }
       while (--cant);
      }
   }
 return 0;
}

#define is_pascal_rword(a,b) SETSE_PR_Search(a,b)
static int SETSE_PR_Search(char *s, int l)
{
 // Is in the length range?
 if (l<=stPR.maxLen)
   {
    int cant=stPR.lenTable[l];
    // Exist any of this length?
    if (cant)
      {
       // Take the pos of the first string of this length
       int pos=stPR.firstWithLength[l];
       uchar c=TVCodePage::toLower(*s);
       do
         {
          // Search a string of this length that starts with the same character
          if (stPR.firstLetters[pos]==c)
            {
             int equals=1;
             // Compare the string
             char *aux=stPR.mainTable[pos]+1;
             s++; c=TVCodePage::toLower(*s);
PRProxSSearch1:
             if (--l==0) // Eureka!
                return pos+1;
PRProxSSearch2:
             if (*aux==c)
               {
                // Next char
                equals++;
                s++; c=TVCodePage::toLower(*s);
                aux++;
                goto PRProxSSearch1;
               }
             // Is usefull the next string?
             if (stPR.equalCharsInNext[pos]>=equals)
               {
                // Yes!, use it, but don't waste the work already done
                aux=stPR.mainTable[++pos]+equals;
                goto PRProxSSearch2;
               }
             return 0;
            }
          pos++;
         }
       while (--cant);
      }
   }
 return 0;
}

#define is_clipper_rword(a,b) SETSE_CR_Search(a,b)
static int SETSE_CR_Search(char *s, int l)
{
 // Is in the length range?
 if (l<=stCR.maxLen)
   {
    int cant=stCR.lenTable[l];
    // Exist any of this length?
    if (cant)
      {
       // Take the pos of the first string of this length
       int pos=stCR.firstWithLength[l];
       uchar c=TVCodePage::toLower(*s);
       do
         {
          // Search a string of this length that starts with the same character
          if (stCR.firstLetters[pos]==c)
            {
             int equals=1;
             // Compare the string
             char *aux=stCR.mainTable[pos]+1;
             s++; c=TVCodePage::toLower(*s);
CRProxSSearch1:
             if (--l==0) // Eureka!
                return pos+1;
CRProxSSearch2:
             if (*aux==c)
               {
                // Next char
                equals++;
                s++; c=TVCodePage::toLower(*s);
                aux++;
                goto CRProxSSearch1;
               }
             // Is usefull the next string?
             if (stCR.equalCharsInNext[pos]>=equals)
               {
                // Yes!, use it, but don't waste the work already done
                aux=stCR.mainTable[++pos]+equals;
                goto CRProxSSearch2;
               }
             return 0;
            }
          pos++;
         }
       while (--cant);
      }
   }
 return 0;
}

static
int isGeneralReservedWord(char *s, int l, int CaseSens, strSETSE &st)
{
 // Is in the length range?
 if (l<=st.maxLen)
   {
    int cant=st.lenTable[l];
    // Exist any of this length?
    if (cant)
      {
       // Take the pos of the first string of this length
       int pos=st.firstWithLength[l];
       uchar c = CaseSens ? *s : TVCodePage::toLower(*s);
       do
         {
          // Search a string of this length that starts with the same character
          if (st.firstLetters[pos]==c)
            {
             int equals=1;
             // Compare the string
             char *aux=st.mainTable[pos]+1;
             s++; c=CaseSens ? *s : TVCodePage::toLower(*s);
stProxSSearch1:
             if (--l==0) // Eureka!
                return pos+1;
stProxSSearch2:
             if (*aux==c)
               {
                // Next char
                equals++;
                s++; c=CaseSens ? *s : TVCodePage::toLower(*s);
                aux++;
                goto stProxSSearch1;
               }
             // Is usefull the next string?
             if (st.equalCharsInNext[pos]>=equals)
               {
                // Yes!, use it, but don't waste the work already done
                aux=st.mainTable[++pos]+equals;
                goto stProxSSearch2;
               }
             return 0;
            }
          pos++;
         }
       while (--cant);
      }
   }
 return 0;
}

static
int is_user_word(char *s, int l, int caseSens)
{
 if (TCEditor::strC.UserWords)
    return isGeneralReservedWord(s,l,caseSens,TCEditor::strC.SearchUserWords);
 return SETSE_UW_Search(s,l);
}

static
int isPartialKeyword(char *s, int l, int CaseSens, TStringCollection *keywords,
                     unsigned &end)
{
 // Do it ASCIIZ for the search
 AllocLocalStr(temp,l+1);
 memcpy(temp,s,l);
 temp[l]=0;
 if (!CaseSens)
    strlwr(temp);

 // Search for it
 ccIndex pos;
 Boolean res=keywords->search(temp,pos);
 if (res) // Full match
    return 1;

 // Now see if it could be a partial
 if (pos) // We ever get the next, but dont go negative
    pos--;
 char *test=(char *)keywords->at(pos);
 int i,len;
 len=strlen(test);
 if (len>=l) // Impossible, quit
    return 0;
 // Check if they match using the length of the keyword
 if (CaseSens)
   {
    for (i=0; i<len; i++)
       {
        if (s[i]!=test[i])
           return 0;
       }
   }
 else
   {
    for (i=0; i<len; i++)
       {
        if (s[i]!=TVCodePage::toLower(test[i]))
           return 0;
       }
   }
 // Ok, reduce the length of the word to the matched part
 end+=len-l;
 return 1;
}

void SETSEDeleteTables(strSETSE &st)
{
 if (st.firstLetters)
   {
    delete[] st.firstLetters;
    st.firstLetters=NULL;
    delete[] st.lenTable;
    delete[] st.firstWithLength;
    delete[] st.mainTable;
    delete[] st.equalCharsInNext;
   }
}

/*****************************************************************************

  Function: Boolean CreateSHShortCutTables(void)

  Initialize the search tables for ReservedWords and UserWords.

*****************************************************************************/

Boolean CreateSHShortCutTables(void)
{
 if (ReservedWords)
   {
    SETSEDeleteTables(stRW);
    if (!SETSECreateTables(stRW,1,ReservedWords))
       return False;
   }
 else
   stRW.maxLen=0;

 if (UserWords)
   {
    SETSEDeleteTables(stUW);
    if (!SETSECreateTables(stUW,1,UserWords))
       return False;
   }
 else
   stUW.maxLen=0;

 if (PascalRWords)
   {
    SETSEDeleteTables(stPR);
    if (!SETSECreateTables(stPR,1,PascalRWords))
       return False;
   }
 else
   stPR.maxLen=0;

 if (ClipperRWords)
   {
    SETSEDeleteTables(stCR);
    if (!SETSECreateTables(stCR,1,ClipperRWords))
       return False;
   }
 else
   stCR.maxLen=0;

 return True;
}


void DestroySHShortCutTables(void)
{
 SETSEDeleteTables(stRW);
 SETSEDeleteTables(stUW);
 SETSEDeleteTables(stPR);
 SETSEDeleteTables(stCR);
}


/*****************************************************************************

  Function: int is_integer_float(const char *name,uint32 l,uint32 rest)

  Parameters:
  name: string to analyze.
  l: length.
  rest: available chars in the line.

  Return:
  0 an invalid number.
  1 an integer.
  2 a float.

  Notes:
  Adapted for length.
  Added u and l.
  Added floats
  Added support for [uU][lL]{0,2} | [lL][uU]?[lL]?  |  [lL][lL]?[uU]?
  Added [lL] | [fF] for floats (Vik's recommendation)

  by SET based in the Robert's version.

*****************************************************************************/

#define OneOrZeroOf(a,b)                   \
         if (dispo==0) return 1;           \
         name++;                           \
         if (!isalnum((uchar)*name)) return 1;    \
         if (*name!=a && *name!=b)         \
            return 0;                      \
         dispo--

#define OkIfSepRet(a)                      \
         if (dispo==0) return a;           \
         name++;                           \
         if (!isalnum((uchar)*name)) return a

static int is_integer_float(const char *name, uint32 &dispo)
{
 while (dispo--)
  {
   if (!ucisdigit(*name))
     {
      if (*name=='U' || *name=='u') // [uU][lL]{0,2} with brut force
        {
         OneOrZeroOf('l','L');
         OneOrZeroOf('l','L');
         OkIfSepRet(1);
         return 0;
        }
      if (*name=='L'  || *name=='l') // [lL]...
        {
         OkIfSepRet(1);
         if (*name=='u' || *name=='U') // ...[uU][lL]?
           {
            dispo--;
            OneOrZeroOf('l','L');
            OkIfSepRet(1);
           }
         else
           if (*name=='l' || *name=='L') // ...[lL][uU]?
             {
              dispo--;
              OneOrZeroOf('u','U');
              OkIfSepRet(1);
             }
         return 0;
        }
      if (*name=='E' || *name=='e')
         goto TryFloat1;
      if (*name=='.')
         goto TryFloat0;
      if (!TVCodePage::isAlpha(*name))
        {
         dispo++;
         return 1;
        }
      return 0;
     }
   name++;
  }

 if (!dispo || *name!='.')
    return 1;

TryFloat0:
 name++;
 //dispo--;
 while (dispo--)
   {
    if (!ucisdigit(*name))
      {
       if (*name=='E' || *name=='e')
          goto TryFloat1;
       if (*name=='l' || *name=='L' || *name=='f' || *name=='F')
         {
          OkIfSepRet(2);
          return 0;
         }
       if (TVCodePage::isAlpha(*name) || *name=='.')
          return 0;
       else
         {
          dispo++;
          return 2;
         }
      }
    name++;
   }

 return 2;

TryFloat1:
 if (!dispo) return 0;
 name++;
 //dispo--;
 if (!ucisdigit(*name) && !((*name=='+' || *name=='-') && dispo && ucisdigit(*(name+1)))) return 0;
 name++;
 dispo--;
 while (dispo--)
   {
    if (!ucisdigit(*name))
      {
       if (*name=='l' || *name=='L' || *name=='f' || *name=='F')
         {
          OkIfSepRet(2);
          return 0;
         }
       if (TVCodePage::isAlpha(*name)) return 0;
       dispo++;
       return 2;
      }
    name++;
   }
 return 2;
}

/*****************************************************************************

  Function: int is_pascal_integer_float(const char *name,uint32 l,uint32 rest)

  Parameters:
  name: string to analyze.
  l: length.
  rest: available chars in the line.

  Return:
  0 an invalid number.
  1 an integer.
  2 a float.

  Notes:
  0.. is an integer + symbols.

  by SET.

*****************************************************************************/

static int is_pascal_integer_float(const char *name, uint32 &dispo)
{
 while (dispo--)
  {
   if (!ucisdigit(*name))
     {
      if (*name=='E' || *name=='e')
         goto TryFloat1;
      if (*name=='.')
         goto TryFloat0;
      if (!TVCodePage::isAlpha(*name))
        {
         dispo++;
         return 1;
        }
      return 0;
     }
   name++;
  }

 if (!dispo || *name!='.')
    return 1;

TryFloat0:
 name++;
 //dispo--;
 while (dispo--)
   {
    if (!ucisdigit(*name))
      {
       if (*name=='E' || *name=='e')
          goto TryFloat1;
       if (TVCodePage::isAlpha(*name))
          return 0;
       if (*name=='.')
         {
          dispo+=2;
          return 1;
         }
       dispo++;
       return 2;
      }
    name++;
   }

 return 2;

TryFloat1:
 if (!dispo) return 0;
 name++;
 //dispo--;
 if (!ucisdigit(*name) && !((*name=='+' || *name=='-') && dispo && ucisdigit(*(name+1)))) return 0;
 name++;
 dispo--;
 while (dispo--)
   {
    if (!ucisdigit(*name))
      {
       if (TVCodePage::isAlpha(*name)) return 0;
       return 2;
      }
    name++;
   }
 return 2;
}


/*****************************************************************************

  Function: int is_hex(const char *name,uint32 l)

  Parameters:
  name: string to analyze.
  l: length.

  Return:
  1 if is a valid hexadecimal number.

  Notes:
  Adapted for length.
  Added u,U,l and L.
  Added a check to avoid more than 16 digits, is that correct?

  by SET based in the Robert's version.

*****************************************************************************/

#define OneOrZeroOfHex(a,b)                \
         if (l==0) return (lori<=19);      \
         name++;                           \
         if (!isalnum((uchar)*name)) return (lori<=19);    \
         if (*name!=a && *name!=b)         \
            return 0;                      \
         l--; lori--

#define OkIfSepRetHex()                    \
         if (l==0) return (lori<=19);      \
         name++;                           \
         if (!isalnum((uchar)*name)) return (lori<=19)

static int is_hex(const char *name, uint32 l)
{
 uint32 lori=l;
 int notYetANumber=1;

 // We can't use a char * as an uint16 * in some RISC machines like SPARC
 //if (l<3 || (*(uint16 *)name!=0x7830 && *(uint16 *)name!=0x5830)) return 0;
 if (l<3 || name[0]!='0' || (name[1]!='x' && name[1]!='X')) return 0;
 name+=2;
 l-=2;
 while (l--)
   {
    if (!ucisxdigit(*name))
      {
       if (*name=='U' || *name=='u') // [uU][lL]{0,2} with brut force
         {
          OneOrZeroOfHex('l','L');
          OneOrZeroOfHex('l','L');
          OkIfSepRetHex();
          return 0;
         }
       if (*name=='L'  || *name=='l') // [lL]...
         {
          OkIfSepRetHex();
          if (*name=='u' || *name=='U') // ...[uU][lL]?
            {
             l--; lori--;
             OneOrZeroOfHex('l','L');
             OkIfSepRetHex();
            }
          else
            if (*name=='l' || *name=='L') // ...[lL][uU]?
              {
               l--; lori--;
               OneOrZeroOfHex('u','U');
               OkIfSepRetHex();
              }
         }
       return 0;
      }
    if (notYetANumber)
      {
       if (*name=='0')
          lori--;
       else
          notYetANumber=0;
      }
    name++;
   }
 return (lori<=10);
}

/*****************************************************************************

  Function: int is_pascal_hex(const char *name,uint32 &dispo)

  Parameters:
  name: string to analyze.
  dispo: disponible length.

  Return:
  1 if is a valid hexadecimal number for pascal.

  by SET.

*****************************************************************************/

static int is_pascal_hex(const char *name, uint32 &dispo)
{
 name++;
 if (!ucisxdigit(*name))
    return 0;
 while (--dispo)
   {
    if (!ucisxdigit(*name))
      {
       if (*name=='.')
          return 0;
       return 1;
      }
    name++;
   }
 return 1;
}

static int is_octal(const char *name,uint32 l)
{
 // The first is a 0
 //while (*(name++)=='0') l--;
 //if (l==0) return 1;
 while (l--)
  {
   if (*name<'0' || *name>'7') return 0;
   name++;
  }
 return 1;
}

unsigned char TableTypesEditor[256]=
{
 0, // \0
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // \t
 0, // \n
 0, // 
 0, // 
 0, // \r
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // 
 0, // x1A
 0, // x1B
 0, // 
 0, // 
 0, // 
 0, // 
 0, //  
 ttedIsClipSymb | ttedIsCsymb2 | ttedIsPasSymb2, // !
 0, // "
 ttedIsClipSymb | ttedIsPasSymb2, // #
 ttedIsClipSymb, // $
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // %
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // &
 0, // '
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // (
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // )
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // *
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // +
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // ,
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // -
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // .
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // /
 ttedIsWordChar, // 0
 ttedIsWordChar, // 1
 ttedIsWordChar, // 2
 ttedIsWordChar, // 3
 ttedIsWordChar, // 4
 ttedIsWordChar, // 5
 ttedIsWordChar, // 6
 ttedIsWordChar, // 7
 ttedIsWordChar, // 8
 ttedIsWordChar, // 9
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // :
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // ;
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // <
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // =
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // >
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // ?
 ttedIsClipSymb | ttedIsPasSymb2, // @
 ttedIsWordChar, // A
 ttedIsWordChar, // B
 ttedIsWordChar, // C
 ttedIsWordChar, // D
 ttedIsWordChar, // E
 ttedIsWordChar, // F
 ttedIsWordChar, // G
 ttedIsWordChar, // H
 ttedIsWordChar, // I
 ttedIsWordChar, // J
 ttedIsWordChar, // K
 ttedIsWordChar, // L
 ttedIsWordChar, // M
 ttedIsWordChar, // N
 ttedIsWordChar, // O
 ttedIsWordChar, // P
 ttedIsWordChar, // Q
 ttedIsWordChar, // R
 ttedIsWordChar, // S
 ttedIsWordChar, // T
 ttedIsWordChar, // U
 ttedIsWordChar, // V
 ttedIsWordChar, // W
 ttedIsWordChar, // X
 ttedIsWordChar, // Y
 ttedIsWordChar, // Z
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // [
 0, // \.
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // ]
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // ^
 ttedIsWordChar, // _
 0, // `
 ttedIsWordChar, // a
 ttedIsWordChar, // b
 ttedIsWordChar, // c
 ttedIsWordChar, // d
 ttedIsWordChar, // e
 ttedIsWordChar, // f
 ttedIsWordChar, // g
 ttedIsWordChar, // h
 ttedIsWordChar, // i
 ttedIsWordChar, // j
 ttedIsWordChar, // k
 ttedIsWordChar, // l
 ttedIsWordChar, // m
 ttedIsWordChar, // n
 ttedIsWordChar, // o
 ttedIsWordChar, // p
 ttedIsWordChar, // q
 ttedIsWordChar, // r
 ttedIsWordChar, // s
 ttedIsWordChar, // t
 ttedIsWordChar, // u
 ttedIsWordChar, // v
 ttedIsWordChar, // w
 ttedIsWordChar, // x
 ttedIsWordChar, // y
 ttedIsWordChar, // z
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // {
 ttedIsClipSymb | ttedIsPasSymb2 | ttedIsCsymb2, // |
 ttedIsClipSymb | ttedIsPasSymb1 | ttedIsCsymb1, // }
 ttedIsCsymb2, // ~
 0, // 
 ttedIsWordChar, // €
 ttedIsWordChar, // 
 ttedIsWordChar, // ‚
 ttedIsWordChar, // ƒ
 ttedIsWordChar, // „
 ttedIsWordChar, // …
 ttedIsWordChar, // †
 ttedIsWordChar, // ‡
 ttedIsWordChar, // ˆ
 ttedIsWordChar, // ‰
 ttedIsWordChar, // Š
 ttedIsWordChar, // ‹
 ttedIsWordChar, // Œ
 ttedIsWordChar, // 
 ttedIsWordChar, // Ž
 ttedIsWordChar, // 
 ttedIsWordChar, // 
 ttedIsWordChar, // ‘
 ttedIsWordChar, // ’
 ttedIsWordChar, // “
 ttedIsWordChar, // ”
 ttedIsWordChar, // •
 ttedIsWordChar, // –
 ttedIsWordChar, // —
 ttedIsWordChar, // ˜
 ttedIsWordChar, // ™
 ttedIsWordChar, // š
 0, // ›
 0, // œ
 0, // 
 0, // ž
 0, // Ÿ
 ttedIsWordChar, //  
 ttedIsWordChar, // ¡
 ttedIsWordChar, // ¢
 ttedIsWordChar, // £
 ttedIsWordChar, // ¤
 ttedIsWordChar, // ¥
 0, // ¦
 0, // §
 0, // ¨
 0, // ©
 0, // ª
 0, // «
 0, // ¬
 0, // ­
 0, // ®
 0, // ¯
 0, // °
 0, // ±
 0, // ²
 0, // ³
 0, // ´
 0, // µ
 0, // ¶
 0, // ·
 0, // ¸
 0, // ¹
 0, // º
 0, // »
 0, // ¼
 0, // ½
 0, // ¾
 0, // ¿
 0, // À
 0, // Á
 0, // Â
 0, // Ã
 0, // Ä
 0, // Å
 0, // Æ
 0, // Ç
 0, // È
 0, // É
 0, // Ê
 0, // Ë
 0, // Ì
 0, // Í
 0, // Î
 0, // Ï
 0, // Ð
 0, // Ñ
 0, // Ò
 0, // Ó
 0, // Ô
 0, // Õ
 0, // Ö
 0, // ×
 0, // Ø
 0, // Ù
 0, // Ú
 0, // Û
 0, // Ü
 0, // Ý
 0, // Þ
 0, // ß
 0, // à
 ttedIsWordChar, // á
 0, // â
 0, // ã
 0, // ä
 0, // å
 0, // æ
 0, // ç
 0, // è
 0, // é
 0, // ê
 0, // ë
 0, // ì
 0, // í
 0, // î
 0, // ï
 0, // ð
 0, // ñ
 0, // ò
 0, // ó
 0, // ô
 0, // õ
 0, // ö
 0, // ÷
 0, // ø
 0, // ù
 0, // ú
 0, // û
 0, // ü
 0, // ý
 0, // þ
 0  // ÿ
};


/*****************************************************************************

  Function: int is_symbol(char *name, uint32 l)

  Parameters:
  name: string to analyze.
  l: length.

  Return:
  1 if is a group of symbols.

  by SET based in a Molnar's idea.

*****************************************************************************/

static int is_symbol(char *name, uint32 l)
{
 int ret=0;

 while (l--)
  {
   if ((ret=TableTypesEditor[(uchar)*(name++)] & ttedIsCsymb)==0)
      return 0;
  }
 return ret & ttedIsCsymb1 ? 1 : 2;
}

/*****************************************************************************

  Function: int is_pascal_symbol(char *name, uint32 l)

  Parameters:
  name: string to analyze.
  l: length.

  Return:
  1 if is a group of symbols.

  Differences with C:
  @ and # are symbols.

  by SET.

*****************************************************************************/

static int is_pascal_symbol(char *name, uint32 l)
{
 int ret=0;

 while (l--)
  {
   if ((ret=TableTypesEditor[(uchar)*(name++)] & ttedIsPasSymb)==0)
      return 0;
  }
 return ret & ttedIsPasSymb1 ? 1 : 2;
}

/*****************************************************************************

  Function: int is_clipper_symbol(char *name, uint32 l)

  Parameters:
  name: string to analyze.
  l: length.

  Return:
  1 if is a group of symbols.

  Differences with C:
  @,# and $ are symbols.

  by SET.

*****************************************************************************/

static int is_clipper_symbol(char *name, uint32 l)
{
 int ret=0;

 while (l--)
  {
   if ((ret=TableTypesEditor[(uchar)*(name++)] & ttedIsClipSymb)==0)
      return 0;
  }
 return 1;
}


inline
static int isWordChar(char c)
{
  return TVCodePage::isAlNum(c) || c == '_';
}


void SyntaxFormatLine( TCEditor * editor,
                       char *DrawBuf,
                       uint32 LinePtr,
                       int Width,
                       uint32 Attr,
                       unsigned lineLength,
                       int      seeTabs
                     )
{
  unsigned bufptr;
  unsigned offset;
  unsigned tabSize=editor->tabSize;
  char color;
  int in_c_comment=Attr & StartInCom;
  int in_string=Attr & StartString;
  uint32 word_end;

  // Colored tabs stuff
  tabFF=0;
  tabColor=OddTabColor;
 
  EDITOR = editor;
  BUFFER = editor->buffer;
  DrawBuf++;
  lineptr = LinePtr;
  bufptr = 0;
  offset = 0;

  // Draw until we reach the EOL of we filled the visible line
  while (offset<=lineLength && bufptr<(unsigned)Width)
   {
    char c2=0;
    int c2_valid = 0;

    // Take a word
    word_end = offset-1;
    do
     {
      word_end++;
     }
    while (word_end <= lineLength &&
           isWordChar(BUFFER[lineptr+word_end]));

    if (word_end > lineLength ||
        (word_end > offset && !isWordChar(BUFFER[lineptr+word_end])))
      word_end--;

    { // Analize the word
      char *name;
      char c1;
      uint32 LenOfName;

      name=&BUFFER[lineptr+offset];
      c1=*name;
      LenOfName=word_end-offset+1;

      if (word_end+1<=lineLength)
        {
         c2_valid = 1;
         c2 = name[1];
        }

      if (TVCodePage::isAlpha(c1))
        color = IdentColor;
      else
        color = NormalColor;

      if (in_c_comment)
        { // The line is commented by the last
         color = CommentColor;
         word_end=0;
         char *s=name=&BUFFER[lineptr];
         while (word_end<lineLength)
           {
            if (*s=='*')
              {
               if (word_end<lineLength)
                 {
                  s++;
                  word_end++;
                  if (*s=='/')
                    {
                     in_c_comment=0;
                     break;
                    }
                 } // more chars
              } // '*'
            s++;
            word_end++;
           } // Of while
        } // Is c comment
      else
         if (in_string || c1 == '\"')
           {
            uint32 ll=lineLength-1;
            char *s=name + (LenOfName - 1); // if in_string LenOfName > 1
            if (!(in_string && c1 == '\"')) // the line may be start with \"
                                            // which means the end of the string
                while (word_end<ll)
                  {
                   s++;
                   word_end++;
                   if (*s=='\\')
                     {
                      if (word_end<ll)
                        {
                         s++;
                         word_end++;
                        }
                     }
                   else
                      if (*s=='\"')
                         break;
                  }
            color = StringColor;
            in_string = 0;
           }
      else
         if (c1 == '\'')
           {
            uint32 ll=lineLength-1;
            char *s=name;
            while (word_end<ll)
              {
               s++;
               word_end++;
               if (*s=='\\')
                 {
                  if (word_end<ll)
                    {
                     s++;
                     word_end++;
                    }
                 }
               else
                  if (*s=='\'')
                     break;
              }
            color = CharColor;
           }
      else
         if (c1 == '\\')
            color = IlegalColor;
      else
         if (c1 == '#')
            color = IlegalColor;
      else
         if (c1 == '/' && c2_valid && (c2 == '/' || c2 == '*'))
           {
            color = CommentColor;
            word_end++;
            if (c2=='/')
               word_end=lineLength-1; // Paint the rest of the line
            else
              {
               // Search the end
               char *s=name;
               s++;
               uint32 ll=lineLength-1;
               while (word_end<ll)
                 {
                  s++;
                  word_end++;
                  if (*s=='*')
                    {
                     if (word_end<ll)
                       {
                        s++;
                        word_end++;
                        if (*s=='/')
                           break;
                       } // more chars
                    } // '*'
                 } // Of while
              } // Is c comment
           }
      else
         if ((TVCodePage::isLower(c1) || c1=='_') && is_reserved(name,LenOfName))
            color = ResColor;
      else
         if ((TVCodePage::isAlpha(c1) || c1=='_') && is_user_word(name,LenOfName,1))
            color = UserColor;
      else
         if (ucisdigit(c1))
           {
            if (c1=='0'  && c2_valid && (c2=='x' || c2=='X' || ucisdigit(c2)))
              {
               // Octal or Hexa or Invalid
               if (is_hex(name,LenOfName))
                  color=HexColor;
               else
                  if (is_octal(name,LenOfName))
                     color=OctalColor;
                  else
                    {
                     color=IlegalColor;
                     // Is a float camuflated?
                     uint32 dispo=lineLength-word_end+LenOfName-1;
                     if (is_integer_float(name,dispo)==2)
                       {
                        color = FloatColor;
                        word_end=lineLength-dispo-1;
                       }
                    }
              }
            else
              {
               // Integer, float or Invalid
               // Calculate the available space in the line
               uint32 dispo=lineLength-word_end+LenOfName-1;
               // Check if the namber is an integer or a float or an invalid
               // sequence
               switch (is_integer_float(name,dispo))
                 {
                  case 0: color = IlegalColor;
                          break;
                  case 1: color = IntColor;
                          break;
                  case 2: color = FloatColor;
                          break;
                 }
               // Adjust the length according to the routine use
               word_end=lineLength-dispo-1;
              }
           }
      else
         if (c1=='.' && c2_valid && ucisdigit(c2))
           {
            uint32 dispo=lineLength-word_end+LenOfName-1;
            // Check if the namber is a float or an invalid sequence
            switch (is_integer_float(name,dispo))
              {
               case 0: color = IlegalColor;
                       break;
               case 2: color = FloatColor;
                       break;
              }
            // Adjust the length according to the routine use
            word_end=lineLength-dispo-1;
           }
      else
         switch (is_symbol(name,LenOfName))
           {
            case 1: color = SymbolColor;
                    break;
            case 2: color = Symbol2Color;
                    break;
           }

      while (offset<=word_end && bufptr<(unsigned)Width)
        {
         if (*(name++)=='\t')
           {
            int tabcount=tabSize-(bufptr % tabSize);
            if (seeTabs)
              { // It paints the tabs to be visible
               while (tabcount--)
                 DrawBuf[(bufptr++)*2]=tabColor;
               ToggleTabColor();
              }
            else
              {
               while (tabcount--)
                 DrawBuf[(bufptr++)*2]=color;
              }
           }
         else
           {
            DrawBuf[bufptr*2]=color;
            bufptr++;
           }
         offset++;
        }
    }
   }
}



void SyntaxFormatLinePascal( TCEditor * editor,
                       char *DrawBuf,
		       uint32 LinePtr,
		       int Width,
                       int check_comment_1,
                       int check_comment_2,
                       unsigned lineLength,
                       int      seeTabs
		     )
{
  unsigned bufptr;
  unsigned tabSize=editor->tabSize;
  unsigned offset;
  char color;
  // Colored tabs stuff
  tabFF=0;
  tabColor=OddTabColor;
 
  EDITOR = editor;
  BUFFER = editor->buffer;
  DrawBuf++;
  lineptr = LinePtr;
  bufptr = 0;
  offset = 0;

  {
    int in_comment1=check_comment_1;
    int in_comment2=check_comment_2;
    uint32 word_end;
    while (offset<=lineLength && bufptr<(unsigned)Width)
     {
      char c2=0;
      int c2_valid = 0;
      word_end = offset-1;
      do
       {
        word_end++;
       }
      while (word_end <= lineLength &&
             isWordChar(BUFFER[lineptr+word_end]));

      if (word_end > lineLength ||
          (word_end > offset && !isWordChar(BUFFER[lineptr+word_end])))
        word_end--;

      {
        char *name;
        char c1;
        uint32 LenOfName;

        name=&BUFFER[lineptr+offset];
        c1=*name;
        LenOfName=word_end-offset+1;

        if (word_end+1<=lineLength)
          {
           c2_valid = 1;
           c2 = BUFFER[lineptr+offset+1];
          }

        if (TVCodePage::isAlpha(c1))
          color = IdentColor;
        else
          color = NormalColor;

        if (in_comment1)
          { // The line is commented by the last
           color = CommentColor;
           word_end=0;
           char *s=name=&BUFFER[lineptr];
           while (word_end<lineLength)
             {
              s++;
              word_end++;
              if (*s=='*')
                {
                 if (word_end<lineLength)
                   {
                    s++;
                    word_end++;
                    if (*s==')')
                      {
                       in_comment1=0;
                       break;
                      }
                   } // more chars
                } // '*'
             } // Of while
          } // Is comment 1
        else
        if (in_comment2)
          { // The line is commented by the last
           color = CommentColor;
           word_end=0;
           char *s=name=&BUFFER[lineptr];
           while (word_end<lineLength)
             {
              s++;
              word_end++;
              if (*s=='}')
                {
                 in_comment2=0;
                 break;
                } // '}'
             } // Of while
          } // Is comment 2
        else
           if (c1 == '\'')
             {
              uint32 ll=lineLength-1;
              char *s=name;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='\'')
                    break;
                }
              color = StringColor;
             }
        else
           if (c1 == '\\')
              color = IlegalColor;
        else
           if (c1 == '(' && c2_valid && c2 == '*')
             {
              color = CommentColor;
              word_end++;
              // Search the end
              char *s=name;
              s++;
              uint32 ll=lineLength-1;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='*')
                   {
                    if (word_end<ll)
                      {
                       s++;
                       word_end++;
                       if (*s==')')
                          break;
                      } // more chars
                   } // '*'
                } // Of while
             }
        else
           if (c1 == '{')
             {
              color = CommentColor;
              word_end++;
              // Search the end
              char *s=name;
              s++;
              uint32 ll=lineLength-1;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='}')
                    break;
                } // Of while
             }
        else
           if (is_pascal_rword(name,LenOfName))
              color = ResColor;
        else
           if (is_user_word(name,LenOfName,1))
              color = UserColor;
        else
           if (c1=='$')
             {
              uint32 dispo=lineLength-word_end+LenOfName-1;
              if (dispo && is_pascal_hex(name,dispo))
                 color=HexColor;
              else
                {
                 color=IlegalColor;
                 dispo--;
                }
              word_end=lineLength-dispo-1;
             }
        else
           if (ucisdigit(c1))
             {
              // Integer, float or Invalid
              // Calculate the available space in the line
              uint32 dispo=lineLength-word_end+LenOfName-1;
              // Check if the namber is an integer or a float or an invalid
              // sequence
              switch (is_pascal_integer_float(name,dispo))
                {
                 case 0: color = IlegalColor;
                         break;
                 case 1: color = IntColor;
                         break;
                 case 2: color = FloatColor;
                         break;
                }
              // Adjust the length according to the routine use
              word_end=lineLength-dispo-1;
             }
        else
           if (c1=='.' && c2_valid && ucisdigit(c2))
             {
              color = IlegalColor;
              word_end++;
             }
        else
           if (c1=='.' && c2_valid && c2=='.')
             {
              color = Symbol2Color;
              word_end++;
             }
        else
           switch (is_pascal_symbol(name,LenOfName))
             {
              case 1: color = SymbolColor;
                      break;
              case 2: color = Symbol2Color;
                      break;
             }

        while (offset<=word_end && bufptr<(unsigned)Width)
          {
           if (*(name++)=='\t')
             {
              int tabcount=tabSize-(bufptr % tabSize);
              if (seeTabs)
                { // It paints the tabs to be visible
                 while (tabcount--)
                   DrawBuf[(bufptr++)*2]=tabColor;
                 ToggleTabColor();
                }
              else
                {
                 while (tabcount--)
                   DrawBuf[(bufptr++)*2]=color;
                }
             }
           else
             {
              DrawBuf[bufptr*2]=color;
              bufptr++;
             }
           offset++;
          }
      }
     }
  }
}


void SyntaxFormatLineClipper( TCEditor * editor,
                       char *DrawBuf,
		       uint32 LinePtr,
		       int Width,
                       int check_c_comment,
                       unsigned lineLength,
                       int      seeTabs
		     )
{
  unsigned bufptr;
  unsigned offset;
  unsigned tabSize=editor->tabSize;
  char color;
  // Colored tabs stuff
  tabFF=0;
  tabColor=OddTabColor;
 
  EDITOR = editor;
  BUFFER = editor->buffer;
  DrawBuf++;
  lineptr = LinePtr;
  bufptr = 0;
  offset = 0;

  {
    int in_c_comment=check_c_comment;
    uint32 word_end;
    while (offset<=lineLength && bufptr<(unsigned)Width)
     {
      char c2=0,c3=0;
      int c2_valid=0, c3_valid=0;
      word_end = offset-1;
      do
       {
        word_end++;
       }
      while (word_end <= lineLength &&
             isWordChar(BUFFER[lineptr+word_end]));

      if (word_end > lineLength ||
          (word_end > offset && !isWordChar(BUFFER[lineptr+word_end])))
        word_end--;

      {
        char *name;
        char c1;
        uint32 LenOfName;

        name=&BUFFER[lineptr+offset];
        c1=*name;
        LenOfName=word_end-offset+1;

        if (word_end+1<=lineLength)
          {
           c2_valid = 1;
           c2 = BUFFER[lineptr+offset+1];
          }
          
        if (word_end+2<=lineLength)
          {
           c3_valid = 1;
           c3 = BUFFER[lineptr+offset+2];
          }

        if (TVCodePage::isAlpha(c1))
          color = IdentColor;
        else
          color = NormalColor;

        if (in_c_comment)
          { // The line is commented by the last
           color = CommentColor;
           word_end=0;
           char *s=name=&BUFFER[lineptr];
           while (word_end<lineLength)
             {
              if (*s=='*')
                {
                 if (word_end<lineLength)
                   {
                    s++;
                    word_end++;
                    if (*s=='/')
                      {
                       in_c_comment=0;
                       // Check for extra chars after the end of a comment,
                       // Clipper 5 doesn't support this
                       while (word_end<lineLength-1)
                         {
                          s++;
                          if (!ucisspace(*s) && *s!=26) // Let the EOF
                            {
                             word_end=lineLength-1;
                             color=IlegalColor;
                             break;
                            }
                          word_end++;
                         }
                       break;
                      }
                   } // more chars
                } // '*'
              s++;
              word_end++;
             } // Of while
          } // Is c comment
        else
           // Strings as "xxxx"
           if (c1 == '\"')
             {
              uint32 ll=lineLength-1;
              char *s=name;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='\"')
                    break;
                }
              color = StringColor;
             }
        else
           // Strings as 'xxxx' are supported too
           if (c1 == '\'')
             {
              uint32 ll=lineLength-1;
              char *s=name;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='\'')
                    break;
                }
              color = StringColor;
             }
        else
           if (c1 == '\\')
              color = IlegalColor;
        else
           // The C++ comments and C comments are supported
           if (c1 == '/' && c2_valid && (c2 == '/' || c2 == '*'))
             {
              color = CommentColor;
              word_end++;
              if (c2=='/')
                 word_end=lineLength-1; // Paint the rest of the line
              else
                {
                 // Search the end
                 char *s=name;
                 s++;
                 uint32 ll=lineLength-1;
                 while (word_end<ll)
                   {
                    s++;
                    word_end++;
                    if (*s=='*')
                      {
                       if (word_end<ll)
                         {
                          s++;
                          word_end++;
                          if (*s=='/')
                             break;
                         } // more chars
                      } // '*'
                   } // Of while
                } // Is c comment
             }
        else
           // The && is like // in C++, is arkaic but ...
           if (c1=='&' && c2_valid && c2=='&')
             {
              color = CommentColor;
              word_end=lineLength-1; // Paint the rest of the line
             }
        else
           // The '* -' is like // in C++, is arkaic but ...
           if (c1=='*' && c2_valid && c2==' ' && c3_valid && c3=='-')
             {
              uint32 dispo=lineLength-word_end+LenOfName-1;
              if (dispo>3)
                {
                 // Clipper 5 doesn't support this if there is no comment
                 color = CommentColor;
                 word_end=lineLength-1; // Paint the rest of the line
                }
              else
                {
                 word_end+=2;
                 color=IlegalColor;
                }
             }
        else
           if (is_clipper_rword(name,LenOfName))
              color = ResColor;
        else
           if (is_user_word(name,LenOfName,0))
              color = UserColor;
        else
           if (ucisdigit(c1))
             {
              // Integer, float or Invalid
              // Calculate the available space in the line
              uint32 dispo=lineLength-word_end+LenOfName-1;
              // Check if the namber is an integer or a float or an invalid
              // sequence
              switch (is_integer_float(name,dispo))
                {
                 case 0: color = IlegalColor;
                         break;
                 case 1: color = IntColor;
                         break;
                 case 2: color = FloatColor;
                         break;
                }
              // Adjust the length according to the routine use
              word_end=lineLength-dispo-1;
             }
        else
           // Clipper supports .NNNN floats
           if (c1=='.' && c2_valid && ucisdigit(c2))
             {
              uint32 dispo=lineLength-word_end+LenOfName-1;
              // Check if the number is a float or an invalid sequence
              switch (is_integer_float(name,dispo))
                {
                 case 0: color = IlegalColor;
                         break;
                 case 2: color = FloatColor;
                         break;
                }
              // Adjust the length according to the routine use
              word_end=lineLength-dispo-1;
             }
        else
           // .t. .f. .and. .or. are special
           if (c1=='.' && c2_valid && TVCodePage::isAlpha(c2))
             {
              uint32 ll=lineLength-1;
              int ok=0;
              char *s=name+1;
              char *s1=s;
              while (word_end<ll)
                {
                 s++;
                 word_end++;
                 if (*s=='.')
                   {
                    word_end++;
                    ll=(uint32)(s-s1);
                    switch (ll)
                      {
                       case 1:
                            ok = *s1=='t' || *s1=='T' || *s1=='f' ||
                                 *s1=='F' || *s1=='y' || *s1=='Y' ||
                                 *s1=='n' || *s1=='N';
                            break;
                       case 2:
                            ok = (*s1=='o' || *s1=='O') &&
                                 (*(s1+1)=='r' || *(s1+1)=='R');
                            break;
                       case 3:
                            ok = ((*s1=='a'     || *s1=='A') &&
                                  (*(s1+1)=='n' || *(s1+1)=='N') &&
                                  (*(s1+2)=='d' || *(s1+2)=='D')) ||
                                 ((*s1=='n'     || *s1=='N') &&
                                  (*(s1+1)=='o' || *(s1+1)=='O') &&
                                  (*(s1+2)=='t' || *(s1+2)=='T'));
                            break;
                      }
                    if (ok)
                       color = ResColor;
                    else
                       color = IlegalColor;
                    break;
                   }
                 if (!TVCodePage::isAlpha(*s))
                   {
                    color = IlegalColor;
                    break;
                   }
                }
             }
        else
           if (is_clipper_symbol(name,LenOfName))
              color = SymbolColor;

        while (offset<=word_end && bufptr<(unsigned)Width)
          {
           if (*(name++)=='\t')
             {
              int tabcount=tabSize-(bufptr % tabSize);
              if (seeTabs)
                { // It paints the tabs to be visible
                 while (tabcount--)
                   DrawBuf[(bufptr++)*2]=tabColor;
                 ToggleTabColor();
                }
              else
                {
                 while (tabcount--)
                   DrawBuf[(bufptr++)*2]=color;
                }
             }
           else
             {
              DrawBuf[bufptr*2]=color;
              bufptr++;
             }
           offset++;
          }
      }
     }
  }
}

char CheckSeqCase;

int CheckForSequence(char *s, int len, int available, char *d)
{
 int pos;

 if (available<len || len==0)
    return 0;
 if (CheckSeqCase)
    for (pos=0; pos<len && s[pos]==d[pos]; pos++);
 else
    for (pos=0; pos<len && s[pos]==TVCodePage::toUpper(d[pos]); pos++);
 return pos==len;
}


int CheckForSequenceNotFirst(char *s, int len, int available, char *d)
{
 int pos;

 if (available<len || len==0)
    return 0;
 if (CheckSeqCase)
    for (pos=1; pos<len && s[pos]==d[pos]; pos++);
 else
    for (pos=1; pos<len && s[pos]==TVCodePage::toUpper(d[pos]); pos++);
 return pos==len;
}


void SyntaxFormatLineGeneric(TCEditor * editor,
                             char *DrawBuf,
                             uint32 LinePtr,
                             int Width,
                             uint32 Attr,
                             unsigned lineLength,
                             int      seeTabs
                            )
{
  unsigned bufptr;
  unsigned offset;
  unsigned tabSize=editor->tabSize;
  char color;
  int in_c_comment1=Attr & StartInCom;
  int in_c_comment2=Attr & StartInCom2;
  int in_string =Attr & StartString;
  int in_string2=Attr & StartString2;
  int in_string3=Attr & StartString3;
  uint32 word_end;
  char Escape;
  int TestNumbers,isInFirstCol=1,PartialKeywords,RelaxNumbers;
  int isInFirstUsed=1,checkEOLC1=1,checkEOLC2=1;
  int EscapeAnywhere;

  // Set the Case Sensitive comparation status to check sequence
  CheckSeqCase    =(TCEditor::strC.Flags1 & FG1_CaseSensitive)!=0;
  TestNumbers     =(TCEditor::strC.Flags1 & FG1_NoNumbers)==0;
  /*NoCheckFirstCol1=(TCEditor::strC.Flags1 & FG1_EOLCInFirstCol1)==0;
  NoCheckFirstCol2=(TCEditor::strC.Flags1 & FG1_EOLCInFirstCol2)==0;
  NoCheckFirstUse1=(TCEditor::strC.Flags1 & FG1_EOLCInFirstUse1)==0;
  NoCheckFirstUse2=(TCEditor::strC.Flags1 & FG2_EOLCInFirstUse2)==0;*/
  PartialKeywords =(TCEditor::strC.Flags1 & FG1_PartialKeyword)!=0;
  RelaxNumbers    =(TCEditor::strC.Flags1 & FG1_RelaxNumbers)!=0;
  EscapeAnywhere  =(TCEditor::strC.Flags2 & FG2_EscapeAnywhere)!=0;
  EDITOR = editor;
  // Colored tabs stuff
  tabFF=0;
  tabColor=OddTabColor;
 
  DrawBuf++;
  lineptr = LinePtr;
  BUFFER = &(editor->buffer[lineptr]);
  bufptr = 0;
  offset = 0;

  // Draw until we reach the EOL of we filled the visible line
  while (offset<=lineLength && bufptr<(unsigned)Width)
   {
    char c2=0;
    int c2_valid = 0;

    // Eat spaces
    color = NormalColor;
    while (offset<=lineLength && bufptr<(unsigned)Width && ucisspace(BUFFER[offset]))
      {
       if (BUFFER[offset]=='\t')
         {
          int tabcount=tabSize-(bufptr % tabSize);
          if (seeTabs)
            { // It paints the tabs to be visible
             while (tabcount--)
               DrawBuf[(bufptr++)*2]=tabColor;
             ToggleTabColor();
            }
          else
            {
             while (tabcount--)
               DrawBuf[(bufptr++)*2]=color;
            }
         }
       else
         {
          DrawBuf[bufptr*2]=color;
          bufptr++;
         }
       offset++;
      }
    if (offset>=lineLength || bufptr==(unsigned)Width)
       return;

    if (isInFirstCol && bufptr!=0)
      {
       isInFirstCol=0;
       if (TCEditor::strC.Flags1 & FG1_EOLCInFirstCol1)
          checkEOLC1=0;
       if (TCEditor::strC.Flags1 & FG1_EOLCInFirstCol2)
          checkEOLC2=0;
      }
    // Take a word
    word_end = offset;
    if (!in_c_comment1 && !in_c_comment2)
      {
       if (word_end<=lineLength && isWordCharBe(BUFFER[word_end]))
         {
          do
           {
            word_end++;
           }
          while (word_end<=lineLength && isWordCharIn(BUFFER[word_end]));
         }
   
       if (word_end > lineLength ||
           (word_end > offset && !isWordCharIn(BUFFER[word_end])))
         word_end--;
      }

    { // Analize the word
      char *name;
      char c1;
      uint32 LenOfName,BytesLeft;

      name=&BUFFER[offset];
      c1=*name;
      LenOfName=word_end-offset+1;
      BytesLeft=lineLength-offset;

      if (word_end+1<=lineLength)
        {
         c2_valid = 1;
         c2 = name[1];
        }

      if (TVCodePage::isAlpha(c1))
        color = IdentColor;
      else
        color = NormalColor;

      if (in_c_comment1)
        { // The line is commented by the last
         color=CommentColor;
         char *s=name=BUFFER+word_end;
         while (word_end<lineLength)
           {
            if (CheckForSequence(TCEditor::strC.CloseCom1,TCEditor::strC.lCloseCom1,BytesLeft,&BUFFER[word_end]))
              {
               word_end+=TCEditor::strC.lCloseCom1-1;
               in_c_comment1=0;
               break;
              }
            s++;
            word_end++;
            BytesLeft--;
           } // Of while
        } // Is c comment
      else
      if (in_c_comment2)
        { // The line is commented by the last
         color=CommentColor;
         char *s=name=BUFFER+word_end;
         while (word_end<lineLength)
           {
            if (CheckForSequence(TCEditor::strC.CloseCom2,TCEditor::strC.lCloseCom2,BytesLeft,&BUFFER[word_end]))
              {
               word_end+=TCEditor::strC.lCloseCom2-1;
               in_c_comment2=0;
               break;
              }
            s++;
            word_end++;
            BytesLeft--;
           } // Of while
        } // Is c comment
      else
         if (EscapeAnywhere && c2_valid && c1==TCEditor::strC.Escape)
           {
            color = SymbolColor;
            word_end=offset+1;
           }
      else
         if (in_string || isString(c1))
           {
            uint32 ll=lineLength-1;
            char *s=name + (LenOfName - 1); // if in_string LenOfName > 1
            Escape=TCEditor::strC.Escape;
            if (!(in_string && isString(c1))) // the line may be start with \"
                                            // which means the end of the string
                while (word_end<ll)
                  {
                   s++;
                   word_end++;
                   if (*s==Escape)
                     {
                      if (word_end<ll)
                        {
                         s++;
                         word_end++;
                        }
                     }
                   else
                      if (isString(*s))
                         break;
                  }
            color = StringColor;
            in_string = 0;
           }
      else // Repeated for string2
         if (in_string2 || isString2(c1))
           {
            uint32 ll=lineLength-1;
            char *s=name + (LenOfName - 1); // if in_string LenOfName > 1
            Escape=TCEditor::strC.Escape;
            if (!(in_string2 && isString2(c1))) // the line may be start with \"
                                            // which means the end of the string
                while (word_end<ll)
                  {
                   s++;
                   word_end++;
                   if (*s==Escape)
                     {
                      if (word_end<ll)
                        {
                         s++;
                         word_end++;
                        }
                     }
                   else
                      if (isString2(*s))
                         break;
                  }
            color = CharColor;
            in_string2 = 0;
           }
      else // Repeated for string3
         if (in_string3 || isString3(c1))
           {
            uint32 ll=lineLength-1;
            char *s=name + (LenOfName - 1); // if in_string LenOfName > 1
            Escape=TCEditor::strC.Escape;
            if (!(in_string3 && isString3(c1))) // the line may be start with \"
                                            // which means the end of the string
                while (word_end<ll)
                  {
                   s++;
                   word_end++;
                   if (*s==Escape)
                     {
                      if (word_end<ll)
                        {
                         s++;
                         word_end++;
                        }
                     }
                   else
                      if (isString3(*s))
                         break;
                  }
            color = StringColor;
            in_string3 = 0;
           }
      else
         if (isCharacter(c1))
           {
            uint32 ll=lineLength-1;
            char *s=name;
            Escape=TCEditor::strC.Escape;
            while (word_end<ll)
              {
               s++;
               word_end++;
               if (*s==Escape)
                 {
                  if (word_end<ll)
                    {
                     s++;
                     word_end++;
                    }
                 }
               else
                  if (isCharacter(*s))
                     break;
              }
            color = CharColor;
           }
      else
         if (c1==TCEditor::strC.Preprocessor || c1==TCEditor::strC.Preprocessor2)
            color = IlegalColor;
      else
         if (CheckForSequence(TCEditor::strC.OpenCom1,TCEditor::strC.lOpenCom1,BytesLeft,name))
           {
            color = CommentColor;
            word_end++;
            // Search the end
            char *s=name;
            s++;
            uint32 ll=lineLength-1;
            while (word_end<ll)
              {
               s++;
               word_end++;
               BytesLeft--;
               if (CheckForSequence(TCEditor::strC.CloseCom1,TCEditor::strC.lCloseCom1,BytesLeft,s))
                 {
                  word_end++;
                  break;
                 }
              } // Of while
           }
       else
         if (CheckForSequence(TCEditor::strC.OpenCom2,TCEditor::strC.lOpenCom2,BytesLeft,name))
           {
            color = CommentColor;
            word_end++;
            // Search the end
            char *s=name;
            s++;
            uint32 ll=lineLength-1;
            while (word_end<ll)
              {
               s++;
               word_end++;
               BytesLeft--;
               if (CheckForSequence(TCEditor::strC.CloseCom2,TCEditor::strC.lCloseCom2,BytesLeft,s))
                 {
                  word_end++;
                  break;
                 }
              } // Of while
           }
      else
         if ((checkEOLC1 &&
              CheckForSequence(TCEditor::strC.EOLCom1,TCEditor::strC.lEOLCom1,BytesLeft,name))
             ||
             (checkEOLC2 &&
              CheckForSequence(TCEditor::strC.EOLCom2,TCEditor::strC.lEOLCom2,BytesLeft,name)))
           {
            color = CommentColor;
            word_end=lineLength-1; // Paint the rest of the line
           }
      else
         if (CheckForSequence(TCEditor::strC.HexStart,TCEditor::strC.lHexStart,BytesLeft,name))
           {
            int extra=TCEditor::strC.lHexStart;
            uint32 dispo=lineLength-word_end+LenOfName-extra;
            name+=extra-1;
            if (dispo && is_pascal_hex(name,dispo))
               color=HexColor;
            else
              {
               color=IlegalColor;
               dispo--;
              }
            name-=extra-1;
            word_end=lineLength-dispo-1;
           }
      else
         if (TestNumbers && ucisdigit(c1))
           {
            // Integer, float or Invalid
            // Calculate the available space in the line
            uint32 dispo=lineLength-word_end+LenOfName-1;
            // Check if the namber is an integer or a float or an invalid
            // sequence
            switch (is_pascal_integer_float(name,dispo))
              {
               case 0: color = RelaxNumbers ? NormalColor : IlegalColor;
                       break;
               case 1: color = IntColor;
                       break;
               case 2: color = FloatColor;
                       break;
              }
            // Adjust the length according to the routine use
            word_end=lineLength-dispo-1;
           }
      else
         if (TestNumbers && c1=='.' && c2_valid && ucisdigit(c2))
           {
            uint32 dispo=lineLength-word_end+LenOfName-1;
            // Check if the namber is a float or an invalid sequence
            switch (is_integer_float(name,dispo))
              {
               case 0: color = RelaxNumbers ? NormalColor : IlegalColor;
                       break;
               case 2: color = FloatColor;
                       break;
              }
            // Adjust the length according to the routine use
            word_end=lineLength-dispo-1;
           }
      else
         if (!PartialKeywords && isGeneralReservedWord(name,LenOfName,CheckSeqCase,TCEditor::strC.Search))
            color = ResColor;
      else // Check if a user word does a full match before checking for a partial
           // match with a reserved (@ref v.s. @refMenu).
         if (is_user_word(name,LenOfName,CheckSeqCase))
            color = UserColor;
      else
         if (PartialKeywords && isPartialKeyword(name,LenOfName,CheckSeqCase,TCEditor::strC.Keywords,word_end))
            color = ResColor;
      else
         if (isSpecialSymb(*name) && c2_valid && isSpecialSCon(c2))
           {
            color = SymbolColor;
            word_end=offset+1;
           }
      else
         if (isSymbol1(*name))
           {
            color = SymbolColor;
            word_end=offset;
           }
      else
         if (isSymbol2(*name))
           {
            color = Symbol2Color;
            word_end=offset;
           }
      else
         if (c1==TCEditor::strC.Escape)
            color=IlegalColor;

      while (offset<=word_end && bufptr<(unsigned)Width)
        {
         if (*(name++)=='\t')
           {
            int tabcount=tabSize-(bufptr % tabSize);
            if (seeTabs)
              { // It paints the tabs to be visible
               while (tabcount--)
                 DrawBuf[(bufptr++)*2]=tabColor;
               ToggleTabColor();
              }
            else
              {
               while (tabcount--)
                 DrawBuf[(bufptr++)*2]=color;
              }
           }
         else
           {
            DrawBuf[bufptr*2]=color;
            bufptr++;
           }
         offset++;
        }
    }
    if (isInFirstUsed)
      {
       isInFirstUsed=0;
       if (TCEditor::strC.Flags1 & FG1_EOLCInFirstUse1)
          checkEOLC1=0;
       if (TCEditor::strC.Flags2 & FG2_EOLCInFirstUse2)
          checkEOLC2=0;
      }
   }
}
