/* Copyright (C) 1999-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#define Uses_stdio
#define Uses_ctype
#define Uses_stdlib
#define Uses_unistd
#define Uses_string
#define Uses_sys_stat
#define Uses_AllocLocal
#define Uses_TScroller
#define Uses_TNSCollection
#define Uses_TWindow
#define Uses_TRect
#define Uses_TPalette
#define Uses_TEvent
#define Uses_TFrame
#define Uses_TScrollBar
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TVCodePage
#define Uses_TScreen

#define Uses_TSLabel
#define Uses_TSVeGroup
#define Uses_TSInputLine
#define Uses_TSButton
#define Uses_TVOSClipboard
#define Uses_MsgBox

#define Uses_TCEditor_Commands

// First include creates the dependencies
#include <easydia1.h>
#include <ceditor.h>
// Second request the headers
#include <easydiag.h>

#define Uses_TEnhancedText
#define Uses_TManPageView
#define Uses_TManWindow

#include <manview.h>

#include <dyncat.h>
#include <diaghelp.h>
#include <rhutils.h>
#include <codepage.h>

static const char *ExtraOps;

// Maximun width of a line in a man page, 256 should be more than enough
int TEnhancedText::maxWidth=255;

static char *CreateTempManPage(char *str);
static char *CreateCommandLine(const char *file, const char *sections);

TEnhancedText::TEnhancedText(const char *aFileName, const char *aCommandLine) :
  TNSCollection(25,25)
{
 fileName=newStr(aFileName);
 commandLine=newStr(aCommandLine);
 isOK=1;
 rows=cols=0;
 xSelStart=ySelStart=xSelEnd=ySelEnd=0;

 FILE *f=fopen(aFileName,"rt");
 if (!f)
    isOK=0;
 else
   {
    char *s;
    int count;
    AllocLocalStr(buf,maxWidth+1);
    AllocLocalStr(line,maxWidth*2);

    while (fgets(buf,maxWidth,f))
      {// Parse it and create a character/attribute string
       int x=0;
       for (count=0, s=buf; *s!='\n' && *s!=0; s++)
          {
           if (s[0]==8)
             {
              if (count && s[1]) // Be sure we won't use -1, you never know
                {
                 if (s[-1]=='_')
                   {
                    line[(count-1)*2+1]=2;
                    line[(count-1)*2]=s[1];
                   }
                 else
                    // Is that true?
                    line[(count-1)*2+1]=1;
                 s++;
                }
             }
           else
             {
              if (*s!='\t')
                {
                 line[count*2]=*s;
                 line[count*2+1]=0;
                 count++;
                 x++;
                }
              else
                {
                 int size=8-x%8;
                 x+=size;
                 while (size--)
                   {
                    line[count*2]=' ';
                    line[count*2+1]=0;
                    count++;
                   }
                }
             }
          }
       LineOfEText *l=(LineOfEText *)new char[sizeof(LineOfEText)+count*2];
       l->len=count;
       memcpy(l->text,line,count*2);
       insert(l);
       rows++;
       if (count>cols)
          cols=count;
      }
    fclose(f);
   }
}

void TEnhancedText::copyLine(int y, int w, ushort *line, char *colors)
{
 ushort space;
 uchar *aux;
 aux=(uchar *)&space;
 aux[0]=' ';
 aux[1]=colors[0];
 aux=(uchar *)line;
 int width=w;

 int i;
 if (y>=getCount())
    for (i=0; i<w; i++) line[i]=space;
 else
   {
    LineOfEText *l=(LineOfEText *)at(y);
    if (l->len)
      {
       int len=min((int)l->len,w);
       char *d=(char *)line;
       char *s=(char *)l->text;
       for (i=0; i<len/*w*/; i++)
          {
           d[i*2]=s[i*2];
           d[i*2+1]=colors[s[i*2+1]];
          }
       w-=len;
       line+=len;
      }
    if (w)
       for (i=0; i<w; i++) line[i]=space;
   }
 // Selection post-processing
 if (y>=ySelStart && y<=ySelEnd)
   {
    //fprintf(stderr,"%d in %d-%d: ",y,ySelStart,ySelEnd);
    int xs=0, xe=width;
    if (y==ySelStart) xs=xSelStart;
    if (xs>width)     xs=width;
    if (y==ySelEnd)   xe=xSelEnd;
    if (xe>width)     xe=width;
    //fprintf(stderr,"%d a %d\n",xs,xe);
    while (xs<xe)
      {
       aux[xs*2+1]=(aux[xs*2+1] & 0xF) | 0x70;
       xs++;
      }
   }
}

Boolean TEnhancedText::hasSelection()
{
 if (ySelStart<ySelEnd ||
     (ySelStart==ySelEnd && xSelStart<xSelEnd))
    return True;
 return False;
}

void TEnhancedText::copyLineText(int y, int xs, int xe, char *dest)
{
 int i=xs;
 LineOfEText *l=(LineOfEText *)at(y);
 if (xs<(int)l->len)
   {
    char *s=(char *)l->text;
    int  xm=min((int)l->len,xe);
    for (; i<xm; dest++, i++)
        *dest=s[i*2];
   }
 for (; i<xe; dest++, i++)
     *dest=' ';
}

/**[txh]********************************************************************

  Description:
  Returns a newly allocated buffer (new[]) containing the selected text. The
size of the buffer is returned in the referenced variable.

  Return: The selection or NULL if nothing is selected.
  
***************************************************************************/

char *TEnhancedText::getSelection(unsigned &len)
{
 if (!hasSelection())
   {
    len=0;
    return NULL;
   }
 if (ySelStart==ySelEnd)
   {
    len=xSelEnd-xSelStart;
    char *res=new char[len];
    copyLineText(ySelStart,xSelStart,xSelEnd,res);
    return res;
   }
 LineOfEText *l;
 int y;
 len=ySelEnd-ySelStart;
 // Bytes in the first line
 l=(LineOfEText *)at(ySelStart);
 len+=xSelStart<(int)l->len ? l->len-xSelStart : 0;
 // In the middle lines
 for (y=ySelStart+1; y<ySelEnd; y++)
    {
     l=(LineOfEText *)at(y);
     len+=l->len;
    }
 // In last line
 len+=xSelEnd;
 // Allocate
 char *dest=new char[len];
 char *s=dest;
 // Fill it
 l=(LineOfEText *)at(ySelStart);
 if (xSelStart<(int)l->len)
   {
    copyLineText(ySelStart,xSelStart,l->len,s);
    s+=l->len-xSelStart;
   }
 *s='\n'; s++;
 for (y=ySelStart+1; y<ySelEnd; y++)
    {
     l=(LineOfEText *)at(y);
     copyLineText(y,0,l->len,s);
     s+=l->len;
     *s='\n'; s++;
    }
 copyLineText(y,0,xSelEnd,s);

 return dest;
}

TEnhancedText::~TEnhancedText()
{
 DeleteArray(fileName);
 DeleteArray(commandLine);
}

TManPageView::TManPageView(const TRect& bounds, TScrollBar *aHScrollBar,
                           TScrollBar *aVScrollBar)
  : TScroller(bounds,aHScrollBar,aVScrollBar)
{
 text=0;
 options |=ofSelectable;
 growMode =gfGrowHiX | gfGrowHiY;
 lockCount=0;
 mustBeRedrawed=False;
}

#define cManPage "\x06\x08\x09"

void (*TManPageView::InsertRoutine)(char *b, long l)=NULL;

TPalette& TManPageView::getPalette() const
{
 static TPalette palette(cManPage,sizeof(cManPage)-1);
 return palette;
}

void TManPageView::getScrollBars(TScrollBar *&hScr, TScrollBar *&vScr)
{
 hScr=hScrollBar;
 vScr=vScrollBar;
}

Boolean TManPageView::clipWinCopy(int id)
{
 Boolean res=False;
 if (text && text->hasSelection())
   {
    if (!TVOSClipboard::isAvailable())
      {
       messageBox(__("Sorry but no OS specific clipboard is available"),mfError | mfOKButton);
       return False;
      }
    unsigned len;
    char *buffer=text->getSelection(len);
    res=TVOSClipboard::copy(id,buffer,len) ? True : False;
    DeleteArray(buffer);
    if (!res)
      {
       messageBox(mfError | mfOKButton,__("Error copying to clipboard: %s"),
                  TVOSClipboard::getError());
       return False;
      }
   }
 return res;
}

void TManPageView::clipCopy()
{
 if (text && text->hasSelection())
   {
    unsigned len;
    char *buffer=text->getSelection(len);
    InsertRoutine(buffer,len);
    DeleteArray(buffer);
   }
}

void TManPageView::handleEvent( TEvent& event )
{
 TScroller::handleEvent(event);
 if (event.what==evMouseDown && event.mouse.doubleClick && text)
   {
    TPoint mouse=makeLocal(event.mouse.where);
    mouse.x+=delta.x;
    mouse.y+=delta.y;
    if (mouse.y<text->getCount())
      {
       LineOfEText *line=(LineOfEText *)text->at(mouse.y);
       int len=line->len,start=mouse.x;
       if (start<len)
         {
          uchar *s=(uchar *)&line->text;
          int end;
          // Ok, we have a pointer to the character under the cursor
          for (end=start; end<len && !ucisspace(s[end*2]) && s[end*2]!=')'; end++);
          if (s[end*2]==')')
            { // Only if we have something like ....)
             for (;start && !ucisspace(s[start*2]); start--);
             if (ucisspace(s[start*2])) start++;
             // OK, that's "something)", now copy it to a buffer
             char *aux;
             AllocLocalStr(name,end-start+2);
             for (aux=name; start<=end; start++,aux++) *aux=s[start*2];
             *aux=0;
             // Check we have only one () pair
             aux=strchr(name,'(');
             if (aux)
               { // We have "something(...)"
                char *section=aux+1;
                *aux=0;
                for (aux++; *aux && *aux!='(' && *aux!=')'; aux++);
                if (*aux==')' && !aux[1])
                  {
                   *aux=0;
                   // Try it.
                   char *cmd=CreateCommandLine(name,section);
                   char *tmp=CreateTempManPage(cmd);
                  
                   InsertText(new TEnhancedText(tmp,cmd));
                   message(owner,evBroadcast,cmMPUpdateTitle,name);
                   scrollTo(0,0);
                   unlink(tmp);
                   free(cmd);
                   free(tmp);
                   drawView();
                   //printf("Man page: %s sección: %s\n",name,section);
                  }
               }
            }
         }
      }
   }
 else if (text && event.what==evMouseDown && event.mouse.buttons==mbLeftButton)
   {// Mouse selection mechanism.
    TPoint mouse=makeLocal(event.mouse.where)+delta;
    int xSt=text->xSelStart=text->xSelEnd=mouse.x;
    int ySt=text->ySelStart=text->ySelEnd=mouse.y;
    draw();
    TPoint last=mouse;
    do
      {
       lock();
       if (event.what==evMouseAuto)
         {
          mouse=makeLocal(event.mouse.where);
          TPoint d=delta;
          if (mouse.x<0)
             d.x--;
          if (mouse.x>=size.x)
             d.x++;
          if (mouse.y<0)
             d.y--;
          if (mouse.y>=size.y)
             d.y++;
          scrollTo(d.x,d.y);
          mouse+=delta;
         if (mouse!=last)
            {
             if (mouse.y<ySt || (mouse.y==ySt && mouse.x<xSt))
               {
                text->xSelStart=mouse.x;
                text->ySelStart=mouse.y;
                text->xSelEnd=xSt;
                text->ySelEnd=ySt;
               }
             else
               {
                text->xSelStart=xSt;
                text->ySelStart=ySt;
                text->xSelEnd=mouse.x;
                text->ySelEnd=mouse.y;
               }
             last=mouse;
             //fprintf(stderr,"%d,%d - %d,%d\n",text->xSelStart,text->ySelStart,text->xSelEnd,text->ySelEnd);
             draw();
            }
         }
       unlock();
      }
    while (mouseEvent(event,evMouseMove+evMouseAuto));
    clearEvent(event);
    if (TVOSClipboard::isAvailable()>1)
       clipWinCopy(1);
   }
 else if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmcCopyClipWin:
            if (TVOSClipboard::isAvailable())
               clipWinCopy(0);
            break;
       case cmcCopy:
            if (InsertRoutine)
               clipCopy();
            break;
      }
   }
}

void TManPageView::setCmdState(uint16 command, Boolean enable)
{
 if (enable && (state & sfActive))
    enableCommand(command);
 else
    disableCommand(command);
}

void TManPageView::updateCommands()
{
 if (!(state & sfActive))
   { // We lost the focus, disable all
    setCmdState(cmcCopyClipWin,False);
    setCmdState(cmcCopy,False);
    return;
   }
 Boolean sel=(text && text->hasSelection()) ? True : False;
 setCmdState(cmcCopyClipWin,(TVOSClipboard::isAvailable() && sel) ? True : False);
 setCmdState(cmcCopy,(InsertRoutine && sel) ? True : False);
}

void TManPageView::InsertText(TEnhancedText *aText)
{
 if (!aText || !aText->isOK)
    return;
 if (text)
    CLY_destroy(text);
 text=aText;
 setLimit(aText->cols,aText->rows);
 //drawView();
}

void TManPageView::unlock()
{
 if (lockCount>0)
   {
    lockCount--;
    if (lockCount==0 && mustBeRedrawed)
       drawView();
   }
}

void TManPageView::draw()
{
 if (lockCount)
   {
    mustBeRedrawed=True;
    return;
   }
 mustBeRedrawed=False;

 char colors[3];
 colors[0]=getColor(1);
 colors[1]=getColor(2);
 colors[2]=getColor(3);
 
 int w=size.x+delta.x,y;
 AllocLocalUShort(line,w+1);
 if (!text || !text->isOK)
   { // Draw an empty window if something went wrong
    ushort space=' ' |  (colors[0]<<8);
    y=w;
    while (y--)
       line[y]=space;
    for (y=0; y<size.y; y++)
        writeLine(0,y,size.x,1,line);
    return;
   }

 for (y=0; y<size.y; y++)
    {
     text->copyLine(y+delta.y,w,line,colors);
     writeLine(0,y,size.x,1,line+delta.x);
    }
 if (state & sfActive)
    updateCommands();
}

void TManPageView::setState(uint16 aState, Boolean enable)
{
 TScroller::setState(aState,enable);
 if (aState==sfActive)
    updateCommands();
}

TManPageView::~TManPageView()
{
 CLY_destroy(text);
}

void TManPageView::write(opstream& os)
{
 TScroller::write(os);
 os.writeString(text->commandLine);
}

void *TManPageView::read(ipstream& is)
{
 TScroller::read(is);
 char *aCommandLine=is.readString();

 char *tmp=CreateTempManPage(aCommandLine);
 InsertText(new TEnhancedText(tmp,aCommandLine));
 unlink(tmp);
 free(tmp);
 DeleteArray(aCommandLine);
 lockCount=0;
 return this;
}

// New color: Like a cyan window plus 2 colors for bold and underline
#define cManWindow "\x8A\x8B\x8C\x8D\x8E\x8F\x90\x91\x92\x93"

TPalette& TManWindow::getPalette() const
{
 static TPalette palette(cManWindow,sizeof(cManWindow)-1);
 return palette;
}


static TRect defaultSizeWindow;
static
TRect &getDefaultSizeWindow()
{
 defaultSizeWindow.a.x=0;
 defaultSizeWindow.a.y=0;
 defaultSizeWindow.b.x=GetDeskTopCols()-4;
 defaultSizeWindow.b.y=GetDeskTopRows()-4;
 return defaultSizeWindow;
}

TManWindow::TManWindow(const char *fileName, const char *name,
                       char *aCommandLine, void (*ir)(char *b, long l)) :
       TWindowInit(&TManWindow::initFrame),
       TWindow(getDefaultSizeWindow(),name,wnNoNumber)
{
 TRect r=getDefaultSizeWindow();
 options|=ofCentered;
 r.grow(-1,-1);

 hScrollBar=standardScrollBar(sbHorizontal | sbHandleKeyboard);
 vScrollBar=standardScrollBar(sbVertical | sbHandleKeyboard);
 page=new TManPageView(r,hScrollBar,vScrollBar);
 page->InsertText(new TEnhancedText(fileName,aCommandLine));
 page->InsertRoutine=ir;
 unlink(fileName);
 insert(page);
 helpCtx=hcManPage;
}

TManWindow::TManWindow(StreamableInit) :
    TWindowInit(NULL),
    TWindow(streamableInit)
{
 page=0;
}

void TManWindow::handleEvent(TEvent& event)
{
 TWindow::handleEvent(event);
 if (event.what==evBroadcast)
   {
    switch (event.message.command)
      {
       case cmMPUpdateTitle:
            DeleteArray((char *)title);
            title=newStr((char *)event.message.infoPtr);
            frame->draw();
            break;
      }
   }
 else if (event.what==evKeyDown && event.keyDown.keyCode==kbEsc)
    close();
}

// These members are needed to initialize page propperly
// Note that page is saved *once*, not twice (from group and explicit)
void TManWindow::write(opstream &os)
{
 options&=~ofCentered;
 TWindow::write(os);
 os << page;
}

void *TManWindow::read(ipstream &is)
{
 TWindow::read(is);
 is >> page;
 page->getScrollBars(hScrollBar,vScrollBar);
 return this;
}


static
int isEmpty(const char *str)
{
 if (!str) return 1;
 for (; *str; str++)
    {
     if (!isspace((unsigned char)*str));
        return 0;
    }
 return 1;
}

static
char *CreateTempManPage(char *str)
{
 DynStrCatStruct st;

 char *tmp=unique_name("man");
 char *tmpErr=open_stderr();
 DynStrCatInit(&st,str);
 DynStrCat(&st," > ");
 DynStrCat(&st,tmp);
 TScreen::System(st.str);
 free(st.str);
 close_stderr();

 struct stat s;
 if (::stat(tmp,&s)!=0 || s.st_size==0)
   {// No stdout info
    unlink(tmp);
    free(tmp);
    return strdup(tmpErr);
   }
 unlink(tmpErr);
 //free(tmpErr); Nope, io.cc makes it
 return tmp;
}

static
char *CreateCommandLine(const char *file, const char *sections)
{
 DynStrCatStruct st;

 DynStrCatInit(&st,"man");
 if (ExtraOps && !isEmpty(ExtraOps))
   {
    DynStrCat(&st," ");
    DynStrCat(&st,(char *)ExtraOps);
   }
 if (sections && !isEmpty(sections))
   {
    #ifdef TVCompf_djgpp
    DynStrCat(&st," -s ");
    #else
    DynStrCat(&st," -S ");
    #endif
    DynStrCat(&st,(char *)sections);
   }
 DynStrCat(&st," ");
 DynStrCat(&st,(char *)file);

 return st.str;
}

TManWindow *CreateManWindow(const char *file, const char *sections,
                            const char *extraOps, void (*ir)(char *b, long l))
{
 ExtraOps=extraOps;
 char *cmd=CreateCommandLine(file,sections);
 char *tmp=CreateTempManPage(cmd);

 TManWindow *ret=new TManWindow(tmp,file,cmd,ir);
 free(cmd);
 free(tmp);

 return ret;
}

#if defined(SEOS_DOS) || defined(SEOS_Win32)
/*
  This routine checks if man is there. If we can't find it we must put a warning
*/
#ifdef TVComp_BCPP
//$todo: Check it better (to SAA)
#define RET_OK 0
#else
#define RET_OK 1
#endif

int CheckForMan(void)
{
 static int isManInstalled=0;

 if (!isManInstalled)
   {
    char *err=open_stdout();
    int ret=TScreen::System("man");
    close_stdout();
    unlink(err);

    if (ret==1)
       isManInstalled=RET_OK;
    else
       messageBox(__("You must install man to use it!"), mfError | mfOKButton);
   }

 return isManInstalled;
}
#else
int CheckForMan(void)
{
 return 1;
}
#endif

/*****************************************************************************

  Dialog used to select the manpage
    
*****************************************************************************/

ManPageOptions op={"setedit","",""};
extern char *strncpyZ(char *dest, const char *orig, int size);

TDialog *ManPageViewSelect(const char *name, ManPageOptions **mpo)
{
 if (!CheckForMan())
    return 0;

 if (name)
    strncpyZ(op.program,name,80);

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Man page to view")));

 TSVeGroup *options=
 MakeVeGroup(new TSLabel(__("~M~an page for ..."),new TSInputLine(prgLen,visibleLen)),
             new TSLabel(__("~S~ection"),new TSInputLine(sectLen,visibleLen)),
             new TSLabel(__("~E~xtra options"),new TSInputLine(extraLen,visibleLen)),
             0);

 col->insert(2,2,options);
 EasyInsertOKCancel(col);

 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;

 *mpo=&op;
 return d;
}

