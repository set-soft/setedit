/*
 *        PARSER.C   V0.66   ML1050
 *
 *        Author:    Laszlo Molnar
 *
 *        This program is free.

 That's the original parser I used upto v0.4.41. Now is optional and the
 default parser is the Burton's one because it incorporates interesting
 features at a low cost.

 Size of code with gcc 2.8.1 and -O2: 3456 bytes.

 */

#include <configed.h>
#define Uses_string
#define Uses_alloca
#define Uses_stdlib
#define Uses_stdio
#define CLY_DoNotDefineUTypes
#include <compatlayer.h>

#include <math.h>
#include <setjmp.h>
#include <signal.h>
#ifdef   TVCompf_djgpp
 #include <float.h>  /* for _clear87() */
#endif

#ifdef HAVE_CALCULATOR

#ifdef TVComp_GCC
 #define ltype   long long
 #define ltypes  "ll"
 #ifdef TVCompf_djgpp
  #define strtol  strtoll
 #else
  #define strtol  strtoq
 #endif 
#else
 #if defined(TVComp_BCPP) || defined(TVComp_MSC)
  #define ltype   __int64
  #define ltypes  "i64"
 #else
  #define ltype   long
  #define ltypes  "l"
 #endif
#endif

#define OP_NEG  6         /* which is '-' in operators[] */

static  char    *operators []= {
        "|","^","&","<<",">>","+","-","*","/","%","**"," "};

#define OPNUM   (sizeof(operators)/sizeof(operators[0]))

#define ULEVEL  6
static  char    precedence [OPNUM]= {
         0 , 1 , 2 , 3  , 3  , 4 , 4 , 5 , 5 , 5 ,  6 , ULEVEL};

static  char    *functions []= {"sinh","cosh","tanh",
                                "asin","acos","atan",
                                "sin","cos","tan",
                                "log10","log","exp",
                                "abs","sqrt",
                                "ceil","floor",
                                "bin","oct","dec","hex"};

#define FNUM    (sizeof(functions)/sizeof(functions[0]))
#define WHITESP " \n\t"
#define SELF    "()~"
#define Y_EOS   0
#define Y_ERR   -1
#define Y_NUMB  1
#define Y_OPER  0x4000
#define Y_UNAR  (0x4800+OPNUM-1)

#define OPENB   0x1000
#define CLOSB   0x2000
#define EPSI    0x3000    /* empty string */
#define BIGE    0xE000

static  char    *yyin;

static  double  *numbers;
static  double  yylval;

static  int     nnumbers;
static  int     radix;
char    yyout[140];

static  jmp_buf fperror;

/********************************/
static int
yylex (void)
{
    int ic;
    char *y=yyin;

    while (*y && strchr (WHITESP,*y)!=NULL)
        y++;
    if (*y==0)
        return Y_EOS;              /* EOS */

    if (strchr (SELF,*y)!=NULL)
        return yyin=y+1,*y;

    for (ic=OPNUM-1; ic>=0; ic--)
        if (strncasecmp (y,operators[ic],strlen (operators[ic]))==0)
            return yyin=y+strlen (operators[ic]),Y_OPER+ic;

    if (*y>='0' && *y<='9')
    {
        yylval=0;
        if (*y=='0')
        {
            if ((y[1]|0x20) == 'x')
                yylval=strtol (y,&yyin,16);
            else if ((y[1]|0x20) == 'b')
            {
                yyin=y+2;
                while (*yyin=='0' || *yyin=='1')
                    yylval=yylval*2+*yyin++-'0';
            }
            else if (y[1] != '.')
                yylval=strtol (y,&yyin,8);
        }
        if (yyin <= y+1)
            yylval=strtod (y,&yyin);

        return Y_NUMB;
    }

    for (ic=0; ic<FNUM; ic++)
    {
        if (strncasecmp (y,functions[ic],strlen (functions[ic])))
            continue;
        y+=strlen (functions[ic]);
        while (*y && strchr (WHITESP,*y)!=NULL)
            y++;
        if (*y == '(')
            return yyin=y+1,OPENB+ic+1;
    }
    if (*y=='\'' && y[2]=='\'')
        return yyin+=3,yylval=y[1],Y_NUMB;
    if (*y==',')
    {
        y++;
        while (*y && strchr (WHITESP,*y)!=NULL)
            y++;
        yylval=*y|0x20;
        radix=yylval=='x' ? 16 : yylval=='b' ? 2 : yylval=='o' ? 8 : 10;
        yyin=y+1;
        return yylex();
    }

    return Y_ERR;
}

/********************************/

#undef R
#undef S
#undef _

#define R 1  /* reduce */
#define S 0  /* shift */
#define A(  a,b,c,d, e,f,g,h,i,j,k ) ((((((((((k*2+j)*2+i)*2+h)*2+g)*2+f)*2+e)*2+d)*2+c)*2+b)*2+a)

static unsigned short opprec_table[]= {
        /*        LOOK AHEAD       */
        /*  a ( ) e  | ^ & < + * P */
/* a */ A(  R,R,R,R, R,R,R,R,R,R,R ),    /* /\ TOKEN ON   */
/* ( */ A(  S,S,S,R, S,S,S,S,S,S,S ),    /* ][ THE TOP OF */
/* ) */ A(  R,R,R,R, R,R,R,R,R,R,R ),    /* \/ THE STACK  */
/* e */ A(  S,S,R,R, S,S,S,S,S,S,S ),

/* | */ A(  S,S,R,R, R,S,S,S,S,S,S ),    /* lowest precedence */
/* ^ */ A(  S,S,R,R, R,R,S,S,S,S,S ),
/* & */ A(  S,S,R,R, R,R,R,S,S,S,S ),
/* < */ A(  S,S,R,R, R,R,R,R,S,S,S ),
/* + */ A(  S,S,R,R, R,R,R,R,R,S,S ),
/* * */ A(  S,S,R,R, R,R,R,R,R,R,S ),
/* P */ A(  S,S,R,R, R,R,R,R,R,R,S )     /* highest precedence */
};

#undef A

#define isoper(x) ((x&0xf000)==Y_OPER)

static int getprec (unsigned w1)
{
    if (!isoper (w1))
        return w1>>12;
    return 4+precedence[w1&0xff];
}

/********************************/        /* functions */
static double mybin (double op) { radix=2; return op; }
static double myoct (double op) { radix=8; return op; }
static double mydec (double op) { radix=10; return op; }
static double myhex (double op) { radix=16; return op; }
/********************************/        /* operators */
static double myor   (double op1,double op2) {return (ltype)op1|(ltype)op2;}
static double myxor  (double op1,double op2) {return (ltype)op1^(ltype)op2;}
static double myand  (double op1,double op2) {return (ltype)op1&(ltype)op2;}
static double left   (double op1,double op2) {return (ltype)op1<<(int)op2;}
static double right  (double op1,double op2) {return (ltype)op1>>(int)op2;}
static double plus   (double op1,double op2) {return op1+op2;}
static double minus  (double op1,double op2) {return op1-op2;}
static double mult   (double op1,double op2) {return op1*op2;}
static double mydiv  (double op1,double op2) {return op1/op2;}
static double mymod  (double op1,double op2) {return (ltype)op1%(ltype)op2;}
static double power  (double op1,double op2) {return pow (op1,op2);}
/********************************/

static double (*op_table[])(double,double)= {myor,myxor,myand,left,right,
                                             plus,minus,mult,mydiv,mymod,
                                             power};

static double (*func_table[])(double)= {sinh,cosh,tanh, asin,acos,atan,
                                        sin,cos,tan, log10,log,exp,
                                        fabs,sqrt, ceil,floor,
                                        mybin,myoct,mydec,myhex};
#define STACKSIZE 100

static int yyparse (unsigned *mondat)
{
    unsigned akt=0,sp=0,ic,top;
    unsigned stack [STACKSIZE];
    double   result;

    stack[0]=EPSI;

    while (!(stack[1]>=BIGE && sp==1 && mondat[akt]==EPSI))
    {
        top=stack[sp];
        if (top>=BIGE)
        {
            if (sp>0)
                top=stack[sp-1];
            else
                top=EPSI;
        }

        if (((opprec_table[getprec (top)] >> getprec (mondat[akt]))&1)==S)
        {
            if (STACKSIZE-1==sp)      /* SHIFT */
                return -3;
            stack[++sp]=mondat[akt];
            if (mondat[akt]!=EPSI)
                akt++;
        }
        else                          /* REDUCE */
        {
            if ((stack[sp]&0xf000)==0x0)  /* E=a */
                stack[sp]+=BIGE;
            else if (sp>2 && (stack[sp-2]&0xf000)==OPENB && stack[sp]==CLOSB &&
                     stack[sp-1]>=BIGE)   /* E=function(E) */
            {
                if ((ic=stack[sp-2]&0xfff)!=0)
                {
                    result=(func_table[ic-1])(numbers[stack[sp-1] & 0xfff]);
                    stack[sp-2]=nnumbers+BIGE;
                    numbers [nnumbers++]=result;
                }
                else
                    stack[sp-2]=stack[sp-1];
                sp-=2;
            }
            else if (sp>2 && stack[sp]>=BIGE && stack[sp-2]>=BIGE &&
                     isoper (stack[sp-1])) /* E=E op E */
            {
                result=(op_table[(stack[sp-1]&0xff)])(numbers[stack[sp-2]&0xfff],
                        numbers[stack[sp]&0xfff]);
                stack[sp-2]=nnumbers+BIGE;
                numbers [nnumbers++]=result;
                sp-=2;
            }
            else if (sp>1 && stack[sp]>=BIGE && (stack[sp-1]&Y_UNAR)==Y_UNAR)
            {
                result=numbers[stack[sp]&0xfff];  /* E=unaryop E */
                if ((stack[sp-1]&0x700)==0x100)
                    result=~(ltype)result;
                else
                    result=-result;
                stack[sp-1]=nnumbers+BIGE;
                numbers [nnumbers++]=result;
                sp--;
            }
            else
                return -1;
        }
    }
    return stack[1]&0xfff;
}

/********************************/
static void 
fperrhandle (int x)
{
    #ifdef TVCompf_djgpp
    _clear87 ();           /* hmm... we need this! */
    #endif
    longjmp (fperror,-4);
}
/********************************/
int
eval (char *mit,char **out)
{
    int      inx=0,ic,jc=0;
    unsigned *mondat;
    static void  (*prevfn)(int);

    radix=10;
    *out=yyout;
    yyout[0]=0;
    nnumbers=0;
    yyin=mit;                    /* basic check */
    while ((ic=yylex())!=Y_EOS && ic!=Y_ERR)
        jc++;                    /* number of lexical elements */

    if (ic==Y_ERR)
        return -1;

    mondat=(unsigned*) alloca ((jc+1)*sizeof(unsigned));
    numbers=(double*) alloca (2*(1+jc)*sizeof(double));
    yyin=mit;

    while ((ic=yylex())!=Y_EOS)
        if (ic==Y_NUMB)
        {
            mondat[inx++]=nnumbers;
            numbers[nnumbers++]=yylval;
        }
        else if (ic=='(')
            mondat[inx++]=OPENB;
        else if (ic==')')
            mondat[inx++]=CLOSB;
        else if (ic=='~')
            mondat[inx++]=Y_UNAR+0x100;
        else if (ic==Y_OPER+OP_NEG)
            {
                if (inx==0 || (inx>0 && (mondat[inx-1]>=Y_OPER ||
                    (mondat[inx-1]&0xf000)==OPENB)))
                {
                    mondat[inx++]=Y_UNAR+0x200;
                }
                else
                    mondat[inx++]=ic;
            }
        else
            mondat[inx++]=ic;
    
    mondat[inx++]=EPSI;

    if (setjmp (fperror)==0)
        prevfn=signal (SIGFPE,fperrhandle);
    else
        return signal (SIGFPE,prevfn),-4;

    ic=yyparse (mondat);
    signal (SIGFPE,prevfn);
    if (ic<0)
        return -3;

    if (radix==10)
        sprintf (yyout,"%.10g",numbers[ic]);
    else if (radix==16)
        sprintf (yyout,"0x%" ltypes "X",(ltype)numbers[ic]);
    else if (radix==8)
        sprintf (yyout,"0%" ltypes "o",(ltype)numbers[ic]);
    else
    {
        unsigned ltype lc=(ltype) numbers[ic];
        yyout[0]='0';
        yyout[1]='b';
        for (ic=0; lc; ic++,lc>>=1)
            yyout[65+2+ic]=(lc&1)+'0';
        if (ic==0)
            yyout[65+2+ic++]='0';
        for (jc=0; jc<ic; jc++)
            yyout[jc+2]=yyout[65+2+ic-jc-1];
        yyout[jc+2]=0;
    }
    return 0;
}

/********************************/
#ifdef TEST
int main (int argc,char **argv)
{
    int ic;
    char *y;
    if (argc < 2)
        return printf ("Usage: %s expression-list\n",argv[0]);
    while (--argc)
        if ((ic=eval (argv[argc],&y)) < 0)
            printf ("Error in expression: %d\n",ic);
        else
            printf("Result: %s\n",yyout);
    return 0;
}
#endif

#endif // HAVE_CALCULATOR

