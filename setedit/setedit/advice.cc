/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <stdlib.h>
#include <stdio.h>

// EasyDiag requests
#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSCheckBoxes
#define Uses_TSHzGroup

// First include creates the dependencies
#include <easydia1.h>
#include <tv.h>
// Second request the headers
#include <easydiag.h>

#define Uses_SETAppDialogs
#include <setapp.h>
#include <edspecs.h>

typedef struct
{
 const char *label;
 const char *variable;
 unsigned mask;
 unsigned options;
} advice;

static const char *AdviceVar1="SET_TIPS1";

const unsigned opsSimple=0, ops40cols=1, ops60cols=2, opsTypeMask=7;
const unsigned opsButOK=0, opsButYesNo=8, opsButNoYes=0x10, opsButMask=0x38;

static
advice Advices[]=
{
 { __("\x3You are about switching to the user screen\n\x3Press ENTER to go back"),
   AdviceVar1, 0x00000001, opsSimple },
 { __("Avoid using files that aren't in a directory that isn't relative to the working one. For example: in another drive. If you do it you'll have problems if the directories are moved or transported to other machine."),
   AdviceVar1, 0x00000002, ops60cols },
 { __("\x3""Do you really want to delete\n\x3this file from disk?"),
   AdviceVar1, 0x00000004, opsSimple | opsButYesNo },
 { __("With this settings SHIFT key will affect the case of the letters during incremental searchs, CAPS-LOCK too. Don't forget it."),
   AdviceVar1, 0x00000008, ops40cols },
 { __("With this settings incremental searchs aren't very intuitive, specially when pressing SHIFT. Don't forget it."),
   AdviceVar1, 0x00000010, ops40cols },
 { __("Holding SHIFT the incremental search is done for the directory names, otherwise applies to files. Don't forget it."),
   AdviceVar1, 0x00000020, ops40cols },
 { __("When using real tabs the following options are usually desired: Autoindent ON, Intelligent C indent OFF, Optimal Fill ON, Don't move inside tabs ON, Tab smart indents OFF, Use indent size OFF and Backspace unindents OFF.\nI see not all are selected this way."),
   AdviceVar1, 0x00000040, ops60cols },
 { __("Tags file in incorrect format,\ndo you want to read more about it?"),
   AdviceVar1, 0x00000080, opsSimple | opsButYesNo },
 { __("This option needs a tag file,\ndo you want to read more about it?"),
   AdviceVar1, 0x00000100, opsSimple | opsButYesNo }
};

const int cantAdvices=sizeof(Advices)/sizeof(advice);

int GiveAdvice(int number)
{
 // Sanity check
 if (number<0 || number>=cantAdvices)
    return 0;
 // Now assign with confidence
 advice *ad=Advices+number;

 // Check if the user doesn't want it
 unsigned ops=EnvirGetIntVar(ad->variable);
 if (ops & ad->mask)
   {
    switch (ad->options & opsButMask)
      {
       case opsButYesNo:
            return cmYes;
       case opsButNoYes:
            return cmNo;
      }
    return 0;
   }

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Advice")));

 TSStaticText *text;
 switch (ad->options & opsTypeMask)
   {
    case ops40cols:
         text=new TSStaticText(ad->label,40);
         break;
    case ops60cols:
         text=new TSStaticText(ad->label,60);
         break;
    default:
         text=new TSStaticText(ad->label);
   }
 col->insert(xTSCenter,2,text);
 TSCheckBoxes *again=new TSCheckBoxes(new TSItem(__("Don't show it next time"),0));
 again->Flags=wSpan;
 col->insert(2,yTSUnder,again,0,text);

 switch (ad->options & opsButMask)
   {
    case opsButYesNo:
         col->insert(xTSCenter,yTSUnder,
                     new TSHzGroup(new TSButton(__("~Y~es"),cmYes,bfDefault),
                                   new TSButton(__("~N~o"),cmNo)),
                     0,again);
         break;
    case opsButNoYes:
         col->insert(xTSCenter,yTSUnder,
                     new TSHzGroup(new TSButton(__("~N~o"),cmNo,bfDefault),
                                   new TSButton(__("~Y~es"),cmYes)),
                     0,again);
         break;
    default:
         col->insert(xTSCenter,yTSUnder,new TSButton(__("O~K~"),cmOK,bfDefault,10),
                     0,again);
   }


 TDialog *d=col->doIt();
 delete col;
 d->options|=ofCentered;
 
 uint32 op=0;
 int ret=execDialog(d,&op);
 if (op)
    EnvirSetIntVar(ad->variable,ops | ad->mask);

 return ret;
}

