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
CommandLine=makeinfo --no-validate --fill-column 78 -I ~90 -o ~1.info ~0
Name=Info (.info) file format
CommandLine=makeinfo --no-validate --fill-column 78 -I ~90 -Dhtml -o ~1.html ~0
Name=HTML file format
CommandLine=makeinfo --no-validate --fill-column 78 -I ~90 -Dtext -o ~1.txt ~0
Name=Text (.txt) file format

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
EndOfPar=@paragraph{}
#
# @* = break line
#
BreakLine=@*
#
# @{value} is the special cross ref.
# ~0 Is the visible name of a reference
# ~1 Is the real name of a reference
#
CrossRef=@xref{~1~~0{,~0~}}
#
# What we must get from a @@ sequence
#
Double@=@@

[DefinedCommands]
#
# The format is @<name>{parameters ...}
#
subtitle=@_subheading{~0}
pre=@example{}
/pre=@end_example

#
# This section says how to translate ASCIIs
#
[ASCIIConvert]
†=@aacute{}
Ç=@eacute{}
°=@iacute{}
¢=@oacute{}
£=@uacute{}
§=@ntilde{}
•=@Ntilde{}
≠=@exclamdown{}
®=@questiondown{}
Ñ=@auml{}
â=@euml{}
ã=@iuml{}
î=@ouml{}
Å=@uuml{}
ö=@Uuml{}
Ö=@agrave{}
ä=@egrave{}
ç=@igrave{}
ï=@ograve{}
ó=@ugrave{}
{=@{
}=@}
<=@lessthan{}
>=@greaterthan{}

#
# Use ~number to use one variable
# Use ~~number{} for conditional, all the code inside {} will become
# conditional
#
[GenNode]
@node{~92,~90,~~1{~1~},~~3{~3~}}
@chapter ~0 (~90 ~91)

@_subheading{Syntax}

~~2{@example
# This line is a comment, but the next is code
 #include @lessthan{}~2@greaterthan{}
@end_example~}
~~4{@example
 ~4;
@end_example~}
~~1{
@_subheading{Member of the class:}
~1
~}

~~5{
@_subheading{Description}

~5
~}
~~6{
@_subheading{Return Value}

~6
~}
~~7{
@_subheading{Example}

~7
~}
@separatenode{}
@c ----------------------------------------------------------------------

[GenMenu]
Start=@menu
#
# ~1 is the visible name
# ~2 is the name of the node
#
Entry=@mitem{~1,~2}
End=@end_menu{}

#
# ~1  is the name of the association
# ~2  is the menu for it
#
[GenAssoMain]
@node{~1,Top}
@section ~1

~2
#
# ~1  is the name of the association
# ~2  is the name without the distinguish
# ~3  is the comment for it
# ~4  is the menu for it
#
[GenAssoRest]
@node{~1,Top}
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

@ifset html
@include txhgen-i.htm
@html
@end ifset

@ifset text
@include txhgen-i.txt
@end ifset

@ifclear text
@ifclear html
@include txhgen-i.txi
@end ifclear
@end ifclear

@ifinfo
~52
@end ifinfo

@c  This title page illustrates only one of the
@c  two methods of forming a title page.

@titlepage
@title ~53
@author ~54

@c  The following two commands
@c  start the copyright page.
@page
@vskip 0pt plus 1filll
~55
@end titlepage

@node{Top}

@ifset html
@htmltitle{~51}
@end ifset

@ifinfo
~56
@end ifinfo

~1

@node{~2,Top}
@section ~2

~3

~4

~5

@contents
@bye

