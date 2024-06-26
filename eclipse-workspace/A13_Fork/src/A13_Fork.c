/*
 ============================================================================
 Name        : A13_Fork02.c
 Author      : EC
 Version     : test für Fork: Child-Prozess erzeugen, Speicheradressen
 Copyright   :
 Description : siehe auch http://openbook.rheinwerk-verlag.de/linux_unix_programmierung
 ============================================================================
 */

#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

static int global_var = 1;

int main (void) {
   pid_t pid;			// hier wird die Prozess-ID abgelegt
   int lokal_var = 1;

   // jetzt wollen wir mit fork() einen Child-Prozess erzeugen
   // und das Ergebnis auswerten
   switch (pid = fork ()) {
   case -1:
      printf ("Fehler bei fork()\n");
      break;
   case 0:
      sleep (1);   /* Kurze Pause */
      printf ("--- Im Kindprozess ---\n");
      printf ("global_var = %d, Speicheradresse : %p\n",
                global_var, &global_var);
      printf ("lokal_var  = %d, Speicheradresse : %p\n",
                lokal_var, &lokal_var);
      ++global_var;
      ++lokal_var;
      printf ("--- Im Kindprozess ---\n");
      printf ("global_var = %d, Speicheradresse : %p\n",
                global_var, &global_var);
      printf ("lokal_var  = %d, Speicheradresse : %p\n",
                lokal_var, &lokal_var);
      break;
   default:
      printf ("--- Im Elternprozess ---\n");
      printf ("global_var = %d, Speicheradresse : %p\n",
                global_var, &global_var);
      printf ("lokal_var  = %d, Speicheradresse : %p\n",
                lokal_var, &lokal_var);
      sleep (2);
      printf ("--- Im Elternprozess ---\n");
      printf ("global_var = %d, Speicheradresse : %p\n",
                global_var, &global_var);
      printf ("lokal_var  = %d, Speicheradresse : %p\n",
                lokal_var, &lokal_var);
      break;
   }
   return EXIT_SUCCESS;
}
