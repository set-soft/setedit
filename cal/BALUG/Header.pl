sub BALUGHeader
{
 my ($hash)=@_;
 my ($oldKeys);

 $oldKeys=${$hash}{'keywords'};
 ${$hash}{'keywords'}='Linux,Argentina,linux,argentina,Buenos Aires,buenos aires,BALUG,lug,users group,usuarios';
 ${$hash}{'keywords'}.=','.$oldKeys if $oldKeys;
 ${$hash}{'css'}='balug.css,black.css:Black guru' unless ${$hash}{'css'};
 ${$hash}{'icon'}='imagenes/micro-tux.png' unless ${$hash}{'icon'};

 return 1;
}
1;
