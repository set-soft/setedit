-----------------------------------------------------------------------------
-- Copyright (c) 2005 by Salvador E. Tropea                                --
-- Distributed under the terms of the GPL license.                         --
--                                                                         --
-- This file is used to test the VHDL syntax highlight.                    --
-----------------------------------------------------------------------------

-- Bit string literals
OK:
B%111_110%
B"111_000"
B""
O"1127"
X"aF45"

Wrong:
-- Not closed
B"11
-- Digit out of base
B%11112%
-- Closed with wrong delimiter (no match)
B"111%
-- Two consecutive _
B%11__0%
-- Digit out of base
O%181%
-- Digit out of base
X%aFg45%

-- Abstract literals (numbers)

OK:
11.11e11
125629
111.32
1_6#1.1af_2#E+1
16#1.1af_2#E12
1_000_000
15#7E#E+1
11.0e2
1E9
1.0e-9
10#110.11#E-2

Wrong:
-- Negative exponent for integer
11_1e-2
-- Digit out of base
112a
-- Incomplete, needs at least 1 digit
111.
-- Two consecutive _
110__20
-- Incomplete, needs #
10#12
-- Wrong base (>16)
17#10#
-- Digit out of base
15#EF#
-- Negative exponent for integer
10#10#E-2


-- String literals
Ok:
"Hello"outside
""outside
"inside""inside"outside
%inside%outside

Wrong:
"
"""
"aaaa

-- Character literals
Always ok:
'a'outside
signal'attribute
'''outside

