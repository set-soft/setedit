/* Copyright (C) 1996-2004 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <ceditint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define Uses_string

#define Uses_TStringCollection
#define Uses_TApplication
#define Uses_TDeskTop

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSRadioButtons
#define Uses_TSTextScroller
#define Uses_TSLabel
#define Uses_TSHzGroup

// First include creates the dependencies
#include <easydia1.h>
#include <tv.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppVarious
#define Uses_SETAppDialogs
#define Uses_SETAppConst
#include <setapp.h>
#include <edmsg.h>
#include <edspecs.h>

// 0 => no tips, 1 => dialog, 2 => message window
static uint32 TipStyle=1;

void getLineNoLF(char *b, int max, FILE *f)
{
 b[0]=0;
 fgets(b,max,f);
 int l=strlen(b);
 if (b[l-1]=='\n')
   {
    b[l-1]=0;
    if (b[l-2]=='\r')
       b[l-2]=0;
   }
}

typedef enum { findStart, findEnd, collectLinks, collectTip, endAll } stateTip;

const char *strStart="~~~Start";
const char *strEnd  ="~~~End";

#define getLine() getLineNoLF(lineBuf,80,f)

void SearchTip(void (*ActionLine)(char *), void (*ActionLink)(char *),
               char *fileName, int &tipNumber)
{
 char lineBuf[80];
 stateTip state=findStart;
 int tipCounter=0,buscar=1;

 FILE *f=fopen(fileName,"rt");
 if (!f)
    return;
 else
   {
    while (buscar)
      {
       getLine();
       while (!feof(f) && state!=endAll)
         {
          switch (state)
            {
             case findStart:
                  if (strcmp(lineBuf,strStart)==0)
                    {
                     tipCounter++;
                     if (tipCounter==tipNumber)
                        state=collectTip;
                     else
                        state=findEnd;
                    }
                  break;
             case findEnd:
                  if (strcmp(lineBuf,strEnd)==0)
                     state=findStart;
                  break;
             case collectLinks:
                  if (strcmp(lineBuf,strStart)==0)
                    {
                     state=endAll;
                     buscar=0;
                    }
                  else
                    if (lineBuf[0]!=0) // Skip empty lines
                       ActionLink(lineBuf);
                  break;
             case collectTip:
                  if (strcmp(lineBuf,strEnd)==0)
                     state=collectLinks;
                  else
                     ActionLine(lineBuf);
                  break;
             case endAll:
                  break;
            }
          getLine();
         }
       if (state==findStart && buscar)
         {
          tipNumber=1;
          tipCounter=0;
          state=findStart;
          rewind(f);
         }
       else
         buscar=0;
      }
   }
 fclose(f);
}

/****************************** Dialog Stuff **********************************/
static TNSCollection *TipText,*TipLinks;
static int maxWLink;

void forDialogLine(char *text)
{
 TipText->insert(newStr(text));
}

void forDialogLink(char *text)
{
 TipLinks->insert(newStr(text));
 int l=strlen(text);
 if (l>maxWLink)
    maxWLink=l;
}

const int cmBaseLinks=0x2260,cmNextTip=0x2280;

class TDialogTips : public TDialog
{
public:
 TDialogTips(const TRect& bounds, const char *aTitle) :
   TWindowInit(&TDialogTips::initFrame),
   TDialog(bounds,aTitle) {}
 virtual void handleEvent(TEvent& event);
};

void TDialogTips::handleEvent(TEvent& event)
{
 TDialog::handleEvent(event);
 if (event.what==evCommand && event.message.command>=0x2260 &&
     event.message.command<=cmNextTip)
   {
    endModal(event.message.command);
    clearEvent(event);
   }
}


static void ShowDialogTip(char *fileName, int &LastTip)
{
 int showNext=1;

 while (showNext)
   {
    TipText=new TNSCollection(10,5);
    TipLinks=new TNSCollection(4,2);
    if (!TipText || !TipLinks)
       return;
   
    maxWLink=0;
    SearchTip(forDialogLine,forDialogLink,fileName,LastTip);
   
    int lines=TipText->getCount();
    if (lines)
      {
       TSViewCol *col=new TSViewCol(new TDialogTips(TRect(1,1,1,1),__("Tip of the day")));
   
       TSTextScroller *text=new TSTextScroller(40,10,TipText,0,lines>10,40);
       char aux[40];
       TVIntl::snprintf(aux,40,__("~T~ip %d"),LastTip);
       TSLabel *Text=new TSLabel(aux,text);
       col->insert(2,1,Text);

       int tips=TipLinks->getCount(),i;
       if (tips)
         {
          TSButton *btA=0,*bt;
          maxWLink+=3;
   
          for (i=0; i<tips; i++)
             {
              bt=new TSButton((char *)TipLinks->at(i),cmBaseLinks+i,bfNormal,maxWLink);
              if (btA)
                 col->insert(xTSRightOf,yTSUnder,bt,Text,btA);
              else
                 col->insert(xTSRightOf,2,bt,Text);
              btA=bt;
             }
         }

       TSRadioButtons *origin = new TSRadioButtons(
                                new TSItem(__("Don't ~s~how it again"),
                                new TSItem(__("Show it once a ~d~ay"),
                                new TSItem(__("Show it in the ~m~essage window"),0))));
       TSLabel *Options=new TSLabel(__("Options"),origin);
       col->insert(2,yTSUnder,Options,0,Text);

       TSHzGroup *buttons=new TSHzGroup(new TSButton(__("O~K~"),cmOK,bfDefault),
                                        new TSButton(__("~N~ext tip"),cmNextTip),4);
       col->insert(xTSCenter,yTSDown,buttons);
   
       TDialog *d=col->doIt();
       d->helpCtx=cmeTipOfTheDay;
       delete col;
      
       d->options|=ofCentered;

       int ret=execDialog(d,&TipStyle);

       if (ret!=cmCancel)
         {
          if (ret>=cmBaseLinks && ret<cmNextTip)
            {
             ShowHelpTopic("setedit",(char *)TipLinks->at(ret-cmBaseLinks));
            }
         }
       showNext=(ret==cmNextTip);
      }
    else
       showNext=0;

    CLY_destroy(TipText);
    CLY_destroy(TipLinks);

    LastTip++;
   }
}
/****************************** End of Dialog Stuff **********************************/

void forMessageLine(char *text)
{
 EdShowMessage(text);
}

void forMessageLink(char *)
{
}

static void ShowMessageTip(char *fileName, int &LastTip)
{
 EdShowMessageI(__("Tip of the day"),True);
 SearchTip(forMessageLine,forMessageLink,fileName,LastTip);
 EdJumpToMessage(0);
 LastTip++;
}

// Format: style,tip,time
// style is 0,1 or 2
// tip is the next tip to show (order in the list)
// time of the last use
//
const char *envTipStyle="SET_TIP_INFO";

void ShowTips(char *fileName, int forceDialog)
{
 int LastTip,oldTipStyle=-1;
 char aux[60],day,today;
 time_t LastUse,Today;
 struct tm *t;

 char *s=(char *)GetVariable(envTipStyle),*end;
 if (s)
   {
    // No tip, dialog or message
    TipStyle=strtol(s,&end,0);
    if (TipStyle>2)
       TipStyle=1;
    else
       oldTipStyle=TipStyle;

    if (!TipStyle && !forceDialog)
       return;

    // Tip number
    s=end+1;
    LastTip=strtol(s,&end,0);
    if (LastTip<=0)
       LastTip=1;

    // Time of the last show
    s=end+1;
    LastUse=(time_t)strtol(s,&end,0);

    // Check a difference of 1 day
    t=localtime(&LastUse);
    day=t->tm_mday;
    Today=time(NULL);
    t=localtime(&Today);
    today=t->tm_mday;
    if (day==today && !forceDialog) // fails if one month
       return;
   }
 else
   {
    TipStyle=1;
    LastTip=1;
   }


 if (forceDialog)
    ShowDialogTip(fileName,LastTip);
 else
    switch (TipStyle)
      {
       case 2:
            ShowMessageTip(fileName,LastTip);
            break;
       case 1:
            ShowDialogTip(fileName,LastTip);
            break;
      }

 // Store the change if the user asked for it
 Today=time(NULL);
 sprintf(aux,"%d,%d,%ld",TipStyle,LastTip,(long)Today);
 InsertEnvironmentVar((char *)envTipStyle,aux);
}


