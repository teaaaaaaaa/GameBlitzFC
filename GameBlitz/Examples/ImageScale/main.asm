//256-color image scaling assembly code

//initialization
#jp ($2)
#nop
#nop
#jp ($3)
$2:
//prepare registers
#ld AB, [0xFF82]
#ld H, [255]
#set (AB), H
#dec AB
#ld H, [0]
#set (AB), H
#inc AB
#inc AB
#ld H, [3]
#set (AB), H
#ld ABCD, [$0]
#ld EFGH, [0x010000]
#copy (ABCD), (EFGH)
//enable horizontal scaling
#ld AB, [0xFF81]
#ld H, [255]
#set (AB), H
#inc AB
//set data copy size
#ld H, [240]
#set (AB), H
#inc AB
#ld H, [200]
#set (AB), H
//controller input
#ld AB, [0xFF92]
#set E, (AB)
#ld G, [1]
#ld ABCD, [$7]
#call ABCD
#ld ABCD, [$6]
#jp X, (ABCD)
#ld ABCD, [$7]
#call ABCD
#ld ABCD, [$5]
#jp X, (ABCD)
#jp ($4)
$6:
#ld EF, [0]
#inc (EF)
#jp ($4)
$5:
#ld EF, [0]
#dec (EF)
$4:
//load horizontal width
#ld AB, [0xFF84]
#ld EF, [0]
#set GH, (EF)
#set (AB), GH
#inc AB
#inc AB
#set (AB), GH
#ld ABCD, [$1]
#ld EFGH, [0x010200]
#copy (ABCD), (EFGH)
#halt
#jp ($2)
$7:
#set F, E
#AND F, G
#cp G, F
#SL E
#ret
$0:
*Include [C:\GameBlitz\Examples\ImageScale\convertedImage\palette.bin]
$1:
*Include [C:\GameBlitz\Examples\ImageScale\convertedImage\image.bin]
$3:
#halt
#jp ($3)
