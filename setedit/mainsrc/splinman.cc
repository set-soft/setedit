/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Special Lines Manager

  Comments:
  The special lines are a little complex thing because they can virtually
come from various sources and your live can vary. I say virtually because
actually the standalone editor can generate it only from the message box,
but in RHIDE the breakpoints are other source.

IMPORTANT!!!!
Internally the splines are list of lines associated with a file name. This
file name can be a file we are editing or not. The splines itself can't be
saved to disk. So they are "internal representations". For this reason they
associate a *full qualified* file name with line numbers. To ensure it all
the functions first expand the name and then does the work (searches for
example). It means that internally we use absolute file names, but you can
specify relative names when adding or deleting splines.
A very important detail is that the editor is even smarter and uses inode
values to diferentiate files. So when we apply the special lines the editor
will apply them even to a file name that isn't the same but represents the
same file in disk.
I'm not sure what problems and limitations can be derived from that.

***************************************************************************/
//#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#endif
#define Uses_string

#define Uses_TCEditor
#define Uses_TNSCollection
#define Uses_TStringCollection
#define Uses_TCEditor_Internal
#include <ceditor.h>
#include <splinman.h>

#define TO_KEY(a) ((void *)(long)(a))

void TSpCollection::insert(int line, int id)
{
 stSpLine *p=new stSpLine;
 p->oline=p->nline=line;
 p->id=id;
 TNSSortedCollection::insert(p);
}

TSpCollection &TSpCollection::operator=(const TSpCollection &pl)
{
 int i;
 freeAll();
 for (i=0; i<pl.count; i++)
    {
     stSpLine *d=new stSpLine, *o=(stSpLine *)pl.items[i];
     d->oline=o->oline;
     d->nline=o->nline;
     d->id=o->id;
     // Maintain the same sorting, even for duplicated items.
     // That's important because duplicated newer entries goes first, but when
     // we copy they are inserted in the reverse order. This can make an old
     // breakpoint hide the CPU line.
     TNSCollection::insert(d);
    }
 return *this;
}

void *TSpCollection::keyOf(void *item)
{
 stSpLine *p=(stSpLine *)item;
 return TO_KEY(p->oline);
}

int TSpCollection::compare(void *s1, void *s2)
{
 return (long)s1-(long)s2;
}

void TSpCollection::freeItem(void *s)
{// Nothing special, just be sure we don t mix delete and delete[]
 delete (stSpLine *)s;
}

// Base cell
typedef struct
{
 char *file; // File owner of the lines
 TSpCollection *SpecialLines;   // Which lines
 TSpCollection *NewSpLines;     // When we modify (add/delete) lines the operation
 // is performed on a copy. It helps the editor to know what changed and avoid
 // drawing the whole window.
} NodeCol;


// A collection to associate files and lines
class TSpLAsoc : public TStringCollection
{
public:
 TSpLAsoc(ccIndex aLimit, ccIndex aDelta) :
   TStringCollection(aLimit,aDelta) { }
 virtual void *keyOf(void *item) { return ((NodeCol *)item)->file; };
 virtual void freeItem( void *item );
 NodeCol *At(ccIndex pos) { return (NodeCol *)at(pos); };
};

void TSpLAsoc::freeItem( void *item )
{
 NodeCol *p=(NodeCol *)item;
 delete[] p->file;
 CLY_destroy(p->SpecialLines);
 CLY_destroy(p->NewSpLines);
 delete p;
}

static TSpLAsoc *SpLines=NULL;

static
void ApplyOneSpLines(void *item, void *)
{
 NodeCol *p=(NodeCol *)item;
 // Note: ApplySpLines is smart and will search the file by its inode.
 if (p->NewSpLines)
   {
    ApplySpLines(p->file,p->NewSpLines);
    // Reflect it
    CLY_destroy(p->SpecialLines);
    p->SpecialLines=p->NewSpLines;
    p->NewSpLines=NULL;
   }
}

/**[txh]********************************************************************

  Description:
  It adds an special line to some file. fileName is the name of the file, it
can be opened or not, line is the line number and idSource is the id of the
section of the program that sets it, is the kind of line.@p
  If TransferNow is True (default) the special lines are transfered
inmediatly to the owner, if not you MUST call @x{SpLinesUpdate}.

***************************************************************************/

void SpLinesAdd(char *fName, int line, int idSource, Boolean TransferNow)
{
 ccIndex pos;

 if (!SpLines)
   {
    SpLines=new TSpLAsoc(4,2);
    if (!SpLines)
       return;
   }
 NodeCol *p;
 line--; // Because the editor uses 0 internally

 // TODO: should we implement some inode mechanism?
 char fileName[PATH_MAX];
 strcpy(fileName,fName);
 CLY_fexpand(fileName);

 if (SpLines->search(fileName,pos))
   {
    p=SpLines->At(pos);
    if (!p->NewSpLines)
      {// We don't have a list to transfer, create it.
       p->NewSpLines=new TSpCollection(0);
       *p->NewSpLines=*p->SpecialLines;
      }
    p->NewSpLines->insert(line,idSource);
    #ifdef DEBUG
    fprintf(stderr,"Adding a spl for: %s (%d), id: %d transfer: %d\n",fileName,line,idSource,TransferNow ? 1 : 0);
    #endif
   }
 else
   {
    // This file doesn't have special lines yet
    p=new NodeCol;
    p->file=newStr(fileName);
    p->SpecialLines=NULL;
    p->NewSpLines=new TSpCollection(8);
    p->NewSpLines->insert(line,idSource);
    SpLines->insert(p);
    #ifdef DEBUG
    fprintf(stderr,"Adding a spl for: NEW %s (%d), id: %d transfer: %d\n",fileName,line,idSource,TransferNow ? 1 : 0);
    #endif
   }
 if (TransferNow)
    ApplyOneSpLines(p,NULL);
}

/**[txh]********************************************************************

  Description:
  This function updates the special lines of ALL the editors. That's made
through calls to ApplySpLines, this function must be provided by the main
part of the editor. Prototype:@p
extern void ApplySpLines(char *fileName,TSpCollection *spLines);

***************************************************************************/

void SpLinesUpdate(void)
{
 if (!SpLines)
    return;
 SpLines->forEach(ApplyOneSpLines,NULL);
}

/**[txh]********************************************************************

  Description:
  This function is called by the function that opens editors from disk. This
routine looks if there are special lines for the new editor and in this case
returns the list.

  Return:
  The special lines array or NULL.

***************************************************************************/

TSpCollection *SpLinesGetFor(char *fName)
{
 if (!SpLines || !fName) // New files doesn't have a name
    return NULL;

 char fileName[PATH_MAX];
 strcpy(fileName,fName);
 CLY_fexpand(fileName);

 ccIndex pos;
 if (SpLines->search(fileName,pos))
    return (SpLines->At(pos))->SpecialLines;
 return NULL;
}

/**[txh]********************************************************************

  Description:
  This function deletes all the special lines associated with the provided
id. I use it for example when the message window is cleaned and hence these
special lines no longer exists.

***************************************************************************/

void SpLinesDeleteForId(int id, const char *file, Boolean aLine, int oLine)
{
 if (!SpLines)
    return;
 ccIndex count=SpLines->getCount();
 ccIndex i;
 Boolean deleted;
 NodeCol *p;
 oLine--;

 char fileName[PATH_MAX];
 if (file)
   {
    strcpy(fileName,file);
    CLY_fexpand(fileName);
    file=fileName;
   }

 for (i=0; i<count;)
    {
     p=SpLines->At(i);
     // If we are looking for a particular file and it isn't just continue
     if (file && strcmp(file,p->file))
       {
        i++;
        continue;
       }
     if (!p->NewSpLines)
       {// We don't have a list to transfer, create it.
        p->NewSpLines=new TSpCollection(0);
        *p->NewSpLines=*p->SpecialLines;
       }
     ccIndex c2,j;
     c2=p->NewSpLines->getCount();
     deleted=False;
     for (j=0; j<c2;)
        {
         stSpLine *st=p->NewSpLines->At(j);
         if (st->id==id &&
             // Not a particular line or the line we want
             (!aLine || st->oline==oLine))
           {
            p->NewSpLines->atFree(j);
            c2--;
            deleted=True;
            // If it was the line we wanted stop searching
            if (aLine)
               break;
           }
         else
           j++;
        }
     if (c2==0) // All deleted
       {
        ApplySpLines(p->file,NULL);
        SpLines->atFree(i);
        count--;
       }
     else
       {
        if (deleted) // Reflex the change
           ApplyOneSpLines(p,NULL);
        i++;
       }
     // If it was the file we wanted we are done
     if (file)
        break;
    }
 if (count==0)
   {
    CLY_destroy(SpLines);
    SpLines=NULL;
   }
}

/**[txh]********************************************************************

  Description:
  This function searchs the specified line and returns the actual value.
To know if the line was actually found use the @var{found} argument. That's
optional.

***************************************************************************/

int SpLineGetNewValueOf(int line, char *fName, Boolean *found)
{
 if (found)
    *found=False;
 if (!SpLines)
    return line;

 char fileName[PATH_MAX];
 strcpy(fileName,fName);
 CLY_fexpand(fileName);

 ccIndex Pos;
 line--;
 if (SpLines->search(fileName,Pos))
   {
    TSpCollection *p=(SpLines->At(Pos))->SpecialLines;
    if (p->search(TO_KEY(line),Pos))
      {
       if (found)
          *found=True;
       return (p->At(Pos))->nline+1;
      }
   }
 return line+1;
}

/**[txh]********************************************************************

  Description:
  This function searchs the specified actual line and returns the original
value. To know if the line was actually found use the @var{found} argument.
That's optional.

***************************************************************************/

int SpLineGetOldValueOf(int line, char *fName, int type, Boolean *found)
{
 if (found)
    *found=False;
 if (!SpLines)
    return line;

 char fileName[PATH_MAX];
 strcpy(fileName,fName);
 CLY_fexpand(fileName);

 ccIndex Pos;
 line--;
 if (SpLines->search(fileName,Pos))
   {
    TSpCollection *p=(SpLines->At(Pos))->SpecialLines;
    ccIndex c=p->getCount();
    ccIndex i;
    for (i=0; i<c; i++)
       {
        stSpLine *st=p->At(i);
        if (st->nline==line && (type==idsplAny || st->id==type))
          {
           if (found)
              *found=True;
           return st->oline+1;
          }
       }
   }
 return line+1;
}

void SpLinesCleanUp()
{
 CLY_destroy(SpLines);
 SpLines=NULL;
}

struct idAndData
{
 int id;
 void *data;
 spLineApplyF apply;
};

static
void SpLinesForEachFile(void *item, void *data)
{
 idAndData *d=(idAndData *)data;
 NodeCol *p=(NodeCol *)item;

 TSpCollection *col=p->SpecialLines;
 int i, c=col->getCount();
 for (i=0; i<c; i++)
    {
     stSpLine *s=col->At(i);
     if (d->id==idsplAny || d->id==s->id)
        d->apply(p->file,s,d->data);
    }
}

void SpLinesForEach(int id, spLineApplyF apply, void *data)
{
 if (!SpLines)
    return;
 idAndData d;
 d.id=id;
 d.data=data;
 d.apply=apply;
 SpLines->forEach(SpLinesForEachFile,&d);
}

