/*****************************************************************************

 Small parser. It supports all the functions found in ML's parser except the
 use of ,base.
 Created by Burton Radons <loth@pacificcoast.net>.

 Size of code with gcc 2.8.1 and -O2: 3224 bytes.

*****************************************************************************/

#include <math.h>
#include <setjmp.h>
#define Uses_ctype
#define Uses_stdio
#define Uses_stdlib
#define Uses_string
#define CLY_DoNotDefineUTypes
#include <compatlayer.h>

#define	isintvar(STR) (STR == '_' || (STR >= 'a' && STR	<= 'z')	|| (STR	>= 'A' && STR <= 'Z') || (STR >= '0' &&	STR <= '9'))
#define	isexpr(STR) (!isintvar (STR))
#define	opsdo(NAME, EXPR) if (matchops (NAME)) EXPR

#ifdef TVComp_GCC
#define ltype long long
#define lprint "ll"
#endif

#if defined(TVComp_BCPP) || defined(TVComp_MSC)
#define ltype   __int64
#define lprint  "i64"
#endif
              
#define	ftype long double
#define	fprint "L"

static char *str, *ret;
static jmp_buf jmp;
static int pradix;

typedef	ftype (*funtype) (ftype	a);
#define	easy(NAME) static ftype	fun_##NAME (ftype a) { return NAME (a);	}
easy (sin); easy (cos);	easy (tan); easy (sinh); easy (cosh); easy (tanh); easy	(asin);
easy (acos); easy (atan); easy (log); easy (log10); easy (exp);	easy (abs); easy (sqrt);
easy (ceil); easy (floor);
#undef easy
#define	easy(NAME, RADIX) static ftype fun_##NAME (ftype a) { pradix = RADIX; return a;	}
easy (bin, 2); easy (oct, 8); easy (dec, 10); easy (hex, 16);
#undef easy

static char *funn [] = { "sin",	"cos", "tan", "sinh", "cosh", "tanh", "asin", "acos",
			 "atan", "log",	"log10", "exp",	"abs", "sqrt", "ceil", "floor",
			 "bin",	"oct", "dec", "hex" };
static funtype funp [] = { fun_sin, fun_cos, fun_tan, fun_sinh,	fun_cosh, fun_tanh, fun_asin,
			   fun_acos, fun_atan, fun_log,	fun_log10, fun_exp, fun_abs, fun_sqrt,
			   fun_ceil, fun_floor,	fun_bin, fun_oct, fun_dec, fun_hex };
#define	nfun (int) (sizeof (funn) / sizeof (*funn))

static ftype expr (void);

static void error (int num)
{
   longjmp (jmp, num);
}

static void skipspace (void)
{
   while (isspace (*str))
      str ++;
}

int matchops (char *name)
{
   skipspace ();
   if (strncmp (str, name, strlen (name)))
      return 0;
   str += strlen (name);
   return 1;
}

static ftype readnum (void)
{
   int c, len;
   char	*top, *end;
   ftype val;
   
   skipspace ();
   if (*str == '.' || (*str >= '0' && *str <= '9'))
   {
      if (*str == '0')
      {
	 if (*++ str ==	'x')
	    return strtol (str + 1, &str, 16);
	 else if (*str == 'b')
	    return strtol (str + 1, &str, 2);
	 else if (*str != '.')
	    return strtol (str,	&str, 8);
      }
      return strtod (str, &str);
   }
   else	if (isexpr (*str))
   {
	   opsdo ("-", return -readnum ());
      else opsdo ("~", return ~(ltype) readnum ());
      else if (matchops	("("))
      {
	 val = expr ();
	 if (!matchops (")"))
	    error (-3);
	 return	val;
      }
   }
   else
   {
      for (top = str; isintvar (*str); str ++);	end = str;
      if (!matchops ("("))
	 error (-3);
      val = expr ();
      if (!matchops (")"))
	 error (-3);
      for (c = 0; c < nfun; c ++)
	 if ((int) (end	- top) == (len = strlen	(funn [c])) && !strncasecmp (top, funn [c], len))
	    return funp	[c] (val);
   }
   error (-1);
   return 0;
}

static ftype term (int lvl)
{
   ftype val;
   if (lvl -- <	0)
      return readnum ();
   val = term (lvl);
   while (1)
   {
      switch (lvl + 1)
      {
	 case 0: opsdo ("**", val = pow	(val, term (lvl)));
	    else return	val;
	    break;
	 case 1: opsdo ("*", val *= term (lvl));
	    else opsdo ("/", val /= term (lvl));
	    else opsdo ("%", val = fmod	(val, term (lvl)));
	    else return	val;
	    break;
	 case 2: opsdo ("+", val += term (lvl));
	    else opsdo ("-", val -= term (lvl));
	    else return	val;
	    break;
	 case 3: opsdo ("<<", val = (ltype) val	<< (ltype) term	(lvl));
	    else opsdo (">>", val = (ltype) val	>> (ltype) term	(lvl));
	    else return	val;
	    break;
	 case 4: opsdo ("&", val = (ltype) val & (ltype) term (lvl));
	    else return	val;
	    break;
	 case 5: opsdo ("^", val = (ltype) val ^ (ltype) term (lvl));
	    else return	val;
	    break;
	 case 6: opsdo ("|", val = (ltype) val | (ltype) term (lvl));
	    else return	val;
	    break;
      }
   }
}

static ftype expr (void)
{
   return term (6);
}

static char ret_buffer[140];

/* Evaluate str, printing the result to ret, return -1 on failure */
int eval (char *_str, char **_ret)
{
   ftype val;
   int c;
   
   pradix = 10;
   str = _str;
   *_ret = ret_buffer;
   ret = ret_buffer;
   if ((c = setjmp (jmp)))
      return c;
   val = expr ();
   if (*str != '\0' && *str != '\n')
      error (-3);
   if (pradix == 10)
      sprintf (ret, "%.10" fprint "g", val);
   else	if (pradix == 16)
      sprintf (ret, "0x%" lprint "X", (ltype) val);
   else	if (pradix == 8)
      sprintf (ret, "0%" lprint	"o", (ltype) val);
   else	if (pradix == 2)
   {
      ltype bin	= val;
      *ret ++ =	'0';
      *ret ++ =	'b';
      for (c = 0; bin >> c; c ++);
      if (c == 0)
	 *ret ++ = '0';
      ret += c;
      *ret -- =	'\0';
      for (; bin; bin >>= 1)
	 *ret -- = (bin	& 1) + '0';
   }
   return 0;
}

#ifdef TEST
int main (int argc, char *argv [])
{
   char buf [256], *res, *s;
   int c;
   printf ("Write expression, get answer.\n");
   while (fgets	(buf, 256, stdin) == buf)
   {
      for (s=buf; *s && *s!='\n'; s++); *s=0;
      c	= eval (buf, &res);
      if (c == 0)
	 printf	("   = %s\n", res);
      else
	 printf	("   Error %d\n", c);
   }
   return 0;
}
#endif /* TEST */
