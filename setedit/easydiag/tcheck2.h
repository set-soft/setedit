/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */

/* ---------------------------------------------------------------------- */
/*      TCheckBoxesArray                                                  */
/*                                                                        */
/*      Palette layout                                                    */
/*        1 = Normal text                                                 */
/*        2 = Selected text                                               */
/*        3 = Normal shortcut                                             */
/*        4 = Selected shortcut                                           */
/* ---------------------------------------------------------------------- */

#if defined( Uses_TCheckBoxesArray ) && !defined( __TCheckBoxesArray )
#define __TCheckBoxesArray

class TRect;
class TSItem;

#define SET_NO_STREAM

class TCheckBoxesArray : public TClusterArray
{

public:

    TCheckBoxesArray( const TRect& bounds, int number);

    virtual void draw();
    
    virtual Boolean mark( int item );
    virtual void press( int item );

    int (*pressCallBack)(int);

private:

    static const char * button;
#if !defined( SET_NO_STREAM )
    virtual const char *streamableName() const
        { return name; }

protected:

    TCheckBoxesArray( StreamableInit );

public:

    static const char * const name;
    static TStreamable *build();
#endif
};

#if !defined( SET_NO_STREAM )
inline ipstream& operator >> ( ipstream& is, TCheckBoxesArray& cl )
    { return is >> (TStreamable&)cl; }
inline ipstream& operator >> ( ipstream& is, TCheckBoxesArray*& cl )
    { return is >> (void *&)cl; }

inline opstream& operator << ( opstream& os, TCheckBoxesArray& cl )
    { return os << (TStreamable&)cl; }
inline opstream& operator << ( opstream& os, TCheckBoxesArray* cl )
    { return os << (TStreamable *)cl; }
#endif // SET_NO_STREAM

inline TCheckBoxesArray::TCheckBoxesArray( const TRect& bounds, int number) :
    TClusterArray( bounds, number ),
    pressCallBack(0)
{
}

#endif  // Uses_TCheckBoxesArray

