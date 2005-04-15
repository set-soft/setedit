/* Copyright (C) 1996-2005 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TPMCollection) && !defined(__TPMCollection__)
#define __TPMCollection__
typedef struct
{
 char   trigger[4];
 uint32 flags;
 char  *str;
 char  *name;
 TNSCollection *vars;
 TNSCollection *defaults;
 unsigned mLenVar;
} PMacroStr;

class TPMCollection : public TStringCollection, public TStringable
{
public:
 TPMCollection(ccIndex aLimit, ccIndex aDelta) :
   TStringCollection(aLimit,aDelta) {};
 virtual void   *keyOf(void *);
 virtual void    freeItem(void *);
 virtual ccIndex insert(void *item);
 PMacroStr *searchByNamePointer(void *name);
 // TStringable:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
};
#endif
