/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

/* ---------------------------------------------------------------------- */
/*      class TClusterArray                                                    */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TClusterArray ) && !defined( __TClusterArray )
#define __TClusterArray

#define SET_NO_STREAM

class TRect;
class TSItem;
class TEvent;
class TPoint;
class TStringCollection;

class TClusterArray : public TView
{

public:

    TClusterArray( const TRect& bounds, int number );
    ~TClusterArray();

    virtual uint32 dataSize();
    void drawBox( const char *icon, char marker );
    virtual void getData( void *rec );
    ushort getHelpCtx();
    virtual TPalette& getPalette() const;
    virtual void handleEvent( TEvent& event );
    virtual Boolean mark( int item );
    virtual void press( int item );
    virtual void movedTo( int item );
    virtual void setData( void *rec );
    virtual void setState( ushort aState, Boolean enable );

protected:

    uint64 value;
    int sel;
    int numItems;

private:

    int column( int item );
    int findSel( TPoint p );
    int row( int item );
#if !defined( SET_NO_STREAM )
    virtual const char *streamableName() const
        { return name; }

protected:

    TClusterArray( StreamableInit );
    virtual void write( opstream& );
    virtual void *read( ipstream& );

public:

    static const char * const name;
    static TStreamable *build();
#endif
};

#if !defined( SET_NO_STREAM )
inline ipstream& operator >> ( ipstream& is, TClusterArray& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TClusterArray*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TClusterArray& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TClusterArray* cl )
    { return os << (TStreamable *)cl; }
#endif // NO_STREAM

#endif  // Uses_TClusterArray

