/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TInputLinePiped) && !defined(__SET_TInputLinePiped_H__)
#define __SET_TInputLinePiped_H__
#define Uses_TInputLinePipedConst

class TCEditWindow;

class TInputLinePiped : public TInputLine
{
public:
 TInputLinePiped( const TRect& bounds, int aMaxLen, unsigned flags=0 );

 // New members virtual to allow override
 virtual int  PipeLine(unsigned pos);
 virtual void CopyToClip(void);
 virtual void CopyFromClip(void);
 virtual void CopyToClipOS(void);
 virtual void CopyFromClipOS(void);
 // Replacements
 virtual void handleEvent( TEvent& event );
 virtual void setState(uint16 aState,Boolean enable);

 unsigned mFlags;
};

#endif

#if defined(Uses_TInputLinePipedConst) && !defined(__SET_TInputLinePipedConst_H__)
#define __SET_TInputLinePipedConst_H__
const int cmtilCopy   = 0x2230;
const int cmtilPaste  = 0x2231;
const int cmtilCopyOS = 0x2232;
const int cmtilPasteOS= 0x2233;
const int tilpNoPipe=1,tilpNoCopy=2,tilpNoPaste=4;
#endif

