/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#if !defined( __SET_UTIL_H )
#define __SET_UTIL_H

#define SetDefStreamOperators(a) \
inline ipstream& operator >> ( ipstream& is, a& cl )   \
    { return is >> (TStreamable&)cl; }                 \
inline ipstream& operator >> ( ipstream& is, a*& cl )  \
    { return is >> (void *&)cl; }                      \
                                                       \
inline opstream& operator << ( opstream& os, a& cl )   \
    { return os << (TStreamable&)cl; }                 \
inline opstream& operator << ( opstream& os, a* cl )   \
    { return os << (TStreamable *)cl; }

#define SetDefStreamMembers(a,b) \
private:                                                             \
    virtual const char *streamableName() const                       \
        { return name; }                                             \
    virtual void *readItem( ipstream& is );                          \
    virtual void writeItem( void *obj, opstream& os );               \
protected:                                                           \
    a( StreamableInit ) : b( streamableInit ) {};                    \
public:                                                              \
    static const char * const name;                                  \
    static TStreamable *build() {return new a( streamableInit );};

#define SetDefStreamMembersCommon(a,b) \
private:                                                             \
    virtual const char *streamableName() const                       \
        { return name; }                                             \
protected:                                                           \
    a( StreamableInit ) : b( streamableInit ) {};                    \
    virtual void write( opstream& );                                 \
    virtual void *read( ipstream& );                                 \
public:                                                              \
    static const char * const name;                                  \
    static TStreamable *build() {return new a( streamableInit );};


#define SetDefStreamMembersNoConst(a) \
private:                                                             \
    virtual const char *streamableName() const                       \
        { return name; }                                             \
protected:                                                           \
    a( StreamableInit );                                             \
    virtual void write( opstream& );                                 \
    virtual void *read( ipstream& );                                 \
public:                                                              \
    static const char * const name;                                  \
    static TStreamable *build() {return new a( streamableInit );};

/* The following macro is a little hack to decrease the compile time
   for the TV lib when compiling all the n*.cc files */

#define n2(CLASS,NAME)                    \
class CLASS                               \
{                                         \
public:                                   \
  static const char * const name;         \
};                                        \
                                          \
const char * const CLASS::name = #NAME;

#define s(TYPE)\
  TStreamableClass R##TYPE( T##TYPE::name, T##TYPE::build, __DELTA(T##TYPE));

#endif  // __SET_UTIL_H
