unsigned char RAM[65535 - 128];
unsigned char IO[128];
unsigned char palettes[512];
unsigned char frameBuffer[240 * 200];

unsigned short safetyNet[1024];

unsigned char ROM[17000000];

unsigned char secondaryFrameBuffer[240 * 200 * 3];
