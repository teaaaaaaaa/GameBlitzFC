//palette cycling assembly code

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
//cycle palettes
#ld G, [0]
#set H, (G)
#ld E, [16]
#add H, E
#set (G), H
$4:
#ld ABCD, [$0]
#set EF, (G)
#SR EF
#add ABCD, EF
#ld EFGH, [0x010000]
#copy (ABCD), (EFGH)
//set data copy size
#ld AB, [0xFF82]
#ld H, [240]
#set (AB), H
#inc AB
#ld H, [200]
#set (AB), H
//copy data
#ld ABCD, [$1]
#ld EFGH, [0x010200]
#copy (ABCD), (EFGH)
$5:
#halt
#jp ($2)
$0:
*Include [C:\GameBlitz\Examples\paletteCycle\convertedImage\paletteArt.bin]
*Include [C:\GameBlitz\Examples\paletteCycle\convertedImage\paletteArt.bin]
$1:
*Include [C:\GameBlitz\Examples\paletteCycle\convertedImage\paletteTunnel.data]
$3:
#halt
#jp ($3)
