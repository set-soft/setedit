/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

    Class: TStringCollectionW

    Description: A simple string collection that can be used for streams.
    
    Class: TNoCaseStringCollection

    Description: Is a TStringCollection but no case sensintive.

    Class: TNoCaseNoOwnerStringCollection

    Description: Is a TStringCollection but no case sensintive, and isn't
                 the owner ship, is used when the item MUST NOT be free.

    Class: TNoCaseSOSStringCollection

    Description: Is like TNoCaseNoOwnerStringCollection but the inserted
                 values are stkHandlers instead of pointers.

    by SET

*****************************************************************************/


#if defined(Uses_TStringCollectionW) && !defined(__TStringCollectionW__)
#define __TStringCollectionW__

class TStringCollectionW : public TStringCollection
{
public:

 TStringCollectionW(ccIndex aLimit, ccIndex aDelta) :
      TStringCollection(aLimit,aDelta) {};

 SetDefStreamMembers(TStringCollectionW,TStringCollection)
};

SetDefStreamOperators(TStringCollectionW)

#endif

#if defined(Uses_TNoCaseStringCollection) && !defined(__TNoCaseStringCollection__)
#define __TNoCaseStringCollection__

class TNoCaseStringCollection : public TStringCollection
{
public:

 TNoCaseStringCollection(ccIndex aLimit, ccIndex aDelta) :
      TStringCollection(aLimit,aDelta) {};
 int compare(void *s1,void *s2) { return strcasecmp((char *)s1,(char *)s2); };

 SetDefStreamMembers(TNoCaseStringCollection,TStringCollection)
};

SetDefStreamOperators(TNoCaseStringCollection)

#endif


#if defined(Uses_TNoCaseNoOwnerStringCollection) && \
    !defined(__TNoCaseNoOwnerStringCollection__)
#define __TNoCaseNoOwnerStringCollection__

class TNoCaseNoOwnerStringCollection : public TStringCollection
{
public:
 TNoCaseNoOwnerStringCollection(ccIndex aLimit, ccIndex aDelta) :
      TStringCollection(aLimit,aDelta) {};
 int compare(void *s1,void *s2) { return strcasecmp((char *)s1,(char *)s2); };
 void freeItem(void *) {};
};

#endif

#if defined(Uses_TNoCaseSOSStringCollection) && \
    !defined(__TNoCaseSOSStringCollection__)
#define __TNoCaseSOSStringCollection__

class TNoCaseSOSStringCollection : public TStringCollection
{
public:
 TNoCaseSOSStringCollection(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
      TStringCollection(aLimit,aDelta)
 { stkL=stk; };
 int compare(void *s1,void *s2);
 void freeItem(void *) {};
 Boolean Search( char *key, ccIndex& index );
 Boolean SearchCase( char *key, ccIndex& index );
 void insert(stkHandler h) { TStringCollection::insert((void *)h); };
 char *atStr(ccIndex index) { return GetString(at(index)); };
 virtual char *GetString( void *h );
 SOStack *stkL;
};

#endif

#if defined(Uses_TSOSStringCollection) && \
    !defined(__TSOSStringCollection__)
#define __TSOSStringCollection__

class TSOSStringCollection : public TNoCaseSOSStringCollection
{
public:
 TSOSStringCollection(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
      TNoCaseSOSStringCollection(aLimit,aDelta,stk) {};
 Boolean SearchCase( char *key, ccIndex& index );
 int compare(void *s1,void *s2);
};

#endif

#if defined(Uses_TNCSAssociative) && \
    !defined(__TNCSAssociative__)
#define __TNCSAssociative__

typedef struct
{
 stkHandler h;      // Handler to the name of the element
 stkHandler Cont;   // Handler to the content of the element
 TNoCaseSOSStringCollection *c;  // Associated list
} stNCSAssociative;

class TNCSAssociative : public TNoCaseSOSStringCollection
{
public:
 TNCSAssociative(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
      TNoCaseSOSStringCollection(aLimit,aDelta,stk) {};
 void freeItem(void *s);
 void insert(char *s, stkHandler hv);
 TNoCaseSOSStringCollection *atCol(ccIndex index);
 virtual char *GetString( void *h );
 void  SetContent(char *s, char *cont);
 char *GetContent(ccIndex index);
};

#endif


#if defined(Uses_TSOSSortedListBox) && !defined(__TSOSSortedListBox__)
#define __TSOSSortedListBox__

class TSOSSortedListBox : public TSortedListBox
{
 public:
 TSOSSortedListBox(const TRect& bounds, ushort aNumCols,
                   TScrollBar *aScrollBar) :
 TSortedListBox(bounds,aNumCols,aScrollBar)
 { SearchPos=USHRT_MAX; ShiftState=0; };
 TSOSSortedListBox(const TRect& bounds, ushort aNumCols,
                   TScrollBar *aHScrollBar,TScrollBar *aVScrollBar,
                   Boolean aCenterOps=False) :
 TSortedListBox(bounds,aNumCols,aHScrollBar,aVScrollBar,aCenterOps)
 { SearchPos=USHRT_MAX; ShiftState=0; };
 void getText(char *dest, ccIndex item, short maxLen);
 void handleEvent(TEvent& event);

 private:
 ushort SearchPos;
 int ShiftState;
};

#endif

#if defined(Uses_TSOSListBox) && !defined(__TSOSListBox__)
#define __TSOSListBox__

class TSOSListBox : public TListBox
{
 public:
 TSOSListBox(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar) :
             TListBox(bounds,aNumCols,aScrollBar) {};
 void getText(char *dest, ccIndex item, short maxLen);
};

#endif

#if defined(Uses_TNSSOSCol) && \
    !defined(__TNSSOSCol__)
#define __TNSSOSCol__

class TNSSOSCol : public TNSCollection
{
public:
 TNSSOSCol(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
      TNSCollection(aLimit,aDelta)
 { stkL=stk; };
 void freeItem(void *) {};
 void insert(stkHandler h) { TNSCollection::insert((void *)h); };
 char *atStr(ccIndex index) { return GetString(at(index)); };
 virtual char *GetString( void *h );
 SOStack *stkL;
};

#endif

#if defined(Uses_TSOSCol) && \
    !defined(__TSOSCol__)
#define __TSOSCol__

class TSOSCol : public TCollection
{
public:
 TSOSCol(ccIndex aLimit, ccIndex aDelta, SOStack *stk) :
      TCollection(aLimit,aDelta)
 { stkL=stk; };
 void freeItem(void *) {};
 void insert(stkHandler h) { TNSCollection::insert((void *)h); };
 char *atStr(ccIndex index) { return GetString(at(index)); };
 virtual char *GetString( void *h );
 SOStack *stkL;
 virtual void *readItem(ipstream &) { return NULL; };
 virtual void writeItem(void *,opstream &) {};
};

/*
class TSOSCollection : public virtual TCollection
{
public:
 TSOSCollection(ccIndex limit, ccIndex delta, SOStack *stk) :
    TCollection(limit,delta) { stkL=stk; };
 // Don't delete items
 virtual void freeItem(void *) {};
 // No virtual because is a wrapper for the virtual
 char *atStr(ccIndex index) { return GetString(at(index)); };
 // Gets the string, depends on the Collection, but we provide the most common
 virtual char *GetString( void *h ) { return stkL->GetStrOf((stkHandler)h); };
 // Another wrapper
 void insert(stkHandler h) { TCollection::insert((void *)h); };
 SOStack *stkL;

private:
 // These 2 silly versions are just to allow TSOSCollection objects that
 // in fact aren't streamable
// virtual void *readItem(ipstream &) { return NULL; };
// virtual void writeItem(void *,opstream &) {};
};

class TSOSSortedCollection : public TSOSCollection,
                             public TSortedCollection
{
public:
 TSOSSortedCollection(ccIndex limit, ccIndex delta, SOStack *stk) :
    TSOSCollection(limit,delta,stk) {};
 // The keyOf can't return the item
 virtual void *keyOf(void *item) { return stkL->GetPointerOf((stkHandler)item); };
 // Overwrites the one from TSOSCollection
 void insert(stkHandler h) { TSortedCollection::insert((void *)h); };
 // Compare remains virtual and hence that's a base class
};

class TSOSStringCollection : public TSOSSortedCollection,
                             public TStringCollection
{
public:
 TSOSStringCollection(ccIndex limit, ccIndex delta, SOStack *stk) :
    TSOSSortedCollection(limit,delta,stk) {};
 void *keyOf(void *item) = TSOSSortedCollection::keyOf;
};

class TSOSNCStringCollection : virtual public TSOSStringCollection
{
public:
 TSOSNCStringCollection(ccIndex limit, ccIndex delta, SOStack *stk) :
    TSOSStringCollection(limit,delta,stk) {};
 // Just No Case.
 int compare(void *s1,void *s2) { return strcasecmp((char *)s1,(char *)s2); };
}

class TSOSListBox : public TListBox
{
public:
 TSOSListBox( const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar ) :
    TListBox(bounds,aNumCols,aScrollBar) {};
 // Just a wrapper
 virtual void newList( TSOSCollection *aList )
    { TListBox::newList((TCollection *)aList); };
 // A better getText to use atStr and not at
 virtual void getText( char *dest, ccIndex item, short maxChars );
};
*/
#endif

