/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]**********************************************************************

  Class: TEditorCollection
  Comments:
  This class is used to hold all the objects from @x{TDskWin (class)}. Is a
collection specially adapted for this task.
@p
  The class support three different types of windows, they are: Editors,
Closed and Non editors. For this reason the class keep track of the number
of windows of each type in the members: Editors, Closed and nonEditors.
The order in the list of classes is: Editors, Non editors and at the end
Closed editors.

**********/
/**[txh]***

  Class: TListEditors
  Comments:
  Is a derived class from TListBox used in the dialog of the list of windows.

*****************************************************************************/

#include <ceditint.h>
#define Uses_string
#define Uses_sys_stat
#define Uses_alloca
#define Uses_TCollection
#define Uses_TListBox
#define Uses_TStreamable
#define Uses_TCEditWindow
#define Uses_TWindow
#define Uses_TApplication
#define Uses_TDeskTop
#define Uses_TScreen
#define Uses_TSSortedListBox
#define Uses_TCEditor_Commands
#include <easydia1.h>
#include <ceditor.h>
#include <easydiag.h>

#include <dskwin.h>
#include <dskedito.h>
#include <dskclose.h>
#include <edcollec.h>
#define Uses_TSetEditorApp
#include <setapp.h>
#include <assert.h>
#include <edspecs.h>

// From edprj.cc
void UpdateProjectResumeFor(char *fileName, TCEditWindow *p);
//   This class is quite particular. Only one object should exist at the same
// time because it controls the desktop (which is only one). But isn't fully
// static because it inherits from TCollection.
//   The only object that can exist is edHelper. During the read process the
// class need this variable to be initialized.
extern TEditorCollection *edHelper;

// Options for the sorting of closed files in removeEditor
const char dscSort=0,dscNoSortPutLast=1,dscNoSortPutFirst=2;

/**[txh]**********************************************************************

 Include: edcollec.h
 Module: Desktop Windows Helper
 Description:
 Just calls to the right getText of the @x{TEditorCollection (class)}.

*****************************************************************************/

void TListEditors::getText(char *dest, ccIndex item, short maxLen)
{
 TEditorCollection *lista=(TEditorCollection *)list();
 lista->getText(dest,item,maxLen);
}

// It creates an EasyDiag version of TListEditors called TSListEditors
ListBoxImplement(ListEditors)

/**[txh]**********************************************************************

  Description:
  Initialize to zero the number of windows.

*****************************************************************************/

TEditorCollection::TEditorCollection(ccIndex aLimit, ccIndex aDelta)
 : TCollection(aLimit,aDelta)
{
 Editors=Closed=nonEditors=0;
 maxClosedToRemember=40;
}

const int edcVersion=2;

/**[txh]**********************************************************************

  Description:
  Writes the collection to disk. It calls to the writeItem member of
each object. See: @x{::writeItem}. The object with CanBeSaved in 0
aren't saved and are discounted from the number of nonEditors.

*****************************************************************************/

void TEditorCollection::write( opstream& os )
{
 ccIndex idx,cant;
 int nonEd=nonEditors;

 unsigned dstOps=GetDSTOptions();
 // Look if the user wants to remove those
 int removeEditors=(dstOps & dstEdMask)==dstEdNever;
 int removeOthers =(dstOps & dstOwMask)==dstOwNever;
 int removeClosed =(dstOps & dstCfMask)==dstCfNever;

 // Discount the non save ones or we'll compute a wrong count when loading
 for (idx=Editors,cant=Editors+nonEditors; idx<cant; idx++)
     if (!(((TDskWin *)items[idx])->CanBeSaved))
        nonEd--;
 os << -edcVersion << TScreen::screenWidth << TScreen::screenHeight;
 os << (removeClosed  ? 0 : Closed) // Remove these () and look how funny it gets
    << (removeOthers  ? 0 : nonEd)
    << (removeEditors ? 0 : Editors)
    << delta << maxClosedToRemember;
 for (idx=0; idx<count; idx++)
    {
     TDskWin *p=(TDskWin *)items[idx];
     if (!((p->type==dktEditor && removeEditors) ||
         (p->type==dktClosed && removeClosed) ||
         (p->type!=dktClosed && p->type!=dktEditor && removeOthers)))
        writeItem(items[idx],os);
    }
}

/**[txh]**********************************************************************

  Description:
  Reads from disk a @x{TDskWin (class)}.

*****************************************************************************/

void *TEditorCollection::readItem( ipstream& is )
{
 TDskWin *p;
 is >> p;
 return p;
}

/**[txh]**********************************************************************

  Description:
  Saves to disk a @x{TDskWin (class)}.

*****************************************************************************/

void TEditorCollection::writeItem( void *p, opstream &os )
{
 if (((TDskWin *)p)->CanBeSaved)
    os << (TDskWin *)p;
}

TStreamable *TEditorCollection::build()
{
 return new TEditorCollection( streamableInit );
}

/**[txh]**********************************************************************

  Description:
  It reads the whole collection from disk and inserts the windows in the
deskTop using a global variable called editorApp that's the application.
@p
  After inserting the windows the function reorders the windows to recreate
the original desTop. All is done with the deskTop locked.

*****************************************************************************/

void *TEditorCollection::read( ipstream& is )
{
 int version;
 uchar wS,hS;
 TDskWinEditor *ped=0;

 unsigned dstOps=GetDSTOptions();
 // Look if the user wants to remove those
 int removeEditors,removeOthers,removeClosed;
 if (HaveFilesCL)
   {
    removeEditors=(dstOps & dstEdMask)==dstEdOnlyIfNoCL;
    removeOthers =(dstOps & dstOwMask)==dstOwOnlyIfNoCL;
    removeClosed =(dstOps & dstCfMask)==dstCfOnlyIfNoCL;
   }
 else
   removeEditors=removeOthers=removeClosed=0;
 int forgetCursorPos=dstOps & dstNoCursorPos;

 if (removeEditors)
    TCEditor::DontLoadFile=1;

 // That's needed if we want to remove some windows from the desktop
 // Is silly but some users wants to remove windows if the command line
 // contains names
 edHelper=this;
 is >> version;
 if (version>=0) // Again! sorry
   {
    Closed=version;
    wS=TScreen::screenWidth;
    hS=TScreen::screenHeight;
    version=0;
   }
 else
   {
    version=-version;
    is >> wS >> hS >> Closed;
   }

 is >> nonEditors >> Editors >> delta;
 if (version>=2)
    is >> maxClosedToRemember;
 else
    maxClosedToRemember=40;

 count=Closed+nonEditors+Editors;
 setLimit(0);
 for ( ccIndex idx = 0; idx < count; idx++ )
     items[idx] = readItem( is );

 // Now insert the views in the correct order
 int c=count;
 ccIndex i=0;
 TDskWin *p;
 int max=0;

 // That's a problem in Linux where we don't set the video mode so if the
 // editor is started with one resolution and then with another the windows
 // aren't repositioned
 TPoint dS;

 dS.x=TScreen::screenWidth-wS;
 dS.y=TScreen::screenHeight-hS;
 int dChange=dS.x!=0 || dS.y!=0;

 editorApp->deskTop->lock();
 while (c)
   {
    p=(TDskWin *)at(i);
    c--;
    if (p->view)
      {
       editorApp->deskTop->insert(p->view);
       if (dChange)
         {
          TRect  r;
          p->view->calcBounds(r,dS);
          p->view->changeBounds(r);
         }
       if (p->ZOrder>max)
          max=p->ZOrder;
      }
    i++;
   }

 while (max)
   {
    c=count; i=0;
    do
      {
       p=(TDskWin *)at(i);
       if (p->view)
         {
          if (p->ZOrder==max)
            {
             p->view->select();
             break;
            }
         }
       i++;
      }
    while (--c);
    max--;
   }

 // Some of the windows could fail to be inserted. For example a read error for an
 // editor invalidates it. Also non-exitant files.
 if (removeClosed)
    DontSortClosed=dscNoSortPutLast; // Don't sort closed by now
 else
   if (removeEditors)
      DontSortClosed=dscNoSortPutFirst; // Don't sort closed by now
 c=count;
 i=0;
 while (c)
   {
    p=(TDskWin *)at(i);
    c--;
    int cAnt=count;

    // We have three different types of dskWin objects
    // 1) Editors
    if (p->type==dktEditor)
      {
       ped=(TDskWinEditor *)p;
       if (!p->view || removeEditors)
         {
          int cEdAnt=Editors;
          // If we just call close() and the object is 0 then the
          // removeEditor will get the real pointer and won't be
          // able to find what to remove, so we do it here.
          if (!p->view)
             removeEditor(ped->edw,False);
          // We can do it because we know edHelper is initialized
          ped->edw->close();
          // When we succeed to remove the editor it created a closed window
          // it makes cAnt==count, but we don't have to increment i
          if (cAnt==count && cEdAnt!=Editors)
             i--;
         }
       else
         if (forgetCursorPos)
            ped->edw->editor->handleCommand(cmcTextStart);
      }
    else
    // 2) Closed files
    if (p->type==dktClosed)
      {
       if (removeClosed && p->DeleteAction(i,False))
          delete p;
      }
    // 3) All the rest
    else
      {
       if (!p->view || removeOthers)
          if (p->DeleteAction(i,False))
             delete p;
      }
    // If the item wasn't removed take the next
    if (count==cAnt)
       i++;
   }
 HaveFilesCL=0;
 TCEditor::DontLoadFile=0;
 if (DontSortClosed!=dscSort)
   {
    DontSortClosed=dscSort;
    sortClosed();
   }
 if (dChange)
    editorApp->deskTop->redraw();
 editorApp->deskTop->unlock();
 return this;
}

/**[txh]**********************************************************************

  Description:
  Overwrites the TCollection calling the TDskWin::GetText member of the
item. See: @x{TDskWin::GetText}.

*****************************************************************************/

void TEditorCollection::getText(char *dest, ccIndex item, short maxLen)
{
 TDskWin *p=(TDskWin *)at(item);
 p->GetText(dest,maxLen);
}

/**[txh]**********************************************************************

  Description:
  Adds a TCEditWindow object (p) to the collection. The second parameter was
used when the syntax highlight wasn't selected by the TCEditor class.
@p
  The function fixes the name is it's ended with a point, assigns a number
to the editor, creates a @x{TDskWinEditor (class)}, object for it and inserts
it in the collection. Additionally it deletes any closed editor with the
same name.
@p
  No checks are made to see if the editor is already on desktop because
the object isn't inserted in the desktop. This task must be done by the
calling routine.

*****************************************************************************/

void TEditorCollection::addEditor(TCEditWindow *p, int )
{
 int i=Editors;
 int number=2; // 1 is for the project
 ccIndex ind=0;
 TDskWinEditor *st;

 while (i)
   {
    st=(TDskWinEditor *)at(ind);
    i--;
    if (st->number!=number)
       break;
    number++;
    ind++;
   }
 Editors++;
 st=new TDskWinEditor(p,number);
 p->number=number;
 atInsert(ind,st);

 // If there is a closed for it kill him
 ind=search(p->editor->fileName,dktClosed);
 if (ind!=-1)
   {
    TDskWinClosed *p=(TDskWinClosed *)at(ind);
    atRemove(ind);
    Closed--;
    delete p;
   }
}

/**[txh]**********************************************************************

  Description:
  Is used to search a view pointed by p of the specified type. It uses for it
the member @x{TDskWin::Compare}.

  Return:
  The ccIndex of the object is found or -1 if not.

*****************************************************************************/

ccIndex TEditorCollection::search(void *p,int type)
{
 ccIndex pos=0;
 TDskWin *st;

 while (pos<count)
   {
    st=(TDskWin *)at(pos);
    if (st->Compare(p,type))
       return pos;
    pos++;
   }
 return -1;
}

/**[txh]**********************************************************************

  Description:
  Is used to search a view pointed by view of any type.

  Return:
  The ccIndex of the object is found or -1 if not.

*****************************************************************************/

ccIndex TEditorCollection::searchByView(void *view)
{
 ccIndex pos=0;
 TDskWin *st;

 while (pos<count)
   {
    st=(TDskWin *)at(pos);
    if (st->view==view)
       return pos;
    pos++;
   }
 return -1;
}


/**[txh]**********************************************************************

  Description:
  Removes and editor from the collection. It adds the editor to the list of
closed editors and deletes the object. Additionally it limits the number of
closed editors to 40 and keeps the list sorted (Laszlo added it). Now the
number is configurable.@p
  This function *does not* delete the editor window itself. It is supposed to
be called by the close() member so it will be destroyed soon. The call is
done indirectly by sending a cmClosingWindow broadcast. If a function
directly calls this function should then destroy the editor.

*****************************************************************************/

void TEditorCollection::removeEditor(TCEditWindow *p, Boolean dontAddClosed)
{
 ccIndex pos=search(p,dktEditor);

 if (pos<0) return;
 TDskWinEditor *st=(TDskWinEditor *)at(pos);
 // The read-only copies doesn't have to be added to the closed list or we get
 // duplicated names!
 if (p->editor->isReadOnly) dontAddClosed=True;
 // Ask for saving
 if (!dontAddClosed && st->edw && !st->edw->editor->valid(cmClose)) return;
 atRemove(pos);
 Editors--;

 if (dontAddClosed)
   {
    delete st;
    return;
   }
 if (p)
    UpdateProjectResumeFor(p->editor->fileName,p);
 if (!st->edw)
    return;

 TDskWinClosed *nuevo=new TDskWinClosed(st->edw);
 if (DontSortClosed==dscNoSortPutLast)
    // That's used during the load. If we are adding a closed window
    // means we are removing editors and closed windows will follow,
    // so we must add at the end.
    insert(nuevo);
 else
    // Add at the begining so we are sure it will survive to the
    // limitation routine.
    atInsert(Editors+nonEditors,nuevo);
 delete st;
 Closed++;

 if (DontSortClosed==dscSort)
    sortClosed();
}

void TEditorCollection::sortClosed(void)
{
 ccIndex pos;
 // Don't keep more than 40 closed, Laci needs much ;-)
 if (Closed>maxClosedToRemember)
   {
    pos=Editors+nonEditors+--Closed;
    TDskWin *p=(TDskWin *)at(pos);
    atRemove(pos);
    delete p;
   }
 //   That's a modified version of the Laszlo's idea to keep the closed windows
 // sorted that of course sorts the old desktop files.
 TDskWinClosed **closedwins = (TDskWinClosed**)alloca(maxClosedToRemember * sizeof(TDskWinClosed*));
 TDskWinClosed *tmp;
 ccIndex       ic,jc;
 int           sorted=0,need_change=0;

 for (ic=0; ic<Closed; ic++)
     closedwins[ic]=(TDskWinClosed *)at(ic+Editors+nonEditors);
 for (ic=0; ic<Closed-1 && !sorted; ic++)
     for (jc=ic+1,sorted=1; jc<Closed; jc++)
        {
         if (strcmp(closedwins[ic]->Name,closedwins[jc]->Name) > 0)
           {
            tmp=closedwins[ic];
            closedwins[ic]=closedwins[jc];
            closedwins[jc]=tmp;
            sorted=0;
            need_change=1;
           }
        }
 if (need_change)
   {
    for (ic=0; ic<Closed; ic++)
        atRemove(Editors+nonEditors);
    for (ic=0; ic<Closed; ic++)
        atInsert(Editors+nonEditors+ic,closedwins[ic]);
   }
}

/**[txh]**********************************************************************

  Description:
  Removes a non editor from the collection. You must provide the pointer and
the type. The object is deleted.

*****************************************************************************/

void TEditorCollection::removeNonEditor(void *p, int type)
{
 ccIndex pos=search(p,type);

 if (pos<0) return;
 void *NonEditor=at(pos);
 atRemove(pos);
 freeItem(NonEditor);
 nonEditors--;
}

/**[txh]********************************************************************

  Description:
  Removes the desktop helper whose view member is the one passed.

***************************************************************************/

void TEditorCollection::removeWindow(void *p)
{
 ccIndex pos=searchByView(p);

 if (pos<0) return;
 TDskWin *st=(TDskWin *)at(pos);
 if (st->type==dktEditor)
    removeEditor((TCEditWindow *)p,False);
 else
    removeNonEditor(p,st->type);
}


/**[txh]**********************************************************************

  Description:
  Searchs an editor by name. For more details @x{::searchEditorbyINode}.

  Return:
  The index in the collection or -1 is not there.

  Example:

TCEditWindow *IsAlreadyOnDesktop(char *fileName, int *cant)
{
 ccIndex pos=edHelper->searchEditorName(fileName,cant);
 if (pos<0)
    return NULL;
 TDskWinEditor *st=(TDskWinEditor *)edHelper->at(pos);
 return st->edw;
}

*****************************************************************************/

ccIndex TEditorCollection::searchEditorName(char *name, int *cant)
{
 int i=Editors;
 ccIndex ind=0;
 TDskWin *st;
 int l;
 TCEditor *ed;
 /* Index of the best match */
 ccIndex indFound=-1;
 Boolean isIndReadOnly=True;
 int Cant=0;

 // If the name
 l=strlen(name)-1;
 if (name[l]=='.')
    name[l]=0;
 while (i)
   {
    st=(TDskWin *)at(ind);
    if (st->type==dktEditor)
      {
       i--;
       ed=((TDskWinEditor *)st)->edw->editor;
       if (strcasecmp(ed->fileName,name)==0)
         {
          Cant++;
          if (indFound==-1 || (isIndReadOnly && !ed->isReadOnly))
            {
             isIndReadOnly=ed->isReadOnly;
             indFound=ind;
            }
         }
      }
    ind++;
   }

 if (cant)
    *cant=Cant;
 return indFound;
}

/**[txh]**********************************************************************

  Description:
  Searchs an editor using the I-Node/Device of the file. If the provided file
name doesn't have an I-Node the routine makes a search by name.@*
  The number of matches is returned in cant. If one editor no read only is
present that's the index reported. If the index reported belongs to a read
only editor it means all the rest are read only. The pointer cant can be NULL
in this case the function doesn't use it.

  Return:
  The index in the collection or -1 is not there.

  Example:

TCEditWindow *IsAlreadyOnDesktop(char *fileName, int *cant)
{
 ccIndex pos=edHelper->searchEditorbyINode(fileName,cant);
 if (pos<0)
    return NULL;
 TDskWinEditor *st=(TDskWinEditor *)edHelper->at(pos);
 return st->edw;
}

*****************************************************************************/

ccIndex TEditorCollection::searchEditorbyINode(char *name, int *cant)
{
#ifdef SEOS_Win32
 // this function must not be called on WIN32 systems.  Inode number reported by
 // stat is always 0.
 #undef NDEBUG
 assert(0);
 return -1;
#else
 int i=Editors;
 ccIndex ind=0;
 TDskWin *st;
 struct stat s;
 TCEditor *ed;
 /* Index of the best match */
 ccIndex indFound=-1;
 Boolean isIndReadOnly=True;
 int Cant=0;

 if (stat(name,&s)!=0)
    return searchEditorName(name,cant);
 while (i)
   {
    st=(TDskWin *)at(ind);
    if (st->type==dktEditor)
      {
       i--;
       ed=((TDskWinEditor *)st)->edw->editor;
       if (ed->DeviceOfFile==s.st_dev && ed->INodeOfFile==s.st_ino)
         {
          Cant++;
          if (indFound==-1 || (isIndReadOnly && !ed->isReadOnly))
            {
             isIndReadOnly=ed->isReadOnly;
             indFound=ind;
            }
         }
      }
    ind++;
   }

 if (cant)
    *cant=Cant;
 return indFound;
#endif
}

void TEditorCollection::forEachEditor(void (*func)(TCEditor *))
{
 int i=Editors;
 ccIndex ind=0;
 TDskWin *st;

 while (i)
   {
    st=(TDskWin *)at(ind);
    if (st->type==dktEditor)
      {
       TCEditor *p=((TDskWinEditor *)st)->edw->editor;
       i--;
       func(p);
      }
    ind++;
   }

 return;
}

static
void SaveIt(TCEditor *p)
{
 if (p->modified)
    p->save();
}

void TEditorCollection::saveEditors(void)
{
 forEachEditor(SaveIt);
}

static
void RedrawIt(TCEditor *p)
{
 p->update(ufView);
}

void TEditorCollection::redrawEditors(void)
{
 forEachEditor(RedrawIt);
}

/**[txh]**********************************************************************

  Description:
  Overwrites the freeItem member. It just calls to the virtual destructor of
the TDskWin object pointed by item.

*****************************************************************************/

void TEditorCollection::freeItem(void *item)
{
 delete (TDskWin *)item;
}

int  TEditorCollection::maxClosedToRemember;
char TEditorCollection::HaveFilesCL=0;
char TEditorCollection::DontSortClosed=0;

