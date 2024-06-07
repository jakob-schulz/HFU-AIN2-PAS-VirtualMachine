/*
 ============================================================================
 Name        : A14_Threads01.c
 Author      : EC
 Version     :
 Copyright   :
 Description : Threads with loop
 	 	 	 : http://www.cs.kent.edu/~ruttan/sysprog/lectures/multi-thread/multi-thread.html
 ============================================================================
 */
#include <stdio.h>       /* standard I/O routines */
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>     /* pthread functions and data structures */

/* function to be executed by the new thread */
void* doTheWork(void* data)
{
	int i;						/* counter, to print numbers */
	int j;						/* counter, for delay        */
	int me = *((int*)data);     /* thread identifying number */
	for (i=0; i<20; i++) {
		for (j=0; j<50000; j++) /* delay loop */
		{
			usleep(100);
		}
		printf("Thread= %d, Counter= %d\n", me, i);
	}
	/* terminate the thread */
	pthread_exit(NULL);
}

/* like any C program, program's execution begins in main */
int main(int argc, char* argv[])
{
	int        thread_id;         /* thread ID for the newly created thread */
	pthread_t  p_thread;       /* thread's structure                     */
	int        mainThr        = 0;  /* Main thread identifying number            */
	int        subThr         = 1;  /* Sub-thread identifying number            */

	printf("Process ID= %d\n", getpid());
	/* create a new thread that will exec doTheWork()  */
	thread_id = pthread_create(&p_thread, NULL, doTheWork, (void*)&subThr); //Erzeugen eines Threads mit einer (thread_Struktur, Attributen, Einstiegsfunktion (ähnlich main), argumente für die Einstiegsfunktion  )?
	if (thread_id != 0) {
		printf("Error\n");
		exit(0);
	}
	/* exec doTheWork() in the main thread as well */
	doTheWork((void*)&mainThr);
	/* NOT REACHED */
	return 0;
}
