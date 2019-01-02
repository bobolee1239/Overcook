/* overcook.cpp */
#include "cook.h"
#include <pthread.h>

using namespace std;
#define NUM_COOKERS 2

void* clerkThread(void* cook_ptr);
void* cookThread(void* cook_ptr);

bool toCon = true;
vector<string> makeList;
pthread_mutex_t mutex;

int main(){
	char steps_file[] = "steps.txt";
	char tools_file[] = "tools.txt";
	
	cook me;
	me.setSteps(steps_file);
	me.setTool(tools_file);


	pthread_t cookers[NUM_COOKERS], clerk;
	pthread_attr_t attr;
	int rc;
	void* status;


	/* ---------- THREADS ----------
	 *
	 * 1. Init and set thread detached attribute 
	 * 2. Create Threads 
	 *
	 */
	
	pthread_mutex_init(&mutex, NULL);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

#ifdef DEBUG
	cout << "In main(): creating clerk ..." << endl;
#endif 

	rc = pthread_create(&clerk, &attr, &clerkThread, (void *)&me);
	if(rc){
#ifdef DEBUG
		cerr << "ERROR; return code from pthread create() is " << rc << endl;
#endif
		exit(-1);
	}

	for(int i=0; i<NUM_COOKERS; ++i){
#ifdef DEBUG
		cout << "In main(): creating cooker #" << i << " ..." << endl;
#endif
		rc = pthread_create(&cookers[i], &attr, &cookThread, (void *)&me);
		if(rc){
#ifdef DEBUG
			cerr << "ERROR; return code from pthread create() is " << rc << endl;
#endif
			exit(-1);
		}
	}

	
	/* 		
     *		FREE ATTR and wait for the other threads 
	 *
	 */
	
	pthread_attr_destroy(&attr);
	rc = pthread_join(clerk, &status);
	if(rc){
#ifdef DEBUG
		cout << "ERROR; return code from pthread_join() is " << rc << endl;
#endif
		exit(-1);
	}
	for(int i=0; i<NUM_COOKERS; ++i){
		rc = pthread_join(cookers[i], &status);
		if(rc){
#ifdef DEBUG
			cout << "ERROR; return code from pthread_join() is " << rc << endl;
#endif
			exit(-1);
		}
	}
	pthread_mutex_destroy(&mutex);


	return 0;
}

void* clerkThread(void* cook_ptr){
	cook* cooker = (cook*)cook_ptr;	
	string order;
	while(toCon){
		order = cooker->getOrder();
		if(order == ""){
			toCon = false;
		} 
#ifdef DEBUG
		else if(order == "storage"){
			cooker->showStorage();
		} 
#endif
		else {
			pthread_mutex_lock(&mutex);
			makeList.push_back(order);
			pthread_mutex_unlock(&mutex);
		}
	}


	return NULL;
}

void* cookThread(void* cook_ptr){
	cook* cooker = (cook*)cook_ptr;

	string order;
	while(toCon){

		// Lock thread 
		pthread_mutex_lock(&mutex);

		if(makeList.empty()){
			pthread_mutex_unlock(&mutex);
			continue;
		}
		order = makeList.back();
		makeList.pop_back();

		// Unlock thread
		pthread_mutex_unlock(&mutex);
		
		//find steps
		const cook::Step* step = cooker->find(order);
		if(step == NULL)	continue;

		//make
		pthread_mutex_lock(&mutex);
		vector<string> materials = cooker->check_storage(step);
		/*
		for(vector<string>::iterator itr=materials.begin(); itr!=materials.end(); ++itr){
			cout << "In main(); "<< (*itr) << ", ";
		}
		*/
		pthread_mutex_unlock(&mutex);

		//stack the missing material if fail to make
		if (materials.empty()){
			// make product
			pthread_mutex_lock(&mutex);
			cooker->consume_and_take(step);
			pthread_mutex_unlock(&mutex);

			cooker->make(step);
			pthread_mutex_lock(&mutex);
			cooker->put_storage_and_return(step);
			pthread_mutex_unlock(&mutex);
		} else if (materials.front() == "notools"){
			// no tools
			continue;
		} else {
			// make needed material first
			pthread_mutex_lock(&mutex);
			makeList.push_back(order);
			for(vector<string>::iterator itr=materials.begin(); itr!=materials.end(); ++itr){
				makeList.push_back(*itr);
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

