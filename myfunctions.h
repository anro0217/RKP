#ifndef MYFUNCTIONS_H

#define MYFUNCTIONS_H
#define BUFSIZE 1024 // Max length of buffer
#define PORT_NO 3333

void decToHex(int dec, unsigned char hex[4]);
void bitSwap(int first, unsigned char *header, unsigned char *array);
int getSize(int side);
void changeHeader(unsigned char bmp_header[62], int Decimal);
unsigned char hexSum(unsigned char hex, int dec);
int myPow(int base, int pow);
void drawBMP(unsigned char bmp_data[], int data_size, int *Values, int NumValues);
void BMPcreator(int *Values, int NumValues);
int Measurement(int **Values);

int FindPID();
void SendViaFile(int *Values, int NumValues);
void ReceiveViaFile(int sig);
void SignalHandler(int sig);
void SendViaSocket(int *Values, int NumValues);
void ReceiveViaSocket();

#endif