/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#include <configed.h>

#include <stdio.h>

#define Uses_TDialog
#define Uses_TScrollBar
#define Uses_TRadioButtons
#define Uses_TRect
#define Uses_TSItem
#define Uses_TLabel
#define Uses_TEvent
#define Uses_TButton
#define Uses_TKeys
#define Uses_TKeys_Extended
#define Uses_TStaticText
#define Uses_TColorSelector
#define Uses_MsgBox
#define Uses_TScreen
#include <tv.h>

#include <tpaltext.h>
#define Uses_SETAppVarious
#define Uses_SETAppDialogs
#define Uses_SETAppConst
#include <setapp.h>

const int MaxCompVal=255;

static char *patterNumShow="123456789012";

class TNumShow : public TStaticText
{
public:
 TNumShow(const TRect &bounds);
 void setValue(int val);
};

TNumShow::TNumShow(const TRect &bounds) :
   TStaticText(bounds,patterNumShow)
{
 setValue(0);
 noIntl=1;
}

void TNumShow::setValue(int val)
{
 sprintf((char *)(text+1),"%d",val);
 *((char *)text)=3;
 draw();
}


class TRadioCol : public TRadioButtons
{
public:
 TRadioCol(const TRect &bounds, TSItem *items, TScrollBar *r, TScrollBar *g,
           TScrollBar *b, TNumShow *rr, TNumShow *rg, TNumShow *rb) :
   TRadioButtons(bounds,items),
   red(r), green(g), blue(b), nred(rr), ngreen(rg), nblue(rb) { press(0); }
 virtual void press(int item);
 virtual void movedTo(int item);
 unsigned getValue(void) { return value; }

protected:
 TScrollBar *red,*green,*blue;
 TNumShow *nred,*ngreen,*nblue;
 void SetBars(void);
};

void TRadioCol::SetBars(void)
{
 int r,g,b;
 EditorPalette->GetOne(value,r,g,b);
 red->setParams(r,0,MaxCompVal,32,1);
 green->setParams(g,0,MaxCompVal,32,1);
 blue->setParams(b,0,MaxCompVal,32,1);
 nred->setValue(r);
 ngreen->setValue(g);
 nblue->setValue(b);
}

void TRadioCol::press(int item)
{
 value=item;
 SetBars();
}

void TRadioCol::movedTo(int item)
{
 press(item);
}

class TDiaPal : public TDialog
{
public:
 TDiaPal(void);
 virtual void handleEvent(TEvent& event);
 virtual void draw(void);
 TRadioCol *colorsel;
 TScrollBar *red;
 TScrollBar *green;
 TScrollBar *blue;
 TNumShow *rn,*gn,*bn;
};

TDiaPal::TDiaPal(void) :
         TWindowInit(&TDiaPal::initFrame),
         TDialog(TRect(0,0,52,19),__("Palette editor"))
{
 options|=ofCentered;
 helpCtx=cmeEditPalette;
 TLabel *rl,*gl,*bl;

 TRect r(14,1,50,2);
 insert(new TStaticText(r,__("Color RGB composition")));
 r.move(0,3);
 red=new TScrollBar(r);
 r.move(0,1);
 rn=new TNumShow(r);
 r.move(0,-2);
 rl=new TLabel(r,__("Red"),red);
 r.move(0,5);
 green=new TScrollBar(r);
 r.move(0,1);
 gn=new TNumShow(r);
 r.move(0,-2);
 gl=new TLabel(r,__("Green"),green);
 r.move(0,5);
 blue=new TScrollBar(r);
 r.move(0,1);
 bn=new TNumShow(r);
 r.move(0,-2);
 bl=new TLabel(r,__("Blue"),blue);

 colorsel=new TRadioCol(TRect(2,2,7,18),
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("",
              new TSItem("", 0 )))))))))))))))),red,green,blue,rn,gn,bn);
 insert(colorsel);
 insert(new TLabel(TRect(1,1,10,2),__("Color"),colorsel));
 insert(rl);
 insert(red);
 insert(rn);
 insert(gl);
 insert(green);
 insert(gn);
 insert(bl);
 insert(blue);
 insert(bn);
 insert(new TButton(TRect(15,16,27,18),__("~O~K"),cmOK,bfDefault));
 insert(new TButton(TRect(34,16,46,18),__("~D~efault"),cmYes,bfNormal));
 selectNext(False);
}

void TDiaPal::draw(void)
{
 TDialog::draw();
 ushort buf[5];
 ushort col;
 unsigned i,j;
 for (i=0; i<16; i++)
    {
     col=(i<<8) | (uchar)TColorSelector::icon;
     for (j=0; j<5; j++)
         buf[j]=col;
     writeBuf(7,i+2,5,1,buf);
    }
}

void TDiaPal::handleEvent(TEvent& event)
{
 if (event.what==evBroadcast && event.message.command==cmScrollBarChanged)
   {
    int r,g,b;
    TScrollBar *p=(TScrollBar *)event.message.infoPtr;
    EditorPalette->GetOne(colorsel->getValue(),r,g,b);
    if (p==red)
      {
       r=p->value;
       rn->setValue(r);
      }
    else
    if (p==green)
      {
       g=p->value;
       gn->setValue(g);
      }
    else
    if (p==blue)
      {
       b=p->value;
       bn->setValue(b);
      }

    EditorPalette->SetOne(colorsel->getValue(),r,g,b);
   }
 else
 if (event.what==evCommand && event.message.command==cmYes)
   {
    EditorPalette->BackToDefault();
    endModal(cmYes);
    clearEvent(event);
    return;
   }
 else
 if (event.what==evKeyDown && event.keyDown.keyCode==kbEsc)
   {
    endModal(cmCancel);
    clearEvent(event);
    return;
   }
 else
 if (event.what==evKeyDown)
   {
    int aux;
    switch (event.keyDown.keyCode)
      {
       case kbR:
            aux=red->value;
            if (aux>0)
              {
               aux--;
               red->setValue(aux);
               rn->setValue(aux);
              }
            break;
       case kbShR:
            aux=red->value;
            if (aux<MaxCompVal)
              {
               aux++;
               red->setValue(aux);
               rn->setValue(aux);
              }
            break;
       case kbG:
            aux=green->value;
            if (aux>0)
              {
               aux--;
               green->setValue(aux);
               gn->setValue(aux);
              }
            break;
       case kbShG:
            aux=green->value;
            if (aux<MaxCompVal)
              {
               aux++;
               green->setValue(aux);
               gn->setValue(aux);
              }
            break;
       case kbB:
            aux=blue->value;
            if (aux>0)
              {
               aux--;
               blue->setValue(aux);
               bn->setValue(aux);
              }
            break;
       case kbShB:
            aux=blue->value;
            if (aux<MaxCompVal)
              {
               aux++;
               blue->setValue(aux);
               bn->setValue(aux);
              }
            break;
      }
   }

 TDialog::handleEvent(event);
}

void EditPalette(void)
{
 if (!TScreen::canSetPalette())
   {
    messageBox(__("The hardware doesn't support this."),mfError | mfOKButton);
    return;
   }
 PalCol *orig=EditorPalette->GetAllPal();
 TDialog *d=new TDiaPal();
 if (execDialog(d,0)==cmCancel)
    EditorPalette->SetAllPal(orig);
 DeleteArray(orig);
}

