/* Copyright (C) 1996-2003 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined( Uses_TProgress ) && !defined( __TProgress )
#define __TProgress

class TRect;

class TProgress : public TView
{
public:
   TProgress(const TRect& r, const char *aMessage);
   ~TProgress();
   virtual void draw();
   virtual TPalette& getPalette() const;
   virtual void update();

protected:
   char *message;
   int   state;
   static char states[4];
   int len;

 SetDefStreamMembersCommon(TProgress,TView)
};

SetDefStreamOperators(TProgress)
#endif      // defined( Uses_TProgress ) && !defined( __TProgress )
