# -*-sdg-*-
# This file sets the behavior of the txh generator
#
# Lines starting with # or spaces are skiped except in strings or in [Generate]
# Be carefull with [ it delimits sections!
#

[Configuration]
#
# The CommandLine indicates what postprocess program is called.
# ~0 is the name of the input file.
# ~1 is the name of the output file.
# ~90 The path for includes
#
#   This file can generate .info, .html and .txt files so 3 command
# lines are provided.
#
CommandLine="makeinfo --no-validate -I ~90 -E ~1 -o ~1.tmp ~0;tex ~1;texindex ~1.??;tex ~1;dvips -o ~1.ps ~1.dvi"
Name="DVI and Postscript format with Tex"

[Delimiters]
# Up to 11 characters
SectionStart=/**[txh]**
# Up to 11 characters
SectionEnd=*********/

[Variables]
#
# Up to 16 definitions
#
# Codes for the behavior of the definitions:
# 1 Normal, put the content if found.
# 2 Repeat, use the last value found in the file, ~no is an exeption, ~clear stops
# 3 If not found replace by the prototype.
# 4 If not found replace by the class.
# 5 If not found replace by the name of the function
# 6 It disables the node generation for this comment. The variable is
#   stored in the first variable that have associations and is in the comment.
#
# Additionally there are 2 special variables:
# 90 Name of the file
# 91 Line number of the end of the comment
# 92 Name of the var 0 in the format: ~0 ~~Distinguish{(~Distinguish)~} (node name)
#
# 0 The first variable is the main index variable
#
AddDefinition=Function,5
# 1
AddDefinition=Class,4
# 2
AddDefinition=Include,2
# 3
AddDefinition=Module,2
# 4
AddDefinition=Prototype,3
# 5
AddDefinition=Description,1
# 6
AddDefinition=Return,1
# 7
AddDefinition=Example,1
# 8
AddDefinition=Comments,6
#
# It says what variable is added to distinguish between 2 vars 0 that are equal
#
Distinguish=1

[Associations]
#
# Up to 8 associations
#
# The associations are between the 0 variable and another variable.
#
# Name in main menu, node, variable, optional to add to each node
#
AddAssoc=List by classes,Classes,1,(class)
AddAssoc=List by modules,Modules,3
AddAssoc=List by files,Files,90

[Replace]
#
#   All must be delimited by ", use as many as you need. They can be used
# in the GenMain section.
#
#   Be carreful the strings are passed by the macro expander.
#
# Name of the generated file
Constant="documen.info"

# Name of the documentation manual, title in HTML
Constant="My documentation"

# Copyright for info files
Constant="This file documents the functions of my library
@p
Copyright 1997
@p
Permission is granted to copy this documentation for free.
@p"

# Name of the printed manual
Constant="My documentation"

# Author
Constant="SET"

# Copyright page
Constant="Copyright @copyright@<{}> 1997
@p
Published by SET
@p
Permission is granted to copy this documentation for free."

# Description for info files
Constant="This document describes the functions of my library
@p
This document applies to version 0.0.1
of the program named A Real World
@p"

[Commands]
#
# @p = end of paragraph
#
EndOfPar="@sp 1"
#
# @* = break line
#
BreakLine="@*"
#
# @{value} is the special cross ref.
# ~0 Is the visible name of a reference
# ~1 Is the real name of a reference
#
CrossRef="@ref{~1~~0{,~0~}}"
#
# What we must get from a @@ sequence
#
Double@=@@

[DefinedCommands]
#
# The format is @<name>{parameters ...}
#
subtitle="@subheading ~0"
pre="@example"
/pre="@end example"

#
# This section says how to translate ASCIIs
#
[ASCIIConvert]
†="@'a"
Ç="@'e"
°="@'i"
¢="@'o"
£="@'u"
§=@~n
•=@~N
≠=@exclamdown{}
®=@questiondown{}
Ñ="@\"a"
â="@\"e"
ã="@\"i"
î="@\"o"
Å="@\"u"
ö="@\"U"
Ö=@`a
ä=@`e
ç=@`i
ï=@`o
ó=@`u
{=@{
}=@}

#
# Use ~number to use one variable
# Use ~~number{} for conditional, all the code inside {} will become
# conditional
#
[GenNode]
@node ~92 ~~3{,~3~}
@section ~0 (~90 ~91)

@subheading Syntax

~~2{@example
# This line is a comment, but the next is code
 #include <~2>
@end example~}
~(if ~4 (print '@example\n' (cutCprot ~4 64 1) ';\n@end example'))
~~1{
@subheading Member of the class:
~1
~}

~~5{
@subheading Description

~5
~}
~~6{
@subheading Return Value

~6
~}
~~7{
@subheading Example

~7
~}
@c ----------------------------------------------------------------------

[GenMenu]
Start="@itemize @bullet"
#
# ~1 is the visible name
# ~2 is the name of the node
#
Entry="@item @ref{~2,~1}."
End="@end itemize"

#
# ~1  is the name of the association
# ~2  is the menu for it
#
[GenAssoMain]
@node ~1,Top
@chapter ~1

~2
#
# ~1  is the name of the association
# ~2  is the name without the distinguish
# ~3  is the comment for it
# ~4  is the menu for it
#
[GenAssoRest]
@node ~1,Top
@section ~2

~3

~4

#
# ~1 Main menu
# ~2 Name of the function list node
# ~3 Menu for all the functions
# ~4 All the associations code
# ~5 All the function nodes
# ~50+ Values from section Replace
#
[GenMain]
\input texinfo   @c -*-texinfo-*-
@c %%**start of header
@setfilename ~50
@settitle ~51
@setchapternewpage odd
@c %%**end of header

@finalout

@titlepage
@title ~53
@author ~54

@page
@vskip 0pt plus 1filll
~55
@end titlepage

@ifinfo
@node Top
@top Main list of chapters

~1
@end ifinfo

@node ~2,Top
@chapter ~2

~3

~5

~4

@contents
@bye

