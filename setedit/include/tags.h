/* Copyright (C) 2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
   
typedef unsigned char uchar;

// Flags for a tag
const uchar
      sttFgLine=1,  // != regex
      sttFgLocal=2, // != global

      sttFgClass   =0x04, // 3 bits
      sttFgEnum    =0x08,
      sttFgUnion   =0x0C,
      sttFgInherits=0x10,
      sttFgStruct  =0x14,
      sttFgPMask   =0x1C,

      sttFgVirtual    =0x20,
      sttFgPureVirtual=0x40,
      sttFgAbstract   =0x60;

struct stTagFile;
class  TTagInfo;

// A tag
struct stTag
{
 const char *id;
 const char *source;
 union
 {
  const char *regex;
  unsigned line;
 };
 const char *partof;
 stTagFile  *tagFile;
 uchar lang;
 uchar kind;
 uchar flags;
};

// To hold one of the description variables found at the beggining of
// the tag files.
struct stTagInfo
{
 const char *var;
 const char *value;
 const char *comment;
};

// Information about one tag files
struct stTagFile
{
 const char *file;
 const char *base;
 time_t      modtime;
 int         entries;
 TTagInfo   *info;
};

// Structures to convert the single letter "kind" field to a descriptive text
struct stTagKind
{
 uchar kind;
 const char *name;
};

struct stTagKinds
{
 unsigned count;
 stTagKind *kinds;
};

// Class to handle the variables for the tag file
class TTagInfo : public TStringCollection, public TStringable
{
public:
 TTagInfo() : TStringCollection(6,2) {};
 int addValue(char *s);
 virtual void freeItem(void *item);
 virtual void *keyOf(void *item);
 void print() { forEach(print1,NULL); };
 static void print1(void *item, void *arg);
 // TStringable things:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };
};

// Class for the tag files
class TTagFiles : public TStringCollection, public TStringable
{
public:
 TTagFiles();
 virtual void freeItem(void *item);
 virtual void *keyOf(void *item);
 void print() { forEach(print1,NULL); };
 static void print1(void *item, void *arg);
 stTagFile *atPos(ccIndex pos) { return (stTagFile *)at(pos); };
 // TStringable things:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };

 //SetDefStreamMembers(TTagFiles,TStringCollection);
};

//SetDefStreamOperators(TTagFiles);

// Class to hold the tags
class TTagCollection : public TStringCollection, public TStringable
{
public:
 TTagCollection();
 ~TTagCollection();
 int addFile(const char *file, int defer=0);
 int addValue(char *s, stTagFile *tf);
 virtual void  freeItem(void *item);
 virtual void *keyOf(void *item);
 virtual int   compare(void *key1, void *key2);

 // TStringable things:
 virtual void getText(char *dest, unsigned item, int maxLen);
 virtual unsigned GetCount(void) { return getCount(); };

 stTag *atPos(ccIndex pos) { return (stTag *)at(pos); };
 void refresh();
 void print() { forEach(print1,NULL); };
 static void print1(void *item, void *arg);
 static const char *getKind(stTag *p);
 static const char *getLanguage(stTag *p);
 static void        getText(char *buf, void *item, int maxLen);
 TStringCollection *getTagFilesList();
 void               deleteTagsFor(stTagFile *p);
 int                save(fpstream& s);
 int                load(fpstream& s);

 TTagFiles         *tagFiles;

protected:
 TStringCollection *files;
 static const char *Languages[];
 static stTagKinds  Kinds[];

 int loadTagsFromFile(stTagFile *p);
};

int TagsSave(fpstream& s);
int TagsLoad(fpstream& s);

