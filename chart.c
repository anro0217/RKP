#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "myfunctions.h"

char* HELP = {
"Command line arguments:\n\
OPERATING MODE:\n\
-send: (default) data creation\n\
-receive: data processing and representation\n\
COMMUNICATION MODE:\n\
-file: (default) data transfer via file\n\
-socket: data transfer via socket\n\
OTHER:\n\
--help: list available switches\n\
--version: about the code"};

int main(int argc, char* argv[])
{
	signal(SIGINT, SignalHandler);
	signal(SIGUSR1, SignalHandler);

	if(strcmp(argv[0],"./chart")!=0){
		printf("Name of the file must be \"chart\"!");
		exit(2);
	}
	if(argc == 1 || argc > 3){
		puts(HELP);
		exit(1);
	}
	if(argc == 2)
	{
		if(strcmp(argv[1],"--help") == 0) //Help case
		{
			puts(HELP);
			exit(0);
		}
		else if(strcmp(argv[1],"--version") == 0) //Version case
		{
			#pragma omp parallel num_threads(3)
			{
				#pragma omp sections //A kiiratási feladat részekre osztása
				{
					#pragma omp section
					{
						printf("Version 1.0\n");
					}
					#pragma omp section
					{
						printf("Created: 2023.04.26.\n");
					}
					#pragma omp section
					{
						printf("Created by: Roland Antal\n");
					}
				}
			}
			exit(0);
		}
		else if(strcmp(argv[1],"-send") == 0)
		{
			//Küldő üzemmód és file komm. mód kiválasztva
			int *Values;
			int NumValues = Measurement(&Values);
			SendViaFile(Values,NumValues);
			free(Values);
			exit(0);
		}
		else if(strcmp(argv[1],"-receive") == 0)
		{
			//Fogadó üzemmód és file komm. mód kiválasztva
			signal(SIGUSR1, ReceiveViaFile);
			while(1) 
			{
				pause();
				printf("BMP file is ready!\n");
			}
		}
		else if(strcmp(argv[1],"-file") == 0)
		{
			//Küldő üzemmód és file komm. mód kiválasztva
			int *Values;
			int NumValues = Measurement(&Values);
			SendViaFile(Values,NumValues);
			free(Values);
			exit(0);
		}
		else if(strcmp(argv[1],"-socket") == 0)
		{
			//Küldő üzemmód és socket komm. mód kiválasztva
			int *Values;
			int NumValues = Measurement(&Values);
			SendViaSocket(Values,NumValues);
			free(Values);
			exit(0);
		}
		else
		{
		 	puts(HELP);
			exit(1);
		}
	}
	if(argc == 3)
	{
		if((strcmp(argv[1],"-send") == 0 && strcmp(argv[2],"-file") == 0) ||
		(strcmp(argv[2],"-send") == 0 && strcmp(argv[1],"-file") == 0))
		{
			//Küldő üzemmód és file komm. mód kiválasztva
			int *Values;
			int NumValues = Measurement(&Values);
			SendViaFile(Values,NumValues);
			free(Values);
			exit(0);
		}
		else if((strcmp(argv[1],"-receive") == 0 && strcmp(argv[2],"-file") == 0) || 
		(strcmp(argv[2],"-receive") == 0 && strcmp(argv[1],"-file") == 0))
		{
			//Fogadó üzemmód és file komm. mód kiválasztva
			signal(SIGUSR1, ReceiveViaFile);
			while(1) 
			{
				pause();
				printf("BMP file is ready!\n");
			}
		}
		else if((strcmp(argv[1],"-send") == 0 && strcmp(argv[2],"-socket") == 0) ||
		(strcmp(argv[2],"-send") == 0 && strcmp(argv[1],"-socket") == 0))
		{
			//Küldő üzemmód és socket komm. mód kiválasztva
			int *Values;
			int NumValues = Measurement(&Values);
			SendViaSocket(Values,NumValues);
			free(Values);
			exit(0);
		}
		else if((strcmp(argv[1],"-receive") == 0 && strcmp(argv[2],"-socket") == 0) ||
		(strcmp(argv[2],"-receive") == 0 && strcmp(argv[1],"-socket") == 0))
		{
			//Fogadó üzemmód és socket komm. mód kiválasztva
			ReceiveViaSocket();
		}
		else
		{
			puts(HELP);
			exit(1);
		} 
	}
	return 0;
}
