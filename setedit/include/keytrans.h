/* Copyright (C) 1996-2001 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <dyncat.h>

#if defined(Uses_TKeyTranslate) && !defined(__TKeyTranslate__)
#define __TKeyTranslate__
const int kbtIsComm=0, kbtIsSComm=1, kbtIsMacro=2, kbtIsSeq=3;
const int kbtStatic=0, kbtDynamic=1, kbtExpanded=2;

typedef struct
{
 unsigned short cant;
 unsigned short commands[0];
} KeyTSeq;

typedef struct
{
 unsigned short key;
 unsigned short flags;
 union
 {
  int offset;
  void *data;
  unsigned short command;
  char *macro;
  KeyTSeq *sequence;
 } d;
} KeyTNode;

typedef struct
{
 unsigned cant;  // Number of entries in this table
 unsigned total; // Number of keys in this table and all the associated
 KeyTNode nodes[0];
} KeyTTable;

class TKeySeqCol;

// This class is a good exercise for the "data structures" topic.
// Is a crazy tree.
class TKeyTranslate : public TStringable
{
public:
 TKeyTranslate(KeyTTable *aBase, int aType=kbtStatic);
 ~TKeyTranslate();
 int get(unsigned key,KeyTNode *ret);
 KeyTNode *search(unsigned key);
 KeyTTable *expand(int &canBeDeleted);
 void compact(void);
 unsigned getLen(void);
 void deleteKey(unsigned wich);
 int addKey(TKeySeqCol *sKeys, void *data, int Type);
 virtual void getText(char *dest, unsigned item, int maxLen);
 int Save(char *name);
 int Load(char *name);
 void ChangeTable(KeyTTable *aBase, int aType=kbtStatic);
 int getType() { return type; };

protected:
 // Inline methodes to symplify the indirections
 // When the table is compacted (relative pointers)
 KeyTSeq *GetTSeqC(KeyTNode *node) { return (KeyTSeq *)(int(base)+node->d.offset); };
 KeyTTable *GetTableC(KeyTNode *node) { return (KeyTTable *)(int(base)+node->d.offset); };
 char *GetMNameC(KeyTNode *node) { return (char *)(int(base)+node->d.offset); };
 // Whe the table is expanded
 KeyTSeq *GetTSeqE(KeyTNode *node)  { return (KeyTSeq *)(node->d.data); };
 KeyTTable *GetTableE(KeyTNode *node) { return (KeyTTable *)(node->d.data); };
 char *GetMNameE(KeyTNode *node) { return (char *)(node->d.data); };
 // Any time
 KeyTSeq *GetTSeq(KeyTNode *node)
 { if (type==kbtExpanded)
      return (KeyTSeq *)(node->d.data);
  return (KeyTSeq *)(int(base)+node->d.offset);
 };
 KeyTTable *GetTable(KeyTNode *node)
 { if (type==kbtExpanded)
      return (KeyTTable *)(node->d.data);
  return (KeyTTable *)(int(base)+node->d.offset);
 };
 char *GetMName(KeyTNode *node)
 { if (type==kbtExpanded)
      return (char *)(node->d.data);
  return (char *)(int(base)+node->d.offset);
 };

 KeyTNode *InsertKey(unsigned key);
 KeyTTable *ExpandTable(KeyTTable *t);
 unsigned MeassureTree(KeyTTable *t);
 void CompactTable(KeyTTable *t);
 void DeleteTree(KeyTTable *t);
 void deleteTree(void);
 unsigned CountKeys(KeyTTable *t);
 void CatFullNameKey(KeyTNode *node, DynStrCatStruct *cat);
 void DeleteKey(KeyTTable *t, unsigned c, unsigned wich);
 KeyTNode *move(unsigned key, int add=0);

 KeyTTable *base;
 KeyTTable *curTable;
 KeyTNode *lastTableInSearch;
 int type;
 int state;
 int cSize;
 unsigned OffSet;
 char *newBase;
 unsigned numKey;
};

extern TKeyTranslate KeyTrans;
#endif

#if defined(Uses_TKeySeqCol) && !defined(__TKeySeqCol__)
#define __TKeySeqCol__
class TKeySeqCol : public TNSCollection, public TStringable
{
public:
 TKeySeqCol(ccIndex aLimit, ccIndex aDelta) :
   TNSCollection(aLimit,aDelta),
   TStringable() { shouldDelete=False; Count=0; };

 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
 void insert(unsigned short key) { TNSCollection::insert((void *)(unsigned)key); };
 void insert(int key) { TNSCollection::insert((void *)key); };
};
#endif

#if defined(Uses_TComSeqCol) && !defined(__TComSeqCol__)
#define __TComSeqCol__
class TComSeqCol : public TNSCollection, public TStringable
{
public:
 TComSeqCol(ccIndex aLimit, ccIndex aDelta) :
   TNSCollection(aLimit,aDelta),
   TStringable() { shouldDelete=False; Count=0; };

 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
 void insert(unsigned short key) { TNSCollection::insert((void *)(unsigned)key); };
 void insert(int key) { TNSCollection::insert((void *)key); };
};
#endif

const int tktMaxKeyName=40;

extern int InterpretKeyName(char *s, ushort &code);
extern void TCEditor_MakeKeyName(char *s, unsigned short key);
// edkeys.cc
extern unsigned short TCEditor_SelectAKey(void);
