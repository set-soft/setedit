/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/*****************************************************************************

  Calculator Interface.
  See the editor files for details.

  Notes:
  defining FLEX_BISON you can use the old flex and bison parser.

*****************************************************************************/

#include <ceditint.h>

#define Uses_stdio
#define Uses_string
#define Uses_TEvent
#define Uses_TInputLine
#define Uses_TButton
#define Uses_TLabel
#define Uses_TRect
#define Uses_THistory
#define Uses_TDialog
#define Uses_TApplication
#define Uses_MsgBox
#define Uses_TDeskTop
#define Uses_TCEditWindow
#define Uses_TInputLinePiped
#define Uses_TCEditor_External
#include <ceditor.h>
#include <calcu.h>
#include <setapp.h>
#include <locale.h>

#ifdef FLEX_BISON
extern "C" char *yyParseString(char *s);
#else
extern "C" int eval(char *mit,char **out);
#endif

class TRect;
class TInputLine;

class TCalcDialog : public TDialog
{
public:
  TCalcDialog(const TRect & bounds, const char *Title, char *StartVal = NULL);
  virtual void handleEvent(TEvent &);
  TInputLine *input;
  TInputLine *result;
};

const int MaxLinePipe=255;

TCalcDialog::TCalcDialog(const TRect & bounds, const char *Title, char *StartVal)
  : TDialog(bounds,Title),
  TWindowInit(TCalcDialog::initFrame)
{
  TRect r;
  helpCtx = hcCalculator;
  r.a.x = 2;
  r.a.y = 2;
  r.b.x = size.x - 5;
  r.b.y = r.a.y + 1;
  // Piped and supports copy & paste
  input = new TInputLinePiped(r,MaxLinePipe);
  insert(new THistory(TRect(r.b.x,r.a.y,r.b.x+3,r.b.y), input,
                      hID_TCalcDialogExp));
  if (StartVal)
    {
     // This will copy MaxLinePipe characters -1. For this reason the source
     // *must* be at least MaxLinePipe in size.
     char aux[MaxLinePipe];
     strncpy(aux,StartVal,MaxLinePipe);
     input->setData(aux);
    }
  insert(input);
  r.move(0,-1);
  insert(new TLabel(r,_("~E~xpression"),input));
  r.move(0,3);
  // Only supports copy
  result = new TInputLinePiped(r,255,tilpNoPipe | tilpNoPaste);
  insert(result);
  r.move(0,-1);
  insert(new TLabel(r,_("~R~esult"),result));
  r.move(0,3);
  r.b.x = r.a.x + 12;
  r.b.y = r.a.y + 2;
  insert(new TButton(r,_("E~v~al"),cmEval,bfDefault));
  r.a.x = r.b.x + 2;
  r.b.x = r.a.x + 12;
  insert(new TButton(r,_("Cancel"),cmCancel,bfNormal));
  r.a.x = r.b.x + 2;
  r.b.x = r.a.x + 12;
  insert(new TButton(r,_("~C~opy"),cmCaCopy,bfNormal));
  r.a.x = r.b.x + 2;
  r.b.x = r.a.x + 12;
  insert(new TButton(r,_("~P~aste"),cmCaPaste,bfNormal));
  input->select();
  options |= ofCentered;
}

// The calculators aren't "locale safe" so I just switch to
// C locale while using them
static
void Eval(char *input_buffer, char **ret)
{
 char *old_locale, *saved_locale;

 old_locale=setlocale(LC_ALL,NULL);
 saved_locale=strdup(old_locale);

 setlocale(LC_ALL,"C");
 #ifdef FLEX_BISON
 ret=yyParseString(input_buffer);
 #else
 int err=eval(input_buffer,ret);
 if (err)
   {
    sprintf(input_buffer,_("Error in expression (%d)"),err);
    messageBox(input_buffer,mfError | mfOKButton);
   }
 #endif

 setlocale(LC_ALL,saved_locale);
 free(saved_locale);
}

void TCalcDialog::handleEvent(TEvent & event)
{
  char *ret;
  TDialog::handleEvent(event);
  switch (event.what)
    {
     case evCommand:
          switch (event.message.command)
            {
             case cmEval:
                 {
                   char input_buffer[256];
                   input->getData(input_buffer);
                   Eval(input_buffer,&ret);
                   result->setData(ret);
                   input->selectAll(True);
                   clearEvent(event);
                 }
                  break;
             case cmCaCopy:
                  event.message.command=cmtilCopy;
                  result->handleEvent(event);
                  break;
             case cmCaPaste:
                  event.message.command=cmtilPaste;
                  input->handleEvent(event);
                  break;
            }
          break;
    }
}

void executeCalc(char *startVal)
{
 TCalcDialog *d;
 d=new TCalcDialog(TRect(10,2,72,11),_("Calculator"),startVal);
 TProgram::deskTop->execView(d);
 // Dialogs should be destroyed or your members won't de deleted.
 destroy(d);
 if (startVal)
    delete startVal;
}
