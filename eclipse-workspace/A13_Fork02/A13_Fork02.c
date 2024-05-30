/*
 ============================================================================
 Name        : A13_Signals.c
 Author      : EC
 Version     :
 Copyright   :
 Description : Fork mit 2 unabh. Laufvariablen (Parent/Child)
 ============================================================================
 */

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

// dies ist, was im Child-Pr. laufen soll
int doChild(){
		int i =0;
		while (1) {
			printf("\tChild: i= %d\n",i);
			i += 1;
			sleep(1);
		}
}

// dies ist, was im Parent-Pr. laufen soll
int doParent(){
	int i = 0;
		while (1) {
			printf("Parent: i= %d\n",i);
			i += 2;
			sleep(1);
		}
}

int main (void) {
   pid_t pid;			// hier wird die Prozess-ID abgelegt

   // jetzt wollen wir mit fork() einen Child-Prozess erzeugen
   // und das Ergebnis auswerten
   switch (pid = fork ()) {
   case -1:
      printf ("Fehler bei fork()\n");
      break;
   case 0:
      doChild();
      break;
   default:
	  sleep (1);   /* Kurze Pause */
      doParent();
      break;
   }
   return EXIT_SUCCESS;
}
