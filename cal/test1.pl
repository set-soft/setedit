#!/usr/bin/perl
require 'cal.pl';

# Set BALUG skin with more priority than Base
@SkinPriority=('BALUG',@SkinPriority);

CreateHTML('index.html');

Insert('Header',
       {'title' => 'BALUG',
        'description' => 'BALUG home page',
        'keywords' => 'setedit',
        'Author' => 'Salvador E. Tropea, salvador@balug.org.ar'
       });

$post="<p>Texto(?) por: <a href=\"mailto:salvador@inti.gov.ar\">SET</a><br>
Web hosting by <a href=\"mailto:mdaniel@csl.net.ar\">MDaniel</a><br>
Created with <a href=\"http://setedit.sourceforge.net/\">SETEDIT</a>,
bueno che! si hay tarados que lo hacen con el notepad
porque no puedo con mi editor que tiene syntax highlight para HTML ;-)
[sin ánimos de ofender solo que hay cosas mucho mejores que notepad]</p>";

Insert('ClosePage',
       {'premaster' => '<i>Copywrong (w) 1998-2002 by Salvador E. Tropea, cualquier uso legal será penado por la ley</i>',
        'prelogos' => $post
       });
0;

