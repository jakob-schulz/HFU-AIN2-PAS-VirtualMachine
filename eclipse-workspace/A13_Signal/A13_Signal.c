/*
 ============================================================================
 Name        : A13_Signals.c
 Author      : EC
 Version     :
 Copyright   :
 Description : kleiner Sig-Handler für USR1 und INT mit Laufschleife
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int i;
int richtung = 1;

void sig_handler(int signo) {
	if (signo == SIGUSR1) {
		printf("received SIGUSR1\n");
		i = 0;
	}
	else if (signo == SIGINT){
		printf("received SIGINT\n");
		i = i * i;
	} else if (signo == SIGKILL)
		printf("received SIGKILL\n");
	else if (signo == SIGSTOP)
		printf("received SIGSTOP\n");
	else if(signo == SIGUSR2)
	{
		printf("received SIGUSR2\n");
		if(richtung == 1)
		{
			richtung = -1;
		}
		else
		{
			richtung = 1;
		}

	}
}

int main(void) {
	printf("Hello World!!!\n"); /* prints !!!Hello World!!! */
	int pid = getpid();

	if (signal(SIGUSR1, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGUSR1\n");
	if (signal(SIGINT, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGINT\n");
	if (signal(SIGKILL, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGKILL\n");
	if (signal(SIGSTOP, sig_handler) == SIG_ERR)
		printf("\ncan't catch SIGSTOP\n");
	if(signal(SIGUSR2, sig_handler) == SIG_ERR)
	{
		printf("\ncan't catch SIGUSR2\n");
	}

	for (i = 0; i * i < 1000000; i = i + richtung) { //hier i * i, weil es ansonsten keine Abbruchbedingung gaebe, wenn Laufvariable negativ ist
		printf("PID= %d, i= %d\n", pid, i);
		sleep(1);
	}

	return EXIT_SUCCESS;
}
