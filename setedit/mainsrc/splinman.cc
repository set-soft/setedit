/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/**[txh]********************************************************************

  Module: Special Lines Manager

  Comments:
  The special lines are a little complex thing because they can virtually
come from various sources and your live can vary. I say virtually because
actually the standalone editor can generate it only from the message box,
but in RHIDE the breakpoints are other source.

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

// The following function must be provided by the main part of the editor.
// Your objetive is transfer the array spLines to the editor associated to fileName
extern void ApplySpLines(char *fileName,int *spLines);

// Here I'm using a trick that isn't so clean, I use collections to hold
// integers instead of void * because both are 32 bits
class TArrayCol : public TNSCollection
{
public:
 TArrayCol(ccIndex aLimit, ccIndex aDelta) :
   TNSCollection(aLimit,aDelta) { shouldDelete=False; } // No owner
 int *getItems() { return (int *)items; } // That's the good thing of protected things, you
                                   // can unprotect them ;-) != private
 void insert(int val) { TNSCollection::insert((void *)(long)val); }
};

// Base cell
typedef struct
{
 char *file; // File owner of the lines
 TArrayCol *SpecialLines;   // Which lines
 TArrayCol *OriginalLines;  // Original line numbers
 TArrayCol *idSources;      // kind of line
} NodeCol;


// A collection to associate files and lines
class TSpLAsoc : public TStringCollection
{
public:
 TSpLAsoc(ccIndex aLimit, ccIndex aDelta) :
   TStringCollection(aLimit,aDelta) {}
 virtual void *keyOf(void *item) { return ((NodeCol *)item)->file; };
 virtual void freeItem( void *item );
};

void TSpLAsoc::freeItem( void *item )
{
 NodeCol *p=(NodeCol *)item;
 delete p->file;
 destroy(p->SpecialLines);
 destroy(p->OriginalLines);
 destroy(p->idSources);
}

static TSpLAsoc *SpLines=0;

/**[txh]********************************************************************

  Description:
  It adds an special line to some file. fileName is the name of the file, it
can be opened or not, line is the line number and idSource is the id of the
section of the program that sets it, is the kind of line.@p
  If TransferNow is True (default) the special lines are transfered
inmediatly to the owner, if not you MUST call @x{SpLinesUpdate}.

***************************************************************************/

void SpLinesAdd(char *fileName, int line, int idSource, Boolean TransferNow)
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
 if (SpLines->search(fileName,pos))
   {
    p=(NodeCol *)(SpLines->at(pos));
    p->SpecialLines->atInsert(p->SpecialLines->getCount()-1,(void *)(long)line);
    p->OriginalLines->insert(line);
    p->idSources->insert(idSource);
    #ifdef DEBUG
    fprintf(stderr,"Adding a spl for: %s (%d), id: %d transfer: %d\n",fileName,line,idSource,TransferNow ? 1 : 0);
    #endif
   }
 else
   {
    // This file doesn't have special lines yet
    p=new NodeCol;
    p->file=strdup(fileName);
    p->SpecialLines=new TArrayCol(8,8);
    p->OriginalLines=new TArrayCol(8,8);
    p->idSources=new TArrayCol(8,8);
    p->SpecialLines->insert(line);
    p->SpecialLines->insert(splEndOfList); // That's because the editor spects a terminated list
    p->OriginalLines->insert(line);
    p->idSources->insert(idSource);
    SpLines->insert(p);
    #ifdef DEBUG
    fprintf(stderr,"Adding a spl for: NEW %s (%d), id: %d transfer: %d\n",fileName,line,idSource,TransferNow ? 1 : 0);
    #endif
   }
 if (TransferNow)
    ApplySpLines(fileName,p->SpecialLines->getItems());
}

// Very important!, the editor needs a terminated list or ... kbum!
static
int *AddaptList(NodeCol *p)
{
 ccIndex c=p->SpecialLines->getCount()-1;
 int *list=p->SpecialLines->getItems();
 list[c]=splEndOfList;
 #ifdef DEBUG
 {
 fprintf(stderr,"Arranging a list for: %s:\n",p->file);
 int i;
 for (i=0; i<=c; i++)
     fprintf(stderr,"line: %d\n",list[i]);
 }
 #endif
 return list;
}

static
void ApplySpLines(void *item, void *)
{
 NodeCol *p=(NodeCol *)item;
 ApplySpLines(p->file,AddaptList(p));
}

/**[txh]********************************************************************

  Description:
  This function updates the special lines of ALL the editors. That's made
through calls to ApplySpLines, this function must be provided by the main
part of the editor. Prototype:@p
extern void ApplySpLines(char *fileName,int *spLines);

***************************************************************************/

void SpLinesUpdate(void)
{
 if (!SpLines)
    return;
 SpLines->forEach(ApplySpLines,0);
}

/**[txh]********************************************************************

  Description:
  This function is called by the function that opens editors from disk. This
routine looks if there are special lines for the new editor and in this case
returns the list.

  Return:
  The special lines array or 0.

***************************************************************************/

int *SpLinesGetFor(char *fileName)
{
 if (!SpLines || !fileName) // New files doesn't have a name
    return 0;
 ccIndex pos;
 if (SpLines->search(fileName,pos))
   {
    NodeCol *p=(NodeCol *)(SpLines->at(pos));
    return AddaptList(p);
   }
 return 0;
}

/**[txh]********************************************************************

  Description:
  This function deletes all the special lines associated with the provided
id. I use it for example when the message window is cleaned and hence these
special lines no longer exists.

***************************************************************************/

void SpLinesDeleteForId(int id)
{
 if (!SpLines)
    return;
 ccIndex count=SpLines->getCount();
 ccIndex i;
 Boolean deleted;
 NodeCol *p;
 for (i=0; i<count;)
    {
     p=(NodeCol *)(SpLines->at(i));
     ccIndex c2,j;
     c2=p->idSources->getCount();
     deleted=False;
     for (j=0; j<c2;)
        {
         if ((long)(p->idSources->at(j))==(long)id)
           {
            p->idSources->atRemove(j);
            p->SpecialLines->atRemove(j);
            p->OriginalLines->atRemove(j);
            c2--;
            deleted=True;
           }
         else
           j++;
        }
     if (c2==0) // All deleted
       {
        ApplySpLines(p->file,0);
        SpLines->atFree(i);
        count--;
       }
     else
       {
        if (deleted) // Reflex the change
           ApplySpLines(p->file,AddaptList(p));
        i++;
       }
    }
 if (count==0)
   {
    destroy(SpLines);
    SpLines=0;
   }
}

/**[txh]********************************************************************

  Description:
  This function searchs the specified line and returns the actual value.

***************************************************************************/

int SpLineGetNewValueOf(int line, char *fileName)
{
 if (!SpLines)
    return line;

 ccIndex Pos;
 line--;
 if (SpLines->search(fileName,Pos))
   {
    NodeCol *p=(NodeCol *)(SpLines->at(Pos));
    ccIndex c=p->OriginalLines->getCount();
    ccIndex i;
    for (i=0; i<c; i++)
       {
        if ((long)(p->OriginalLines->at(i))==(long)line)
           return (long)(p->SpecialLines->at(i))+1;
       }
   }
 return line+1;
}
