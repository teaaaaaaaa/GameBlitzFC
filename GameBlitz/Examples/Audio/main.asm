//audio playback assembly code
<audio.blitz>

//initialization
#jp ($3)
#nop
#nop
#ld ABCD, [$0]
#ld EFGH, [6]
#set (EFGH), ABCD
//prepare registers
$2:
#ld EF, [0]
#ld GH, [0]
#set (GH), EF
$4:
#ld EFGH, [6]
#set ABCD, (EFGH)
#ld GH, [0]
#set EF, (GH)
#add ABCD, EF
#inc (GH)
#inc (GH)
#ld EF, [0xFF90]
#set GH, (ABCD)
#set (EF), GH
#ld EFGH, [6]
#set ABCD, (EFGH)
#ld GH, [0]
#set EF, (GH)
#add ABCD, EF
#inc (GH)
#inc (GH)
#ld EF, [0xFF8E]
#set GH, (ABCD)
#set (EF), GH
//end of loop
#jp ($4)
$0:
*Include [C:\GameBlitz\Examples\Audio\PCM\PCMData.raw]
$3:
#ld EFGH, [6]
#set ABCD, (EFGH)
#ld GH, [1600]
#add ABCD, GH
#ld EFGH, [0]
#set (EF), GH
#ld EFGH, [6]
#set (EFGH), ABCD
#halt
#jp ($3)
