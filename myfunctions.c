#include "myfunctions.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <omp.h>

void decToHex(int dec, unsigned char hex[4])
{
	int i;
	for (i = 0; i < 4; i++)
	{
		hex[i] = dec & 0xFF;
		dec >>= 8;
	}
}
void bitSwap(int first, unsigned char *header, unsigned char *array)
{
	int j = 0;
	for (int i = first; i < first + 4; i++)
	{
		header[i] = array[j++];
	}
}
int getSize(int side)
{
	int width;
	if (side % 32 == 0)
		width = side / 8;
	else
		width = (side / 32 + 1) * 4;
	return width * side + 62;
}
void changeHeader(unsigned char bmp_header[62], int Decimal)
{
	unsigned char hex_res[4]; // Segédtömbök
	unsigned char hex_size[4];

	decToHex(Decimal, hex_res);
	decToHex(getSize(Decimal), hex_size);
	bitSwap(2, bmp_header, hex_size);
	bitSwap(18, bmp_header, hex_res);
	bitSwap(22, bmp_header, hex_res);
}
unsigned char hexSum(unsigned char hex, int dec)
{											  
	return (unsigned char)((hex + dec) & 0xFF);
}
int myPow(int base, int pow)
{
	int result = base;
	if (pow == 0)
		return 1;
	for (int i = 1; i < pow; i++)
	{
		result *= base;
	}
	return result;
}
void drawBMP(unsigned char bmp_data[], int data_size, int *Values, int NumValues)
{
	memset(bmp_data, 0x00, data_size);

	int columns = data_size / NumValues;
	int index = (data_size / 2 - columns);
	int i = 0;

	if (NumValues % 2 != 0)
		index += columns / 2;

	bmp_data[index] = hexSum(bmp_data[index], myPow(2, (7 - i % 8)));
	i++;
	for (int i = 1; i < NumValues; i++)
	{
		if (NumValues % 2 == 0)
		{
			if (Values[i] > 0 && Values[i] > NumValues / 2)
				Values[i] = NumValues / 2;
			if (Values[i] < 0 && Values[i] < -1 * (NumValues / 2 - 1))
				Values[i] = -1 * (NumValues / 2 - 1);
		}
		else
		{
			if (Values[i] > 0 && Values[i] > NumValues / 2)
				Values[i] = NumValues / 2;
			if (Values[i] < 0 && Values[i] < -1 * (NumValues / 2))
				Values[i] = -1 * (NumValues / 2);
		}

		if (Values[i] > Values[i - 1])
			index += columns;
		else if (Values[i] < Values[i - 1])
			index -= columns;
		bmp_data[index + i / 8] = hexSum(bmp_data[index + i / 8], myPow(2, (7 - i % 8)));
	}
}
void BMPcreator(int *Values, int NumValues)
{
	int f = open("chart.bmp", O_CREAT | O_TRUNC | O_WRONLY, 0644);
	if (f == -1)
	{
		printf("Failed to create file.\n");
		exit(3);
	}
	unsigned char bmp_header[62] = {
		0x42, 0x4D, 0x00, 0x00, 0x00, 0x00, // 0x42, 0x4D, A teljes file merete, (62 byte + pixeltomb)
		0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, // 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00,
		0x00, 0x00, 0x28, 0x00, 0x00, 0x00, // 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // a kep szelessege es
		0x00, 0x00, 0x01, 0x00, 0x01, 0x00, // magassaga,  0x01, 0x00, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Ez a 4 sor
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mind
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // változatlan
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // marad
		0xFF, 0x00, 0x00, 0xFF, 0x00, 0xFF, // B G R A színek a két
		0xFF, 0xFF							// pixelnek
	};
	changeHeader(bmp_header, NumValues); // Header beállítása a megfelelő értékekre

	int data_size = getSize(NumValues) - 62;
	unsigned char bmp_data[data_size];

	drawBMP(bmp_data, data_size, Values, NumValues);

	write(f, &bmp_header, sizeof(bmp_header));
	write(f, &bmp_data, sizeof(bmp_data));
	close(f);
}
int Measurement(int **Values)
{
	srand(time(NULL));
	int szam = time(NULL);
	szam %= 3600;
	int tmp_mins = (szam / 60) % 15;
	int tmp_secs = szam % 60;
	int seconds = tmp_mins * 60 + tmp_secs;
	if (seconds < 100)
		seconds = 100; // 100 min, egyébként az eltelt másodpercek a negyed órában

	float tmp_rand;
	int *Numbers = malloc(seconds * sizeof(int));
	int nums = 0;

	Numbers[0] = nums;
	for (int i = 1; i < seconds; i++)
	{
		tmp_rand = rand() / (float)RAND_MAX;

		if (tmp_rand < 0.428571)
			nums++;
		else if (tmp_rand > 1.0 - (11.0 / 31.0))
			nums--;

		Numbers[i] = nums;
	}
	*Values = Numbers;

	return seconds;
}

int FindPID()
{
	DIR *dir = opendir("/proc");
	if (dir == NULL)
	{
		printf("Error opening directory");
		exit(4);
	}
	struct dirent *entry;
	pid_t ownpid = getpid();
	char filename[256];
	char line[256];
	while ((entry = readdir(dir)) != NULL)
	{
		if (entry->d_name[0] < '0' || entry->d_name[0] > '9')
			continue;

		strncpy(filename, "/proc/", 256);
		strncat(filename, entry->d_name, 256 - strlen(filename));
		strncat(filename, "/status", 256 - strlen(filename));

		FILE *fp = fopen(filename, "r");
		if (fp == NULL)
		{
			printf("Error opening the file");
			exit(5);
		}
		while (fgets(line, 256, fp))
		{ // Az strncmp() úgy működik mint az strcmp(), annyi különbséggel, hogy csak az első n karaktert hasonltja
			if (strncmp(line, "Name:", 5) == 0 && strstr(line, "\tchart\n") != NULL)
			{ // Az strstr() fgv nem NULL pointert ad vissza ha az első
				while (fgets(line, 256, fp))
				{ // string tartalmazza a második stringet.
					if (strncmp(line, "Pid:", 4) == 0)
					{
						int pid = atoi(strtok(line + 4, " \t\n"));
						if (pid != ownpid)
						{ // A PID figyelmen kívül hagyása, ha az megegyezik a program saját PID-jével
							fclose(fp);
							closedir(dir);
							return pid;
						}
					}
				}
			}
		}
		fclose(fp);
	}
	closedir(dir);
	return -1;
}
void SendViaFile(int *Values, int NumValues)
{
	FILE *fp = fopen("Measurement.txt", "w");
	if (fp == NULL)
	{
		printf("Error opening file!\n");
		free(Values);
		exit(6);
	}
	for (int i = 0; i < NumValues; i++)
		fprintf(fp, "%d\n", Values[i]);
	fclose(fp);

	int pid = FindPID();
	if (pid == -1)
	{
		printf("Could not find receiving process.\n");
		free(Values);
		exit(8);
	}
	kill(pid, SIGUSR1);
}
void ReceiveViaFile(int sig)
{
	FILE *fp = fopen("Measurement.txt", "r");
	if (fp == NULL)
	{
		printf("Error opening file.\n");
		exit(7);
	}
	int *Values = NULL;
	int NumValues = 0;
	char line[256];
	while (fgets(line, 256, fp))
	{
		NumValues++;
		int *temp = (int *)realloc(Values, NumValues * sizeof(int));
		if (temp == NULL)
		{
			printf("Error allocating memory.\n");
			free(Values);
			exit(9);
		}
		Values = temp;
		Values[NumValues - 1] = atoi(line);
	}
	fclose(fp);
	BMPcreator(Values, NumValues);
	free(Values);
}
void SignalHandler(int sig)
{
	switch (sig)
	{
	case SIGINT:
		printf("\r     \r"); // A leállításkor terminálba kerülő ^C karakterek felülírása
		printf("Interrupt signal received. Good bye!\n");
		exit(0);
		break;
	case SIGUSR1:
		printf("File transfer service is not available!\n");
		exit(10);
		break;
	case SIGALRM:
		printf("Server did not respond in time.\n");
		exit(11);
		break;
	default:
		printf("Unknown signal received.\n");
		exit(99);
		break;
	}
}
void SendViaSocket(int *Values, int NumValues)
{
	/*********************** Declarations **********************/
	int s;					   // socket ID
	int bytes;				   // received/sent bytes
	int flag;				   // transmission flag
	char on;				   // sockopt option
	unsigned int server_size;  // length of the sockaddr_in server
	struct sockaddr_in server; // address of server

	/************************ Initialization ********************/
	on = 1;
	flag = 0;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	server.sin_port = htons(PORT_NO);
	server_size = sizeof server;

	/************************ Creating socket *******************/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		printf("Socket creation error");
		free(Values);
		exit(12);
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

	/************************ Sending NumValues *****************/
	bytes = sendto(s, &NumValues, sizeof(NumValues), flag, (struct sockaddr *)&server, server_size);
	if (bytes <= 0)
	{
		printf("Sending NumValues error");
		free(Values);
		close(s);
		exit(13);
	}
	printf(" %i bytes have been sent to server.\n", bytes);
	signal(SIGALRM, SignalHandler);
	alarm(1); // 1 másodperces időzítő

	/****************** Receive server's response ***************/
	int server_response;
	bytes = recvfrom(s, &server_response, sizeof(server_response), flag, (struct sockaddr *)&server, &server_size);
	if (bytes < 0)
	{
		printf("Receiving error");
		free(Values);
		close(s);
		exit(14);
	}
	signal(SIGALRM, SIG_IGN); // Ha időben választ kaptunk a szervertől, akkor ignoráljuk a SIGALRM jelet
	alarm(0);				  // Időzítő kikapcsolása

	if (server_response != NumValues)
	{
		fprintf(stderr, "NumValues mismatch: expected %d, received %d\n", NumValues, server_response);
		close(s);
		exit(15);
	}

	/************************ Sending data **********************/
	bytes = sendto(s, Values, NumValues * sizeof(int), flag, (struct sockaddr *)&server, server_size);
	if (bytes <= 0)
	{
		printf("Sending data error");
		close(s);
		exit(16);
	}
	printf(" %i bytes have been sent to server.\n", bytes);

	/****************** Receive server's response ***************/
	int received_bytes;
	bytes = recvfrom(s, &received_bytes, sizeof(received_bytes), flag, (struct sockaddr *)&server, &server_size);
	if (bytes < 0)
	{
		printf("Receiving error");
		close(s);
		exit(14);
	}

	if (received_bytes != NumValues * sizeof(int))
	{
		fprintf(stderr, "Data size mismatch: expected %ld bytes, received %d bytes\n", NumValues * sizeof(long unsigned int), received_bytes);
		close(s);
		exit(17);
	}

	/************************ Closing ***************************/
	close(s);
}
void ReceiveViaSocket()
{
	/*********************** Declarations ***********************/
	int s;							   // socket ID
	int bytes;						   // received/sent bytes
	char on;						   // sockopt option
	unsigned int client_size;		   // length of the sockaddr_in client
	struct sockaddr_in server, client; // address of server and client

	/*********************** Initialization ********************/
	on = 1;
	client_size = sizeof client;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT_NO);

	/************************ Creating socket *******************/
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0)
	{
		printf("Socket creation error");
		exit(18);
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on);
	setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof on);

	/************************ Binding ***************************/
	if (bind(s, (struct sockaddr *)&server, sizeof server) < 0)
	{
		printf("Binding error");
		exit(19);
	}
	while(1)
	{
		/************************ Receiving NumValues ****************/
		int NumValues;
		bytes = recvfrom(s, &NumValues, sizeof(NumValues), 0, (struct sockaddr *)&client, &client_size);
		if (bytes < 0)
		{
			printf("Receiving error");
			close(s);
			exit(20);
		}

		printf("%d values are being sent from client with IP address %s\n", NumValues, inet_ntoa(client.sin_addr));

		/***************** Sending server response *******************/
		bytes = sendto(s, &NumValues, sizeof(NumValues), 0, (struct sockaddr *)&client, client_size);
		if (bytes <= 0)
		{
			printf("Sending response error");
			close(s);
			exit(21);
		}

		/************************ Receiving data **********************/
		int *Values = malloc(NumValues * sizeof(int));
		bytes = recvfrom(s, Values, NumValues * sizeof(int), 0, (struct sockaddr *)&client, &client_size);
		if (bytes < 0)
		{
			printf("Receiving error");
			free(Values);
			close(s);
			exit(22);
		}

		/***************** Sending server response *******************/
		int data_size = NumValues * sizeof(int);
		bytes = sendto(s, &data_size, sizeof(data_size), 0, (struct sockaddr *)&client, client_size);
		if (bytes <= 0)
		{
			printf("Sending response error");
			free(Values);
			close(s);
			exit(23);
		}
		BMPcreator(Values, NumValues);
		free(Values);
	}

	/************************ Closing ***************************/
	close(s);
}