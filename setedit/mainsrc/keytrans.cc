/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  Test to make a really complex translator from a keyboard sequence to a
  command sequence.

*****************************************************************************/
#define Uses_stdlib
#define Uses_stdio
#define Uses_string
#define Uses_limits

#define Uses_TNSCollection
#define Uses_TEvent
#define Uses_MsgBox
#define Uses_TStringable
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TGKey
#include <settvuti.h>
#define Uses_TCEditor_Commands
#define Uses_TCEditor_External
#include <ceditor.h>
//#define NDEBUG
#include <assert.h>
#include <dyncat.h>
#define Uses_TKeyTranslate
#define Uses_TComSeqCol
#define Uses_TKeySeqCol
#include <keytrans.h>


static char *Error=0;
static void StoreError(const char *error,char *file);
static const char *loadedFile=0;

TKeyTranslate::TKeyTranslate(KeyTTable *aBase, int aType) :
   TStringable()
{
 assert(type!=kbtExpanded);
 base=aBase;
 curTable=aBase;
 type=aType;
 state=0;
 // That's a helper to avoid counting the values by hand
 Count=base->total=CountKeys(base);
}

TKeyTranslate::~TKeyTranslate()
{
 deleteTree();
}

KeyTNode *TKeyTranslate::search(unsigned key)
{
 KeyTNode *array=curTable->nodes,*cmpval;
 int nelem=curTable->cant,lim;

 for (lim=nelem; lim!=0; lim>>=1)
    {
     cmpval=&array[lim>>1];
     if (cmpval->key==(int)key)
        return cmpval;
     if (cmpval->key<(int)key)
       { /* key > p: move right */
        array=cmpval+1;
        lim--;
       } /* else move left */
    }
 return 0;
}

inline
void AlignSize(unsigned &aux)
{
 if (aux & 3)
    aux=(aux | 3)+1;
}

#define EOS 0

void TKeyTranslate::getText(char *dest, unsigned item, int maxLen)
{
 if (item>=base->total)
   {
    *dest=EOS;
    return;
   }
 DynStrCatStruct cat;
 // That initializes the structure
 DynStrCatInit(&cat,0,0);
 KeyTTable *t=base;
 unsigned c=0,i=0;

 while (1)
   {
    KeyTNode *node=&(t->nodes[i]);
    if (node->flags==kbtIsSComm)
      {
       KeyTTable *nT=GetTableE(node);
       if (item<c+nT->total)
         { // Is in a deeper table
          CatFullNameKey(node,&cat);
          t=nT;
          i=0;
         }
       else
         {
          c+=nT->total;
          i++;
         }
      }
    else
      {
       if (c==item)
         {
          CatFullNameKey(node,&cat);
          break;
         }
       c++;
       i++;
      }
   }
 #ifndef NDEBUG
 //printf("%s\n",cat.str);
 #endif
 strncpy(dest,cat.str,maxLen);
 dest[maxLen]=EOS;
 delete cat.str;
}

void TKeyTranslate::deleteKey(unsigned which)
{
 assert(type==kbtExpanded);
 assert(base->total>which);
 DeleteKey(base,0,which);
 Count=base->total;
}

void TKeyTranslate::DeleteKey(KeyTTable *t, unsigned baseNumKey, unsigned which)
{
 unsigned cant=t->cant;
 unsigned i;

 for (i=0; i<cant; )
    {
     KeyTNode *node=&(t->nodes[i]);
     if (node->flags==kbtIsSComm && baseNumKey!=which)
       {
        KeyTTable *nT=GetTableE(node);
        if (which<baseNumKey+nT->total)
          { // Is in a deeper table
           DeleteKey(nT,baseNumKey,which);
           if (nT->cant==0)
             { // The table is empty
              delete nT;
              // Now reduce it
              memcpy(node,node+1,sizeof(KeyTNode)*(cant-i-1));
              t->cant--;
             }
           t->total--;
           return;
          }
        else
          { // Skip the table
           baseNumKey+=nT->total;
           i++;
          }
       }
     else
       {
        if (baseNumKey==which)
          {
           if (node->flags==kbtIsMacro || node->flags==kbtIsSeq)
              delete[] node->d.macro;
           else if (node->flags==kbtIsSComm)
              // It removes a branch in the tree
              DeleteTree(GetTableE(node));
           memcpy(node,node+1,sizeof(KeyTNode)*(cant-i-1));
           t->cant--;
           t->total--;
           return;
          }
        baseNumKey++;
        i++;
       }
    }
}

void TCEditor_MakeKeyName(char *s, unsigned short key)
{
 char *b=s;

 #define A(cas,a,b)  if (key & cas) { *s=a; *(s+1)=b; s+=2; }
 A(kbShiftCode,'S','h')
 A(kbCtrlCode,'C','t')
 A(kbAltRCode,'a','l')
 A(kbAltLCode,'A','l')
 #undef A
 *s=0;
 strcat(b,TGKey::NumberToKeyName(key & kbKeyMask));
}

int InterpretKeyName(char *s, ushort &code)
{
 ushort lcode=0;

 #define A(cas,a,b)  if (*s==a && *(s+1)==b) { s+=2; lcode|=cas; }
 A(kbShiftCode,'S','h')
 A(kbCtrlCode,'C','t')
 A(kbAltRCode,'a','l')
 A(kbAltLCode,'A','l')
 #undef A
 ushort n=TGKey::KeyNameToNumber(s);
 if (n==(ushort)-1)
    return 1;
 code=lcode | n;
 return 0;
}

void TKeyTranslate::CatFullNameKey(KeyTNode *node, DynStrCatStruct *cat)
{
 assert(type==kbtExpanded);
 int i;
 char b[tktMaxKeyName]; // must be enough for any key name

 TCEditor_MakeKeyName(b,node->key);
 if (node->flags==kbtIsSComm)
    strcat(b," ");
 else
    strcat(b,"  ->  ");
 DynStrCat(cat,b);

 KeyTSeq *se;
 switch (node->flags)
   {
    case kbtIsComm:
         DynStrCat(cat,TranslateEdCommand(node->d.command));
         break;
    case kbtIsMacro:
         DynStrCat(cat,GetMNameE(node));
         break;
    case kbtIsSeq:
         se=GetTSeqE(node);
         for (i=0; i<se->cant; i++)
            {
             char *p=TranslateEdCommand(se->commands[i]);
             if (p)
               {
                DynStrCat(cat,p);
                DynStrCat(cat," ");
               }
            }
         break;
   }
}

unsigned TKeyTranslate::CountKeys(KeyTTable *t)
{
 unsigned cant=t->cant;
 unsigned i,aCount=0;

 for (i=0; i<cant; i++)
    {
     KeyTNode *node=&(t->nodes[i]);
     if (node->flags==kbtIsSComm)
       {
        KeyTTable *nT=GetTableC(node);
        unsigned c=CountKeys(nT);
        nT->total=c;
        aCount+=c;
       }
     else
        aCount++;
    }
 return aCount;
}

int TKeyTranslate::get(unsigned key,KeyTNode *ret)
{
 assert(type!=kbtExpanded);
 KeyTNode *p=search(key);
 if (!p)
   {
    rewind();
    return 0;
   }
 memcpy(ret,p,sizeof(KeyTNode));
 switch (ret->flags)
   {
    case kbtIsSComm:
         state++;
         curTable=GetTableC(ret);
         return -1;
    case kbtIsMacro:
         ret->d.data=GetMNameC(ret);
         break;
    case kbtIsSeq:
         ret->d.data=GetTSeqC(ret);
         break;
   }
 rewind();
 return 1;
}

KeyTNode *TKeyTranslate::move(unsigned key, int add)
{
 KeyTNode *array=curTable->nodes,*p=0;
 int c=curTable->cant,i,found=0,num;

 for (i=0,num=0; i<c; i++)
    {
     p=array+i;
     if (p->key==(int)key)
       {
        found++;
        break;
       }
     if (p->flags==kbtIsSComm)
        num+=(GetTable(p))->total;
     else
       num++;
    }
 if (!found || !p)
    return 0;

 curTable->total+=add;
 numKey+=num;
 if (p->flags==kbtIsSComm)
   {
    state++;
    lastTableInSearch=p;
    curTable=GetTable(p);
   }

 return p;
}

/**[txh]********************************************************************

  Description:
  Expands the tree to be easylly editable. That means: all small chuncks
of memory. Calls @x{::ExpandTable}, to do the job.@p
  Returns the old compacted block that must be deleted by the caller. That's
to allow an easy undo during the expanded phase. Additionally canBeDeleted
indicates if delete can be used or the pointer is to the original.

***************************************************************************/

KeyTTable *TKeyTranslate::expand(int &canBeDeleted)
{
 assert(type!=kbtExpanded);
 canBeDeleted=0;
 KeyTTable *newT=ExpandTable(base);
 if (!newT)
    return 0;
 canBeDeleted=(type==kbtDynamic);
 type=kbtExpanded;
 KeyTTable *oldT=base;
 base=newT;

 return oldT;
}

/**[txh]********************************************************************

  Description:
  That's the function that recursivelly travels the tree generating a
fully unpacked version.

***************************************************************************/

KeyTTable *TKeyTranslate::ExpandTable(KeyTTable *t)
{
 unsigned cant=t->cant;
 unsigned i,size=sizeof(KeyTTable)+cant*sizeof(KeyTNode);
 KeyTTable *nAux;

 KeyTTable *nT=(KeyTTable *)new char[size];
 if (!nT)
    return 0;
 memcpy(nT,t,size);
 for (i=0; i<cant; i++)
    {
     KeyTNode *node=&(nT->nodes[i]);
     KeyTSeq *s,*s2;
     switch (node->flags)
       {
        case kbtIsComm:
             break;
        case kbtIsSComm:
             nAux=ExpandTable(GetTableC(node));
             if (!nAux)
                return 0;
             node->d.data=nAux;
             break;
        case kbtIsMacro:
             node->d.data=newStr(GetMNameC(node));
             break;
        case kbtIsSeq:
             s=GetTSeqC(node);
             size=sizeof(unsigned short)*(s->cant+1);
             s2=(KeyTSeq *)(new char[size]);
             if (!s2)
                return 0;
             memcpy(s2,s,size);
             node->d.data=s2;
             break;
       }
    }
 return nT;
}

/**[txh]********************************************************************

  Description:
  Calculates how much memory I need to store the tree in one block.

***************************************************************************/

unsigned TKeyTranslate::MeassureTree(KeyTTable *t)
{
 assert(type==kbtExpanded);
 unsigned cant=t->cant;
 unsigned i,size=sizeof(KeyTTable)+cant*sizeof(KeyTNode),aux;

 for (i=0; i<cant; i++)
    {
     KeyTNode *node=&(t->nodes[i]);
     KeyTSeq *s;
     switch (node->flags)
       {
        case kbtIsComm:
             break;
        case kbtIsSComm:
             size+=MeassureTree(GetTableE(node));
             break;
        case kbtIsMacro:
             aux=strlen(GetMNameE(node))+1;
             AlignSize(aux);
             size+=aux;
             break;
        case kbtIsSeq:
             s=GetTSeqE(node);
             aux=sizeof(unsigned short)*(s->cant+1);
             AlignSize(aux);
             size+=aux;
             break;
       }
    }
 return size;
}

/**[txh]********************************************************************

  Description:
  Reports the size of the structure, can't be called when static.

***************************************************************************/

unsigned TKeyTranslate::getLen(void)
{
 assert(type!=kbtStatic);
 if (type==kbtExpanded)
    return MeassureTree(base);
 return cSize;
}

/**[txh]********************************************************************

  Description:
  Deletes the current table and uses the new one.

***************************************************************************/

void TKeyTranslate::ChangeTable(KeyTTable *aBase, int aType)
{
 deleteTree();
 base=aBase;
 curTable=aBase;
 type=aType;
 state=0;
 Count=base->total=CountKeys(base);
}

/**[txh]********************************************************************

  Description:
  Compacts the tree in one block, switable for writing to disk. Calls to
@x{::CompactTable}, to do the job.

***************************************************************************/

void TKeyTranslate::compact(void)
{
 assert(type==kbtExpanded);
 cSize=MeassureTree(base);
 OffSet=0;
 newBase=new char[cSize];
 if (!newBase)
    return;
 CompactTable(base);
 deleteTree();
 type=kbtDynamic;
 curTable=base=(KeyTTable *)newBase;
 state=0;
}

void TKeyTranslate::CompactTable(KeyTTable *t)
{
 unsigned cant=t->cant;
 unsigned i,size=sizeof(KeyTTable)+cant*sizeof(KeyTNode),aux;

 KeyTTable *nT=(KeyTTable *)(newBase+OffSet);
 memcpy(nT,t,size);
 OffSet+=size;
 for (i=0; i<cant; i++)
    {
     KeyTNode *node=&(nT->nodes[i]);
     KeyTSeq *s;
     switch (node->flags)
       {
        case kbtIsComm:
             break;
        case kbtIsSComm:
             aux=OffSet;
             CompactTable(GetTableE(node));
             node->d.offset=aux;
             break;
        case kbtIsMacro:
             aux=strlen(GetMNameE(node))+1;
             AlignSize(aux);
             strcpy((char *)(newBase+OffSet),GetMNameE(node));
             node->d.offset=OffSet;
             OffSet+=aux;
             break;
        case kbtIsSeq:
             s=GetTSeqE(node);
             aux=sizeof(unsigned short)*(s->cant+1);
             AlignSize(aux);
             memcpy((KeyTSeq *)(newBase+OffSet),s,aux);
             node->d.offset=OffSet;
             OffSet+=aux;
             break;
       }
    }
}

void TKeyTranslate::deleteTree(void)
{
 if (type==kbtDynamic)
    delete[] base;
 else
    if (type==kbtExpanded)
       DeleteTree(base);
}

void TKeyTranslate::DeleteTree(KeyTTable *t)
{
 unsigned cant=t->cant;
 unsigned i;

 for (i=0; i<cant; i++)
    {
     KeyTNode *node=&(t->nodes[i]);
     switch (node->flags)
       {
        case kbtIsComm:
             break;
        case kbtIsSComm:
             DeleteTree(GetTableE(node));
             break;
        case kbtIsMacro:
             delete GetMNameE(node);
             break;
        case kbtIsSeq:
             delete GetTSeqE(node);
             break;
       }
    }
 delete t;
}

void TKeySeqCol::getText(char *dest, unsigned item, int maxLen)
{
 char b[tktMaxKeyName];
 TCEditor_MakeKeyName(b,(unsigned short)((unsigned long)at(item)));
 strncpy(dest,b,maxLen);
 dest[maxLen]=EOS;
}

void TComSeqCol::getText(char *dest, unsigned item, int maxLen)
{
 char b[40];
 *b=0;

 char *s=TranslateEdCommand((unsigned long)(at(item)));
 if (s)
   {
    strcpy(b,"cmc");
    strcat(b,s);
   }
 strncpy(dest,b,maxLen);
 dest[maxLen]=EOS;
}

/**[txh]********************************************************************

  Description:
  Inserts a KeyTNode in the curTable and assigns key to it. The rest of
data must be arranged by the caller.

***************************************************************************/

KeyTNode *TKeyTranslate::InsertKey(unsigned key)
{
 KeyTNode *array=curTable->nodes;
 int c=curTable->cant,i,updateBase;

 updateBase=curTable==base;
 // look the position for it to keep the table sorted
 i=0;
 while (i<c && array[i].key<(int)key) i++;
 // Make room for one more
 curTable->cant++;
 //curTable->total++; Later
 curTable=(KeyTTable *)realloc(curTable,sizeof(KeyTTable)+curTable->cant*sizeof(KeyTNode));
 // Insert the key
 if (i!=c)
    memmove(&(curTable->nodes[i+1]),&(curTable->nodes[i]),(c-i)*sizeof(KeyTNode));
 curTable->nodes[i].key=key;
 // If this table is conected to another don't forget that realloc can
 // change the pointer.
 if (lastTableInSearch)
    lastTableInSearch->d.data=curTable;
 if (updateBase)
    base=curTable;
 return &(curTable->nodes[i]);
}

/**[txh]********************************************************************

  Description:
  That's a very complex thing because I must:@p
1) Check if the key can be added without problems.@*
2) Add a new key to the tree, it can mean extend the tree.@*
3) Update the counters of keys in each table.@p

  Return:
  A non-negative value if there are a key already defined for the
sequence or part of the sequence. The value is returned to allow to the
program ask for deletion.@p
  -1 if the sequence will break more than one key assigment.@p
  -2 if the key was added.

***************************************************************************/

int TKeyTranslate::addKey(TKeySeqCol *sKeys, void *data, int Type, int *keyDef)
{
 assert(type==kbtExpanded);
 int c=sKeys->getCount();
 int i,ok=0;
 KeyTNode *node;
 //char b[100];

 numKey=0;
 lastTableInSearch=0;
 rewind();
 for (i=0; i<c; i++)
    {
     node=move((unsigned long)(sKeys->at(i)));
     if (!node)
       {
        ok=1;
        break;
       }
     if (node->flags!=kbtIsSComm)
       {
        /*printf("Ya tiene asignado algo (%d)\n",numKey);
        getText(b,numKey,99);*/
        rewind();
        return numKey;
       }
    }
 if (!ok)
   {
    //printf("No se puede ya que tiene m s de 1 asignada\n");
    rewind();
    if (keyDef)
       *keyDef=numKey;
    return -1;
   }
 node=InsertKey((unsigned long)(sKeys->at(i)));
 // Now if the sequence is larger create a ramification in the tree to hold it
 while (i<(c-1))
   {
    node->flags=kbtIsSComm;
    KeyTTable *nT=(KeyTTable *)new char[sizeof(KeyTTable)+sizeof(KeyTNode)];
    node->d.data=nT;
    nT->cant=1;
    nT->total=0; // Later
    node=nT->nodes;
    node->key=(unsigned short)(unsigned long)(sKeys->at(++i));
   }
 // Now node points to an end of the tree
 // The valid Types are kbtIsMacro kbtIsSeq
 if (Type==kbtIsMacro)
   { // Easy ;-)
    node->flags=kbtIsMacro;
    node->d.data=newStr((char *)data);
   }
 else
   {
    TComSeqCol *p=(TComSeqCol *)data;
    c=p->getCount();
    if (c==1)
      {
       node->flags=kbtIsComm;
       node->d.command=(unsigned short)((unsigned long)p->at(0));
      }
    else
      { // The never ending story ...
       KeyTSeq *s=(KeyTSeq *)new char[sizeof(KeyTSeq)+sizeof(unsigned short)*c];
       s->cant=c;
       for (i=0; i<c; i++)
           s->commands[i]=(unsigned short)((unsigned long)p->at(i));
       node->flags=kbtIsSeq;
       node->d.data=s;
      }
   }
 // Hoppefully the key is there
 rewind();
 // But what about the counters?
 Count++;
 c=sKeys->getCount();
 for (i=0; i<c; i++)
    {
     #ifdef NDEBUG
     move((unsigned)(sKeys->at(i)),1);
     #else
     node=move((unsigned long)(sKeys->at(i)),1);
     assert(node!=0);
     #endif
    }
 rewind();

 return -2;
}

static char Signature[]="SET's editor keyboard binding file\x1A";;
const int Version=4;

int TKeyTranslate::Save(char *name)
{
 assert(type!=kbtExpanded);
 FILE *f=fopen(name,"wb");
 if (!f)
    return 1;
 fwrite(Signature,sizeof(Signature),1,f);
 fputc(Version,f);
 ushort w=TGKey::GetAltSettings();
 fwrite(&w,sizeof(w),1,f);
 if (type==kbtStatic)
   { // Don't save the keys if not needed
    unsigned c=0;
    fwrite(&c,sizeof(cSize),1,f);
   }
 else
   {
    fwrite(&cSize,sizeof(cSize),1,f);
    fwrite(base,cSize,1,f);
   }
 int translateKeyPad=TGKey::GetKbdMapping(TGKey::dosTranslateKeypad);
 fwrite(&translateKeyPad,sizeof(translateKeyPad),1,f);
 fclose(f);
 return 0;
}

#undef GenError
#define GenError(a) \
{\
 StoreError(a,name);\
 fclose(f);\
 return 1;\
}

int TKeyTranslate::Load(char *name)
{
 char buf[sizeof(Signature)];
 FILE *f=fopen(name,"rb");
 if (!f)
    return 0;
 fread(buf,sizeof(Signature),1,f);
 if (strcmp(buf,Signature))
    GenError(__("Wrong file"));

 int V=fgetc(f);
 if (V>Version || V<3)
    GenError(__("Wrong version"));

 ushort w;
 fread(&w,sizeof(w),1,f);
 TGKey::SetAltSettings(w);

 int lcSize,replaceK=0;
 fread(&lcSize,sizeof(lcSize),1,f);
 if (lcSize)
   {
    newBase=new char[lcSize];
    fread(newBase,lcSize,1,f);
    replaceK=1;
   }
 if (ferror(f))
   {
    delete newBase;
    GenError(__("Error while reading"));
   }
 if (V>=4)
   {
    int translateKeyPad;
    fread(&translateKeyPad,sizeof(translateKeyPad),1,f);
    TGKey::SetKbdMapping(translateKeyPad ? TGKey::dosTranslateKeypad : TGKey::dosNormalKeypad);
   }
 fclose(f);
 if (replaceK)
   {
    deleteTree();
    type=kbtDynamic;
    curTable=base=(KeyTTable *)newBase;
   }

 return 0;
}


/******************************* DEFAULT key assigment *******************************/





// That's a trick to have a hardcoded tree. Is dirty
#define DeclareTable(a) \
typedef struct \
{ unsigned cant; unsigned total; KeyTNode nodes[a]; } KeyTTable##a

#define DeclareSeq(a) \
typedef struct \
{ unsigned short cant; unsigned short commands[a]; } KeyTSeq##a

#define pSeq(a) (((char *)&a)-((char *)&base))
#define dSeqSel(name,comm) \
        KeyTSeq3 name={ 3, {cmcSelectOn,comm,cmcSelectOff} }

// Doesn't exist by now, will be implemented
//const cmcSelectOn=1000,cmcSelectOff=1001;

DeclareSeq(3);

extern KeyTSeq3 ShUp,ShDn,ShHome,ShEnd,ShPgUp,ShPgDn,ShL,ShR,ShCtL,ShCtR,
                ShCtHome,ShCtEnd,ShCtPgUp,ShCtPgDn;

// 195 keys defined
const int NormalSize=88,CtrlQSize=50,CtrlKSize=61;
#define NormalTable KeyTTable88
#define CtrlQTable  KeyTTable50
#define CtrlKTable  KeyTTable61
DeclareTable(88);
DeclareTable(50);
DeclareTable(61);

extern CtrlQTable CtrlQ;
extern CtrlKTable CtrlK;

NormalTable base=
{ NormalSize,0,
 { // MUST be sorted
  // Normal 14
  { kbEsc, kbtIsComm, {cmcHideSelect} }, // 31
  { kbBackSpace, kbtIsComm, {cmcBackSpace} }, // 42
  { kbEnter, kbtIsComm, {cmcNewLine} }, // 44
  { kbHome, kbtIsComm, {cmcLineStart} }, // 69
  { kbUp, kbtIsComm, {cmcLineUp} }, // 70
  { kbPgUp, kbtIsComm, {cmcPageUp} }, // 71
  { kbLeft, kbtIsComm, {cmcCharLeft} }, // 72
  { kbRight, kbtIsComm, {cmcCharRight} }, // 73
  { kbEnd, kbtIsComm, {cmcLineEnd} }, // 74
  { kbDown, kbtIsComm, {cmcLineDown} }, // 75
  { kbPgDn, kbtIsComm, {cmcPageDown} }, // 76
  { kbInsert, kbtIsComm, {cmcInsMode} }, // 77
  { kbDelete, kbtIsComm, {cmcDelCharClear} }, // 78
  { kbMacro, kbtIsComm, {cmcExpandCode} }, // 98
  // Shift 10
  { kbShBackSpace, kbtIsComm, {cmcBackSpace} }, // 42
  { kbShEnter, kbtIsComm, {cmcNewLine} }, // 44
  { kbShHome, kbtIsSeq, {pSeq(ShHome)} }, // 69
  { kbShUp, kbtIsSeq, {pSeq(ShUp)} }, // 70
  { kbShPgUp, kbtIsSeq, {pSeq(ShPgUp)} }, // 71
  { kbShLeft, kbtIsSeq, {pSeq(ShL)} }, // 72
  { kbShRight, kbtIsSeq, {pSeq(ShR)} }, // 73
  { kbShEnd, kbtIsSeq, {pSeq(ShEnd)} }, // 74
  { kbShDown, kbtIsSeq, {pSeq(ShDn)} }, // 75
  { kbShPgDn, kbtIsSeq, {pSeq(ShPgDn)} }, // 76
  // Control 46
  { kbCtA, kbtIsComm, {cmcWordLeft} }, // 1
  { kbCtC, kbtIsComm, {cmcPageDown} }, // 3
  { kbCtD, kbtIsComm, {cmcCharRight} }, // 4
  { kbCtE, kbtIsComm, {cmcLineUp} }, // 5
  { kbCtF, kbtIsComm, {cmcWordRight} }, // 6
  { kbCtG, kbtIsComm, {cmcDelChar} }, // 7
  { kbCtH, kbtIsComm, {cmcBackSpace} }, // 8
  { kbCtJ, kbtIsComm, {cmcGotoEditorLine} }, // 10
  { kbCtK, kbtIsSComm, {pSeq(CtrlK)} }, // 11
  { kbCtL, kbtIsComm, {cmcSearchAgain} }, // 12
  { kbCtM, kbtIsComm, {cmcNewLine} }, // 13
  { kbCtN, kbtIsComm, {cmcInsertNewLine} }, // 14
  { kbCtO, kbtIsComm, {cmcIndentMode} }, // 15
  { kbCtP, kbtIsComm, {cmcIntelIndentMode} }, // 16
  { kbCtQ, kbtIsSComm, {pSeq(CtrlQ)} }, // 17
  { kbCtR, kbtIsComm, {cmcPageUp} }, // 18
  { kbCtS, kbtIsComm, {cmcCharLeft} }, // 19
  { kbCtT, kbtIsComm, {cmcDelWord} }, // 20
  { kbCtU, kbtIsComm, {cmcUndo} }, // 21
  { kbCtV, kbtIsComm, {cmcInsMode} }, // 22
  // Moved to menues { kbCtW, kbtIsComm, {cmcScrollDown} }, // 23
  { kbCtX, kbtIsComm, {cmcLineDown} }, // 24
  { kbCtY, kbtIsComm, {cmcDelLine} }, // 25
  // Moved to menues { kbCtZ, kbtIsComm, {cmcScrollUp} }, // 26
  { kbCtOpenBrace, kbtIsComm, {cmcSearchStart} }, // 27
  { kbCtCloseBrace, kbtIsComm, {cmcSearchEnd} }, // 29
  { kbCt0, kbtIsComm, {cmcGotoMark0} }, // 32
  { kbCt1, kbtIsComm, {cmcGotoMark1} }, // 33
  { kbCt2, kbtIsComm, {cmcGotoMark2} }, // 34
  { kbCt3, kbtIsComm, {cmcGotoMark3} }, // 35
  { kbCt4, kbtIsComm, {cmcGotoMark4} }, // 36
  { kbCt5, kbtIsComm, {cmcGotoMark5} }, // 37
  { kbCt6, kbtIsComm, {cmcGotoMark6} }, // 38
  { kbCt7, kbtIsComm, {cmcGotoMark7} }, // 39
  { kbCt8, kbtIsComm, {cmcGotoMark8} }, // 40
  { kbCt9, kbtIsComm, {cmcGotoMark9} }, // 41
  { kbCtBackSpace, kbtIsComm, {cmcDelPrevWord} }, // 42
  { kbCtTab, kbtIsComm, {cmcSmartIndent} }, // 43
  { kbCtEnter, kbtIsComm, {cmcLoadFileUnderCur} }, // 44
  { kbCtSpace, kbtIsComm, {cmcExpandCode} }, // 52
  { kbCtHome, kbtIsComm, {cmcFirstLineInScreen} }, // 69
  { kbCtPgUp, kbtIsComm, {cmcTextStart} }, // 71
  { kbCtLeft, kbtIsComm, {cmcWordLeft} }, // 72
  { kbCtRight, kbtIsComm, {cmcWordRight} }, // 73
  { kbCtEnd, kbtIsComm, {cmcLastLineInScreen} }, // 74
  { kbCtPgDn, kbtIsComm, {cmcTextEnd} }, // 76
  { kbCtInsert, kbtIsComm, {cmcCopy} }, // 77
  { kbCtDelete, kbtIsComm, {cmcClear} }, // 78
  // Shift Ctrl 15
  { kbShCtA, kbtIsSeq, {pSeq(ShCtL)} }, // 1
  { kbShCtC, kbtIsSeq, {pSeq(ShPgUp)} }, // 3
  { kbShCtD, kbtIsSeq, {pSeq(ShR)} }, // 4
  { kbShCtE, kbtIsSeq, {pSeq(ShUp)} }, // 5
  { kbShCtF, kbtIsSeq, {pSeq(ShCtR)} }, // 6
  { kbShCtR, kbtIsSeq, {pSeq(ShPgUp)} }, // 18
  { kbShCtS, kbtIsSeq, {pSeq(ShL)} }, // 19
  { kbShCtX, kbtIsSeq, {pSeq(ShDn)} }, // 24
  { kbShCt0, kbtIsComm, {cmcSearchClPar} }, // 32
  { kbShCt9, kbtIsComm, {cmcSearchOpPar} }, // 41
  { kbShCtTab, kbtIsComm, {cmcSmartUnIndent} }, // 43
  { kbShCtHome, kbtIsSeq, {pSeq(ShCtHome)} }, // 69
  { kbShCtPgUp, kbtIsSeq, {pSeq(ShCtPgUp)} }, // 71
  { kbShCtLeft, kbtIsSeq, {pSeq(ShCtL)} }, // 72
  { kbShCtRight,kbtIsSeq, {pSeq(ShCtR)} }, // 73
  { kbShCtEnd,  kbtIsSeq, {pSeq(ShCtEnd)} }, // 74
  { kbShCtPgDn, kbtIsSeq, {pSeq(ShCtPgDn)} }, // 76
  { kbShCtInsert, kbtIsComm, {cmcReplaceSelect} } // 77
 }
};

CtrlQTable CtrlQ=
{ CtrlQSize,0,
 { // MUST be sorted
  // Normal 26
  { kbA, kbtIsComm, {cmcReplace} }, // 1
  { kbB, kbtIsComm, {cmcGoBeginBlock} }, // 2
  { kbC, kbtIsComm, {cmcTextEnd} }, // 3
  { kbD, kbtIsComm, {cmcLineEnd} }, // 4
  { kbE, kbtIsComm, {cmcFirstLineInScreen} }, // 5
  { kbF, kbtIsComm, {cmcFind} }, // 6
  { kbH, kbtIsComm, {cmcDelStart} }, // 8
  { kbK, kbtIsComm, {cmcGoEndBlock} }, // 11
  { kbL, kbtIsComm, {cmcSelLength} }, // 12
  { kbM, kbtIsComm, {cmcChooseMacro} }, // 13
  { kbP, kbtIsComm, {cmcJumpLastCursorPos} }, //
  { kbR, kbtIsComm, {cmcTextStart} }, // 18
  { kbS, kbtIsComm, {cmcLineStart} }, // 19
  { kbX, kbtIsComm, {cmcLastLineInScreen} }, // 24
  { kbY, kbtIsComm, {cmcDelEnd} }, // 25
  { kbEsc, kbtIsComm, {cmcSearchComplement} }, // 31
  { kb0, kbtIsComm, {cmcGotoMark0} }, // 32
  { kb1, kbtIsComm, {cmcGotoMark1} }, // 33
  { kb2, kbtIsComm, {cmcGotoMark2} }, // 34
  { kb3, kbtIsComm, {cmcGotoMark3} }, // 35
  { kb4, kbtIsComm, {cmcGotoMark4} }, // 36
  { kb5, kbtIsComm, {cmcGotoMark5} }, // 37
  { kb6, kbtIsComm, {cmcGotoMark6} }, // 38
  { kb7, kbtIsComm, {cmcGotoMark7} }, // 39
  { kb8, kbtIsComm, {cmcGotoMark8} }, // 40
  { kb9, kbtIsComm, {cmcGotoMark9} }, // 41

   // Control 24 All twice for the people that lets the finger in Ctrl
  { kbCtA, kbtIsComm, {cmcReplace} }, // 1
  { kbCtB, kbtIsComm, {cmcGoBeginBlock} }, // 2
  { kbCtC, kbtIsComm, {cmcTextEnd} }, // 3
  { kbCtD, kbtIsComm, {cmcLineEnd} }, // 4
  { kbCtE, kbtIsComm, {cmcFirstLineInScreen} }, // 5
  { kbCtF, kbtIsComm, {cmcFind} }, // 6
  { kbCtH, kbtIsComm, {cmcDelStart} }, // 8
  { kbCtK, kbtIsComm, {cmcGoEndBlock} }, // 11
  { kbCtL, kbtIsComm, {cmcSelLength} }, // 12
  { kbCtM, kbtIsComm, {cmcChooseMacro} }, // 13
  { kbCtR, kbtIsComm, {cmcTextStart} }, // 18
  { kbCtS, kbtIsComm, {cmcLineStart} }, // 19
  { kbCtX, kbtIsComm, {cmcLastLineInScreen} }, // 24
  { kbCtY, kbtIsComm, {cmcDelEnd} }, // 25
  { kbCt0, kbtIsComm, {cmcGotoMark0} }, // 32
  { kbCt1, kbtIsComm, {cmcGotoMark1} }, // 33
  { kbCt2, kbtIsComm, {cmcGotoMark2} }, // 34
  { kbCt3, kbtIsComm, {cmcGotoMark3} }, // 35
  { kbCt4, kbtIsComm, {cmcGotoMark4} }, // 36
  { kbCt5, kbtIsComm, {cmcGotoMark5} }, // 37
  { kbCt6, kbtIsComm, {cmcGotoMark6} }, // 38
  { kbCt7, kbtIsComm, {cmcGotoMark7} }, // 39
  { kbCt8, kbtIsComm, {cmcGotoMark8} }, // 40
  { kbCt9, kbtIsComm, {cmcGotoMark9} } // 41
 }
};

CtrlKTable CtrlK=
{ CtrlKSize,0,
 { // MUST be sorted
  // Normal 25
  { kbB, kbtIsComm, {cmcStartSelect} }, // 2
  { kbC, kbtIsComm, {cmcCopyBlock} }, // 3
  { kbH, kbtIsComm, {cmcHideSelect} }, // 8
  { kbI, kbtIsComm, {cmcIndentBlkOne} }, // 9
  { kbK, kbtIsComm, {cmcEndSelect} }, // 11
  { kbL, kbtIsComm, {cmcMarkLine} }, // 12
  { kbM, kbtIsComm, {cmcToUpper} }, // 13
  { kbO, kbtIsComm, {cmcToLower} }, // 15
  { kbR, kbtIsComm, {cmcReadBlock} }, // 18
  { kbT, kbtIsComm, {cmcMarkWord} }, // 20
  { kbU, kbtIsComm, {cmcUnIndentBlkOne} }, // 21
  { kbV, kbtIsComm, {cmcMoveBlock} }, // 22
  { kbW, kbtIsComm, {cmcWriteBlock} }, // 23
  { kbY, kbtIsComm, {cmcCut} }, // 25
  { kb0, kbtIsComm, {cmcPutMark0} }, // 32
  { kb1, kbtIsComm, {cmcPutMark1} }, // 33
  { kb2, kbtIsComm, {cmcPutMark2} }, // 34
  { kb3, kbtIsComm, {cmcPutMark3} }, // 35
  { kb4, kbtIsComm, {cmcPutMark4} }, // 36
  { kb5, kbtIsComm, {cmcPutMark5} }, // 37
  { kb6, kbtIsComm, {cmcPutMark6} }, // 38
  { kb7, kbtIsComm, {cmcPutMark7} }, // 39
  { kb8, kbtIsComm, {cmcPutMark8} }, // 40
  { kb9, kbtIsComm, {cmcPutMark9} }, // 41
  { kbTab, kbtIsComm, {cmcIndentBlk} }, // 43

  // Shift 11
  { kbShA, kbtIsComm, {cmcToggleMoveOnPaste} }, // 1
  { kbShB, kbtIsComm, {cmcSelRectStart} }, // 2
  { kbShC, kbtIsComm, {cmcSelRectCopy} }, // 3
  { kbShH, kbtIsComm, {cmcSelRectHide} }, // 8
  { kbShK, kbtIsComm, {cmcSelRectEnd} }, // 11
  { kbShL, kbtIsComm, {cmcSelRectDel} }, // 12
  { kbShM, kbtIsComm, {cmcSelRectMove} }, // 13
  { kbShP, kbtIsComm, {cmcSelRectPaste} }, // 16
  { kbShT, kbtIsComm, {cmcSelRectCut} }, // 20
  { kbShV, kbtIsComm, {cmcSelRectMove} }, // 22
  { kbShTab, kbtIsComm, {cmcUnIndentBlk} }, // 43

  // Control 25
  { kbCtB, kbtIsComm, {cmcStartSelect} }, // 2
  { kbCtC, kbtIsComm, {cmcCopyBlock} }, // 3
  { kbCtH, kbtIsComm, {cmcHideSelect} }, // 8
  { kbCtI, kbtIsComm, {cmcIndentBlkOne} }, // 9
  { kbCtK, kbtIsComm, {cmcEndSelect} }, // 11
  { kbCtL, kbtIsComm, {cmcMarkLine} }, // 12
  { kbCtM, kbtIsComm, {cmcToUpper} }, // 13
  { kbCtO, kbtIsComm, {cmcToLower} }, // 15
  { kbCtR, kbtIsComm, {cmcReadBlock} }, // 18
  { kbCtT, kbtIsComm, {cmcMarkWord} }, // 20
  { kbCtU, kbtIsComm, {cmcUnIndentBlkOne} }, // 21
  { kbCtV, kbtIsComm, {cmcMoveBlock} }, // 22
  { kbCtW, kbtIsComm, {cmcWriteBlock} }, // 23
  { kbCtY, kbtIsComm, {cmcCut} }, // 25
  { kbCt0, kbtIsComm, {cmcPutMark0} }, // 32
  { kbCt1, kbtIsComm, {cmcPutMark1} }, // 33
  { kbCt2, kbtIsComm, {cmcPutMark2} }, // 34
  { kbCt3, kbtIsComm, {cmcPutMark3} }, // 35
  { kbCt4, kbtIsComm, {cmcPutMark4} }, // 36
  { kbCt5, kbtIsComm, {cmcPutMark5} }, // 37
  { kbCt6, kbtIsComm, {cmcPutMark6} }, // 38
  { kbCt7, kbtIsComm, {cmcPutMark7} }, // 39
  { kbCt8, kbtIsComm, {cmcPutMark8} }, // 40
  { kbCt9, kbtIsComm, {cmcPutMark9} }, // 41
  { kbCtTab, kbtIsComm, {cmcIndentBlk} } // 43
 }
};

dSeqSel(ShHome,cmcLineStart);
dSeqSel(ShUp,cmcLineUp);
dSeqSel(ShPgUp,cmcPageUp);
dSeqSel(ShL,cmcCharLeft);
dSeqSel(ShR,cmcCharRight);
dSeqSel(ShEnd,cmcLineEnd);
dSeqSel(ShDn,cmcLineDown);
dSeqSel(ShPgDn,cmcPageDown);
dSeqSel(ShCtL,cmcWordLeft);
dSeqSel(ShCtR,cmcWordRight);
dSeqSel(ShCtHome,cmcFirstLineInScreen);
dSeqSel(ShCtEnd,cmcLastLineInScreen);
dSeqSel(ShCtPgUp,cmcTextStart);
dSeqSel(ShCtPgDn,cmcTextEnd);


/******************************* INTERFACE routines *******************************/





TKeyTranslate KeyTrans((KeyTTable *)&base);

static void StoreError(const char *error,char *file)
{
 char buf[PATH_MAX+80];
 char *aux=TVIntl::getTextNew(error);
 TVIntl::snprintf(buf,PATH_MAX+80,__("%s, file \"%s\"\n"),aux,file);
 Error=strdup(buf);
 DeleteArray(aux);
}

void ShowKeyBindError(void)
{
 if (Error)
   {
    messageBox(mfError | mfOKButton,__("Error while loading the keyboard binding: %s"),Error);
    Error=0;
   }
}

int LoadKeysForTCEditor(char *name)
{
 loadedFile=newStr(name);
 return KeyTrans.Load(name);
}

void LoadKeysForTCEditorFreeMemory()
{
 DeleteArray(loadedFile);
}

int  SaveKeyBind(char *name)
{
 if (KeyTrans.Save(name))
    return 1;
 if (loadedFile)
   {
    if (strcmp(name,loadedFile)!=0)
       ShowSavePoint(name);
    DeleteArray(loadedFile);
    loadedFile=0;
   }
 return 0;
}

int KeyBackToDefault(Boolean ask)
{
 // If the user is useing defaults let as-is
 if (KeyTrans.getType()==kbtStatic)
    return 0;

 if (ask)
    if (messageBox(__("You'll lose all the changes"),mfOKCancel)==cmCancel)
       return 0;

 KeyTrans.ChangeTable((KeyTTable *)&base);
 return 1;
}

#ifdef TEST
#include <stdio.h>

int main(void)
{
 char b[100];
/* unsigned i,c;

 do
   {
    while (!TGKey::kbhit());
    TGKey::gkey();
    KeyTrans.get(TGKey::Abstract,TGKey::ascii);
   }
 while (TGKey::Abstract!=kbEsc);
 KeyTrans.expand();

 c=KeyTrans.getCount();
 printf("Size: %u (%d)\n",KeyTrans.getLen(),c);
 for (i=0; i<c; i++)
     KeyTrans.getText(b,i,99);
 printf("Deleting key 36\n");
 KeyTrans.deleteKey(36);
 c=KeyTrans.getCount();
 printf("Size: %u (%d)\n",KeyTrans.getLen(),c);
 for (i=0; i<c; i++)
     KeyTrans.getText(b,i,99);

 KeyTrans.compact();
 printf("Size: %u\n",KeyTrans.getLen());
 do
   {
    while (!TGKey::kbhit());
    TGKey::gkey();
    KeyTrans.get(TGKey::Abstract,TGKey::ascii);
   }
 while (TGKey::Abstract!=kbEsc);
 */
 printf("\n");
 // A test sequence
 TKeySeqCol tkc(4,5);
 /*tkc.insert(kbCtQ); // 0
 tkc.insert(kbI);  // 1*/
 tkc.insert(kbCtK); // 0
 tkc.insert(kbA);  // 1
 tkc.insert(kbShT);  // 1
 tkc.insert(kbT);  // 1

 //tkc.getText(b,0,99);
 //printf(b);
 //printf("\n");

 // A commands sequence
 TComSeqCol tcc(3,5);
 tcc.insert(cmcSelectOn);
 tcc.insert(cmcLineUp);
 tcc.insert(cmcSelectOff);
 tcc.getText(b,0,99);
 printf(b);
 printf("\n");

 KeyTrans.expand();
 printf("Agrega retorna: %d",KeyTrans.addKey(&tkc,&tcc,kbtIsSeq));
 printf("\n");
 KeyTrans.compact();

 // Now see if it's there
 KeyTrans.get(kbCtK,0);
 KeyTrans.get(kbA,0);
 KeyTrans.get(kbShT,0);
 KeyTrans.get(kbT,0);

 do
   {
    while (!TGKey::kbhit());
    TGKey::gkey();
    KeyTrans.get(TGKey::Abstract,TGKey::ascii);
   }
 while (TGKey::Abstract!=kbEsc);

 KeyTrans.Save("l:/pepe.dat");

 return 0;
}
#endif
