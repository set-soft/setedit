sub CommonHeader
{
 my ($hash)=@_;

 ${$hash}{'doctype'}='-//W3C//DTD HTML 4.01//EN' unless ${$hash}{'doctype'};
 ${$hash}{'resource-type'}='document' unless ${$hash}{'resource-type'};
 ${$hash}{'distribution'}='global' unless ${$hash}{'distribution'};
 ${$hash}{'Content-Type'}='text/html; charset=ISO-8859-1' unless ${$hash}{'Content-Type'};
 ${$hash}{'Content-Style-Type'}='text/css' unless ${$hash}{'Content-Style-Type'};

 return 1;
}

1;
