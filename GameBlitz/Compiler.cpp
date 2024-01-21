#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#define opcodeBits 0b0000000000111111
#define register1Bits 0b0000000111000000
#define register2Bits 0b0000111000000000
#define register1SizeBits 0b0011000000000000
#define register2SizeBits 0b1100000000000000
#define controlBits 0b1100000000000000
#define bit11 0b0000100000000000
#define bit9 0b0000001000000000
                             
std::streamsize fileSize;
std::vector<std::string> filesToCompile;
std::string valueStr;
std::string headerStr;
std::string line;
const size_t MAX_FILE_SIZE = 16000000;
size_t foundPos1;
size_t foundPos2;
size_t i2;
;unsigned char isHex;
;unsigned short symbolOffset;
;unsigned short syntaxOffset;
;unsigned short setInstruction;
;unsigned char getInstructionType;
;unsigned int fileSizeCount;
;unsigned int getSymbolAddress;
;unsigned int getAddress;
;unsigned int getFileAddress;
;unsigned int extractedValue;
;unsigned short syntaxString;
;unsigned short syntaxAddress;
;unsigned short syntaxTableIndex;
;unsigned char isFileNamed;
;unsigned char getRegister1;
;unsigned char getRegister2;
;const char* inputString;

std::string instructions;
const char* getFileName;
unsigned int instructionOffset;
unsigned int instructionOffset2;
unsigned int instructionOffset3;

struct InstructionTypePair {
    const char* name;
    unsigned char value;
};

struct registerStatePair {
    const char* state1;
    const char* state2;
    const char* state3;
    const char* state4;
    unsigned char registerTypeBits;
    unsigned char registerTypeBits2;
    unsigned char index;
};

struct Symbol {
	unsigned int symbolAddress;
    unsigned int address;
    unsigned char size;
};

unsigned char outputFile[128000000];

unsigned int SymbolLookupTable[10000];
unsigned int syntaxLookupTable[10000];
unsigned char SymbolLookupTableFlags[10000];
unsigned char syntaxLookupTableFlags[10000];

Symbol SymbolAddressTable[10000];
Symbol syntaxAddressTable[10000];

InstructionTypePair instructionTypes[] = {
        {"#nop", 0b01000000},
        {"#inc", 0b00000001},
        {"#dec", 0b00000001},
        {"#cp ", 0b00000011},
        {"#jr ", 0b00100001},
        {"#jp ", 0b00100101},
        {"#jru ", 0b00100001},
        {"#copy", 0b00001111},
        {"#copyT ", 0b00001111},
        {"#SL ", 0b00000001},
        {"#SR ", 0b00000001},
        {"#RL ", 0b00000001},
        {"#RR ", 0b00000001},
        {"#OR ", 0b00000011},
        {"#XOR ", 0b00000011},
        {"#AND ", 0b00000011},
        {"#NOT ", 0b00000001},
        {"#call ", 0b00000001},
        {"#ret", 0b01000000},
        {"#ld sp, ", 0b00000001},
        {"#set ", 0b00000011},
        {"#set ", 0b00001011},
        {"#push ", 0b00000001},
        {"#pop ", 0b00000001},
        {"#jp (", 0b00010000},
        {"#ld ", 0b10010001},
        {"#sub ", 0b00000011},
        {"#add ", 0b00000011},
        {"#subCLIP ", 0b00000011},
        {"#addCLIP ", 0b00000011},
        {"#set (", 0b00000111},
        {"#inc (", 0b00000101},
        {"#dec (", 0b00000101},
        {"#halt", 0b01000000},
        {"dummyValue", 0b01000000},		//do not attempt to use this, it will result in an error at runtime.
    };
    
InstructionTypePair subInstructionTypes[] =
{
{"#jp NX, ", 0b01000000},
{"#jp X, ", 0b10000000},
{"#jp GRX, ", 0b11000000},
{"#jp LX, ", 0b00001000},
};

//Instruction types are split into bits:
//bit 0: if set to 1, register 1 is used as the first argument, if set to 0, there is no first argument.
//bit 1: if set to 1, register 2 is used as the second argument, if set to 0, there is no second argument.
//bit 2: if set to 1, register 1 is used as an address, if set to 0, it's used as a value.
//bit 3: if set to 1, register 2 is used as an address, if set to 0, it's used as a value.
//bit 4: if set to 1, a value is appended to the instruction, if set to 0, no value is appended.
//bit 5: if set to 1, the compiler checks for sub-instructions, if set to 0, there are no sub-instructions.
//bit 6: if set to 1, the compiled instruction is only 1 byte. If set to 0, it's more than 1 byte.
//bit 7: if set to 1, the appended value is an address. If set to 0, it's a value.

registerStatePair getRegisterBits[] =
{
{" ABCD", ", ABCD", " (ABCD)", ", (ABCD)", 0b00100000, 0b10000000, 0},
{" EFGH", ", EFGH", " (EFGH)", ", (EFGH)", 0b00100000, 0b10000000, 1},
{" AB", ", AB", " (AB)", ", (AB)", 0b00010000, 0b01000000, 0},
{" CD", ", CD", " (CD)", ", (CD)", 0b00010000, 0b01000000, 1},
{" EF", ", EF", " (EF)", ", (EF)", 0b00010000, 0b01000000, 2},
{" GH", ", GH", " (GH)", ", (GH)", 0b00010000, 0b01000000, 3},
{" A", ", A", " (A)", ", (A)", 0b00000000, 0b00000000, 0},
{" B", ", B", " (B)", ", (B)", 0b00000000, 0b00000000, 1},
{" C", ", C", " (C)", ", (C)", 0b00000000, 0b00000000, 2},
{" D", ", D", " (D)", ", (D)", 0b00000000, 0b00000000, 3},
{" E", ", E", " (E)", ", (E)", 0b00000000, 0b00000000, 4},
{" F", ", F", " (F)", ", (F)", 0b00000000, 0b00000000, 5},
{" G", ", G", " (G)", ", (G)", 0b00000000, 0b00000000, 6},
{" H", ", H", " (H)", ", (H)", 0b00000000, 0b00000000, 7},
};

;int i;

//registers: a,b,c,d,e,f,g,h

unsigned char shiftTable[] =
{
1,2,4
};

void clearConsole() {
#ifdef _WIN32
    // For Windows
    system("cls");
#else
    // For Unix-like systems (Linux, macOS)
    system("clear");
#endif
}

void searchForRegisters1()
{
//reset variables
instructionOffset = 0;
instructionOffset2 = 0;
foundPos1 = 34;
//find position 1
while (foundPos1 >= 32 & instructionOffset <= 13)
{
foundPos1 = line.find(getRegisterBits[instructionOffset].state1);
instructionOffset ++;
}
instructionOffset --;
foundPos2 = 34;
//find position 2
while (foundPos2 >= 32 & instructionOffset2 <= 13)
{
if (instructionOffset2 != instructionOffset)
{
foundPos2 = line.find(getRegisterBits[instructionOffset2].state1);
}
instructionOffset2 ++;
}
instructionOffset2 --;
//compare position 1 and position 2
if (foundPos1 <= foundPos2 || foundPos2 > 34)
{
instructionOffset3 = instructionOffset;
}
if (foundPos2 < foundPos1)
{
instructionOffset3 = instructionOffset2;
}
//write data
if (foundPos1 < 32)
{
if ((getInstructionType & 1) > 0)
{
setInstruction |= ((getRegisterBits[instructionOffset3].registerTypeBits) << 8);
setInstruction |= ((getRegisterBits[instructionOffset3].index) << 6);
}
else
{
std::cout << "Error: too many arguments." << std::endl;
while(1)
{
}
}
}
}

void searchForRegisters2()
{
instructionOffset = 0;
instructionOffset2 = 0;
foundPos1 = 34;
while (foundPos1 >= 32 & instructionOffset <= 13)
{
foundPos1 = line.find(getRegisterBits[instructionOffset].state2);
instructionOffset ++;
}
instructionOffset --;
if (foundPos1 < 32)
{
if ((getInstructionType & 1) > 0)
{
setInstruction |= ((getRegisterBits[instructionOffset].registerTypeBits2) << 8);
setInstruction |= ((getRegisterBits[instructionOffset].index) << 9);
}
else
{
std::cout << "Error: too many arguments." << std::endl;
while(1)
{
}
}
}
}

void searchForRegisters3()
{
instructionOffset = 0;
instructionOffset2 = 0;
foundPos1 = 34;
while (foundPos1 >= 32 & instructionOffset <= 13)
{
foundPos1 = line.find(getRegisterBits[instructionOffset].state3);
instructionOffset ++;
}
instructionOffset --;
foundPos2 = 34;
while (foundPos2 >= 32 & instructionOffset2 <= 13)
{
if (instructionOffset2 != instructionOffset)
{
foundPos2 = line.find(getRegisterBits[instructionOffset2].state3);
}
instructionOffset2 ++;
}
instructionOffset2 --;
if (foundPos1 <= foundPos2 || foundPos2 > 34)
{
instructionOffset3 = instructionOffset;
}
if (foundPos2 < foundPos1)
{
instructionOffset3 = instructionOffset2;
}
if (foundPos1 < 32)
{
if ((getInstructionType & 1) > 0)
{
setInstruction |= ((getRegisterBits[instructionOffset3].registerTypeBits) << 8);
setInstruction |= ((getRegisterBits[instructionOffset3].index) << 6);
}
else
{
std::cout << "Error: too many arguments." << std::endl;
while(1)
{
}
}
}
}

void searchForRegisters4()
{
instructionOffset = 0;
instructionOffset2 = 0;
foundPos1 = 34;
while (foundPos1 >= 32 && instructionOffset <= 13)
{
foundPos1 = line.find(getRegisterBits[instructionOffset].state4);
instructionOffset ++;
}
instructionOffset --;
if (foundPos1 < 32)
{
if ((getInstructionType & 1) > 0)
{
setInstruction |= ((getRegisterBits[instructionOffset].registerTypeBits2) << 8);
setInstruction |= ((getRegisterBits[instructionOffset].index) << 9);
}
else
{
std::cout << "Error: too many arguments." << std::endl;
while(1)
{
}
}
}
}

void searchForKeyWords()
{
foundPos1 = 34;
instructionOffset = 34;
setInstruction = 0;
valueStr = "";
getInstructionType = 0;
extractedValue = 0;
// Search for keyword1
while (instructionOffset <= 34 && foundPos1 >= 34) {
    foundPos1 = line.find(instructionTypes[instructionOffset].name);
    instructionOffset -- ;
}
instructionOffset ++;
if (instructionOffset == 21)
{
foundPos1 = line.find(", (");
if (foundPos1 >= 34)
{
instructionOffset = 20;
}
foundPos1 = 0;
}
if (foundPos1 <= 34)
{
getInstructionType = instructionTypes[instructionOffset].value;
std::cout << "instruction found: " << instructionTypes[instructionOffset].name << std::endl;
setInstruction = instructionOffset;
}
else
{
std::cout << "instruction not found." << std::endl;
return;
}
if ((getInstructionType & 1) > 0)
{
getRegister1 = 0;
if ((getInstructionType & 4) == 0)
{
searchForRegisters1();
}
else
{
searchForRegisters3();
}
}
if ((getInstructionType & 2) > 0)
{
getRegister2 = 0;
if ((getInstructionType & 8) == 0)
{
searchForRegisters2();
}
else
{
searchForRegisters4();
}
}
foundPos2 = 0;
//sub instructions
if ((getInstructionType & 32) > 0)
{
instructionOffset = 0;
foundPos1 = 64;
while (foundPos1 >= 64 & instructionOffset < 4) {
    foundPos1 = line.find(subInstructionTypes[instructionOffset].name);
    instructionOffset ++;
}
instructionOffset --;
setInstruction = setInstruction | ((subInstructionTypes[instructionOffset].value) << 8);
}
if ((getInstructionType & 16) > 0)
{
if ((getInstructionType & 128) == 0)
{
foundPos1 = line.find("(");
foundPos2 = line.find(")");
}
else
{
foundPos1 = line.find("[");
foundPos2 = line.find("]");
}
if (foundPos1 < 33 & foundPos2 < 33)
{
valueStr = line.substr(foundPos1 + 1, foundPos2 - foundPos1 - 1);
unsigned char isSymbol = 0;
if (valueStr.substr(0, 1) == "$")
{
valueStr = line.substr(foundPos1 + 2, foundPos2 - foundPos1 - 2);
isSymbol = 1;
}
if (valueStr.substr(0, 1) == ":")
{
valueStr = line.substr(foundPos1 + 2, foundPos2 - foundPos1 - 2);
isSymbol = 2;
}
isHex = 0;
if (valueStr.substr(0, 2) == "0x")
{
isHex = 1;
}
inputString = valueStr.c_str();
extractedValue = strtoul(inputString, NULL, isHex ? 16 : 10);
if (isSymbol == 1)
{
SymbolAddressTable[symbolOffset].symbolAddress = extractedValue;
SymbolAddressTable[symbolOffset].address = fileSizeCount + 2;
SymbolAddressTable[symbolOffset].size = 1;
symbolOffset ++;
extractedValue = 0xFFFFFFFF;
}
if (isSymbol == 2)
{
syntaxAddressTable[syntaxOffset].symbolAddress = extractedValue;
syntaxAddressTable[syntaxOffset].address = fileSizeCount + 2;
syntaxAddressTable[syntaxOffset].size = 1;
syntaxOffset ++;
extractedValue = 0xFFFFFFFF;
}
}
else
{
std::cout << "error: missing limiter/delimiter []" << std::endl;
while(1)
{
}
}
}
std::cout << "Extracted substring:" << valueStr << std::endl;
std::cout << "opcode: " << setInstruction << std::endl;
std::cout << "value: " << extractedValue << std::endl;
outputFile[fileSizeCount] = setInstruction;
fileSizeCount ++;
if ((getInstructionType & 0b01010000) == 0)
{
outputFile[fileSizeCount] = (setInstruction >> 8);
fileSizeCount ++;
}
if ((getInstructionType & 16) > 0)
{
if (extractedValue <= 0xFF)
{
outputFile[fileSizeCount] = setInstruction >> 8;
fileSizeCount ++;
outputFile[fileSizeCount] = extractedValue;
fileSizeCount ++;
}
if (extractedValue > 0xFF & extractedValue <= 0xFFFF)
{
setInstruction = setInstruction | 0b0100000000000000;
outputFile[fileSizeCount] = setInstruction >> 8;
fileSizeCount ++;
outputFile[fileSizeCount] = extractedValue;
outputFile[fileSizeCount + 1] = extractedValue >> 8;
fileSizeCount += 2;
}
if (extractedValue > 0xFFFF)
{
setInstruction = setInstruction | 0b1000000000000000;
outputFile[fileSizeCount] = setInstruction >> 8;
fileSizeCount ++;
outputFile[fileSizeCount] = extractedValue;
outputFile[fileSizeCount + 1] = extractedValue >> 8;
outputFile[fileSizeCount + 2] = extractedValue >> 16;
outputFile[fileSizeCount + 3] = extractedValue >> 24;
fileSizeCount += 4;
}
}
}

void setSymbol()
{
foundPos1 = line.find("$");
foundPos2 = line.find(":");
if (foundPos2 < 33)
{
valueStr = line.substr(foundPos1 + 1, foundPos2 - foundPos1 - 1);
isHex = 0;
if (valueStr.substr(0, 2) == "0x")
{
isHex = 1;
}
const char* inputString = valueStr.c_str();
extractedValue = strtoul(inputString, NULL, isHex ? 16 : 10);
if (extractedValue <= 10000)
{
if (SymbolLookupTableFlags[extractedValue] == 0)
{
SymbolLookupTable[extractedValue] = fileSizeCount;
SymbolLookupTableFlags[extractedValue] = 1;
}
else
std::cout << "warning: duplicate symbol ignored" << std::endl;
}
else
{
std::cout << "error: symbol value greater than 10,000" << std::endl;
while(1)
{
}
}
}
else
{
std::cout << "error: missing delimiter ':'" << std::endl;
while(1)
{
}
}
}

void mapSymbols()
{
symbolOffset = 0;
while (symbolOffset < 10000)
{
if (SymbolAddressTable[symbolOffset].size > 0)
{
getSymbolAddress = SymbolAddressTable[symbolOffset].symbolAddress;
getAddress = SymbolAddressTable[symbolOffset].address;
getFileAddress = SymbolLookupTable[getSymbolAddress] + 0x00020000;
outputFile[getAddress] = getFileAddress;
outputFile[getAddress + 1] = getFileAddress >> 8;
outputFile[getAddress + 2] = getFileAddress >> 16;
outputFile[getAddress + 3] = getFileAddress >> 24;
}
symbolOffset ++;
}
symbolOffset = 0;
while (symbolOffset < 10000)
{
if (syntaxAddressTable[symbolOffset].size > 0)
{
getSymbolAddress = syntaxAddressTable[symbolOffset].symbolAddress;
getAddress = syntaxAddressTable[symbolOffset].address;
getFileAddress = syntaxLookupTable[getSymbolAddress];
outputFile[getAddress] = getFileAddress;
outputFile[getAddress + 1] = getFileAddress >> 8;
outputFile[getAddress + 2] = getFileAddress >> 16;
outputFile[getAddress + 3] = getFileAddress >> 24;
}
symbolOffset ++;
}
}

void fetchData()
{
foundPos1 = line.find("[");
foundPos2 = line.find("]");
if (foundPos1 < 5000 & foundPos2 < 5000)
{
valueStr = line.substr(foundPos1 + 1, foundPos2 - foundPos1 - 1);
std::ifstream dataInclude(valueStr.c_str(), std::ios::binary | std::ios::ate);
if (dataInclude.is_open())
{
i2 = 0;
// Seek to the end of the file to get its size
dataInclude.seekg(0, std::ios::end);
std::streampos size = dataInclude.tellg();

// Allocate a buffer to read the file
std::vector<char> buffer(size);

// Reset file pointer to the beginning and read the file content
dataInclude.seekg(0, std::ios::beg);
dataInclude.read(buffer.data(), size);
while (i2 < size)
{
outputFile[fileSizeCount] = buffer[i2];
fileSizeCount ++;
i2 ++;
}
}
else {
std::cout << "Error: Unable to open the included file." << std::endl;
}
}
else
{
std::cout << "error: missing limiter/delimiter []" << std::endl;
while(1)
{
}
}
}

void setOffsets()
{
foundPos1 = line.find("{");
foundPos2 = line.find("}");
if (foundPos1 < 1024 & foundPos2 < 1024)
{
if ((foundPos2 - foundPos1 - 2) < 1024)
{
valueStr = line.substr(foundPos1 + 2, foundPos2 - foundPos1 - 2);
inputString = valueStr.c_str();
isHex = 0;
extractedValue = strtoul(inputString, NULL, isHex ? 16 : 10);
if (SymbolLookupTableFlags[extractedValue] == 0)
{
std::cout << "error: symbol hasn't been declared yet" << std::endl;
while(1)
{
}
}
if (syntaxLookupTableFlags[extractedValue] == 0)
{
getFileAddress = SymbolLookupTable[extractedValue] + 0x00020000;
syntaxLookupTable[extractedValue] = fileSizeCount;
}
else
{
std::cout << "warning: duplicate symbol ignored" << std::endl;
return;
}
}
else
{
std::cout << "underflow: symbol is missing" << std::endl;
}
syntaxString = line.find("}: ");
foundPos1 = line.find("{$");
foundPos2 = line.find(";");
if (syntaxString < 1024 & foundPos2 < 1024  & foundPos1 < 1024)
{
syntaxString += 3;
foundPos2 = syntaxString;
while (line[foundPos2] != 59)
{
foundPos1 = syntaxString;
foundPos2 = syntaxString;
while (line[foundPos2] != 44 & line[foundPos2] != 59)
{
foundPos2 ++;
}
valueStr = line.substr(foundPos1, foundPos2 - foundPos1);
if (foundPos2 == foundPos1)
{
std::cout << "error: value cannot be null" << std::endl;
while(1)
{
}
}
inputString = valueStr.c_str();
isHex = 0;
extractedValue = strtoul(inputString, NULL, isHex ? 16 : 10);
syntaxAddress = getFileAddress + extractedValue;
outputFile[fileSizeCount] = syntaxAddress;
outputFile[fileSizeCount + 1] = syntaxAddress >> 8;
outputFile[fileSizeCount + 2] = syntaxAddress >> 16;
outputFile[fileSizeCount + 3] = syntaxAddress >> 24;
fileSizeCount += 4;
syntaxString = foundPos2 + 1;
}
}
else
{
std::cout << "error: broken syntax ('{$' '}: ' ';')" << std::endl;
while(1)
{
}
}
}
else
{
std::cout << "error: missing limiter/delimiter {}" << std::endl;
while(1)
{
}
}
}

void nameFile()
{
foundPos1 = line.find("<");
foundPos2 = line.find(">");
if (foundPos1 < 1024 & foundPos2 < 1024 & foundPos2 > foundPos1 + 1)
{
if (isFileNamed == 1)
{
std::cout << "You already named this file, remember?" << std::endl;
return;
}
headerStr = line.substr(foundPos1 + 1, foundPos2 - foundPos1 - 1);
isFileNamed = 1;
}
else
{
std::cout << "Unable to name file." << std::endl;
}
}

int save()
{
// Save the vector to an output file
	if (isFileNamed == 0)
	{
	headerStr = "output.blitz";
	}	
    std::ofstream outputFileH(headerStr.c_str(), std::ios::binary);
    if (outputFileH.is_open()) {
        // Write vector contents to the output file
        if (fileSizeCount <= MAX_FILE_SIZE)
        {
        outputFileH.write(reinterpret_cast<char*>(outputFile), fileSizeCount);
    	}
    	else
    	{
    	std::cout << "Error: ROM size greater than max ROM size (16 megabytes)." << std::endl;
    	std::cout << "Result: " << (fileSizeCount / 1000000) << " megabytes" << std::endl;
    	while(1)
    	{
    	}
    	}

        // Close the output file
        outputFileH.close();
        std::cout << "Data saved to " << headerStr.c_str() << std::endl;
    } else {
        std::cout << "Error: Unable to open the output file." << std::endl;
    }
}
        

int tokenize()
{
for (size_t i = 0; i < filesToCompile.size(); ++i) {
        std::ifstream inputFile(filesToCompile[i].c_str()); // Open the file using .c_str()
        if (inputFile.is_open()) {
            while (std::getline(inputFile, line)) {
                if (!line.empty()) { // Ensure the line is not empty
				switch (line[0])
				{
				case 32:		//space
					break;
				case 47:		// the "/" symbol
					break;
				case 35:		//# symbol
					std::cout << "# symbol detected!" << std::endl;
					searchForKeyWords();
					break;
				case 36:		//$ symbol
					std::cout << "$ symbol detected!" << std::endl;
					setSymbol();
					break;
				case 42:		//* symbol
					std::cout << "* symbol detected!" << std::endl;
					fetchData();
					break;
				case 123:		//{ symbol
					std::cout << "{ symbol detected!" << std::endl;
					setOffsets();
					break;
				case 60:
					nameFile();
					break;
				default:
				std::cout << "Unknown symbol" << std::endl;
				}
                    // Do something with the first character (e.g., print it)
                    std::cout << "line " << filesToCompile[i] << ": " << line << std::endl;
                }
            }
            inputFile.close();
        } else {
            std::cerr << "Unable to open file: " << filesToCompile[i] << std::endl;
        }
    }
}

int main(int argc, char** argv)
{
    // Check if arguments were provided
    if (argc > 1) {
        // Start from argv[1] as argv[0] is the program name
        for (int i = 1; i < argc; ++i) {
            filesToCompile.push_back(argv[i]); // Store file names
        }
    } else {
        std::cerr << "No input files specified." << std::endl;
        return 1;
    }
	
	tokenize();
	std::cout << "mapping symbols..." << std::endl;
	mapSymbols();
	save();
    // Call your compiler logic here with filesToCompile
	while(1)
	{
	
	}
    return 0;
}
