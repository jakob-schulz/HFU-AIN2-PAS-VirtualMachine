#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

//Folgendes soll im Child Prozess laufen:
int doChild() {
	int i = 0;
	int richtung = 1;
	//Es ist moeglich den Signal-Handler innerhalb einer Funktion zu definieren. Man kann dann jedoch auch nur innerhalb dieser auf diesen Signal Handler zugreifen
	void sig_handler_child(int signo) {
		if (signo == SIGUSR1) {
			printf("received SIGUSR1\n");
			i = 0;
		} else if (signo == SIGINT) {
			printf("received SIGINT\n");
			i = i * i;
		} else if (signo == SIGKILL)
			{printf("received SIGKILL\n");}
		else if (signo == SIGSTOP)
			{printf("received SIGSTOP\n");}
		else if (signo == SIGUSR2) {
			printf("received SIGUSR2\n");
			if (richtung == 1) {
				richtung = -1;
			} else {
				richtung = 1;
			}
		}
		else if(signo == SIGHUP)
		{
			printf("My Parent wants me to hang up!!");
			exit(0);
		}
	}
	//Festlegen, bei welchen Signal, welcher Signal-Handler aufgerufen wird.
	signal(SIGUSR1, sig_handler_child);
	signal(SIGINT, sig_handler_child);
	signal(SIGKILL, sig_handler_child);
	signal(SIGSTOP, sig_handler_child);
	signal(SIGUSR2, sig_handler_child);
	signal(SIGHUP, sig_handler_child);

	while (richtung == 1) {
		printf("\tChild: i= %d\n", i);
		i += 1;
		sleep(1);
	}
	while (richtung == -1) {
		printf("\tChild: i= %d\n", i);
		i = i - 1;
		sleep(1);
	}
}

//Folgendes soll im Parent-Prozess laufen
int doParent(pid_t child_pid) {
	int i = 0;
	//Signal-Handler fuer den Parent-Prozess
	void sig_handler_parent(int signum)
	{
		if(signum == SIGHUP)
		{
			kill(child_pid,SIGSTOP);
		}
		if(signum == SIGCONT)
		{
			kill(child_pid,SIGCONT);
		}
	}
	//Festlegen, bei welchen Signal, welcher Signal-Handler aufgerufen wird
	signal(SIGHUP, sig_handler_parent);
	signal(SIGCONT, sig_handler_parent);
	while (1) {
		printf("Parent: i= %d\n", i);
		if (i == 20) {
			kill(child_pid, SIGUSR2); //Schickt Signal SIGUSR2 an den Child-Prozess
		}
		if (i == 30) {
			kill(child_pid, SIGUSR1);
		}
		if (i == 70)
		{
			kill(child_pid, SIGHUP);
			break;
		}
		i += 2;
		sleep(1);
	}
}

int main(void) {
	pid_t pid; // hier wird die Prozess-ID abgelegt
	//Mit fork einen Child-Prozess (genaue Kopie des Prozesses) erzeugen und Ergebnis auswerten
	switch (pid = fork()) { //fork gibt als Rueckgabe die PID des Child-Prozesses
	case -1:
		printf("Fehler bei fork()\n");
		break;
	case 0:
		doChild();
		break;
	default:
		sleep(1); /* Kurze Pause */
		doParent(pid); //bekommt die Prozess-ID uebergeben, damit man weis, was Child-Prozess ist.
		break;
	}
	return EXIT_SUCCESS;
}