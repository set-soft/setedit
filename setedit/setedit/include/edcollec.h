/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
struct stEditorId;
class TCEditWindow;

class TEditorCollection : public TCollection
{
public:

 TEditorCollection(ccIndex aLimit, ccIndex aDelta);
 virtual void freeItem(void *item);
 void addEditor(TCEditWindow *p,int SelectHL=0);
 void addNonEditor(TDskWin *p) { atInsert(Editors,p); nonEditors++; };
 void removeEditor(TCEditWindow *p, Boolean dontAddClosed);
 void removeNonEditor(void *p, int type);
 void removeWindow(void *p);
 void getText(char *dest, ccIndex item, short maxLen);
 ccIndex search(void *p,int type);
 ccIndex searchByView(void *view);
 ccIndex searchEditorName(char *name, int *cant);
 ccIndex searchEditorbyINode(char *name, int *cant);
 ccIndex searchEditorbyINode(stEditorId *id, char *name, int *cant);
 TDskWin *searchByNumber(int num);
 void forEachEditor(void (*func)(TCEditWindow *));
 void forEachNonEditor(int type, void (*func)(TDskWin *,void *), void *data);
 void saveEditors(void);
 void redrawEditors(void);
 void reIdEditors(void);
 void sortClosed(void);
 int  GetMaxWindowNumber(void);
 void ReEnumerate();
 int Editors;
 int Closed;
 int nonEditors;
 static int maxClosedToRemember;
 // Members used during the load to change the normal behavior
 static char HaveFilesCL;
 static char DontSortClosed;

 const char *streamableName() const
     { return name; }
 void *readItem( ipstream& is );
 void writeItem( void *p, opstream &os );
 void write( opstream& os );
 void *read(ipstream& is);

protected:

    TEditorCollection( StreamableInit ) : TCollection ( streamableInit ) {};

public:

    static const char * const name;
    static TStreamable *build();
};

inline ipstream& operator >> ( ipstream& is, TEditorCollection& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TEditorCollection*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TEditorCollection& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TEditorCollection* cl )
    { return os << (TStreamable *)cl; }


class TListEditors : public TListBox
{
public:
 TListEditors(const TRect& bounds, ushort aNumCols, TScrollBar *aHScrollBar,
              TScrollBar *aVScrollBar, Boolean aCenterOps=False) :
   TListBox(bounds,aNumCols,aHScrollBar,aVScrollBar,aCenterOps) {};
 virtual void getText(char *dest, ccIndex item, short maxLen);
};

#if defined(Uses_TSListEditors) && !defined(TSListEditorsDefined)
// Easydiag object for it.
ListBoxSpecialize(TSListEditors);
#endif
