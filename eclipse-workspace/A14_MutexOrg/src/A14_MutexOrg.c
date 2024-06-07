/*
 ============================================================================
 Name        : A14_MutexOrg.c
 Author      : EC
 Version     :
 Copyright   :
 Description : Dining Philosophers w/ Mutex in C, Ansi-style
 Ref		 : http://osandnetworkingcslab.wordpress.com
 	 	 	 : Note - contains C-Programming bugs to be fixed
 ============================================================================
 */

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <unistd.h>		// for sleep

void *philoWork(int n);
pthread_t philosopher[5];
pthread_mutex_t chopstick[5];

int main()
{
	int i,k;
	void *msg;
	for(i=0;i<5;i++)
	{
		k=pthread_mutex_init(&chopstick[i],NULL);
		if(k==-1)
		{
			printf("\n Mutex initialization failed");
			exit(1);
		}
	}
	for(i=0;i<5;i++)
	{
		usleep(100);
		k=pthread_create(&philosopher[i],NULL,(void *)philoWork, (int *) (long)i);
		if(k!=0)
		{
			printf("\n Thread creation error \n");
			exit(1);
		}
	}
	for(i=0;i<5;i++)
	{
		k=pthread_join(philosopher[i],&msg);
		if(k!=0)
		{
			printf("\n Thread join failed \n");
			exit(1);
		}
	}
	for(i=0;i<5;i++)
	{
		k=pthread_mutex_destroy(&chopstick[i]);
		if(k!=0)
		{
			printf("\n Mutex Destroyed \n");
			exit(1);
		}
	}
	printf("\nAll Philosophers are satisfied...\n");
	return 0;
}

void *philoWork(int n)
{
	printf("\nPhilosopher %d is thinking ",n + 1);
	pthread_mutex_lock(&chopstick[n]);
	printf(" -> %d got left chopstick ",n + 1);
	pthread_mutex_lock(&chopstick[(n+1)%5]);
	printf("\nPhilosopher %d is eating ",n + 1);
	pthread_mutex_unlock(&chopstick[n]);
	pthread_mutex_unlock(&chopstick[(n+1)%5]);
	printf("\nPhilosopher %d Finished eating ",n + 1);

	return 0;
}
