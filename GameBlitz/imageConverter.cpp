#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "tinyfiledialogs.cpp"
#define WindowSizeX 240
#define WindowSizeY 200

;unsigned char fade;
;unsigned int programPointer;
;unsigned int CPU1programPointer;
;unsigned int CPU2programPointer;
;unsigned int i;
;unsigned short getRGB,getPixel;
;unsigned char getRGBLOW,getRGBHIGH;
;int test;
;unsigned int getImageRGB;
;unsigned short paletteSearch, paletteIndex;
;unsigned int imageWriteOffset;
;unsigned char getRED, getGREEN, getBLUE;
;unsigned char isRGBMatch;
;unsigned int imageOffset;
;unsigned char isPPMFile;
;unsigned char headerCheck;
;unsigned char imageWidth;
;unsigned int getLineOffset;
;unsigned int frameBufferOffset;
;unsigned char rowCount;
;unsigned int dataOffset;
;unsigned char headerOffset;
const size_t MAX_FILE_SIZE = 1000000;

unsigned short imagePaletteData[256];
unsigned int imagePaletteOffsets[256];
unsigned char fileDataArray[1000000];
unsigned short palettes[] =
{
//15bpp palettes (0x010080)
0,1057,2114,3171,4228,5285,6342,7399,8456,9513,10570,11627,12684,13741,14798,15855,
16912,17969,19026,20083,21140,22197,23254,24311,25368,26425,27482,28539,29596,30653,31710,32767,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
unsigned char frameBuffer[1000000]; //0x010280
unsigned char secondaryFrameBuffer[240 * 200 * 3];
unsigned char headerCompare[] =
{
0x50, 0x36, 0x0A, 0x23, 0x20, 0x43, 0x72, 0x65, 0x61, 0x74, 0x65, 0x64, 
0x20, 0x62, 0x79, 0x20, 0x47, 0x49, 0x4D, 0x50, 0x20, 0x76, 0x65, 0x72, 
0x73, 0x69, 0x6F, 0x6E,
};
                             
GLuint textureID;
std::vector<unsigned char> fileData;
std::streamsize fileSize;

// Export image data to a file
void exportImage(const char* filename, unsigned char* imageData, size_t imageSize) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(imageData), imageSize);
        std::wcerr << L"Image export successful!" << std::endl;
        file.close();
    } else {
        std::wcerr << L"Error: couldn't open image output file." << std::endl;
    }
}

// Export palette data to a file
void exportPalette(const char* filename, unsigned short* palettes) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(palettes), 512);
        std::wcerr << L"Palette export successful!" << std::endl;
        file.close();
    } else {
    	std::wcerr << L"Error: couldn't open palette output file." << std::endl;
    }
}
                             
void graphicsInit()
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    imageWidth = 240;
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
    	std::wcerr << L"Error: File size exceeds the maximum allowed size (1 megabyte)." << std::endl;
    	file.close();
    	return;
		}

        // Resize the vector to accommodate the file data
        data.resize(static_cast<size_t>(fileSize));

        // Read the file data into the vector
        file.read(reinterpret_cast<char*>(data.data()), fileSize);
        headerCheck = 0;
        isPPMFile = 1;
        while (headerCheck < 28)
        {
        if (fileData[headerCheck] != headerCompare[headerCheck])
        isPPMFile = 0;
        headerCheck ++;
        }
        if (isPPMFile == 0) {
    	std::wcerr << L"Error: not a valid ppm file." << std::endl;
    	file.close();
    	return;
		}

        // Close the file
        file.close();
    } else {
        // Handle the case where the file couldn't be opened
        std::cerr << "Error opening file: " << filePath << std::endl;
    }
}

void convertImage()
{
i = headerOffset;
imageWriteOffset = 0;
paletteIndex = 0;
while ((i + 3) <= fileSize)
{
getImageRGB = fileDataArray[i];
getRED = (fileDataArray[i] >> 3);
getImageRGB += (fileDataArray[i + 1] << 8);
getGREEN = (fileDataArray[i + 1] >> 3);
getImageRGB += (fileDataArray[i + 2] << 16);
getBLUE = (fileDataArray[i + 2] >> 3);
isRGBMatch = 0;
paletteSearch = 0;
while(paletteSearch <= 256 & isRGBMatch == 0)
{
if (imagePaletteOffsets[paletteSearch] == getImageRGB)
isRGBMatch = 1;
paletteSearch ++;
}
paletteSearch --;
if (isRGBMatch == 0)
{
imagePaletteOffsets[paletteIndex] = getImageRGB;
palettes[paletteIndex] = (getRED) | (getGREEN << 5) | (getBLUE << 10);
frameBuffer[imageWriteOffset] = paletteIndex;
paletteIndex ++;
}
else
{
frameBuffer[imageWriteOffset] = paletteSearch;
}
imageWriteOffset ++;
i += 3;
if (paletteIndex > 256)
{
std::cout << "Error: too many colors, must be 256 or less" << std::endl;
return;
}
}
}

void openFileDialogAndLoadFile() {
    const char* filterPatterns[] = {"*.ppm"}; // Adjust file extensions as needed

    // Show the file dialog to select a file
    const char* filePath = tinyfd_openFileDialog(
        "Select File",
        "",
        1, // Number of filter patterns
        filterPatterns,
        "Image Files (.ppm)",
        0
    );

    // Check if the user selected a file
    if (filePath != NULL) {
        // Load the selected file into memory
        loadFileIntoArray(filePath,fileData);
    }
}

void initialize()
{
i = 0;
while (i <= 1000000)
{
fileDataArray[i] = 0;
i ++;
}
i = 0;
while(i <= fileSize)
{
fileDataArray[i] = fileData[i];
i ++;
}
fileData.resize(0);
convertImage();
convertImage();
}

void display()
{
	frameBufferOffset = 0;
	rowCount = 0;
	getLineOffset = (imageOffset * imageWidth);
	dataOffset = 0;
	for (i = 0;i < 48000;i ++) {
	if (rowCount < imageWidth)
	{
    getPixel = frameBuffer[dataOffset + getLineOffset];
    getRGB = palettes[getPixel];
    secondaryFrameBuffer[frameBufferOffset] = (getRGB & 31) << 3;
    secondaryFrameBuffer[frameBufferOffset + 1] = ((getRGB >> 5) & 31) << 3;
    secondaryFrameBuffer[frameBufferOffset + 2] = ((getRGB >> 10) & 31) << 3;
    dataOffset ++;
	}
	else
	{
	secondaryFrameBuffer[frameBufferOffset] = 0;
    secondaryFrameBuffer[frameBufferOffset + 1] = 0;
    secondaryFrameBuffer[frameBufferOffset + 2] = 0;
	}
	rowCount ++;
	if (rowCount >= 240)
	{
	rowCount = 0;
	}
    frameBufferOffset += 3;
	}
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 240, 200, 0, GL_RGB, GL_UNSIGNED_BYTE, secondaryFrameBuffer);
	// Bind the texture
    glBindTexture(GL_TEXTURE_2D, textureID);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0); glVertex2f(0, 0);
    glTexCoord2f(1, 0); glVertex2f(240, 0);
    glTexCoord2f(1, 1); glVertex2f(240, 200);
    glTexCoord2f(0, 1); glVertex2f(0, 200);
    glEnd();
	glutSwapBuffers();
	glutPostRedisplay();
}

void buttons(unsigned char key,int x,int y)
{
	if(key=='e'){exportImage("image.bin",frameBuffer,(fileSize - headerOffset) / 3);exportPalette("palette.bin",palettes);}
	if(key=='d'){if (imageOffset < (1000000 - (200 * imageWidth))) imageOffset ++;}
	if(key=='w'){if (imageOffset > 0) imageOffset --;}
	if(key=='s'){if (imageWidth > 1) imageWidth --;}
	if(key=='z'){if (imageWidth < 240) imageWidth ++; if (imageOffset >= (1000000 - (200 * imageWidth))) imageOffset = (1000000 - (200 * imageWidth));}
	if (key=='o'){openFileDialogAndLoadFile();if (fileSize <= MAX_FILE_SIZE & isPPMFile == 1) initialize();}
	if(key=='j'){if (headerOffset > 50) headerOffset --; convertImage();}
	if(key=='k'){if (headerOffset < 70) headerOffset ++; convertImage();}
}

void init()
{
	graphicsInit();
	gluOrtho2D(0,240,200,0);
	headerOffset = 61;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(480,400);
	glutCreateWindow("image converter");
	glutDisplayFunc(display);
	glutKeyboardFunc(buttons);
	init();
	glutMainLoop();
}
