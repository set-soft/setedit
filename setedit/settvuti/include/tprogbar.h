/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined( Uses_TProgressBar ) && !defined( __TProgressBar )
#define __TProgressBar

class TRect;

class TProgressBar : public TView
{
public:
   // default the background char to 178, but you can pass any char you want
   TProgressBar(const TRect& r, unsigned long iters, char abackChar='²');
   ~TProgressBar();
   virtual void draw();
   virtual TPalette& getPalette() const;
   virtual void update(unsigned long aProgress);

   inline unsigned long getTotal();	// get the maximum iteration
   inline unsigned long getProgress();	// get the current iteration

   // change the percentage ( calls the update function )
   void setTotal(unsigned long newTotal); // set the maximum iteration
   void setProgress(unsigned long newProgress); // set the current iteration

protected:
   char          backChar;   // background character
   unsigned long total;      // total iterations to complete 100 %
   unsigned long progress;   // current iteration value
   char *        bar;	     // thermometer bar
   unsigned int  dispLen;    // length of bar
   unsigned int  curPercent; // current percentage
   unsigned int  curWidth;
   unsigned int  numOffset;  // offset in the string to display the percentage
   double        charValue;

private:
   int calcPercent();     // calculate new percentage

 SetDefStreamMembersCommon(TProgressBar,TView)
};

SetDefStreamOperators(TProgressBar)
#endif      // defined( Uses_TProgressBar ) && !defined( __TProgressBar )
