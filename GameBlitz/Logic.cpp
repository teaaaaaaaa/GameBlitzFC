#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
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
#include <Windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

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
;unsigned int maxCycles;
;unsigned char getBit;
;unsigned char OperationFlag;
;unsigned int getRegisterValueDIV;
;unsigned short getScale;
;unsigned short newAudioData;
;unsigned char isAudio;
;unsigned int temporarydata;
;unsigned int audioBufferIndex;
;unsigned char clipLeft;
;unsigned char getRED,getGREEN,getBLUE;
;unsigned int storeRegisterResult;
;unsigned int storeAddress;
;unsigned int storeAddress2;
;unsigned int storeData;
;unsigned char getComparison;
;unsigned char isInstructionTerminated;
;unsigned char getSymbol;
;unsigned char getSymbolOffset;
;unsigned char getX;
const size_t MAX_FILE_SIZE = 128000000;
unsigned char audioBuffer [133433 * 2];
unsigned char audioCounter;

GLuint textureID;
std::vector<unsigned char> fileData;
std::streamsize fileSize;

//registers: a,b,c,d,e,f,g,h

unsigned char registers[] =
{
0,0,0,0,0,0,0,0,
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
        "#inc",
        "#dec",
        "#halt",
    };
    
struct microInstructionData {
	unsigned char symbol1;
    unsigned char symbol2;
    unsigned char symbol3;
    unsigned char symbol4;
    unsigned char symbol5;
    unsigned char symbol6;
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
1,2,4
};

// Open the default wave output device
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;

void audioInit()
{
	hWaveOut = 0;
	// Initialize the wave format
    WAVEFORMATEX waveFormat;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	waveFormat.nChannels = 1; // Mono
	waveFormat.nSamplesPerSec = (133433 * 30); // Sample rate
	waveFormat.wBitsPerSample = 16; // Bits per sample
	waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.cbSize = 0; // No extra data
	// Initialize the wave header
    memset(&waveHeader, 0, sizeof(waveHeader)); // Clear the header
    // Open the default wave output device
	waveOutOpen(&hWaveOut, WAVE_MAPPER, &waveFormat, 0, 0, CALLBACK_NULL);
}

inline void playAudio() {
    // Play the audio buffer
    waveHeader.dwBufferLength = audioBufferIndex >> 1;     // Set the buffer size
    waveHeader.lpData = reinterpret_cast<LPSTR>(audioBuffer);
    waveOutPrepareHeader(hWaveOut, &waveHeader, sizeof(waveHeader));
    waveOutWrite(hWaveOut,&waveHeader,sizeof(waveHeader));
}
                             
void graphicsInit()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
    	std::wcerr << L"Error: File size exceeds the maximum allowed size (128 megabytes)." << std::endl;
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
while (i < 128000000)
{
RAM[i + 0x020000] = 0;
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
stackPointer = 0xFFFF - 4;
CPU1programPointer = 0x020000;
CPU2programPointer = 0x020008;
programPointer = CPU1programPointer;
lowerRegisters = 0;
upperRegisters = 0;
registerMask = 0;
registerMask2 = 0;
i = 0;
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
if (registerSize == 1 & getRegister > 3)
{
std::wcout << L"Register does not exist" << std::endl;
displayError();
}
if (registerSize == 2 & getRegister > 1)
{
std::wcout << L"Register does not exist" << std::endl;
displayError();
}
if (registerSize == 3)
{
std::wcout << L"Size does not exist" << std::endl;
displayError();
}
}

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
getRegisterValue = (lowerRegisters & registerMask) >> (registerShiftTable[getRegister + (registerSize << 3)]);
}
else
{
getRegisterValue = (upperRegisters & registerMask) >> (registerShiftTable[getRegister + (registerSize << 3)] - 32);
}
}

void getArgument()
{
OperationFlag = 0;
if ((getInstruction & bit11) == 0)
{
switch (registerSize2)
{
case 0:
break;
case 1:
if ((X & 1) != 1)
{
OperationFlag = 1;
}
break;
case 2:
if ((X & 2) != 2)
{
OperationFlag = 1;
}
break;
case 3:
if ((X & 4) != 4)
{
OperationFlag = 1;
}
break;
}
}
else if ((X & 8) != 8)
{
OperationFlag = 1;
}
}

void JR()
{
signedRegister = getRegisterValue;
getArgument();
if (OperationFlag == 0)
programPointer += signedRegister;
}

void JP()
{
getArgument();
if (OperationFlag == 0)
programPointer = getRegisterValue;
}

void JRU()
{
getArgument();
if (OperationFlag == 1)
return;
if ((getInstruction & bit9) == 0)
{
programPointer += getRegisterValue;
}
else
{
programPointer -= getRegisterValue;
}
}

void copy()
{
copySize = IO[2];
getScale = IO[3] + (IO[4] << 8);
if ((IO[1] & 4) > 0)
{
getRegisterValue2 += copySize;
}
clipLeft = IO[5];
copyDataOffset = 0;
for (copyDataOffset = 0; copyDataOffset < copySize; copyDataOffset ++)
{
if (getRegisterValue > ((0x020000 + fileSize)))
{
std::wcout << L"Warning: memory transfer access outside of ROM denied" << std::endl;
if (programPointer >= 0x020000)
{
printf("address from ROM: %x\n", programPointer - 0x020000);
}
else
{
printf("address from RAM: %x\n", programPointer);
}
return;
}
if ((IO[1] & 2) > 0)
{
getRegisterValueDIV = getRegisterValue + (copyDataOffset / (copySize / getScale));
}
else
{
getRegisterValueDIV = getRegisterValue + copyDataOffset;
}
if (getRegisterValue2 <= 0x01BE00)
{
if (copyDataOffset >= clipLeft)
RAM[getRegisterValue2] = RAM[getRegisterValueDIV];
}
if ((IO[1] & 4) == 0)
getRegisterValue2 ++;
else
getRegisterValue2 --;
}
if ((IO[1] & 1) == 0)
{
getRegisterValue2 += 240;
if ((IO[1] & 4) == 0)
getRegisterValue2 -= copySize;
}
updateRegister1();
updateRegister2();
}

void copyT()
{
copySize = IO[2];
getScale = IO[3] + (IO[4] << 8);
if ((IO[1] & 4) > 0)
{
getRegisterValue2 += copySize;
}
clipLeft = IO[5];
copyDataOffset = 0;
for (copyDataOffset = 0; copyDataOffset < copySize; copyDataOffset ++)
{
if (getRegisterValue > ((0x020000 + fileSize)))
{
std::wcout << L"Warning: memory transfer access outside of ROM denied" << std::endl;
if (programPointer >= 0x020000)
{
printf("address from ROM: %x\n", programPointer - 0x020000);
}
else
{
printf("address from RAM: %x\n", programPointer);
}
return;
}
if ((IO[1] & 2) > 0)
{
getRegisterValueDIV = getRegisterValue + (copyDataOffset / (copySize / getScale));
}
else
{
getRegisterValueDIV = getRegisterValue + copyDataOffset;
}
if (getRegisterValue2 <= 0x01BE00 & RAM[getRegisterValueDIV] > 0)
{
if (copyDataOffset >= clipLeft)
RAM[getRegisterValue2] = RAM[getRegisterValueDIV];
}
if ((IO[1] & 4) == 0)
getRegisterValue2 ++;
else
getRegisterValue2 --;
}
if ((IO[1] & 1) == 0)
{
getRegisterValue2 += 240;
if ((IO[1] & 4) == 0)
getRegisterValue2 -= copySize;
}
updateRegister1();
updateRegister2();
}

void NOP()
{
cycleCount ++;
}

void INC()
{
getRegisterValue ++;
updateRegister1();
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void DEC()
{
getRegisterValue --;
updateRegister1();
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void CP()
{
X = 0;
if (getRegisterValue != getRegisterValue2)
{
X ++;
}
if (getRegisterValue == getRegisterValue2)
{
X += 2;
}
if (getRegisterValue > getRegisterValue2)
{
X += 4;
}
if (getRegisterValue < getRegisterValue2)
{
X += 8;
}
cycleCount ++;
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
if (registerSize2 <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void LDADA()
{
storeAddress = getRegisterValue;
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void LDADB()
{
storeAddress2 = getRegisterValue2;
if (registerSize2 <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void LDVPA()
{
if (programPointer <= (0x020000 + fileSize))
{
getRegisterValue = RAM[programPointer + 2];
if (registerSize2 >= 1)
{
getRegisterValue += (RAM[programPointer + 3]) << 8;
}
if (registerSize2 == 2)
{
getRegisterValue += (RAM[programPointer + 4]) << 16;
getRegisterValue += (RAM[programPointer + 5]) << 24;
}
}
if (registerSize2 <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void ITR()
{
getArgument();
if (OperationFlag == 1)
isInstructionTerminated = 1;
cycleCount ++;
}

void STAD1()
{
if (getRegisterValue <= 0x01BE00)
{
RAM[getRegisterValue] = getRegisterValue2;
if (registerSize2 >= 1)
{
RAM[storeAddress + 1] = (getRegisterValue2 << 8);
}
if (registerSize2 == 2)
{
RAM[getRegisterValue + 2] = (getRegisterValue2 << 16);
RAM[getRegisterValue + 3] = (getRegisterValue2 << 24);
}
}
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
if (registerSize2 <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void STAR()
{
if (getRegisterValue <= (0x020000 + fileSize))
{
storeData = RAM[getRegisterValue];
storeData |= (RAM[getRegisterValue + 1] << 8);
storeData |= (RAM[getRegisterValue + 2] << 16);
storeData |= (RAM[getRegisterValue + 3] << 24);
storeData &= (registerMaskTable[getRegister + (registerSize << 3)] >> registerShiftTable[getRegister + (registerSize << 3)]);
getRegisterValue = storeData;
}
if (registerSize <= 1)
cycleCount ++;
else
cycleCount += 2;
}

void EI()
{
isInstructionTerminated = 1;
}

void PUSHP()
{
RAM[stackPointer] = programPointer >> 24;
stackPointer --;
RAM[stackPointer] = programPointer >> 16;
stackPointer --;
RAM[stackPointer] = programPointer >> 8;
stackPointer --;
RAM[stackPointer] = programPointer;
stackPointer --;
cycleCount += 2;
}

void POPP()
{
programPointer = 0;
programPointer += RAM[stackPointer];
stackPointer ++;
programPointer += RAM[stackPointer << 8];
stackPointer ++;
programPointer += RAM[stackPointer << 16];
stackPointer ++;
programPointer += RAM[stackPointer << 24];
stackPointer ++;
cycleCount += 2;
}

void PUSHA()
{
RAM[stackPointer] = getRegisterValue >> 24;
stackPointer --;
RAM[stackPointer] = getRegisterValue >> 16;
stackPointer --;
RAM[stackPointer] = getRegisterValue >> 8;
stackPointer --;
RAM[stackPointer] = getRegisterValue;
stackPointer --;
cycleCount += 2;
}

void POPA()
{
getRegisterValue = 0;
getRegisterValue += RAM[stackPointer];
stackPointer ++;
getRegisterValue += RAM[stackPointer << 8];
stackPointer ++;
getRegisterValue += RAM[stackPointer << 16];
stackPointer ++;
getRegisterValue += RAM[stackPointer << 24];
stackPointer ++;
cycleCount += 2;
}

void LDSP()
{
stackPointer = getRegisterValue;
cycleCount += 2;
}

microInstructionData microCodes[] =
{
{0,21,0,0,0,0},		//nop
{1,21,0,0,0,0},		//inc
{2,21,0,0,0,0},		//dec
{3,21,0,0,0,0},		//cp
{7,4,13,21,0,0},		//jr
{7,0,0,0,0,0,},		//jp
};

void (*microInstructions[])() = {
NOP,	//0, no operation.
INC,	//1, increment.
DEC,	//2, decrement.
CP,		//3, compare register A to register B.
LDVPA,	//6, load value from memory at address of program pointer + 2
ITR,	//7, if the comparison is false, the instruction terminates
STAD1,	//8, store register B at address 1.
STAR,	//9, store data from loaded address at register A.
copy,	//10, a special instruction that copies data without the CPU's involvement.
copyT,	//11, like the copy instruction but with transparency. Refer to assembly documentation for more info.
JP,		//12, jump to address 1
JR,		//13, jump relative to address 1 (signed)
JRU,	//14, jump relative to address 1 (unsigned)
PUSHP,	//15, push program pointer to stack
POPP,	//16, pop stack value to program pointer
PUSHA,	//17, push register A to stack
POPA,	//18, pop stack value to register A
LDSP,	//19, load stack pointer with register A
NOP,	//placeholder
EI		//21, end instruction
};

void CPU()
{
	cycleCount = 0;
	getInstruction = 0;
	while(cycleCount <= maxCycles)
	{
	fade = IO[0];
	if (programPointer <= (0x020000 + fileSize))
	{
    getInstruction = (RAM[programPointer]) + ((RAM[programPointer + 1]) << 8);
	}
	opcode = getInstruction & 63;
	getRegister = ((getInstruction & register1Bits) >> 6);
	registerSize = ((getInstruction & register1SizeBits) >> 12);
	getRegister2 = ((getInstruction & register2Bits) >> 9);
	registerSize2 = ((getInstruction & register2SizeBits) >> 14);
	checkBounds();
	prepareRegisters();
	isInstructionTerminated = 0;
    for (getSymbolOffset = 0; isInstructionTerminated == 0; getSymbolOffset ++;)
    {
    
    }
    
    //std::wcout << L"opcode: " << instructionNames[opcode] << std::endl;
    //printf("address from ROM: %x\n", programPointer - 0x020000);
    //printf("Registers: %x\n", registers);
    if (isAudio == 0)
    {
    if (audioCounter > 1)
    {
    newAudioData = IO[6] + (IO[7] << 8);
    newAudioData -= 32768;
    audioBuffer[audioBufferIndex] = newAudioData;
    audioBuffer[audioBufferIndex + 1] = newAudioData >> 8;
    audioBufferIndex += 2;
    audioCounter = 0;
	}
	audioCounter ++;
	}
	}
}

void getFade()
{
i8 = 0;
while (1)
{
getRED = ((palettes[i8]) & 31);
getGREEN = ((palettes[i8] >> 5) & 31);
getBLUE = ((palettes[i8] >> 10) & 31);
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
secondaryPalettes[i8] = getRED + (getGREEN << 5) + (getBLUE << 10);
if (i8 < 255)
i8 ++;
else
return;
}
}

void display()
{
	if (isExecuting == 1)
	{
	playAudio();
	programPointer = CPU1programPointer;
	isAudio = 1;
	CPU();
	CPU1programPointer = programPointer;
	IO[8] = 0;
	IO[9] = 0;
	i = 0;
	i16 = 0;
	i2 = 0;
	getFade();
	i16 = 0;
	for (i16 = 0;i16 < 48000;i16 ++) {
    getPixel = frameBuffer[i16];
    getRGB = secondaryPalettes[getPixel];
    secondaryFrameBuffer[i2] = (getRGB & 31) << 3;
    secondaryFrameBuffer[i2 + 1] = ((getRGB >> 5) & 31) << 3;
    secondaryFrameBuffer[i2 + 2] = ((getRGB >> 10) & 31) << 3;
    i2 += 3;
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(240, 0);
    glTexCoord2f(1, 1); glVertex2f(240, 200);
    glTexCoord2f(0, 1); glVertex2f(0, 200);
    glEnd();
    programPointer = CPU2programPointer;
    isAudio = 0;
    audioBufferIndex = 0;
    CPU();
    CPU2programPointer = programPointer;
    glutSwapBuffers();
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, secondaryFrameBuffer);
	// Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glutPostRedisplay();
}

void buttons(unsigned char key,int x,int y)
{
	if(key=='a'){frameBuffer[test] = test; test ++;}
	if(key=='d'){frameBuffer[test] = test; test --;}
	if(key=='w'){IO[0] ++;}
	if(key=='s'){IO[0] --;}
	if (key=='o'){openFileDialogAndLoadFile(); isExecuting = 1;}
	if (key=='f'){
	i = 0;
	while(i < 128)
	{
	std::wcout << L"IO values: " << IO[i] << std::endl;
	i ++;
	}
	i = 0;
	while(i < 128)
	{
	std::wcout << L"Absolute IO values: " << RAM[i + 0x00010000] << std::endl;
	i ++;
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
	glutCreateWindow("");
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	init();
	audioInit();
	glutMainLoop();
}
