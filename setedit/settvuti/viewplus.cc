/****************************************************************************

  TViewPlus class, copyright (c) 1996 by Salvador E. Tropea (SET)

  Designed for the TCEditor class to be used by Robert Hîhne in your RHIDE.

  You can use this file for any purpose if you left the copyrights untouched
and give me some credit for these functionalities.

  E-Mail: salvador@inti.edu.ar

  Telephone: (+5411) 4759-0013

  Postal Address:
  Salvador E. Tropea
  CurapaligÅe 2124
  (1678) Caseros - 3 de Febrero
  Prov: Buenos Aires
  Argentina

  These routines are compatible with Borland's TVision 1.03 and the port of
those routines to DJGPP.

  The following routines are low level routines that could be putted in
the TView class. But I made that in a separated class derived from TView
to avoid the modfication of the original lib.

 Conventions:
 Returns:
 0 Ok, 1 Bad

****************************************************************************/

#define Uses_TView
#define Uses_TGroup
#define Uses_TScreen
#define Uses_TEventQueue
#include <tv.h>

#define Uses_TViewPlus
#include "viewplus.h"

#ifdef TVCompf_djgpp
#include <dos.h>
#include <go32.h>
#endif

// From View.cc
extern TPoint shadowSize;

// SET: Be careful about nibble order
#ifdef TV_BIG_ENDIAN
 #define GetAttr(a) ((a) & 0xFF)
 #define ChangeAttr(v,a)  (uint16)(((v) & 0xFF00) | (a))
 #define AttrOffset 0
#else
 #define GetAttr(a) ((a) << 8)
 #define ChangeAttr(v,a)  (((v) & 0xFF) | (a))
 #define AttrOffset 1
#endif

// Set the attribute of the X,Y coordinate
int TViewPlus::setAttrOfCoor(int x, int y, char attr)
{
 // Is visible
 if (!(state & sfVisible)) return 1;
 // Is inserted
 if (owner==NULL) return 1;

 // Is inside of the window
 if (x<0 || x>size.x) return 1;
 if (y<0 || y>size.y) return 1;

 // Adjust the local x and y to the owner's coordinates
 y+=origin.y;
 x+=origin.x;

 TRect *clip=&owner->clip;
 // Is in the allowed part of screen
 if (y<clip->a.y || y>=clip->b.y || x<clip->a.x || x>clip->b.x) return 1;

 // Take the top of the views
 TView *view=owner->last;
 TView *target=this;

 int aux;
 #if 0
 int IsUnderShadow=0;
 #endif
 #define Ytest aux
 #define Xtest aux
 while (1)
   {
    view=view->next;
    if (view==target) break;

    // If not visible forget it
    if (!(view->state & sfVisible)) continue;

    // Check the Y range
    Ytest=view->origin.y;
    if (y<Ytest) continue;
    Ytest+=view->size.y;
    #if 0  // For this application the shadow is like a solid thing
    if (y>=Ytest)
      {
       // Test the shadow
       if (!(view->state & sfShadow)) continue;
       Ytest+=shadowSize.y;
       if (y>=Ytest) continue;
       // Ok the Y is under a shadow, now, is the X in the range of the
       // shadow?
       Xtest=view->origin.x+shadowSize.x;
       if (x>=Xtest && x<Xtest+view->size.x) IsUnderShadow++;
       continue;
      }
    #else
    if (view->state & sfShadow)
       Ytest+=shadowSize.y; // Add the shadow
    if (y>=Ytest) continue;
    #endif

    // Check the X range
    Xtest=view->origin.x;
    if (x<Xtest) continue;
    Xtest+=view->size.x;
    #if 0  // For this application the shadow is like a solid thing
    if (x>=Xtest)
      {
       // Test the shadow
       if (!(view->state & sfShadow)) continue;
       Xtest+=shadowSize.x;
       if (x>=Xtest) continue;
       // Ok the X is under a shadow, now, is the Y in the range of the
       // shadow?
       Ytest=view->origin.y+shadowSize.y;
       if (y>=Ytest && y<Ytest+view->size.y) IsUnderShadow++;
       continue;
      }
    #else
    if (view->state & sfShadow)
       Xtest+=shadowSize.x;
    if (x>=Xtest) continue;
    #endif

    // OK is under this object, no draw needed
    return 1;
   }

 // Well, this position is visible, now do it
 if (owner->buffer)
   {
    // If the view have a buffer draw on it
    unsigned offset = y*owner->size.x+x;
    if (owner->buffer==TScreen::screenBuffer)
      {
       // Is to the screen
       #if 0
       disable();
       #endif
       int OverMouse = (y==TEventQueue::curMouse.where.y) && (x==TEventQueue::curMouse.where.x);
       #if 0
       enable();
       #endif
       if (OverMouse)
          TMouse::hide();
       // Ancient code: *(char *)(MK_FP(0xB800,1+(offset<<1)))=attr;
       #ifdef TVCompf_djgpp

       long _buffer;
       #if 1 // def RHIDE // RHIDE supports dual display
       _buffer = (TScreen::dual_display ? 0xb0000 : ScreenPrimary) + (offset<<1) + 1;
                 //TScreen::GetPage()*0x1000;
       #else
       _buffer = ScreenPrimary + (offset<<1) + 1 + TScreen::GetPage()*0x1000;
       #endif
       dosmemput((const void *)(&attr),1,_buffer);

       #else // DJGPP
       TScreen::setCharacter(offset,
        ChangeAttr(TScreen::getCharacter(offset),
                   GetAttr((unsigned char)attr)));
       #endif
       if (OverMouse)
          TMouse::show();
      }
    else
      { // Is to the cache buffer
       *((char *)(&owner->buffer[offset])+AttrOffset)=attr;
      }
   }
 // Now see if is locked
 if (owner->lockFlag) return 0;
 // Nop, then pass the info to the owner, it's a recursive process, that stops
 // when the real owner of the screen is updated.
 ((TViewPlus *)owner)->setAttrOfCoor(x,y,attr);
 return 0;
}

// read the attributes of a Column
int TViewPlus::getAttrsOfCol(int x, int y1, int y2, char *attr)
{
 if (owner)
   {
    char *buffer=(char *)owner->buffer;

    if (buffer)
      {
       y1+=origin.y;
       y2+=origin.y;
       x+=origin.x;
       int sx=owner->size.x,i;
       unsigned offset=((x+y1*sx)<<1)+1;
       sx*=2;
       for (i=0; y1<=y2; y1++, i++, offset+=sx)
           attr[i]=buffer[offset];
      }
    else
       return 1;
    return 0;
   }
 return 1;
}

// read the attributes of a row
int TViewPlus::getAttrsOfRow(int x1, int x2, int y, char *attr)
 {
 if (owner)
   {
    char *buffer=(char *)owner->buffer;

    if (buffer)
      {
       y+=origin.y;
       x1+=origin.x;
       x2+=origin.x;
       int sx=owner->size.x,i;
       unsigned offset=((x1+y*sx)<<1)+1;
       for (i=0; x1<=x2; x1++, i++, offset+=2)
           attr[i]=buffer[offset];
      }
    else
       return 1;
    return 0;
   }
 return 1;
}


