sub BALUGClosePage
{
 my ($hash)=@_;

 print FilOut "<p><hr><div>${$hash}{'premaster'} <i><a href=\"mailto:salvador@balug.org.ar\">webmaster</a></i>.</div><p>\n";
 print FilOut "${$hash}{'prelogos'}\n";
 print FilOut "<div class=cent>
<table class=cent><tr>
 <td>
   <a class=invis href=\"http://validator.w3.org/check?uri=http://www.balug.org.ar/$NameHTMLFile;outline=1\">
   <img src=\"http://www.w3.org/Icons/valid-html401\" alt=\"Valid HTML 4.01!\" height=31 width=88></a>
   <br>
   <a class=invis href=\"http://jigsaw.w3.org/css-validator/validator?uri=http://www.balug.org.ar/$NameHTMLFile\">
   <img src=\"http://jigsaw.w3.org/css-validator/images/vcss\" alt=\"Valid CSS!\" height=31 width=88></a>
 </td>
 <td><img src=\"/imagenes/gimp2.gif\" alt=GIMP HEIGHT=96 WIDTH=144></td>
 <td>
   <a class=invis href=\"http://www.anybrowser.org/campaign/\">
   <img src=\"/imagenes/bvucs-n1.jpg\" alt=\"Any browser!\" height=30 width=100></a>
   <br>
   <a class=invis href=\"http://www.opencontent.org/\">
   <img src=\"/imagenes/button-takeone.jpg\" alt=\"Open Content!\" height=33 width=85></a>
 </td>
</tr></table>
<p>
</div>\n";

 return 1;
}
1;
