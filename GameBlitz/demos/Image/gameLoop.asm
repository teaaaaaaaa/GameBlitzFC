<SundownMayhem.blitz>
//Sundown mayhem code

//initialization
#jp ($2)
#nop
#nop
#jp ($3)
$2:
//prepare registers
#ld ABCD, [0x010002]
#ld H, [255]
#set (ABCD), H
#ld ABCD, [$0]
#ld EFGH, [0x010080]
#copy (ABCD), (EFGH)
#copy (ABCD), (EFGH)
#copy (ABCD), (EFGH)
//prepare
#ld EFGH, [0x00010280]
#ld ABCD, [0x010002]
#ld H, [64]
#set (ABCD), H
#ld H, [0]
#ld ABCD, [$1]
#ld EFGH, [0x010280]
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
#copyT (ABCD), (EFGH)
$4:
#halt
#jp ($2)
$7:
#set F, E
#AND F, G
#cp G, F
#SL E
#ret
$0:
*Include [C:\GameBlitzNemulator\demos\Image\convertedImage\palette.bin]
$1:
*Include [C:\GameBlitzNemulator\demos\Image\convertedImage\image.bin]
$5:
*Include [C:\GameBlitzNemulator\demos\Image\Audio\alexSoundFont.bin]
$3:
#halt
#jp ($3)
