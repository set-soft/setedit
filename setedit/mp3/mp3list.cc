/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>

#ifdef SUP_MP3
/*****************************************************************************

Falta agregar opciones al men£ de iniciar y parar de tocar una lista =>
va pidiendo archivos de aca previo chequear que haya alguna. Ser¡a bueno
habilitar y deshabilitar los comandos del men£.

*****************************************************************************/

#include <stdio.h>

#define Uses_TSListBox
#define Uses_TStringCollection
#define Uses_TSLabel
#define Uses_TEvent
#define Uses_TApplication
#define Uses_TDialog
#define Uses_TSHzGroup
#define Uses_TSButton
#define Uses_fpstream
#define Uses_MsgBox
#define Uses_TStreamable
#define Uses_TStreamableClass
#define Uses_FileOpenAid
#define Uses_TNoSortedStringCollection
#define Uses_TFileDialog
#define Uses_fcntl
#define Uses_limits
#define Uses_string
#define Uses_unistd

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

#define SUP_MP3
#define Uses_PrivateMP3Info
#include <intermp3.h>

// for execDialog
#define Uses_SETAppDialogs
#include <setapp.h>

#include <fileopen.h>

static int LastServed=0;
const char *mp3ListExt="*.[lm][s3][tu]";
const char M3USignature[]="#EXTM3U";

static TNoSortedStringCollection *List=0;

class TListDiag : public TDialog
{
public:
 TListDiag(const TRect& bounds, const char *aTitle) :
   TWindowInit(&TListDiag::initFrame),
   TDialog(bounds,aTitle) {};
 virtual void handleEvent(TEvent &event);
 void SaveList(char *s);
 void LoadList(char *s);
 void ImportM3U(const char *name);

 TListBox *listBox;
 int style;
};

static
void EnableComms(void)
{
 TView::enableCommand(cmaDeleteMP3);
 TView::enableCommand(cmaInsertMP3);
}

static
void DisableComms(void)
{
 TView::disableCommand(cmaDeleteMP3);
 TView::disableCommand(cmaInsertMP3);
}


const char *MP3ext="*.mp[23]";

void TListDiag::handleEvent(TEvent &event)
{
 char fileName[PATH_MAX];

 TDialog::handleEvent( event );
 if (event.what==evCommand)
   {
    switch (event.message.command)
      {
       case cmaInsertMP3:
       case cmaAddMP3:
            strcpy(fileName,MP3ext);
            style=(event.message.command==cmaAddMP3);
            GenericFileDialog(__("Select a file"),fileName,(char *)MP3ext,
                              hID_OpenMP3,fdMultipleSel | fdSelectButton);
            if (List->getCount())
               EnableComms();
            break;
       case cmaDeleteMP3:
            // Note: we can't trust in the disabled command, this task is performed
            // during idle and if you delete the entries quickly you can get here.
            if (listBox->focused<List->getCount())
              {
               List->atRemove(listBox->focused);
               listBox->setRange(List->getCount());
               listBox->drawView();
               if (!List->getCount())
                  DisableComms();
              }
            break;
       case cmaSaveMP3List:
            strcpy(fileName,mp3ListExt);
            if (GenericFileDialog(__("Save file list"),fileName,0,hID_SaveMP3,
                fdDialogForSave)!=cmCancel)
               SaveList(fileName);
            break;
       case cmaLoadMP3List:
            strcpy(fileName,mp3ListExt);
            if (GenericFileDialog(__("Load list of files"),fileName,0,hID_OpenMP3)
                !=cmCancel)
              {
               LoadList(fileName);
               if (List->getCount())
                  EnableComms();
              }
            break;
       default:
            return;
      }
   }
 else
   if (event.what==evBroadcast)
     {
      switch (event.message.command)
        {
         case cmFileDialogFileSelected:
              if (style)
                 List->insert(newStr((char *)event.message.infoPtr));
              else
                 List->atInsert(listBox->focused,
                                newStr((char *)event.message.infoPtr));
              listBox->setRange(List->getCount());
              listBox->drawView();
              break;
         default:
              return;
        }
     }
   else
     return;
 clearEvent(event);
}

static char *Signature="SET's Files list\x1A";
const int Version=0x100;

void TListDiag::SaveList(char *name)
{
 fpstream *f=new fpstream(name,CLY_IOSOut|CLY_IOSBin);

 if (f)
   {
    fpstream &s=*f;

    // Analize the extension
    char *ext=strrchr(name,'.');
    if (ext && strcasecmp(ext,".m3u")==0)
      {// Export as M3U file
       f->close();
       delete f;
       FILE *f=fopen(name,"wt");
       if (f)
         {
          fprintf(f,"%s\n",M3USignature);
          int i,c=List->getCount();
          for (i=0; i<c; i++)
              fprintf(f,"%s\n",(char *)(List->at(i)));
         }
       return;
      }
    s.writeString(Signature);
    s << Version << List;

    if (!f)
      {
       messageBox(__("Could not create the list file."),mfOKButton | mfError);
       f->close();
       ::remove(name);
      }
    else
       f->close();
   }
 delete f;
}

void TListDiag::ImportM3U(const char *name)
{
 TNoSortedStringCollection *newList=new TNoSortedStringCollection(10,5);

 FILE *f=fopen(name,"r");
 if (!f) return;

 char buffer[PATH_MAX+4],*s;
 while (!feof(f))
   {
    if (fgets(buffer,PATH_MAX+2,f))
      {
       if (buffer[0]=='#') continue;
       buffer[PATH_MAX]=0;
       for (s=buffer; *s && *s!='\n' && *s!='\r'; s++);
       *s=0;
       newList->insert(newStr(buffer));
      }
   }
 if (newList->getCount())
   {
    listBox->newList(newList);
    List=newList;
   }
 fclose(f);
}

void TListDiag::LoadList(char *name)
{
 char buffer[80];

 #ifdef BROKEN_CPP_OPEN_STREAM
 // In this way we avoid the destruction of the file
 int h=open(name, O_RDONLY | O_BINARY);
 fpstream *f=new fpstream(h);
 #else
 fpstream *f=new fpstream(name,CLY_IOSIn | CLY_IOSBin);
 #endif
 
 if (!f)
    messageBox(__("Could not open the list file"),mfOKButton | mfError);
 else
   {
    // Check for .m3u
    f->readBytes(buffer,sizeof(M3USignature));
    f->seekg(0);
    if (strncmp(buffer,M3USignature,sizeof(M3USignature)-1)==0)
      {
       ImportM3U(name);
       f->close();
      }
    else
      {
       fpstream &s=*f;
       int fileVersion;
   
       s.readString(buffer,80);
       if (strcmp(buffer,Signature)!=0)
         {
          messageBox(__("Wrong file type."), mfOKButton | mfError);
          return;
         }
       s >> fileVersion;
       TNoSortedStringCollection *newList;
       s >> newList;
   
       if (!f)
          messageBox(__("Error reading the file list"),mfOKButton | mfError);
       else
         {
          if (newList)
            {
             listBox->newList(newList);
             List=newList;
            }
         }
       f->close();
      }
   }
 delete f;
}

void MP3ListSaveState(opstream &os)
{
 if (List)
    os << 1 << List << LastServed;
 else
    os << 0;
}

void MP3ListLoadState(ipstream &is)
{
 int aux;

 is >> aux;
 if (aux)
   {
    is >> List >> LastServed;
    if (List)
       TView::enableCommand(cmeMP3PlayList);
   }
 else
   {
    List=0;
    LastServed=0;
   }
}

struct
{
 TCollection *col;
 unsigned focused;
} dataRect;

void MP3EditPlayList()
{
 if (!List)
    List=new TNoSortedStringCollection(5,3);
 if (List->getCount())
    EnableComms();
 else
    DisableComms();

 TListDiag *ListDia=new TListDiag(TRect(1,1,1,1),__("Play List"));
 TSViewCol *col=new TSViewCol(ListDia);

 TSListBox *listB=new TSListBox(60,12,tsslbVertical | tsslbHorizontal,1,PATH_MAX);
 TSLabel   *ListB=new TSLabel(__("List of files"),listB);
 ListDia->listBox=(TListBox *)listB->view;

 TSHzGroup *buttons=new TSHzGroup(new TSButton(__("~A~dd"),cmaAddMP3),
                    new TSHzGroup(new TSButton(__("~I~nsert"),cmaInsertMP3),
                    new TSHzGroup(new TSButton(__("~D~elete"),cmaDeleteMP3),
                    new TSHzGroup(new TSButton(__("~S~ave"),cmaSaveMP3List),
                    new TSHzGroup(new TSButton(__("~L~oad"),cmaLoadMP3List),
                                  new TSButton(__("~O~k"),cmOK,bfDefault))))));

 col->insert(2,1,ListB);
 col->insert(xTSCenter,yTSDown,buttons);
 col->doIt();
 delete col;

 dataRect.col=List;
 dataRect.focused=0;

 ListDia->options|=ofCentered;
 execDialog(ListDia,&dataRect);

 if (List->getCount()>0)
    TView::enableCommand(cmeMP3PlayList);
 else
   {
    TView::disableCommand(cmeMP3PlayList);
    TView::disableCommand(cmeMP3StopList);
   }
}

char *MP3ListGetNext()
{
 if (LastServed<List->getCount())
    return (char *)List->at(LastServed++);
 return 0;
}

void MP3ListResetCount()
{
 LastServed=0;
}

int MP3ListHavePrev()
{
 return LastServed>1;
}

int MP3ListHaveNext()
{
 return LastServed<List->getCount();
}

void MP3ListGoBack()
{
 if (MP3ListHavePrev())
    LastServed-=2;
}
#else

#define Uses_TNoSortedStringCollection
#include <settvuti.h>

void MP3ListLoadState(ipstream &is)
{
 int aux;
 int LastServed;
 TNoSortedStringCollection *List;

 is >> aux;
 if (aux)
   {
    is >> List >> LastServed;
    if (List)
       delete List;
   }
 else
   {
    List=0;
    LastServed=0;
   }
}

#endif
