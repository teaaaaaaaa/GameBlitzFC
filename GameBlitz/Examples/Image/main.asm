//15bpp image assembly code

//initialization
#jp ($2)
#nop
#nop
#jp ($3)
$2:
//prepare registers
#ld AB, [0xFF82]
#ld H, [240]
#set (AB), H
#inc AB
#ld H, [200]
#set (AB), H
#ld ABCD, [$0]
#ld EFGH, [0]
#copy (ABCD), (EFGH)
//set mode
#ld AB, [0xFF94]
#ld H, [1]
#set (AB), H
//prepare
#ld ABCD, [$1]
#ld EFGH, [0x010200]
#copy (ABCD), (EFGH)
$4:
#halt
#jp ($2)
$0:
*Include [C:\GameBlitz\Examples\Image\convertedImage\PLN0.bin]
$1:
*Include [C:\GameBlitz\Examples\Image\convertedImage\PLN1.bin]
$3:
#halt
#jp ($3)
