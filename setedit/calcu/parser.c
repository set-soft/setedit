/*****************************************************************************

 Advanced parser. It supports all the functions found in ML's parser and also
 definition of variables, functions and loops.
 Created by Burton Radons <loth@pacificcoast.net>.

 Exception: Here , is used for a different purpose than in ML's parser.

 Size of code with gcc 2.8.1 and -O2: 8824 bytes.

*****************************************************************************/

#include <configed.h>
#include <ctype.h>
#include <math.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#define Uses_string
#define CLY_DoNotDefineUTypes
#include <compatlayer.h>

#ifdef HAVE_CALCULATOR

#define isprenum(STR) (STR == '.' || (STR >= '0' && STR <= '9'))
#define isprevar(STR) (STR == '_' || (STR >= 'a' && STR <= 'z') || (STR >= 'A' && STR <= 'Z'))
#define isintvar(STR) (isprevar (STR) || (STR >= '0' && STR <= '9'))
#define isexpr(STR)   (!isintvar (STR))

#define resize(PTR, LEN) ((PTR) = realloc (PTR, sizeof (*(PTR)) * (LEN)))
#define matchlen(NAME) ((int) (end - top) == (len = strlen (NAME)) && !strncasecmp (top, NAME, len))
#define opsdo(NAME, LEN, EXPR) if (matchops (NAME, LEN)) EXPR
#define findtop() for (top = str; isintvar (*str); str ++); end = str;
#define matchops(NAME, LEN) (!strncmp (ops, NAME, LEN) && skipops (LEN))

#ifdef TVComp_GCC
#define ltype long long
#define lprint "ll"
#endif

#if defined(TVComp_BCPP) || defined(TVComp_MSC)
#define ltype   __int64
#define lprint  "i64"
#define strncasecmp strnicmp
#endif
              
#ifdef TVOSf_Linux
// Why it fails in my machine?!
#define ftype double
#define fprint ""
#else
#define ftype long double
#define fprint "L"
#endif

static char *str = NULL;
static char ops [16];
static ftype arg [16];
static int cfun = -1;
static jmp_buf jmp;
static char *ret;
static int pradix;

typedef ftype (*bfuntype) (ftype a);
#define easy(NAME) static ftype fun_##NAME (ftype a) { return NAME (a); }
easy (sin); easy (cos); easy (tan); easy (sinh); easy (cosh); easy (tanh); easy (asin);
easy (acos); easy (atan); easy (log); easy (log10); easy (exp); easy (abs); easy (sqrt);
easy (ceil); easy (floor);
#undef easy
#define easy(NAME, RADIX) static ftype fun_##NAME (ftype a) { pradix = RADIX; return a; }
easy (bin, 2); easy (oct, 8); easy (dec, 10); easy (hex, 16);
#undef easy

static char *bfunn [] = { "sin", "cos", "tan", "sinh", "cosh", "tanh", "asin", "acos",
                          "atan", "log", "log10", "exp", "abs", "sqrt", "ceil", "floor",
                          "bin", "oct", "dec", "hex" };
static bfuntype bfunp [] = { fun_sin, fun_cos, fun_tan, fun_sinh, fun_cosh, fun_tanh, fun_asin,
                             fun_acos, fun_atan, fun_log, fun_log10, fun_exp, fun_abs, fun_sqrt,
                             fun_ceil, fun_floor, fun_bin, fun_oct, fun_dec, fun_hex };
#define bnfun (int) (sizeof (bfunn) / sizeof (*bfunn))

static char **funn = NULL;
static char ***funa = NULL; /* Arguments name-list for temporary binding */
static char **func = NULL; /* Function contents */
static int nfun = 0;

static char **varn = NULL;
static ftype *varv = NULL;
static int nvar = 0;

static ftype expr (void);

static void error (char *str)
{
   strcpy (ret, str);
   longjmp (jmp, 1);
}

static char *My_strndup (char *ptr, int len)
{
   char *dat = malloc (len + 1);
   memmove (dat, ptr, len);
   dat [len] = '\0';
   return dat;
}

static void skipspace (void)
{
   // The cast is a workaround for a bug in Solaris 9 for SPARC
   while (isspace ((int)*str))
      str ++;
}

static int skipops (int cnt)
{
   while (cnt --)
   {
      skipspace ();
      str ++;
   }
   return 1;
}

static char *readops (void)
{
   char *top = str;
   char *ptr = ops;
   while (1)
   {
      skipspace ();
      if (*str == '\0' || isintvar (*str))
         break;
      *ptr ++ = *str ++;
   }
   *ptr = '\0';
   str = top;
   return ops;
}

static ftype *findvar (char *top, char *end)
{
   int c, len;
   if (cfun != -1)
   {
      for (c = 0; funa [cfun] [c] != NULL; c ++)
         if (!strncasecmp (funa [cfun] [c], top, (int) (end - top)))
            return &arg [c];
   }
   for (c = 0; c < nvar; c ++)
      if (matchlen (varn [c]))
         break;
   if (c >= nvar)
   {
      resize (varn, c + 1);
      resize (varv, c + 1);
      varn [c] = My_strndup (top, (int) (end - top));
      varv [c] = 0;
      nvar ++;
   }
   return &varv [c];
}

static ftype readnum (void)
{
   ftype a [16];
   int c, d, len;
   char *top, *end;
   ftype val = 0;
   
   skipspace ();
   if (isprenum (*str))
   {
      if (*str == '0')
      {
         if (*++ str == 'x')
            return strtol (str + 1, &str, 16);
         else if (*str == 'b')
            return strtol (str + 1, &str, 2);
         else if (*str != '.')
            return strtol (str, &str, 8);
      }
      val = strtod (str, &str);
      if (*str == 'x')
      {
         if (val < 1 || val > 36)
            error ("Radix out of range (from 2 to 36)\n");
         val = strtol (str + 1, &str, (int) val);
      }
      return val;
   }
   else if (isexpr (*str))
   {
      readops ();
           opsdo ("++", 2, { findtop (); return ++ *findvar (top, end); })
      else opsdo ("--", 2, { findtop (); return -- *findvar (top, end); })
      else opsdo ("-", 1, return -readnum ());
      else opsdo ("+", 1, return readnum ());
      else opsdo ("~", 1, return ~(ltype) readnum ());
      else opsdo ("!", 1, return !readnum ());
      else if (matchops ("(", 1))
      {
         ftype val = expr ();
         while (*str == ',')
            str ++, val = expr ();
         readops ();
         if (!matchops (")", 1))
            error ("Unbalanced or illegal expression");
         return val;
      }
   }
   else if (isprevar (*str))
   {
      findtop ();
      for (c = 0; c < bnfun; c ++)
         if (matchlen (bfunn [c]))
         {
            readops ();
            if (!matchops ("(", 1))
               error ("Expected (");
            val = expr ();
            readops ();
            if (!matchops (")", 1))
               error ("Too many arguments");
            return bfunp [c] (val);
         }

      readops ();
      if (matchops ("(", 1))
      {
         char *ostr;
         int ocfun;
         ftype val;

         for (c = 0; str [c] != '('; c ++)
            if (str [c] == ')')
            {
               str += c;
               readops ();
               str -= c;
               if (ops [1] == '=' && ops [2] != '=')
               {
                  for (c = 0; c < nfun; c ++)
                     if (matchlen (funn [c]))
                        goto redef;
                  goto newdef;
               }
               break;
            }
         for (c = 0; c < nfun; c ++)
            if (matchlen (funn [c]))
            {
               for (d = 0; funa [c] [d] != NULL; d ++)
               {
                  a [d] = arg [d];
                  arg [d] = expr ();
                  readops ();
                  if (!matchops (funa [c] [d + 1] != NULL ? "," : ")", 1))
                     error ("Too few or too many arguments");
               }
               if (d == 0)
               {
                  readops ();
                  if (!matchops (")", 1))
                     error ("Expected )");
               }
               ostr = str;   str = func [c];
               ocfun = cfun; cfun = c;
               val = expr ();
               while (*str == ',')
                  str ++, val = expr ();
               str = ostr;   cfun = ocfun;
               while (d --)
                  arg [d] = a [d];
               return val;
            }

      newdef:
         resize (funn, c + 1);
         resize (funa, c + 1);
         resize (func, c + 1);
      redef:
         funn [c] = My_strndup (top, (int) (end - top));
         funa [c] = NULL;

         readops (); d = 0;
         if (!matchops (")", 1))
         while (1)
         {
            for (top = str; isintvar (*str); str ++);
            resize (funa [c], d + 1);
            funa [c] [d] = My_strndup (top, (int) (str - top));
            readops ();
            d ++;
            if (matchops (")", 1))
               break;
            else if (!matchops (",", 1))
               error ("Broken command");
         }

         readops ();
         if (!matchops ("=", 1))
            error ("Expected =");
         resize (funa [c], d + 1);
         funa [c] [d] = NULL;
         func [c] = strdup (str);
         if (c >= nfun)
            nfun ++;
         str += strlen (str) - 1;
         return 0;
      }
      else
      {
         ftype *vp = findvar (top, end);
              opsdo ("=", 1, return *vp = expr ());
         else opsdo ("+=", 2, return *vp += expr ());
         else opsdo ("-=", 2, return *vp -= expr ());
         else opsdo ("*=", 2, return *vp *= expr ());
         else opsdo ("/=", 2, return *vp /= expr ());
         else opsdo ("%=", 2, return *vp = fmod (*vp, expr ()));
         else opsdo ("&=", 2, return *vp = (ltype) (*vp) & (ltype) expr ());
         else opsdo ("^=", 2, return *vp = (ltype) (*vp) ^ (ltype) expr ());
         else opsdo ("!=", 2, return *vp = (ltype) (*vp) | (ltype) expr ());
         else opsdo ("<<=", 3, return *vp = (ltype) (*vp) << (ltype) expr ());
         else opsdo (">>=", 3, return *vp = (ltype) (*vp) >> (ltype) expr ());
         else opsdo ("++", 2, return (*vp) ++);
         else opsdo ("--", 2, return (*vp) --);
         else return *vp;
      }
   }
   error ("Retarded command");
   return 0;
}

static ftype term (int lvl)
{
   ftype val;
   if (lvl -- < 0)
      return readnum ();
   val = term (lvl);
   while (1)
   {
      readops ();
      switch (lvl + 1)
      {
         case 0:
            opsdo ("**", 2, val = pow (val, term (lvl)));
            else
               return val;
            break;
         case 1:
                 opsdo ("*", 1, val *= term (lvl));
            else opsdo ("/", 1, val /= term (lvl));
            else opsdo ("%", 1, val = fmod (val, term (lvl)));
            else return val;
            break;
         case 2:
                 opsdo ("+", 1, val += term (lvl));
            else opsdo ("-", 1, val -= term (lvl));
            else return val;
            break;
         case 3:
                 opsdo ("<<", 2, val = (ltype) val << (ltype) term (lvl));
            else opsdo (">>", 2, val = (ltype) val >> (ltype) term (lvl));
            else return val;
            break;
         case 4:
                 opsdo ("<",  1, val = val <  term (lvl));
            else opsdo ("<=", 2, val = val <= term (lvl));
            else opsdo (">",  1, val = val >  term (lvl));
            else opsdo (">=", 2, val = val >= term (lvl));
            else return val;
            break;
         case 5:
                 opsdo ("==", 2, val = val == term (lvl));
            else opsdo ("!=", 2, val = val != term (lvl));
            else return val;
            break;
         case 6:
                 if (!strncmp (ops, "&&", 2)) return val;
            else opsdo ("&", 1, val = (ltype) val & (ltype) term (lvl));
            else return val;
            break;
         case 7:
                 opsdo ("^", 1, val = (ltype) val ^ (ltype) term (lvl));
            else return val;
            break;
         case 8:
                 if (!strncmp (ops, "||", 2)) return val;
            else opsdo ("|", 1, val = (ltype) val | (ltype) term (lvl));
            else return val;
            break;
         case 9:
                 opsdo ("&&", 2, val = val && term (lvl));
            else return val;
            break;
         case 10:
                 opsdo ("||", 2, val = val || term (lvl));
            else return val;
            break;
         case 11:
            if (matchops ("?", 1))
            {
               int c;
               if (val)
               {
                  val = expr ();
                  skipspace ();
                  if (*str ++ != ':')
                     error ("Expected :");
                  for (c = 0; *str != '\0' && c >= 0; str ++)
                     if (*str == '(') c ++;
                     else if (*str == ')') c --;
               }
               else
               {
                  for (c = 0; *str != '\0' && c >= 0; str ++)
                     if (*str == '?') c ++;
                     else if (*str == ':') c --;
                  if (str [-1] != ':')
                     error ("Expected :");
                  val = expr ();
               }
            }
            else return val;
      }
   }
}

static ftype expr (void)
{
   return term (11);
}

static char ret_buffer[140];

/* Evaluate str, printing the result to ret, return 0 on failure */
int eval (char *_str, char **_ret)
{
   ftype val;
   int c;
   
   pradix = 10;
   str = _str;
   *_ret = ret_buffer;
   ret = ret_buffer;
   if (setjmp (jmp))
      return 1;
   val = expr ();
   while (*str == ',')
      str ++, val = expr ();
   if (*str != '\0' && *str != '\n')
      error ("Broken command");
   if (pradix == 10)
      sprintf (ret, "%.10" fprint "g", val);
   else if (pradix == 16)
      sprintf (ret, "0x%" lprint "X", (ltype) val);
   else if (pradix == 8)
      sprintf (ret, "0%" lprint "o", (ltype) val);
   else if (pradix == 2)
   {
      ltype bin = val;
      *ret ++ = '0';
      *ret ++ = 'b';
      for (c = 0; bin >> c; c ++);
      if (c == 0)
         *ret ++ = '0';
      ret += c;
      *ret -- = '\0';
      for (; bin; bin >>= 1)
         *ret -- = (bin & 1) + '0';
   }
   return 0;
}

#ifdef TEST
int main (int argc, char *argv [])
{
   char buf [256], *res, *s;
   int c;
   printf ("Write expression, get answer.\n");
   while (fgets (buf, 256, stdin) == buf)
   {
      eval (buf, &res);
      printf ("   = %s\n", res);
   }
   return 0;
}
#endif /* TEST */

#endif // HAVE_CALCULATOR

