/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */

struct stSpLine
{
 int oline, nline;
 int id;
};

class TNSSortedCollection;

class TSpCollection : public TNSSortedCollection
{
public:
 TSpCollection(ccIndex aLimit) :
  TNSSortedCollection(aLimit,8) { duplicates=True; };
 void insert(int line, int id);
 TSpCollection &operator=(const TSpCollection &pl);
 stSpLine *At(ccIndex i) { return (stSpLine *)at(i); };

 virtual void *keyOf(void *item);
 virtual int compare(void *s1, void *s2);
 virtual void freeItem(void *s);
};

// The following function must be provided by the main part of the editor.
// Your objetive is transfer the array spLines to the editor associated to fileName
extern void ApplySpLines(char *fileName, TSpCollection *spLines);

typedef void (*spLineApplyF)(const char *file, stSpLine *spline, void *data);

void SpLinesAdd(char *fileName, int line, int idSource, Boolean TransferNow=True);
void SpLinesUpdate(void);
TSpCollection *SpLinesGetFor(char *fName);
void SpLinesDeleteForId(int id, const char *file=NULL, Boolean aLine=False, int oLine=0);
int  SpLineGetNewValueOf(int line, char *fileName, Boolean *found=NULL);
int  SpLineGetOldValueOf(int line, char *fName, int type, Boolean *found);
void SpLinesCleanUp();
void SpLinesForEach(int id, spLineApplyF apply, void *data=NULL);

const int idsplAny=0,
          idsplError=1,
          idsplBreak=2,
          idsplRunLine=3;
