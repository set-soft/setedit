/* Copyright (C) 1996,1997,1998,1999,2000 by Salvador E. Tropea (SET),
   see copyrigh file for details */
#ifdef __cplusplus
extern "C" {
#endif
#define TrigTableSize  (360*2)
#define cos_table (sin_table+TrigTableSize/4)
#define tsinConst (1/255.0)

extern int sin_table[];

void MA1_CreateLookUp(void);

#ifndef PI
#define PI 3.14159265358979323846
#endif

/**[txh]*****

 Function: MA1_WrapAngleG (macro)
 Prototype: MA1_WrapAngleG(a)
 Module: Math 1
 Include: math1.h
 Description:
 If the angle is greater than 360 degrees it decrease the value in 360.@p
 To be used after an increment of less than 360 degrees.@p

************/
#define MA1_WrapAngleG(a)  if (a>=TrigTableSize) a-=TrigTableSize
/**[txh]*****

 Function: MA1_WrapAngleB (macro)
 Prototype: MA1_WrapAngleB(a)
 Description:
 If the angle is less than 0 degrees it increase the value in 360.@p
 To be used after an decrement of less than 360 degrees.@p

************/
#define MA1_WrapAngleB(a)  if (a<0) a+=TrigTableSize
/**[txh]*****

 Function: MA1_WrapAngleG2 (macro)
 Prototype: MA1_WrapAngleG2(a)
 Description:
 If the angle is greater than 720 degrees it decrease the value in 720.@p
 To be used after an increment of less than 720 degrees.@p

************/
#define MA1_WrapAngleG2(a) if (a>=TrigTableSize*2) a-=TrigTableSize*2
/**[txh]*****

 Function: MA1_WrapAngleB2 (macro)
 Prototype: MA1_WrapAngleB2(a)
 Description:
 If the angle is less than 0 degrees it increase the value in 720.@p
 To be used after an decrement of less than 720 degrees.@p

************/
#define MA1_WrapAngleB2(a) if (a<0) a+=TrigTableSize*2
/**[txh]*****

 Function: MA1_FromDegrees (macro)
 Prototype: MA1_FromDegrees(a)
 Description:
 Converts A from degrees to a suitable table for the look-up table.@p
 A must be in [0,720] degrees range.@p

 Return:
 The converted value.

************/
#define MA1_FromDegrees(a) ((a)*(TrigTableSize/360))
/**[txh]*****

 Function: MA1_FromRad (macro)
 Prototype: MA1_FromRad(a)
 Description:
 Converts A from radians to a suitable table for the look-up table.@p
 A must be in [0,720] degrees range.@p

 Return:
 The converted value.

************/
#define MA1_FromRad(a)     ((a)*TrigTableSize/(2*PI))
/**[txh]*****

 Function: MA1_IncAngRad (macro)
 Prototype: MA1_IncAngRad(a,b)
 Description:
 Adds B to A and then wraps the value. Use it to increment angles safetly.@p
 B is in radians, the convertion is made with macros so if you provide a
constant there is no penalty.@p
 B must be less than 360 degrees.@p

************/
#define MA1_IncAngRad(a,b) a+=MA1_FromRad(b); MA1_WrapAngleG(a)
/**[txh]*****

 Function: MA1_IncAng (macro)
 Prototype: MA1_IncAng(a,b)
 Description:
 Adds B to A and then wraps the value. Use it to increment angles safetly.@p
 B is the units of the array. @x{MA1_FromRad (macro)}. @x{MA1_FromDegrees (macro)}.@p
 B must be less than 360 degrees.@p

************/
#define MA1_IncAng(a,b)    a+=b; MA1_WrapAngleG(a)
/**[txh]*****

 Function: MA1_DecAng (macro)
 Prototype: MA1_DecAng(a,b)
 Description:
 Substracts B from A and then wraps the value. Use it to decrement angles
safetly.@p
 B is the units of the array. @x{MA1_FromRad (macro)}. @x{MA1_FromDegrees (macro)}.@p
 B must be less than 360 degrees.@p

************/
#define MA1_DecAng(a,b)    a-=b; MA1_WrapAngleB(a)
/**[txh]*****

 Function: MA1_DegToRad (macro)
 Prototype: MA1_DegToRad(a)
 Description:
 Converts A degrees to radians.

 Return:
 The converted value in double format.

************/
#define MA1_DegToRad(a)    (a*PI/180)

/**[txh]*****

 Description:
 Calculates the sin of V and returns a value in the [-1,1] range.@p
 Is inline. V is in the units of the array.@p

 Return:
 A double.

************/
extern inline double tsin(int v)
{
 return sin_table[v]*tsinConst;
}

/**[txh]*****

 Description:
 Calculates the cos of V and returns a value in the [-1,1] range.@p
 Is inline. V is in the units of the array.@p

 Return:
 A double.

************/
extern inline double tcos(int v)
{
 return cos_table[v]*tsinConst;
}

/**[txh]*****

 Description:
 Is a wrapper to avoid the direct use of the array.@p
 Is inline an is just return sin_table[v];

************/
extern inline int t255sin(int v)
{
 return sin_table[v];
}

/**[txh]*****

 Description:
 Is a wrapper to avoid the direct use of the array.@p
 Is inline an is just return cos_table[v];

************/
extern inline int t255cos(int v)
{
 return cos_table[v];
}
#ifdef __cplusplus
}
#endif

