/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]**********************************************************************

  Module: Desktop Windows Helper
  Comments:
  This classes are used to handle the windows of the editor. All are based
on an abstract class called @x{TDskWin (class)}. This class provides the
basic structure for all the windows supported in the editor.
@p
  In addition the editor uses a class called @x{TEditorCollection (class)},
to hold the windows. This structure basically complements the concept of
the desktop in TVision. From my point of view it could be part of the
TDeskTop class.
@p
  The editor uses it:
@p
@<pre>
  extern TEditorCollection *edHelper;
@</pre>
  Then the windows are inserted using:
@p
@<pre>
  edHelper->addEditor(p,SelectHL);
@</pre>
for editors and:
@p
@<pre>
  edHelper->addNonEditor(p);
@</pre>
for non editors.
@p
  For more information see the classes named above.

***********/
/**[txh]****

  Class: TDskWin
  Comments:
  That's the base class for all the desktop helper windows. All the members
are undefined except @x{TDskWin::GetText}.
@p
  The class is derived from TStreamable and helps to save any window in the
desktop even when the window isn't a TStreamable one.
@p
@<subtitle>{Data members:}
@<pre>
int type;
@</pre>
  Is the type of the object. One important thing of this classes is that you
can know to what class belongs each object. That's used for example to search
a window of some kind exluding the rest.
@p
@<pre>
int CanBeSaved;
@</pre>
  It says if the window can be saved to disk. Some special windows like
the project window aren't saved with the rest.
@p
@<pre>
int ZOrder;
@</pre>
  This value is used to restore the exact position of the window when reading
a desktop file from disk.
@p
@<pre>
TView *view;
@</pre>
  Point to the view of the window. Generally the derived classes have
another member to use internally and with the correct type.

*****************************************************************************/

#define Uses_string
#define Uses_TStreamable
#define Uses_TProgram
#define Uses_TDeskTop
#include <ceditor.h>

#include <dskwin.h>

#define Uses_SETAppVarious
#include <setapp.h> // closeView

/**[txh]**********************************************************************

 Function: GetText (3 vals)
 Include: dskwin.h
 Description:
  The GetText member is used by the TListEditors to provide a
description of the window in the List of windows dialog.
See: @x{TListEditors (class)}.
@p
  This version is used to concatenate the contents of s with the dest
contents without exceding maxLen. Is called by various of the derived
classes. Isn't the one called by TListEditors.

 Return:
  A char pointer to the description of the window.

*****************************************************************************/

char *TDskWin::GetText(char *dest,char *s, short maxLen)
{
 int max=maxLen-strlen(dest);
 int l=strlen(s);
 if (l>=max)
   {
    strncat(dest,s,max);
    dest[maxLen]=0;
   }
 else
    strcat(dest,s);

 return dest;
}

/**[txh]**********************************************************************

  Function:  GoAction
  Prototype: int GoAction(ccIndex i)
  Description:
  This member is called when the user selects the Go button in the List of
Windows dialog. It must select the window. This task is made in an indirect
way using:
@p
@<pre>
extern TView *setFocusTo;
extern Boolean focusChanged;

int TDskWinHelp::GoAction(ccIndex )
{
 TProgram::deskTop->lock();
 setFocusTo=window;
 focusChanged=True;

 return 0;
}
@</pre>

  But can be very different like in the case of the @x{TDskWinClosed (class)},
where this action opens a previously closed editor.

  Return:
  1 if the object must be deleted. That's used by closed windows.

**************/

int TDskWin::GoAction(ccIndex )
{
 TProgram::deskTop->lock();
 setFocusTo=view;
 focusChanged=True;

 return 0;
}

/**[txh]*******

  Description:
  This member is called when the user selects the Delete button in the List of
Windows dialog. It can close the window, hide it or remove it. The default
behavior is call closeView passing the TView pointer.@p
  The arguments are the position in the list and a Boolean value indicating
if the associated file in disk must be deleted too.

Return:
  1 if the object must be destroyed.

**************/

int TDskWin::DeleteAction(ccIndex, Boolean)
{
 closeView(view,0);
 return 0;
}

/**[txh]********************************************************************

  Description:
  Used to know the number of the window associated with it.
  
  Return: The number of the window or a negative value if it doesn't have
a number. The negative value is -type.
  
***************************************************************************/

int TDskWin::GetNumber()
{
 return -type;
}

/**[txh]*******

  Function:  Compare
  Prototype: int Compare(void *view, int type)
  Description:
  Is used during searchs. If the view is the one owned by the class and
the type is the right it must return a non zero value.

  Return:
  Non zero if the requested object is this.

**************/
/**[txh]*******

  Function:  GetText
  Prototype: char *GetText(char *dest, short maxLen)
  Description:
  The GetText member is used by the TListEditors class to provide a
description of the window in the List of windows dialog.
See @x{TListEditors (class)}.
@p
  This version is the virtual one. It must copy to dest the description of
the window limiting it to maxLen.

  Return:
  A char pointer to the description of the window.

*****************************************************************************/

void InsertInOrder(TDeskTop *dsk,TDskWin *win)
{
 int z=win->ZOrder;
 TView *v=0;

 if (z>=0)
   {
    if (z==0)
       dsk->insertBefore(win->view,0);
    else
      {
       v=dsk->at(z);
       dsk->insertBefore(win->view,v);
      }
   }
 else
   dsk->insert(win->view);
}

void AddAndInsertDskWin(TDskWin *win)
{
 AddNonEditorToHelper(win);
 InsertInOrder(TProgram::deskTop,win);
}

void TDskWin::write(opstream& os)
{
}

void *TDskWin::read(ipstream& is)
{
 return this;
}

const char *TDskWin::streamableName() const
{
 return NULL;
}

int  TDskWin::Compare(void *p, int t)
{
 return (t==type) && p==(void *)view;
}

