sub BaseHeader
{
 my ($hash)=@_;
 my ($css,@cssa);

 print FilOut "<!DOCTYPE HTML PUBLIC \"${$hash}{'doctype'}\">\n" if (${$hash}{'doctype'});
 print FilOut "<html>\n<head>\n";
 print FilOut "<title>${$hash}{'title'}</title>\n" if (${$hash}{'title'});
 print FilOut "<meta name=\"description\" content=\"${$hash}{'description'}\">\n" if (${$hash}{'description'});
 print FilOut "<meta name=\"keywords\" content=\"${$hash}{'keywords'}\">\n" if (${$hash}{'keywords'});
 print FilOut "<meta name=\"resource-type\" content=\"${$hash}{'resource-type'}\">\n" if ${$hash}{'resource-type'};
 print FilOut "<meta name=\"distribution\" content=\"${$hash}{'distribution'}\">\n" if ${$hash}{'distribution'};
 print FilOut "<meta name=\"Author\" content=\"${$hash}{'Author'}\">\n" if ${$hash}{'Author'};
 print FilOut "<meta http-equiv=\"Content-Style-Type\" content=\"${$hash}{'Content-Style-Type'}\">\n" if ${$hash}{'Content-Style-Type'};
 print FilOut "<meta http-equiv=\"Content-Type\" content=\"${$hash}{'Content-Type'}\">\n" if ${$hash}{'Content-Type'};
 $css=${$hash}{'css'};
 if ($css)
   {
    @cssa=split(/,/,$css);
    print FilOut "<link rel=\"stylesheet\" type=\"text/css\" href=\"@cssa[0]\">\n";
    shift @cssa;
    foreach $css (@cssa)
      {
       $css=~/([^:]*):(.*)/;
       print FilOut "<link rel=\"alternate stylesheet\" type=\"text/css\" href=\"$1\" title=\"$2\">\n";
      }
   }
 print FilOut "<link rel=\"shortcut icon\" href=\"${$hash}{'icon'}\">\n" if (${$hash}{'icon'});
 print FilOut "</head>\n<body>\n";

 return 0;
}

1;
