/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if defined(Uses_TEnhancedText) && !defined(__TEnhancedText__)
#define __TEnhancedText__

typedef struct
{
 unsigned len;
 ushort text[0];
} LineOfEText;

class TNSCollection;

class TEnhancedText : public TNSCollection
{
public:
 TEnhancedText(const char *aFileName, const char *aCommandLine);
 ~TEnhancedText();
 void copyLine(int y, int w, ushort *line, char *colors);

 const char *fileName;
 const char *commandLine;
 char isOK;
 static int maxWidth;
 int rows, cols;
};

#endif // __TEnhancedText__


#if defined(Uses_TManPageView) && !defined(__TManPageView__)
#define __TManPageView__

class TScroller;
class TRect;
class TScrollBar;
class TPalette;
class TEnhancedText;

class TManPageView : public TScroller
{
public:
 TManPageView(const TRect& bounds, TScrollBar *aHScrollBar,
              TScrollBar *aVScrollBar);
 ~TManPageView();
 virtual void draw();
 virtual TPalette& getPalette() const;
 virtual void handleEvent( TEvent& event );
 void InsertText(TEnhancedText *aText);

protected:
 TEnhancedText *text;
 
private:
 virtual const char *streamableName() const { return name; }
protected:
 TManPageView(StreamableInit) : TScroller(streamableInit) { text=0; };
 virtual void write(opstream&);
 virtual void *read(ipstream&);
public:
 static const char * const name;
 static TStreamable *build() {return new TManPageView(streamableInit);};
};

inline ipstream& operator >> ( ipstream& is, TManPageView& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TManPageView*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TManPageView& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TManPageView* cl )
    { return os << (TStreamable *)cl; }

#endif // __TManPageView__

#if defined(Uses_TManWindow) && !defined(__TManWindow__)
#define __TManWindow__

class TWindow;
class TManPageView;
class TPalette;

class TManWindow : public TWindow
{
public:
 TManWindow(const char *fileName, const char *name, char *aCommandLine);
 virtual void handleEvent( TEvent& event );
 virtual TPalette& getPalette() const;
 const char *getFileName() { return title; };

protected:
 TManPageView *page;

private:
 virtual const char *streamableName() const { return name; }
protected:
 TManWindow(StreamableInit);
 // These members are needed to initialize page propperly
 #if 1
 virtual void write(opstream&);
 virtual void *read(ipstream&);
 #endif
public:
 static const char * const name;
 static TStreamable *build() {return new TManWindow(streamableInit);};
};

inline ipstream& operator >> ( ipstream& is, TManWindow& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TManWindow*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TManWindow& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TManWindow* cl )
    { return os << (TStreamable *)cl; }
    
const int hcManPage=0x2120,
          cmMPUpdateTitle=0x2120;

extern TManWindow *CreateManWindow(const char *file, const char *sections,
                                   const char *extraOps);

#endif // __TManWindow__
