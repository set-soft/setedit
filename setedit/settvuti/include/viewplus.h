/****************************************************************************

  TViewPlus class header, copyright (c) 1996 by Salvador E. Tropea (SET)

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

  These class is designed ONLY to get access to private members of TView, do
not add variables to it, only functions. If you really need to add vars take
care about side effects, this class asumes that can use a TView or TGroup
just like another TViewPlus casting the pointer.

****************************************************************************/

#if defined( Uses_TViewPlus ) && !defined( __TViewPlus )
#define __TViewPlus

class TViewPlus : public TView
{

public:

 TViewPlus( const TRect& bounds ) : TView(bounds) {};
 int setAttrOfCoor(int x, int y, char attr);
 int getAttrsOfCol(int x, int y1, int y2, char *attr);
 int getAttrsOfRow(int x1, int x2, int y, char *attr);

protected:

 TViewPlus( StreamableInit ) : TView( streamableInit ) {};

};

#endif // TViewPlus
