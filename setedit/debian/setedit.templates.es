Template: setedit/central_desktop_file
Type: select
Choices: Create a desktop file in each directory, Use a central desktop file
Choices-es: Crear archivos de escritorio en cada directorio, Usar un único archivo de escritorio global
Default: Create a desktop file in each directory
Description: Desktop file policy selection?
 The editor stores configuration options in files called desktop files.
 These files also store information about what files are opened and the
 size, position, etc. of the windows.
 .
 You can have only one global file for this or one in each directory you
 use the editor.
Description-es: ¿Comportamiento de archivos de escritorio predeterminado?
 El editor guarda las opciones de configuración en archivos llamados
 "archivos de escritorio".
 Éstos archivos también guardan información acerca de cuales archivos están
 abiertos y el tamaño, posición, etc. de las ventanas.
 .
 Se puede tener un solo archivo global para ésto o un archivo por cada
 directorio donde se utilice el editor.

Template: setedit/tabs_for_indent
Type: select
Choices: Spaces, Tabs
Choices-es: Espacios, Tabuladores
Default: Spaces
Description: Indent by spaces or tabs?
 The editor is set by default to indent text using spaces. To configure the
 editor to use tabs more than one option (in the editor, not here) must be
 selected (which if you answer to use tabs here that will be done for you).
Description-es: ¿Indentar por espacios o tabuladores?
 El editor predeterminadamente viene seteado para indentar texto utilizando
 espacios. Para configurar al editor para que use tabuladores, mas de una
 opción (en el editor, no aquí) tiene que setearse (que en el caso de que Ud.
 responda que desea utilizar tabuladores aquí, ese seteo será realizado para
 Ud.).

Template: setedit/create_backups
Type: select
Choices: Yes, No
Choices-es: Si, No
Default: Yes
Description: Do you want to create backups of modified files?
 Each time the editor stores a modified file to disk a backup file can be
 created in case you want to revert the changes. This is specially useful
 when you already exited the editor and hence undo option isn't available.
Description-es: ¿Desea que se creen respaldos de los archivos modificados?
 Cada vez que el editor grabe un archivo modificado al disco, un archivo de
 respaldo puede ser creado en caso de que se quiera revertir los cambios.
 Ésto es especialmente útil cuando Ud. salió del editor y entonces no tiene
 posibilidad de activar la opción de deshacer los cambios.
