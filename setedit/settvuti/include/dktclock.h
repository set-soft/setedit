/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TDeskTopClock) && !defined( __TDeskTopClock__ )
#define __TDeskTopClock__

class TDeskTopClock : public TView
{
public:
  TDeskTopClock(const TRect & r);
  void update(void);
  virtual void draw();
  static int mode; // 1 => 24hs 0 => AM/PM

protected:
  time_t curTime;
  char   putSeparator;
};
#endif
