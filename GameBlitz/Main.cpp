#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include "tinyfiledialogs.cpp"
#include "Memory.cpp"
#define WindowSizeX 240
#define WindowSizeY 200
#define opcodeBits 0b0000000000111111
#define register1Bits 0b0000000111000000
#define register2Bits 0b0000111000000000
#define register1SizeBits 0b0011000000000000
#define register2SizeBits 0b1100000000000000
#define controlBits 0b1100000000000000
#define bit11 0b0000100000000000
#define bit9 0b0000001000000000
#include "portaudio.h"
#define FRAMES_PER_BUFFER   (512)
#define NUM_CHANNELS        (1)
#define SAMPLE_FORMAT       (paInt32)
#define SAMPLE_RATE       (30720)

;unsigned char X;
;unsigned short stackPointer;
;unsigned char fade;
;unsigned int programPointer;
;unsigned int CPU1programPointer;
;unsigned int CPU2programPointer;
;int i;
;unsigned short i16;
;unsigned char i8;
;int i2;
;unsigned short getRGB;
;unsigned char getPixel;
;unsigned char getRGBLOW,getRGBHIGH;
;int test;
;unsigned char isExecuting;
;unsigned short getInstruction;
;unsigned char opcode;
;unsigned int cycleCount;
;unsigned char getRegister;
;unsigned int getRegisterValue;
;unsigned char registerOffset;
;unsigned char registerSize;
;unsigned char registerShift;
;unsigned int registerMask;
;unsigned int upperRegisters;
;unsigned int lowerRegisters;
;unsigned int CPU1upperRegisters;
;unsigned int CPU1lowerRegisters;
;unsigned int CPU2upperRegisters;
;unsigned int CPU2lowerRegisters;
;unsigned char getRegister2;
;unsigned char registerSize2;
;unsigned int registerMask2;
;unsigned int getRegisterValue2;
;unsigned int copyAddress1;
;unsigned int copyAddress2;
;unsigned char copySize;
;unsigned char copyDataOffset;
;unsigned int addressJump;
;signed int signedRegister;
;signed short signedRegister16;
;signed char signedRegister8;
;unsigned int maxCycles;
;unsigned char getBit;
;unsigned char OperationFlag;
;unsigned int getRegisterValueDIV;
;unsigned short getScale;
;unsigned short newAudioData;
;unsigned char isAudio;
;unsigned char temporarydata8;
;unsigned short temporarydata16;
;unsigned int temporarydata;
;unsigned int audioBufferIndex;
;unsigned char clipLeft;
;unsigned char clipUp;
;unsigned char getRED,getGREEN,getBLUE;
;unsigned char isInsIncrement;
;unsigned int storeRegisterResult;
;unsigned char isDebugOpen;
;unsigned short imageScaleOffset;
;unsigned int imageScaleOffsetOrigin;
;unsigned int imageScaleOffsetYOrigin;
;unsigned int debugOffset;
;unsigned short windowTimer;
;unsigned int maxDataCopy;
;unsigned int bandwidth;
;unsigned short audioCounter;
;unsigned char isError;
;unsigned char grayscaleMode;
;unsigned char APFlag;
;int debugWindowID; // To store the debug window ID
unsigned char isCPU;
const size_t MAX_FILE_SIZE = 16000000;
signed short audioBuffer [800];
signed short audioBuffer2 [800];
signed int audioInBuffer [1200];
unsigned short audioCounter2;
unsigned short audioCounter3;
unsigned int samplePointer;
unsigned short audioPlaybackOffset;
unsigned int sampleCounter;
unsigned char audioBufferToggle;
unsigned int imagePointTopLeft;
unsigned int imagePoint1;
unsigned int imagePoint2;
unsigned int imagePoint3;
unsigned int dynamicImagePoint;
unsigned char XOrigin;
unsigned char YOrigin;
unsigned int YScaleCounter;
unsigned short YScaleCounter2;
unsigned short YScale = 255;
unsigned char YDataSize = 200;
unsigned char currentRow;
unsigned char graphicsMode;
unsigned char absoluteWidth;
bool keyStates[256] = { false }; // Assuming ASCII keys

void keyPressed(unsigned char key, int x, int y) {
    keyStates[key] = true;
}

void keyReleased(unsigned char key, int x, int y) {
    keyStates[key] = false;
}

GLuint textureID;
std::vector<unsigned char> fileData;
std::streamsize fileSize;
PaStream* stream;

//registers: a,b,c,d,e,f,g,h

const unsigned char instructionTypes[] = {
        0b01000000,
        0b00000001,
        0b00000001,
        0b00000011,
        0b00100001,
        0b00100101,
        0b00100001,
        0b00001111,
        0b00001111,
        0b00000001,
        0b00000001,
        0b00000001,
        0b00000001,
        0b00000011,
        0b00000011,
        0b00000011,
        0b00000001,
        0b00000001,
        0b01000000,
        0b00000001,
        0b00000011,
        0b00001011,
        0b00000001,
        0b00000001,
        0b00010000,
        0b10010001,
        0b00000011,
        0b00000011,
        0b00000011,
        0b00000011,
        0b00000111,
        0b00000101,
        0b00000101,
        0b01000000,
        0b01000000,
    };
    
const unsigned char errorFlagTable[] =
{
0,0,0,0,0,0,0,0,
0,0,0,0,1,1,1,1,
0,0,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,
};

const char* instructionNames[] = {
        "#nop",
        "#inc",
        "#dec",
        "#cp",
        "#jr",
        "#jp",
        "#jru",
        "#copy",
        "#copyT",
        "#SL",
        "#SR",
        "#RL",
        "#RR",
        "#OR",
        "#XOR",
        "#AND",
        "#NOT",
        "#call",
        "#ret",
        "#ld sp",
        "#set",
        "#set",
        "#push",
        "#pop",
        "#jp",
        "#ld",
        "#sub",
        "#add",
        "#subCLIP ",
        "#addCLIP ",
        "#set",
        "#inc (REGAddress)",
        "#dec (REGAddress)",
        "#halt",
        "#AOUT",
    };
    
const char* registerNames[] =
{
" A", " B", " C", " D", " E", " F", " G", " H",
" AB", " CD", " EF", " GH", " [invalid]", " [invalid]", " [invalid]", " [invalid]",
" ABCD", " EFGH", " [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]",
" [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]", " [invalid]",
};

unsigned char instructionFlagTable[] =
{
1,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
1,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
0,
1,
1
};
    
const unsigned int registerMaskTable[] =
{
0b11111111,
0b1111111100000000,
0b111111110000000000000000,
0b11111111000000000000000000000000,
0b11111111,
0b1111111100000000,
0b111111110000000000000000,
0b11111111000000000000000000000000,
0b1111111111111111,
0b11111111111111110000000000000000,
0b1111111111111111,
0b11111111111111110000000000000000,
0,0,0,0,
0b11111111111111111111111111111111,
0b11111111111111111111111111111111,
0,0,0,0,0,0,
};

const unsigned char registerShiftTable[] =
{
0,
8,
16,
24,
32,
40,
48,
56,
0,
16,
32,
48,
0,
0,
0,
0,
0,
32,
0,
0,
0,
0,
0,
0,
};

unsigned int registerMaskTable2[] =
{
0b00000000000000000000000010000000,
0b00000000000000001000000000000000,
0b10000000000000000000000000000000,
};

unsigned char shiftTable[] =
{
1,2,4,8
};

struct paletteT {
	unsigned char RED;
    unsigned char GREEN;
    unsigned char BLUE;
};

paletteT paletteLookupTable[65536];

unsigned short imageOffsetTable[256];
unsigned short imageOffsetTableX[256];		//separate table so that it doesn't get flipped like the other one

paletteT paletteToRGB[256];

std::bitset<0x0001C000> writeFlagBuffer;

void audioInit()
{
Pa_Initialize();
// Open a PortAudio stream for playback
    Pa_OpenDefaultStream(&stream, 0, 2, SAMPLE_FORMAT, SAMPLE_RATE,
                               FRAMES_PER_BUFFER, NULL, NULL);
// Start the audio stream
    Pa_StartStream(stream);
}
                             
void graphicsInit()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	for (i = 0; i < 65535; i ++)
	{
	paletteLookupTable[i].RED = ((i) & 31) << 3;
	paletteLookupTable[i].GREEN = ((i >> 5) & 31) << 3;
	paletteLookupTable[i].BLUE = ((i >> 10) & 31) << 3;
	}
}

void loadFileIntoArray(const char* filePath, std::vector<unsigned char>& data) {
    // Open the file in binary mode
    std::ifstream file(filePath, std::ios::binary);

    if (file) {
        // Determine the file size
        file.seekg(0, std::ios::end);
        fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        
		if (fileSize > MAX_FILE_SIZE) {
    	std::wcerr << L"Error: File size exceeds the maximum allowed size (16 megabytes)." << std::endl;
    	isExecuting = 255;
    	return;
		}

        // Resize the vector to accommodate the file data
        data.resize(static_cast<size_t>(fileSize));

        // Read the file data into the vector
        file.read(reinterpret_cast<char*>(data.data()), fileSize);

        // Close the file
        file.close();
    } else {
        // Handle the case where the file couldn't be opened
        std::cerr << "Error opening file: " << filePath << std::endl;
    }
}

void initialize()
{
i = 0;
while (i < 16000000)
{
RAM[i + 0x020000] = 0;
i ++;
}
i = 0;
while (i < 0x1BD7F)
{
RAM[i] = 0;
i ++;
}
i = 0;
while (i < fileSize)
{
RAM[i + 0x020000] = fileData[i];
i ++;
}
fileData.resize(0);
maxCycles = 133333;
stackPointer = 0xFFFF - 129;
CPU1programPointer = 0x020000;
CPU2programPointer = 0x020008;
programPointer = CPU1programPointer;
lowerRegisters = 0;
upperRegisters = 0;
registerMask = 0;
registerMask2 = 0;
i = 0;
isExecuting = 1;
maxDataCopy = 1048576;
isError = 0;
}

void openFileDialogAndLoadFile() {
    const char* filterPatterns[] = {"*.blitz"}; // Adjust file extensions as needed

    // Show the file dialog to select a file
    const char* filePath = tinyfd_openFileDialog(
        "Select File",
        "",
        1, // Number of filter patterns
        filterPatterns,
        "Game Files (.blitz)",
        0
    );

    // Check if the user selected a file
    if (filePath != NULL) {
        // Load the selected file into memory
        loadFileIntoArray(filePath,fileData);
        if (fileSize <= MAX_FILE_SIZE)
        initialize();
    }
}

void clearConsole() {
#ifdef _WIN32
    // For Windows
    system("cls");
#else
    // For Unix-like systems (Linux, macOS)
    system("clear");
#endif
}

void displayError()
{
cycleCount = maxCycles + 1;
printf("address from ROM: %x\n", programPointer - 0x020000);
printf("address from RAM: %x\n", programPointer);
isExecuting = 2;
opcode = 0;
}

void checkBounds()
{
if (errorFlagTable[(registerSize << 3) + getRegister] == 1)
{
std::wcout << L"Register does not exist (2)" << std::endl;
isError = 1;
}
if (registerSize == 3)
{
std::wcout << L"Size does not exist (1)" << std::endl;
isError = 1;
}
if (errorFlagTable[(registerSize2 << 3) + getRegister2] == 1)
{
isError = 1;
}
if (registerSize2 == 3)
{
std::wcout << L"Size does not exist (2)" << std::endl;
isError = 1;
}
if (isError == 1)
displayError();
}

void handleRegisterSize0() {
    getRegisterValue = RAM[programPointer + 2];
}

void handleRegisterSize1() {
    getRegisterValue = RAM[programPointer + 2];
    getRegisterValue |= (RAM[programPointer + 3] << 8);
}

void handleRegisterSize2() {
    getRegisterValue = RAM[programPointer + 2];
    getRegisterValue |= (RAM[programPointer + 3] << 8);
    getRegisterValue |= (RAM[programPointer + 4] << 16);
    getRegisterValue |= (RAM[programPointer + 5] << 24);
}

// Function table
void (*registerFunctions[])() = {
    handleRegisterSize0,
    handleRegisterSize1,
    handleRegisterSize2
};

void handleRegisterSize10() {
getRegisterValue = RAM[getRegisterValue2];
}

void handleRegisterSize11() {
getRegisterValue = RAM[getRegisterValue2];
getRegisterValue |= ((RAM[getRegisterValue2 + 1]) << 8);
}

void handleRegisterSize12() {
getRegisterValue = RAM[getRegisterValue2];
getRegisterValue |= ((RAM[getRegisterValue2 + 1]) << 8);
getRegisterValue |= ((RAM[getRegisterValue2 + 2]) << 16);
getRegisterValue |= ((RAM[getRegisterValue2 + 3]) << 24);
}

// Function table
void (*registerFunctions2[])() = {
    handleRegisterSize10,
    handleRegisterSize11,
    handleRegisterSize12
};

inline void updateRegister1()
{
if ((registerShiftTable[getRegister + (registerSize << 3)]) < 32)
{
lowerRegisters = lowerRegisters & ~registerMask;
lowerRegisters = lowerRegisters | ((getRegisterValue << (registerShiftTable[getRegister + (registerSize << 3)])) & registerMask);
}
else
{
upperRegisters = upperRegisters & ~registerMask;
upperRegisters = upperRegisters | ((getRegisterValue << (registerShiftTable[getRegister + (registerSize << 3)] - 32)) & registerMask);
}
}

inline void updateRegister2()
{
if ((registerShiftTable[getRegister2 + (registerSize2 << 3)]) < 32)
{
lowerRegisters = lowerRegisters & ~registerMask2;
lowerRegisters = lowerRegisters | ((getRegisterValue2 << (registerShiftTable[getRegister2 + (registerSize2 << 3)])) & registerMask2);
}
else
{
upperRegisters = upperRegisters & ~registerMask2;
upperRegisters = upperRegisters | ((getRegisterValue2 << (registerShiftTable[getRegister2 + (registerSize2 << 3)] - 32)) & registerMask2);
}
}

void prepareRegisters()
{
//register 1
registerMask = registerMaskTable[getRegister + (registerSize << 3)];
if ((registerShiftTable[getRegister + (registerSize << 3)]) < 32)
{
getRegisterValue = (lowerRegisters & registerMask) >> (registerShiftTable[getRegister + (registerSize << 3)]);
}
else
{
getRegisterValue = (upperRegisters & registerMask) >> (registerShiftTable[getRegister + (registerSize << 3)] - 32);
}
//register 2
registerMask2 = registerMaskTable[getRegister2 + (registerSize2 << 3)];
if ((registerShiftTable[getRegister2 + (registerSize2 << 3)]) < 32)
{
getRegisterValue2 = (lowerRegisters & registerMask2) >> (registerShiftTable[getRegister2 + (registerSize2 << 3)]);
}
else
{
getRegisterValue2 = (upperRegisters & registerMask2) >> (registerShiftTable[getRegister2 + (registerSize2 << 3)] - 32);
}
}

inline void decrement()
{
getRegisterValue --;
updateRegister1();
}

inline void increment()
{
getRegisterValue ++;
updateRegister1();
}

void compare()
{
X = 0;
if (getRegisterValue == getRegisterValue2)
{
X |= 1;
}
if (getRegisterValue > getRegisterValue2)
{
X |= 2;
}
if (getRegisterValue < getRegisterValue2)
{
X |= 4;
}
//printf("Register: %x\n", getRegisterValue);
//printf("Register 2: %x\n", getRegisterValue2);
//printf("comparison: %x\n", X);
}

//operation flag: 0 is false, 1 is true

void getArgument()
{
OperationFlag = 0;
if ((getInstruction & bit11) == 0)
{
switch (registerSize2) {
	case 1:
	if ((X & 1) == 0)
	OperationFlag = 1;
	break;
	case 2:
	if ((X & 1) == 1)
	OperationFlag = 1;
	break;
	case 3:
	if ((X & 2) == 2)
	OperationFlag = 1;
}
}
else if ((X & 4) == 4)
{
OperationFlag = 1;
}
//printf("comparison: %x\n", X);
//printf("flag: %x\n", OperationFlag);
}

inline void jumpRelative()
{
if (registerSize == 0)
signedRegister8 = getRegisterValue;
if (registerSize == 1)
signedRegister16 = getRegisterValue;
if (registerSize == 2)
signedRegister = getRegisterValue;
getArgument();
if (OperationFlag == 1)
{
programPointer += signedRegister;
isInsIncrement = 1;
}
}

inline void jumpRegister()
{
getArgument();
if (OperationFlag == 1)
{
programPointer = getRegisterValue;
isInsIncrement = 1;
}
}

void jumpRelativeUnsigned()
{
getArgument();
if (OperationFlag == 0)
return;
if ((getInstruction & bit9) == 0)
{
programPointer += getRegisterValue;
}
else
{
programPointer -= getRegisterValue;
}
isInsIncrement = 1;
}

void copy()
{
if (bandwidth >= maxDataCopy || getRegisterValue2 > 0x1BD7F || getRegisterValue > (0x020000 + fileSize))
return;		
copySize = IO[2];
YDataSize = IO[3];
clipLeft = IO[10];
clipUp = IO[11];
XOrigin = IO[8];
YOrigin = IO[9];
if (IO[12] > 0)
absoluteWidth = IO[12];
else
absoluteWidth = copySize;
if ((IO[1] & 2) == 2)
{
getScale = IO[4] + (IO[5] << 8);
getScale &= 32767;
imageScaleOffsetOrigin = ((255 - getScale) * XOrigin) + 255;
}
else
{
getScale = 255;
imageScaleOffsetOrigin = 255;
}
if ((IO[1] & 8) == 8)
{
YScale = IO[6] + (IO[7] << 8);
YScale &= 32767;
YScaleCounter = ((255 - YScale) * YOrigin) + 255;
YScaleCounter2 = YScaleCounter;
}
else
{
YScale = 255;
YScaleCounter = 255;
YScaleCounter2 = YScaleCounter;
}
i16 = 0;
for (i8 = 0; i8 < 255; i8 ++)
{
imageOffsetTableX[i8] = i16;
i16 += getScale;
}
i16 = 0;
if ((IO[1] & 16) == 0)
{
for (i8 = 0; i8 < 255; i8 ++)
{
imageOffsetTable[i8] = i16;
i16 += absoluteWidth;
}
}
else
{
for (i8 = YDataSize; i8 > 0; i8 --)
{
imageOffsetTable[i8] = i16;
i16 += absoluteWidth;
}
}
imagePoint1 = getRegisterValue;
imagePoint2 = getRegisterValue + copySize;
imagePoint3 = getRegisterValue2 + copySize;
for (currentRow = 0; currentRow < YDataSize; currentRow ++)
{
imageScaleOffset = imageScaleOffsetOrigin + imageOffsetTableX[clipLeft];
getRegisterValue2 += clipLeft;
if (bandwidth >= maxDataCopy || getRegisterValue2 > 0x1BD7F)
return;
if ((YScaleCounter >> 8) >= clipUp & currentRow >= clipUp)
{
if ((IO[1] & 4) == 0)
{
for (dynamicImagePoint = imagePoint1 + clipLeft; dynamicImagePoint < imagePoint2; dynamicImagePoint ++)
{
if ((imageScaleOffset >> 8) < copySize & (YScaleCounter2 >> 8) < YDataSize & (imageScaleOffset >> 8) >= clipLeft)
RAM[getRegisterValue2] = RAM[imagePoint1 + (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])];
imageScaleOffset += getScale;
writeFlagBuffer.set(getRegisterValue2, 1);
getRegisterValue2 ++;
bandwidth ++;
}
}
else
{
for (dynamicImagePoint = imagePoint1 + clipLeft; dynamicImagePoint < imagePoint2; dynamicImagePoint ++)
{
if ((imageScaleOffset >> 8) < copySize & (YScaleCounter2 >> 8) < YDataSize & (imageScaleOffset >> 8) >= clipLeft)
RAM[getRegisterValue2] = RAM[imagePoint2 - (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])];
imageScaleOffset += getScale;
writeFlagBuffer.set(getRegisterValue2, 1);
getRegisterValue2 ++;
bandwidth ++;
}
}
}
else
{
getRegisterValue2 += absoluteWidth;
getRegisterValue2 -= clipLeft;
}
if ((IO[1] & 1) == 0 & copySize <= 240)
{
getRegisterValue2 += 240;
getRegisterValue2 -= absoluteWidth;
}
YScaleCounter += YScale;
YScaleCounter2 += YScale;
getRegisterValue += absoluteWidth;
}
updateRegister1();
updateRegister2();
APFlag = 1;
}

void copyTransparent()
{
if (bandwidth >= maxDataCopy || getRegisterValue2 > 0x1BD7F || getRegisterValue > (0x020000 + fileSize))
return;		
copySize = IO[2];
YDataSize = IO[3];
clipLeft = IO[10];
clipUp = IO[11];
XOrigin = IO[8];
YOrigin = IO[9];
if (IO[12] > 0)
absoluteWidth = IO[12];
else
absoluteWidth = copySize;
if ((IO[1] & 2) == 2)
{
getScale = IO[4] + (IO[5] << 8);
getScale &= 32767;
imageScaleOffsetOrigin = ((255 - getScale) * XOrigin) + 255;
}
else
{
getScale = 255;
imageScaleOffsetOrigin = 255;
}
if ((IO[1] & 8) == 8)
{
YScale = IO[6] + (IO[7] << 8);
YScale &= 32767;
YScaleCounter = ((255 - YScale) * YOrigin) + 255;
YScaleCounter &= 65535;
YScaleCounter2 = YScaleCounter;
}
else
{
YScale = 255;
YScaleCounter = 255;
YScaleCounter2 = YScaleCounter;
}
i16 = 0;
for (i8 = 0; i8 < 255; i8 ++)
{
imageOffsetTableX[i8] = i16;
i16 += getScale;
}
i16 = 0;
if ((IO[1] & 16) == 0)
{
for (i8 = 0; i8 < 255; i8 ++)
{
imageOffsetTable[i8] = i16;
i16 += absoluteWidth;
}
}
else
{
for (i8 = YDataSize; i8 > 0; i8 --)
{
imageOffsetTable[i8] = i16;
i16 += absoluteWidth;
}
}
imagePoint1 = getRegisterValue;
imagePoint2 = getRegisterValue + copySize;
imagePoint3 = getRegisterValue2 + copySize;
for (currentRow = 0; currentRow < YDataSize; currentRow ++)
{
imageScaleOffset = imageScaleOffsetOrigin + imageOffsetTableX[clipLeft];
getRegisterValue2 += clipLeft;
if (bandwidth >= maxDataCopy || getRegisterValue2 > 0x1BD7F)
return;
if ((YScaleCounter >> 8) >= clipUp & currentRow >= clipUp)
{
if ((IO[1] & 4) == 0)
{
for (dynamicImagePoint = imagePoint1 + clipLeft; dynamicImagePoint < imagePoint2; dynamicImagePoint ++)
{
if ((imageScaleOffset >> 8) < copySize & (YScaleCounter2 >> 8) < YDataSize & (imageScaleOffset >> 8) >= clipLeft & (RAM[imagePoint1 + (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])]) > 0)
RAM[getRegisterValue2] = RAM[imagePoint1 + (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])];
imageScaleOffset += getScale;
writeFlagBuffer.set(getRegisterValue2, 1);
getRegisterValue2 ++;
bandwidth ++;
}
}
else
{
for (dynamicImagePoint = imagePoint1 + clipLeft; dynamicImagePoint < imagePoint2; dynamicImagePoint ++)
{
if ((imageScaleOffset >> 8) < copySize & (YScaleCounter2 >> 8) < YDataSize & (imageScaleOffset >> 8) >= clipLeft & (RAM[imagePoint1 + (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])]) > 0)
RAM[getRegisterValue2] = RAM[imagePoint2 - (imageScaleOffset >> 8) + (imageOffsetTable[YScaleCounter2 >> 8])];
imageScaleOffset += getScale;
writeFlagBuffer.set(getRegisterValue2, 1);
getRegisterValue2 ++;
bandwidth ++;
}
}
}
else
{
getRegisterValue2 += copySize;
getRegisterValue2 -= clipLeft;
}
if ((IO[1] & 1) == 0 & copySize <= 240)
{
getRegisterValue2 += 240;
getRegisterValue2 -= absoluteWidth;
}
YScaleCounter += YScale;
YScaleCounter2 += YScale;
getRegisterValue += absoluteWidth;
}
updateRegister1();
updateRegister2();
APFlag = 1;
}

void shiftLeft()
{
getRegisterValue = getRegisterValue >> 1;
updateRegister1();
}

void shiftRight()
{
getRegisterValue = getRegisterValue << 1;
updateRegister1();
}

void rotateLeft()
{
getBit = getRegisterValue & 1;
getRegisterValue = getRegisterValue >> 1;
if (getBit == 1)
{
getRegisterValue = getRegisterValue | registerMaskTable2[registerSize];
}
updateRegister1();
}

void rotateRight()
{
getBit = getRegisterValue & (registerMaskTable2[registerSize]);
getRegisterValue = getRegisterValue << 1;
if (getBit > 0)
{
getRegisterValue ++;
}
updateRegister1();
}

inline void OR()
{
getRegisterValue = getRegisterValue | getRegisterValue2;
updateRegister1();
}

inline void XOR()
{
getRegisterValue = getRegisterValue ^ getRegisterValue2;
updateRegister1();
}

inline void AND()
{
getRegisterValue = getRegisterValue & getRegisterValue2;
updateRegister1();
}

inline void NOT()
{
getRegisterValue = ~getRegisterValue;
updateRegister1();
}

void callFunction()
{
programPointer += 3;
RAM[stackPointer] = programPointer >> 24;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = programPointer >> 16;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = programPointer >> 8;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = programPointer;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
programPointer = getRegisterValue;
isInsIncrement = 1;
APFlag = 1;
}

void returnFromFunction()
{
programPointer = 0;
stackPointer ++;
programPointer |= RAM[stackPointer];
stackPointer ++;
programPointer |= (RAM[stackPointer] << 8);
stackPointer ++;
programPointer |= (RAM[stackPointer] << 16);
stackPointer ++;
programPointer |= (RAM[stackPointer] << 24);
isInsIncrement = 1;
}

inline void setRegisterRegister()
{
getRegisterValue = getRegisterValue2;
updateRegister1();
}

void setRegisterRegisterData()
{
if (getRegisterValue2 <= (0x020000 + fileSize))
{
registerFunctions2[registerSize2]();
updateRegister1();
}
}

void push()
{
RAM[stackPointer] = getRegisterValue;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = getRegisterValue >> 8;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = getRegisterValue >> 16;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
RAM[stackPointer] = getRegisterValue >> 24;
writeFlagBuffer.set(stackPointer, 1);
stackPointer --;
APFlag = 1;
}

void pop()
{
getRegisterValue = 0;
stackPointer ++;
getRegisterValue |= RAM[stackPointer];
stackPointer ++;
getRegisterValue |= RAM[stackPointer << 8];
stackPointer ++;
getRegisterValue |= RAM[stackPointer << 16];
stackPointer ++;
getRegisterValue |= RAM[stackPointer << 24];
}

void jumpAddress()
{
addressJump = 0;
addressJump += RAM[programPointer + 2];
addressJump += (RAM[programPointer + 3]) << 8;
addressJump += (RAM[programPointer + 4]) << 16;
addressJump += (RAM[programPointer + 5]) << 24;
programPointer = addressJump & (registerMaskTable[getRegister2 + (registerSize2 << 3)] >> registerShiftTable[getRegister2 + (registerSize2 << 3)]);
isInsIncrement = 1;
}

void loadRegisterValue()
{
registerFunctions[registerSize2]();
updateRegister1();
programPointer += (shiftTable[registerSize2]);
//std::wcout << L"load register value" << std::endl;
//printf("lower Registers: %x\n", lowerRegisters);
//printf("upper Registers: %x\n", upperRegisters);
//printf("Register value: %x\n", getRegisterValue);
}

inline void subtractRegisterRegister()
{
getRegisterValue -= getRegisterValue2;
updateRegister1();
}

inline void addRegisterRegister()
{
getRegisterValue += getRegisterValue2;
updateRegister1();
}

void subtractRegisterRegisterClamp()
{
if (getRegisterValue >= getRegisterValue2)
getRegisterValue -= getRegisterValue2;
else
getRegisterValue = 0;
updateRegister1();
}

void addRegisterRegisterClamp()
{
storeRegisterResult = getRegisterValue + getRegisterValue2;
if (storeRegisterResult < getRegisterValue)
{
storeRegisterResult = registerMaskTable[registerSize];
}
getRegisterValue = storeRegisterResult;
updateRegister1();
}

void setAddressRegisterData()
{
if (getRegisterValue <= 0x1BD7F)
{
RAM[getRegisterValue] = getRegisterValue2;
writeFlagBuffer.set(getRegisterValue, 1);
if (registerSize2 >= 1)
{
RAM[getRegisterValue + 1] = (getRegisterValue2 >> 8);
writeFlagBuffer.set(getRegisterValue + 1, 1);
}
if (registerSize2 == 2)
{
RAM[getRegisterValue + 2] = (getRegisterValue2 >> 16);
writeFlagBuffer.set(getRegisterValue + 2, 1);
RAM[getRegisterValue + 3] = (getRegisterValue2 >> 24);
writeFlagBuffer.set(getRegisterValue + 3, 1);
}
}
//std::wcout << L"set address register data" << std::endl;
//printf("lower Registers: %x\n", lowerRegisters);
//printf("upper Registers: %x\n", upperRegisters);
//printf("Register value 1: %x\n", getRegisterValue);
//printf("Register value 2: %x\n", getRegisterValue2);
APFlag = 1;
}

inline void incrementCycles1()
{
if (registerSize <= 1)
cycleCount ++;
if (registerSize == 2)
cycleCount += 2;
}

inline void incrementCycles2()
{
if (registerSize <= 1)
cycleCount ++;
if (registerSize == 2)
cycleCount += 2;
if (registerSize2 <= 1)
cycleCount ++;
if (registerSize2 == 2)
cycleCount += 2;
}

void incrementAddressRegister()
{
if (getRegisterValue <= (0x1BD7F))
{
if (registerSize == 0)
{
temporarydata8 = RAM[getRegisterValue];
temporarydata8 ++;
RAM[getRegisterValue] = temporarydata8;
writeFlagBuffer.set(getRegisterValue, 1);
}
if (registerSize == 1)
{
temporarydata16 = (RAM[getRegisterValue + 1] << 8);
temporarydata16 |= RAM[getRegisterValue];
temporarydata16 ++;
RAM[getRegisterValue + 1] = temporarydata16 >> 8;
writeFlagBuffer.set(getRegisterValue + 1, 1);
RAM[getRegisterValue] = temporarydata16;
writeFlagBuffer.set(getRegisterValue, 1);
}
if (registerSize == 2)
{
temporarydata = (RAM[getRegisterValue + 3] << 24);
temporarydata |= (RAM[getRegisterValue + 2] << 16);
temporarydata |= (RAM[getRegisterValue + 1] << 8);
temporarydata |= RAM[getRegisterValue];
temporarydata ++;
RAM[getRegisterValue + 3] = temporarydata >> 24;
writeFlagBuffer.set(getRegisterValue + 3, 1);
RAM[getRegisterValue + 2] = temporarydata >> 16;
writeFlagBuffer.set(getRegisterValue + 2, 1);
RAM[getRegisterValue + 1] = temporarydata >> 8;
writeFlagBuffer.set(getRegisterValue + 1, 1);
RAM[getRegisterValue] = temporarydata;
writeFlagBuffer.set(getRegisterValue, 1);
}
}
APFlag = 1;
}

void decrementAddressRegister()
{
if (getRegisterValue <= (0x1BD7F))
{
if (registerSize == 0)
{
temporarydata8 = RAM[getRegisterValue];
temporarydata8 --;
RAM[getRegisterValue] = temporarydata8;
writeFlagBuffer.set(getRegisterValue, 1);
}
if (registerSize == 1)
{
temporarydata16 = (RAM[getRegisterValue + 1] << 8);
temporarydata16 |= RAM[getRegisterValue];
temporarydata16 --;
RAM[getRegisterValue + 1] = temporarydata16 >> 8;
writeFlagBuffer.set(getRegisterValue + 1, 1);
RAM[getRegisterValue] = temporarydata16;
writeFlagBuffer.set(getRegisterValue, 1);
}
if (registerSize == 2)
{
temporarydata = (RAM[getRegisterValue + 3] << 24);
temporarydata |= (RAM[getRegisterValue + 2] << 16);
temporarydata |= (RAM[getRegisterValue + 1] << 8);
temporarydata |= RAM[getRegisterValue];
temporarydata --;
RAM[getRegisterValue + 3] = temporarydata >> 24;
writeFlagBuffer.set(getRegisterValue + 3, 1);
RAM[getRegisterValue + 2] = temporarydata >> 16;
writeFlagBuffer.set(getRegisterValue + 2, 1);
RAM[getRegisterValue + 1] = temporarydata >> 8;
writeFlagBuffer.set(getRegisterValue + 1, 1);
RAM[getRegisterValue] = temporarydata;
writeFlagBuffer.set(getRegisterValue, 1);
}
}
APFlag = 1;
}

void CPU()
{
	bandwidth = 0;
	cycleCount = 0;
	getInstruction = 0;
	fade = IO[0];
	while(cycleCount <= maxCycles)
	{
	if (programPointer <= (0x020000 + fileSize))
	{
    getInstruction = (RAM[programPointer]) + ((RAM[programPointer + 1]) << 8);
	}
	opcode = getInstruction & 63;
	if (((instructionTypes[opcode]) & 64) > 0)
	{
	getInstruction &= 255;
	}
	getRegister = ((getInstruction & register1Bits) >> 6);
	registerSize = ((getInstruction & register1SizeBits) >> 12);
	getRegister2 = ((getInstruction & register2Bits) >> 9);
	registerSize2 = ((getInstruction & register2SizeBits) >> 14);
	if (instructionFlagTable[opcode] == 0 & opcode <= 34)
	{
	checkBounds();
	}
	prepareRegisters();
	isInsIncrement = 0;
	//printf("address from ROM: %x\n", programPointer - 0x020000);
	
    
    	switch (opcode) {
    
        case 0:
        programPointer --;
        cycleCount ++;
            break;
        case 1:
        increment();
        incrementCycles1();
            break;
        case 2:
        decrement();
        incrementCycles1();
            break;
        case 3:
        compare();
        incrementCycles2();
            break;
        case 4:
        jumpRelative();
        incrementCycles1();
        	break;
        case 5:
        jumpRegister();
        incrementCycles1();
        	break;
        case 6:
        jumpRelativeUnsigned();
        incrementCycles1();
            break;
        case 7:
        copy();
        incrementCycles2();
            break;
        case 8:
        copyTransparent();
        incrementCycles2();
            break;
        case 9:
        shiftLeft();
        incrementCycles1();
            break;
        case 10:
        shiftRight();
        incrementCycles1();
            break;
        case 11:
        rotateLeft();
        incrementCycles1();
            break;
        case 12:
        rotateRight();
        incrementCycles1();
        	break;
        case 13:
        OR();
        incrementCycles2();
            break;
        case 14:
        XOR();
        incrementCycles2();
            break;
        case 15:
        AND();
        incrementCycles2();
            break;
        case 16:
        NOT();
        incrementCycles1();
            break;
        case 17:
        callFunction();
        incrementCycles1();
        cycleCount += 2;
            break;
        case 18:
        returnFromFunction();
        incrementCycles1();
        programPointer --;
        cycleCount += 2;
            break;
        case 19:
        stackPointer = getRegisterValue;
        incrementCycles1();
        cycleCount += 2;
            break;
        case 20:
        setRegisterRegister();
        incrementCycles2();
            break;
        case 21:
        setRegisterRegisterData();
        incrementCycles2();
            break;
        case 22:
        push();
        cycleCount += 2;
        programPointer --;
        	break;
        case 23:
        pop();
        cycleCount += 2;
        programPointer --;
        	break;
        case 24:
        jumpAddress();
        incrementCycles1();
        	break;
        case 25:
        loadRegisterValue();
        incrementCycles1();
            break;
        case 26:
        subtractRegisterRegister();
        incrementCycles2();
            break;
        case 27:
        addRegisterRegister();
        incrementCycles2();
            break;
        case 28:
        subtractRegisterRegisterClamp();
        incrementCycles2();
            break;
        case 29:
        addRegisterRegisterClamp();
        incrementCycles2();
            break;
        case 30:
        setAddressRegisterData();
        incrementCycles2();
        	break;
        case 31:
        incrementAddressRegister();
        incrementCycles1();
        	break;
        case 32:
        decrementAddressRegister();
        incrementCycles1();
        	break;
        case 33:
        cycleCount = maxCycles + 1;
        programPointer ++;
        return;
        
        default:
		std::wcout << L"Instruction does not exist" << std::endl;
		displayError();
        ;
        
    
    }
    if (isCPU == 1 & APFlag == 1)
    {
    if (audioCounter2 < 400 & writeFlagBuffer[0xFF8E] == 1 & writeFlagBuffer[0xFF8F] == 1)
	{
	newAudioData = RAM[0xFF8E] + (RAM[0xFF8F] << 8);
	audioBuffer[audioCounter2] = newAudioData;
	audioCounter2 ++;
	}
	if (audioCounter3 < 400 & writeFlagBuffer[0xFF90] == 1 & writeFlagBuffer[0xFF91] == 1)
	{
	newAudioData = RAM[0xFF90] + (RAM[0xFF91] << 8);
	audioBuffer2[audioCounter3] = newAudioData;
	audioCounter3 ++;
	}
	writeFlagBuffer.set(0xFF8E, 0);
	writeFlagBuffer.set(0xFF8F, 0);
	writeFlagBuffer.set(0xFF90, 0);
	writeFlagBuffer.set(0xFF91, 0);
	APFlag = 0;
    }
    //if (IO[8] > 0)
    //{
    //std::cout << "opcode: " << instructionNames[opcode] << registerNames[(registerSize << 3) + getRegister] << registerNames[(registerSize2 << 3) + getRegister2] << std::endl;
    //printf("Register: %x\n", getRegisterValue);
    //printf("Register 2: %x\n", getRegisterValue2);
	//}
	if (isInsIncrement == 0)
    programPointer += 2;
	}
}

void getFade()
{
i8 = 0;
while (1)
{
getRGB = (palettes[i8 << 1]) + ((palettes[(i8 << 1) + 1]) << 8);
getRED = ((getRGB) & 31);
getGREEN = ((getRGB >> 5) & 31);
getBLUE = ((getRGB >> 10) & 31);
if (getRED >= fade)
{
getRED -= fade;
}
else
{
getRED = 0;
}
if (getGREEN >= fade)
{
getGREEN -= fade;
}
else
{
getGREEN = 0;
}
if (getBLUE >= fade)
{
getBLUE -= fade;
}
else
{
getBLUE = 0;
}
getRGB = getRED + (getGREEN << 5) + (getBLUE << 10);
paletteToRGB[i8].RED = paletteLookupTable[getRGB].RED;
paletteToRGB[i8].GREEN = paletteLookupTable[getRGB].GREEN;
paletteToRGB[i8].BLUE = paletteLookupTable[getRGB].BLUE;
if (i8 < 255)
i8 ++;
else
return;
}
}

void display()
{
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(240, 0);
    glTexCoord2f(1, 1); glVertex2f(240, 200);
    glTexCoord2f(0, 1); glVertex2f(0, 200);
    glEnd();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, secondaryFrameBuffer);
	// Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glutSwapBuffers();
    glutPostRedisplay();
}
    

void debugDisplay()
{
i = 0;
i2 = 0;
for (i16 = 0;i16 < 48000;i16 ++) {
getPixel = RAM[i16 + debugOffset];
getRGB = palettes[getPixel << 1] + (palettes[(getPixel << 1) + 1] << 8);
if (grayscaleMode == 0)
{
secondaryFrameBuffer[i2] = paletteLookupTable[getRGB].RED;
secondaryFrameBuffer[i2 + 1] = paletteLookupTable[getRGB].GREEN;
secondaryFrameBuffer[i2 + 2] = paletteLookupTable[getRGB].BLUE;
}
else
{
secondaryFrameBuffer[i2] = getPixel;
secondaryFrameBuffer[i2 + 1] = getPixel;
secondaryFrameBuffer[i2 + 2] = getPixel;
}
i2 += 3;
}
glBegin(GL_QUADS);
glTexCoord2f(0, 0); glVertex2f(0, 0);
glTexCoord2f(1, 0); glVertex2f(240, 0);
glTexCoord2f(1, 1); glVertex2f(240, 200);
glTexCoord2f(0, 1); glVertex2f(0, 200);
glEnd();
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, secondaryFrameBuffer);
// Bind the texture
glBindTexture(GL_TEXTURE_2D, textureID);
glutSwapBuffers();
glutPostRedisplay();
}

void updateKeys()
{
	RAM[0x0FF92] = 0;
	RAM[0x0FF93] = 0;
	if (keyStates['a']){RAM[0x0FF92] |= 1;}		//d-pad left
	if (keyStates['d']){RAM[0x0FF92] |= 2;}		//d-pad right
	if (keyStates['w']){RAM[0x0FF92] |= 4;}		//d-pad up
	if (keyStates['s']){RAM[0x0FF92] |= 8;}		//d-pad down
	if (keyStates['e']){RAM[0x0FF92] |= 16;}		//start
	if (keyStates['r']){RAM[0x0FF92] |= 32;}		//select
	if (keyStates['z']){RAM[0x0FF92] |= 64;}		//A button
	if (keyStates['x']){RAM[0x0FF92] |= 128;}		//B button
	if (keyStates['c']){RAM[0x0FF93] |= 1;}		//X button
	if (keyStates['v']){RAM[0x0FF93] |= 2;}		//Y button
	if (keyStates['f']){RAM[0x0FF93] |= 4;}		//left bumper
	if (keyStates['g']){RAM[0x0FF93] |= 8;}		//right bumper
	if (keyStates['j']){if (debugOffset < (0x020000 + fileSize) - 48000) debugOffset ++;}
	if (keyStates['k']){if (debugOffset > 0) debugOffset --;}
	if (keyStates['n']){if (debugOffset <= (0x020000 + fileSize) - (48000 + 240)) debugOffset += 240; else debugOffset = (0x020000 + fileSize) - 48000;}
	if (keyStates['m']){if (debugOffset >= 240) debugOffset -= 240; else debugOffset = 0;}
	if (keyStates['o']){openFileDialogAndLoadFile(); keyStates['o'] = 0;}
	if (keyStates['u']){grayscaleMode ++; if (grayscaleMode > 1) grayscaleMode = 0; keyStates['u'] = 0;}
	if (keyStates['l']){}		//test
	if (keyStates['b']){}		//test
	if (keyStates['h'] & windowTimer == 0){
	if (isDebugOpen == 0)
	{
	isDebugOpen = 1;
	debugOffset = 0;
	windowTimer = 60;
	debugWindowID = glutCreateWindow("graphics debug");
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(480,400);
	glutDisplayFunc(debugDisplay);
	glutKeyboardFunc(keyPressed);
    glutKeyboardUpFunc(keyReleased);
	graphicsInit();
    gluOrtho2D(0,240,200,0);
    glutMainLoop();
	}
	else
	{
	glutDestroyWindow(debugWindowID);
	isDebugOpen = 0;
	windowTimer = 60;
	}
	}
	if (windowTimer > 0)
	windowTimer --;
}

void runInt()
{
updateKeys();
if (isExecuting == 1)
{
programPointer = CPU1programPointer;
upperRegisters = CPU1upperRegisters;
lowerRegisters = CPU1lowerRegisters;
isCPU = 0;
CPU();
CPU1programPointer = programPointer;
CPU1upperRegisters = upperRegisters;
CPU1lowerRegisters = lowerRegisters;
programPointer = CPU2programPointer;
upperRegisters = CPU2upperRegisters;
lowerRegisters = CPU2lowerRegisters;
audioBufferIndex = 0;
audioCounter2 = 0;
audioCounter3 = 0;
isCPU = 1;
CPU();
samplePointer = 0;
i = 0;
for (i16 = 0; i16 < 512; i16 ++)
{
audioInBuffer[i] = ((audioBuffer[(samplePointer >> 9)]) << 14);
i += 2;
samplePointer += audioCounter2;
}
samplePointer = 0;
i = 1;
for (i16 = 0; i16 < 512; i16 ++)
{
audioInBuffer[i] = ((audioBuffer2[(samplePointer >> 9)]) << 14);
i += 2;
samplePointer += audioCounter3;
}
Pa_WriteStream(stream, audioInBuffer, FRAMES_PER_BUFFER);
//paCallback(audioInBuffer,audioOutBuffer,FRAMES_PER_BUFFER,NULL,0,NULL);
CPU2programPointer = programPointer;
CPU2upperRegisters = upperRegisters;
CPU2lowerRegisters = lowerRegisters;
graphicsMode = RAM[0xFF94];
if (graphicsMode > 2)
{
std::wcout << L"mode does not exist" << std::endl;
displayError();
}
if (graphicsMode == 0)
{
i = 0;
i2 = 0;
getFade();
for (i16 = 0;i16 < 48000;i16 ++) {
getPixel = frameBuffer[i16];
secondaryFrameBuffer[i2] = paletteToRGB[getPixel].RED;
secondaryFrameBuffer[i2 + 1] = paletteToRGB[getPixel].GREEN;
secondaryFrameBuffer[i2 + 2] = paletteToRGB[getPixel].BLUE;
i2 += 3;
}
}
if (graphicsMode == 1)
{
i = 0;
i2 = 0;
for (i16 = 0;i16 < 48000;i16 ++) {
getRGB = (RAM[i16]) + ((frameBuffer[i16]) << 8);
getRED = ((getRGB) & 31);
getGREEN = ((getRGB >> 5) & 31);
getBLUE = ((getRGB >> 10) & 31);
getRGB = getRED + (getGREEN << 5) + (getBLUE << 10);
secondaryFrameBuffer[i2] = paletteLookupTable[getRGB].RED;
secondaryFrameBuffer[i2 + 1] = paletteLookupTable[getRGB].GREEN;
secondaryFrameBuffer[i2 + 2] = paletteLookupTable[getRGB].BLUE;
i2 += 3;
}
}
if (graphicsMode == 2)
{
i = 0;
i2 = 0;
getFade();
for (i16 = 0;i16 < 48000;i16 ++) {
getPixel = frameBuffer[i16];
temporarydata8 = RAM[i16];
secondaryFrameBuffer[i2] = ((paletteToRGB[getPixel].RED) >> 1) + ((paletteToRGB[temporarydata8].RED) >> 1);
secondaryFrameBuffer[i2 + 1] = ((paletteToRGB[getPixel].GREEN) >> 1) + ((paletteToRGB[temporarydata8].GREEN) >> 1);
secondaryFrameBuffer[i2 + 2] = ((paletteToRGB[getPixel].BLUE) >> 1) + ((paletteToRGB[temporarydata8].BLUE) >> 1);
i2 += 3;
}
}
}
}

void init()
{
	graphicsInit();
	glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(240, 0);
    glTexCoord2f(1, 1); glVertex2f(240, 200);
    glTexCoord2f(0, 1); glVertex2f(0, 200);
    glEnd();
    gluOrtho2D(0,240,200,0);
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(480,400);
	glutCreateWindow("Game blitz");
	glutKeyboardFunc(keyPressed);
    glutKeyboardUpFunc(keyReleased);
    glutIdleFunc(runInt);
	glutDisplayFunc(display);
	init();
	audioInit();
	glutMainLoop();
}
