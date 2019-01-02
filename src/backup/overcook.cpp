/* 	FILE  		: overcook.cpp 
 *	AUTHOR		: Brian, Lee
 *	Description	: Homework 3 of Embedded Operating System
 *
 */

#include "cook.h"
#include <pthread.h>

using namespace std;

#define NUM_COOKERS 2

void* clerk_handler(void *overcook_ptr);
void* cook_handler(void* overcook_ptr);

pthread_mutex_t mutex;

int main(){

	char steps_file[] = "steps.txt";
	char tools_file[] = "tools.txt";
	
	pthread_t clerk, cooks[NUM_COOKERS];
	pthread_attr_t attr;
	int rc;
	void* status;

	cook me;
	me.setSteps(steps_file);
	me.setTool(tools_file);
	
	/* creating threads */
	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// 1. creating clerk 
	rc = pthread_create(&clerk, &attr, &clerk_handler, (void *)&me);	
	if(rc){
		cerr << "ERROR; return code from  pthread_create() is " << rc << endl;
		exit(-1);
	}

	// 2. creating cooks
	for(int i=0; i<NUM_COOKS; ++i){
		rc = pthread_create(&cooks[i], &attr, &cook_handler, (void *)&me);
		if(rc){
			cerr << "ERROR; return code from pthread_create() is " << rc << endl;
			exit(-1);
		}
	}

	/* wait for threads and destroy attributes */
	pthread_attr_destroy(&attr);	
	for(int i=0; i<NUM_COOKERS; ++i){
		pthread_join(cooks[i]);
	}

	pthread_mutex_destroy(&mutex);

	return 0;
}

void* clerk_handler(void *overcook_ptr){

	return NULL;
}

void* cook_handler(void* overcook_ptr){

	while(1){
		//get order
		
		//find steps
		
		//make
		
		//stack the missing material if fail to make
		
		//continue if success to make
		
		//exit if all orders are done
		
		
	}

	return NULL;
}
