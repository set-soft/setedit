/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*
Ver:
TSViewCol: El destructor debe romper los objetos o ya lo hace TNSCollection?
19/3/98 2h escib.
19/3/98 1h de pensar en el viaje
19/3/98 2h en casa.
20/3/98 2h en casa. Organizaci¢n en headers, etc.
22/3/98 1h 40' Suficiente avance como para modelar el ^QA perfecto!
*/
#define Uses_string
#include <stdio.h>
#include <assert.h>
#define Uses_TApplication

#define Uses_TSButton
#define Uses_TSStaticText
#define Uses_TSInputLine
#define Uses_TSInputLinePiped
#define Uses_TSHzGroup
#define Uses_TSVeGroup
#define Uses_TSLabel
#define Uses_TSCheckBoxes
#define Uses_TSRadioButtons
#define Uses_TSSortedListBox
//#define Uses_TSSOSSortedListBox
#define Uses_TSTextScroller
#define Uses_TSSlider
#define Uses_TSNoStaticText

#include <easydia1.h>
#include <settvuti.h>
#include <easydiag.h>

class TMiApp : public TApplication
{
public:
 TMiApp() :
    TProgInit( TMiApp::initStatusLine,
               TMiApp::initMenuBar,
               TMiApp::initDeskTop
             ),
    TApplication()
    { insertDialog(); }
 virtual void setState(ushort,Boolean) {};
 void insertDialog();
};

/*** Test 1 a very easy dialog
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Test"));

 // First we create the object without specifying the position
 TSStaticText *text=new TSStaticText("Hello, how are you?");
 TSButton *but=new TSButton("Press it");
 // Center X and start at y=2
 col->insert(xTSCenter,2,text);
 // Center X and put it under the text
 col->insert(xTSCenter,yTSUnder,but,0,text);
 // That's all now let the wizard do the job ;-)
 col->doIt(deskTop);
 delete col;
}
**************/

/******** Test 2 same but it tests the TSHzGroup
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Test"));

 // First we create the object without specifying the position
 TSStaticText *text=new TSStaticText("Hello, how are you?");
 // That's a little tricky: I simply create groups of 2 SViews and as I
 // need more than 2 I just put a group as second element
 TSHzGroup *but123=new TSHzGroup(new TSButton("Button 1"),
                                 new TSHzGroup(new TSButton("Button 2"),
                                               new TSButton("Button 3")));
 // Center X and start at y=2
 col->insert(xTSCenter,2,text);
 // Center X and put it under the text
 col->insert(xTSCenter,yTSUnder,but123,0,text);
 // That's all now let the wizard do the job ;-)
 col->doIt(deskTop);
 delete col;
}
************/

/******** Test 3 Labels and input lines
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Test"));

 // First we create the object without specifying the position
 TSInputLine *text=new TSInputLine(20,(ushort)100);
 TSLabel *label=new TSLabel("Enter a text",text);

 // As the label contains the text we just need to insert the label
 // Center X and start at y=2
 col->insert(xTSCenter,2,label);
 // That's all now let the wizard do the job ;-)
 col->doIt(deskTop);
 delete col;
}
**********/


/******** Test 4 CheckBoxes and RadioButtons
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Test"));

 // First we create the object without specifying the position
 TSCheckBoxes *options = new TSCheckBoxes(
     new TSItem(__("~C~ase sensitive"),
     new TSItem(__("~W~hole words only"),
     new TSItem(__("Re~g~ular expressions"),
     new TSItem(__("E~x~tended regex"),
     new TSItem(__("~P~rompt on replace"),
     new TSItem(__("~R~eplace all"), 0 )))))));
 TSLabel *label=new TSLabel("Some options",options);

 TSRadioButtons *scope = new TSRadioButtons(
                        new TSItem( __("~F~rom cursor"),
                        new TSItem( __("~E~ntire scope"), 0 )));
 TSLabel *label2=new TSLabel("Origin", scope);

 // As the label contains the text we just need to insert the label
 col->insert(xTSCenter,1,label);
 col->insert(xTSCenter,yTSUnder,label2,0,label);
 // That's all now let the wizard do the job ;-)
 col->doIt(deskTop);
 delete col;
}
***********/

/********
void TMiApp::insertDialog()
{
 //TDialog *d = new TDialog( TRect( 0, 0, 76, 18 ), __("Replace") );
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Replace"));

 //TInputLinePiped *control=new TInputLinePiped(TRect(3,3,34,4),80);
 //d->insert( control );
 //d->insert(
 //    new TLabel( TRect( 2, 2, 33, 3 ), __("~T~ext to find"), control ) );
 //d->insert( new THistory( TRect( 34, 3, 37, 4 ), control, hID_TextSearchEditor ) );
 #define FixWTest 30

 TSInputLine *textF=new TSInputLine(80,1,(ushort)100,FixWTest);
 TSLabel *TextToFind=new TSLabel("~T~ext to find",textF);

 //TInputLinePiped *control2=new TInputLinePiped(TRect(39,3,70,4),80);
 //d->insert( control2 );
 //d->insert( new TLabel( TRect( 38, 2, 69, 3 ), __("~N~ew text"), control2 ) );
 //d->insert( new THistory( TRect( 70, 3, 73, 4 ), control2, hID_TextReplaceEditor ) );
 TSInputLine *textN=new TSInputLine(80,1,(ushort)101,FixWTest);
 TSLabel *newText=new TSLabel("~N~ew text",textN);

 //TCheckBoxes *options = new TCheckBoxes( TRect( 3, 6, 37, 12 ),
 //    new TSItem(__("~C~ase sensitive"),
 //    new TSItem(__("~W~hole words only"),
 //    new TSItem(__("Re~g~ular expressions"),
 //    new TSItem(__("E~x~tended regex"),
 //    new TSItem(__("~P~rompt on replace"),
 //    new TSItem(__("~R~eplace all"), 0 )))))));
 //d->insert( options );
 //d->insert(
 //    new TLabel( TRect( 2, 5, 35, 6 ), __("Options"), options ) );
 TSCheckBoxes *options = new TSCheckBoxes(
     new TSItem(__("~C~ase sensitive"),
     new TSItem(__("~W~hole words only"),
     new TSItem(__("Re~g~ular expressions"),
     new TSItem(__("E~x~tended regex"),
     new TSItem(__("~P~rompt on replace"),
     new TSItem(__("~R~eplace all"), 0 )))))));//,FixWTest);
 TSLabel *Options=new TSLabel("Options",options);

 //TRadioButtons *origin = new TRadioButtons( TRect( 39, 6, 73, 8 ),
 //                        new TSItem( __("~G~lobal"),
 //                        new TSItem( __("~S~elected text"), 0 )));
 //d->insert( origin );
 //d->insert(
 //    new TLabel( TRect( 38, 5, 71, 6 ), __("Scope"), origin ) );
 TSRadioButtons *origin = new TSRadioButtons(
                         new TSItem( __("~G~lobal"),
                         new TSItem( __("~S~elected text"), 0 )));//,FixWTest);
 TSLabel *Origin=new TSLabel("Scope",origin);

 //TRadioButtons *scope = new TRadioButtons( TRect( 39, 10, 73, 12 ),
 //                       new TSItem( __("~F~rom cursor"),
 //                       new TSItem( __("~E~ntire scope"), 0 )));
 //d->insert( scope );
 //d->insert(
 //    new TLabel( TRect( 38, 9, 71, 10 ), __("Origin"), scope ) );
 TSRadioButtons *scope =new TSRadioButtons(
                        new TSItem( __("~F~rom cursor"),
                        new TSItem( __("~E~ntire scope"), 0 )));//,FixWTest);
 TSLabel *Scope=new TSLabel("Origin",scope);

 //d->insert(
 //    new TButton( TRect( 29, 15, 39, 17 ), __("O~K~"), cmOK, bfDefault ) );
 //d->insert(
 //    new TButton( TRect( 41, 15, 59, 17 ), __("Replace ~A~ll"), cmYes, bfNormal) );
 //d->insert(
 //    new TButton( TRect( 61, 15, 74, 17 ),__("Cancel"), cmCancel, bfNormal ) );
 //TSHzGroup *but123=new TSHzGroup(new TSButton("O~K~",cmOK,bfDefault),
 //                  new TSHzGroup(new TSButton("Replace ~A~ll",cmYes),
 //                  new TSButton("Cancel",cmCancel)));

 col->insert(2,2,TextToFind);
 col->insert(xTSRightOf,2,newText,TextToFind);
 col->insert(2,yTSUnder,Options,0,TextToFind);
 col->insert(xTSRightOf,yTSUnder,Origin,TextToFind,newText);
 col->insert(xTSRightOf,yTSUnder,Scope,TextToFind,Origin);
 EasyInsertOKCancel(col,2);
 //col->insert(xTSCenter,yTSDown,but123);

 //d->options|=ofCentered;
 //d->selectNext( False );
 //return d;
 col->doIt(deskTop);
 delete col;
}
**********/

/****
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),"Replace"));

 TSStaticText *text=new TSStaticText("Replace this ocurrence?");
 TSHzGroup *buttons=new TSHzGroup(new TSButton("Yes",cmYes,bfDefault),
                    new TSHzGroup(new TSButton("No",cmNo),
                    new TSHzGroup(new TSButton("All",cmOK),
                                  new TSButton("Cancel",cmCancel))));

 col->insert(xTSCenter,2,text);
 col->insert(xTSCenter,yTSUnder,buttons,0,text);
 col->doIt(deskTop);
 delete col;
}
****/

/***
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Goto Line")));

 TSInputLine *lineInp=new TSInputLine(10);
 TSHzLabel *lineLabel=new TSHzLabel(__("~T~ext to find"),lineInp);

 col->insert(2,2,lineLabel);
 EasyInsertOKCancel(col);

 col->doIt(deskTop);
 delete col;
}
****/

/************
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Goto Line")));

 TSSortedListBox *list=new TSSortedListBox(20,5,tsslbHorizontal);
 TSLabel *label=new TSLabel("~S~ome o~p~tions 567890123",list);

 col->insert(2,2,label);
 EasyInsertOKCancel(col);

 col->doIt(deskTop);
 delete col;
}
************/


/************
void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Goto Line")));

 TNSCollection *list=new TNSCollection(14,3);
 list->insert("1234567890abcdefghijklmnopqrstuvwxyz");
 list->insert("2");
 list->insert("3");
 list->insert("4");
 list->insert("5");
 list->insert("6");
 list->insert("7");
 list->insert("8");
 list->insert("9");
 list->insert("10");
 list->insert("11");
 list->insert("12");
 list->insert("13");
 list->insert("14");
 TSTextScroller *pp=new TSTextScroller(30,10,list,1,1);

 col->insert(2,2,pp);

 col->doIt(deskTop);
 delete col;
}
************/

/************/
static TNoStaticText *LeftVal;
static TNoStaticText *RightVal;
static int Join=1;

static
void CallBack(int value, TScrollBarCB *, void *data)
{
 int which=(int)data;
 char buffer[4];
 sprintf(buffer,"%d",value);
 if (which)
    RightVal->setText(buffer);
 else
    LeftVal->setText(buffer);
}

void TMiApp::insertDialog()
{
 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("Sliders")));

 TSNoStaticText *lval=new TSNoStaticText("  0");
 LeftVal=(TNoStaticText *)lval->view;
 TSNoStaticText *rval=new TSNoStaticText("  0");
 RightVal=(TNoStaticText *)rval->view;
 TSSlider *sl=new TSSlider(12,True," Vol ",CallBack,(void *)0,CallBack,(void *)1,&Join);
 sl->Set(50,50);

 col->insert(2,2,sl);
 col->insert(2,yTSUnder,lval,0,sl);
 col->insert(xTSRightOf,yTSUnder,rval,lval,sl);
 col->doIt(deskTop);
 delete col;
}
/************/


/************
#define TCEDITOR_VERSION 0x414
void TMiApp::insertDialog()
{
 char b[54];
 TSStaticText *ant,*cur;

 TSViewCol *col=new TSViewCol(new TDialog(TRect(1,1,1,1),__("About")));

 TSView::yDefSep=0;
 sprintf(b,_("SET's Editor v%X.%X.%X, (c) 1996-1998"),TCEDITOR_VERSION>>16,
         (TCEDITOR_VERSION>>8) & 0xFF,TCEDITOR_VERSION & 0xFF);
 ant=new TSStaticText(b);
 col->insert(xTSCenter,2,ant);

 #define i(a)  cur=new TSStaticText(a); \
               col->insert(xTSCenter,yTSUnder,cur,0,ant); \
               ant=cur
 i(__("by Salvador Eduardo Tropea"));
 i(__("A text editor especially designed for programmers."));

 TSView::yDefSep=1;
 i(__("Made in Argentina"));
 #undef i
 #define i(a)  cur=new TSStaticText(a); \
               col->insert(2,yTSUnder,cur,0,ant); \
               ant=cur
 i(__("Thanks to:"));
 TSView::yDefSep=0;
 i(__("The FSF and GNU people for such good tools"));
 i(__("DJ Delorie and collaborators for porting it to DOS"));
 i(__("Robert H”hne for porting TVision and creating RHIDE"));
 i(__("The University of California, Berkeley and its contributors"));
 i(__("All my friends that support my project (Laszlo, Marek, etc)"));
 #undef i

 TSView::yDefSep=1;
 col->insert(xTSCenter,yTSUnder,new TSButton("O~K~",cmOK,bfDefault),0,ant);

 col->doIt(deskTop);
 delete col;
}
************/

int main(void)
{
 TMiApp *app=new TMiApp();
 app->run();
 delete app;
 return 0;
}
