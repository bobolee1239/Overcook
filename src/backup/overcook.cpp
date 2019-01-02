/* 	FILE  		: overcook.cpp 
 *	AUTHOR		: Brian, Lee
 *	Description	: Homework 3 of Embedded Operating System
 *
 */

#include "cook.h"
#include <signal.h>
#include <pthread.h>

using namespace std;

#define NUM_COOKERS 4

void* clerk_handler(void *overcook_ptr);
void* cook_handler(void* overcook_ptr);

/* thread variables: define globally for sending signal sake */
pthread_mutex_t mutex;
bool toCon = true;

int main(){
	
	char steps_file[] = "steps.txt";
	char tools_file[] = "tools.txt";

	cook me;
	me.setSteps(steps_file);
	me.setTool(tools_file);
	
	pthread_t clerk, cooks[NUM_COOKERS];
	pthread_attr_t attr;
	int rc;
	void* status;

	/* creating threads */
	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// 1. creating clerk 
#ifdef DEBUG
	cout << "In main(): creating clerk ..." << endl;
#endif
	rc = pthread_create(&clerk, &attr, &clerk_handler, (void *)&me);	
	if(rc){
#ifdef DEBUG
		cerr << "ERROR; return code from  pthread_create() is " << rc << endl;
#endif 
		exit(-1);
	}

	// 2. creating cooks
	for(int i=0; i<NUM_COOKERS; ++i){
#ifdef DEBUG
		cout << "In main(): creating cook #" << i << " ..." << endl;
#endif
		rc = pthread_create(&cooks[i], &attr, &cook_handler, (void *)&me);
		if(rc){
#ifdef DEBUG
			cerr << "ERROR; return code from pthread_create() is " << rc << endl;
#endif
			exit(-1);
		}
	}

	/* wait for threads and destroy attributes */
	pthread_attr_destroy(&attr);	

	rc = pthread_join(clerk, &status);
	if(rc){
#ifdef DEBUG
		cerr << "ERROR; return code from pthread_join() is " << rc << endl;
#endif
		exit(-1);
	}

	for(int i=0; i<NUM_COOKERS; ++i){
		pthread_join(cooks[i], &status);
		if(rc){
#ifdef DEBUG
			cerr << "ERROR; return code from pthread_join() is " << rc << endl;
#endif
			exit(-1);
		}
	}

	pthread_mutex_destroy(&mutex);
	
	cout << "end" << endl;
	return 0;
}

void* clerk_handler(void *overcook_ptr){
	cook* overcook = (cook *)overcook_ptr;
	string order;
	int rc;

	while(1){
		order = overcook->getOrder();
		if(order == ""){
			/*
			 // sending signal for each cook thread to terminate 
			for(int i=0; i<NUM_COOKERS; ++i){
				rc = pthread_kill(cooks[i], SIGQUIT);
				// TODO : check resuts 
				
			}
			*/
			toCon = false;
			pthread_exit(NULL);
		}

#ifdef DEBUG
		else if(order == "storage"){
			overcook->showStorage();
		}
#endif
		else{
			pthread_mutex_lock(&mutex);
			overcook->push_order(order);
			pthread_mutex_unlock(&mutex);
		}
	}
}

void* cook_handler(void* overcook_ptr){
	cook* overcook = (cook*)overcook_ptr;

	string order;
	vector<string> materials;

	while(toCon){
		/* get order */
		pthread_mutex_lock(&mutex);
		order = overcook->pop_order();
		pthread_mutex_unlock(&mutex);

		/* if order list is empty */
		if(order == "")		continue;

		/* find steps */
		const cook::Step* step = overcook->find(order);
		if(step == NULL)	continue;
		
		/* check materials and tools */
		pthread_mutex_lock(&mutex);
		materials = overcook->check_n_take(step);
		pthread_mutex_unlock(&mutex);

		if(materials.empty()){
			/* make */
			string product = overcook->make(step);

			/* return tools and update storage*/
			pthread_mutex_lock(&mutex);
			overcook->put_storage_and_return_tools(step);
			pthread_mutex_unlock(&mutex);
		} 
		else if(materials.back() == "notools"){ continue; } 
		else{
			/* stack the missing material if fail to make */
			pthread_mutex_lock(&mutex);
			overcook->push_order(order);
			overcook->push_orders(materials);
			pthread_mutex_unlock(&mutex);
		}
	}

	pthread_exit(NULL);
}
