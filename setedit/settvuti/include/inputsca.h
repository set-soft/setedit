/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
/* ---------------------------------------------------------------------- */
/*      class TInputScanKey                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Passive                                                     */
/*        2 = Active                                                      */
/*        3 = Selected                                                    */
/*        4 = Arrows                                                      */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TInputScanKey ) && !defined( __TInputScanKey )
#define __TInputScanKey

class TRect;
class TEvent;

class TInputScanKey : public TView
{

public:

    TInputScanKey(const TPoint& pos);
    ~TInputScanKey();

    virtual uint32 dataSize();
    virtual void draw();
    virtual void getData( void *rec );
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual void setData( void *rec );

    unsigned char Scan,Ascii;

private:

#if !defined( NO_STREAM )
    virtual const char *streamableName() const
        { return name; }

protected:

    TInputScanKey( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif // NO_STREAM
};

#if !defined( NO_STREAM )
inline ipstream& operator >> ( ipstream& is, TInputScanKey& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TInputScanKey*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TInputScanKey& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TInputScanKey* cl )
    { return os << (TStreamable *)cl; }
#endif // NO_STREAM

#endif  // Uses_TInputScanKey

