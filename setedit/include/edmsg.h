/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */

class TRect;

typedef struct
{
 int Line;
 int Column;
 int offset,len;
} FileInfo;

#ifdef STANDALONE
// This file is used by the standalone editor to declare more things
class TSOSListBox;
class TSOSListBoxMsg;

#ifdef Uses_TSOSListBoxMsg

const unsigned lbmNothing=0, lbmReachedMsg=1, lbmWrap=2;

class TSOSListBoxMsg : public TSOSListBox
{
public:
 TSOSListBoxMsg(const TRect& bounds, ushort aNumCols, TScrollBar *aScrollBar)
 : TSOSListBox(bounds,aNumCols,aScrollBar) { }

 virtual void focusItem(ccIndex item);
 virtual void selectItem(ccIndex item);
 virtual void handleEvent(TEvent& event);
 virtual void setState(uint16 aState, Boolean enable);
 void selectNext(int offset=1);
 void selectPrev(int offset=1);
 int  getLineOf(int pos);
 void updateCommands(int enable);
 void saveAs();
 void save(char *name);
 void copyClipboard(Boolean osClipboard);

 int  selectOK;
 int  haveJumpLines;
 static unsigned opsEnd;
 static unsigned opsBeep;
};
#endif

class TEdMsgDialog : public TDialog
{
public:
  TEdMsgDialog(const TRect &r,const char *t);
  virtual void changeBounds(const TRect &);
  virtual void close(void);
  virtual void handleEvent(TEvent& event);
  TSOSListBoxMsg *MsgList;
  ~TEdMsgDialog();
};

extern TEdMsgDialog *EdMessageWindowInit(int Insert=1);
extern int  EdMessageCantMessages(void);
extern void EdMessageSelectNext(void);
extern void EdMessageSelectPrev(void);
extern void EdJumpToFirstError(void);

#endif

int  DumpFileToMessage(char *file, const char *from, uint32 SMOps,
                       char *(*ParseFun)(char *buf,FileInfo &fI,char *&fileName)=0,
                       int kill=1);
// Incremental version of the DumpFileToMessage
void DumpFileToMessageInit(char *file, const char *from, uint32 SMOps,
                           char *(*ParseFun)(char *buf,FileInfo &fI,char *&fileName)=0);
int  DumpFileToMessageParseMore(int Lines, int *goBack);
void DumpFileToMessageEnd();

// This is provided by edmsg.cc in the editor or by rhideint.cc in libset
const uint32 edsmUpdateSpLines=1,edsmRemoveOld=2,edsmDontSelect=4,
             edsmDontUpdate=8,edsmNoHzReset=16;
// Mutually exclusive options for the scroll behavior
const uint32 edsmScrollMask=0xC0000000,edsmEverScroll=0,edsmNeverScroll=0x40000000,
             edsmScrollIfNoFocus=0x80000000,edsmScrollShifter=0x40000000;
extern void EdShowMessage(const char *msg,Boolean remove_old=False,Boolean resetHz=True);
extern void EdShowMessageI(const char *msg,Boolean remove_old=False,Boolean resetHz=True);
extern void EdShowMessage(const char *msg, unsigned Options);
extern void EdShowMessageI(const char *msg, unsigned Options);
extern void EdShowMessageFile(const char *msg, FileInfo &fInfo, char *fileName,
                              unsigned Options=0);
extern void EdShowMessageUpdate(unsigned Options);
extern void EdJumpToMessage(ccIndex item);
extern Boolean EdMessageGetSize(TRect &r);

