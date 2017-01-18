#include <iostream>
#include "thread.h"
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>

#include "DiskRequester.cpp"

using namespace std;
struct Worker{
	string disk_name;
	int worker_id;
};
struct Request_form
{
	int track_num;
	cv *cv2;
};
unsigned int max_disk_queue;
int num_of_argv;
map<string, Request_form> request_queue;
mutex mutex1;
unsigned int active_requester;
int current_track = 0;

void requester(void *a){
	//active_requester = active_requester + 1;

	Worker *worker = (Worker *) a;
	string disk_name_string = worker->disk_name;
	int worker_id_int = worker->worker_id;
	DiskRequester w(disk_name_string, worker_id_int);


	int track;
	ifstream ds(disk_name_string);
	while(ds>>track){
		mutex1.lock();
		if((request_queue.size() == max_disk_queue) || (request_queue.find(disk_name_string) != request_queue.end())){
			w.cv1.wait(mutex1);
		}
		cout<<"requester "<<w.file_name<<" track "<<track<<endl;
		Request_form track_and_cv;
		track_and_cv.track_num = track;
		track_and_cv.cv2 = &w.cv1;
		request_queue[disk_name_string] = track_and_cv;
		mutex1.unlock();
	}
	active_requester = active_requester - 1;
}

void servicer(void *a){
	while(true){
		unsigned int largest_possible_request = min(max_disk_queue, active_requester);
		if(request_queue.size() != largest_possible_request){
			continue;
		}
		string file_to_delete;
		int track_to_service;
		cv *cv_to_service;
		int diff = abs(request_queue.begin()->second.track_num - current_track);
		for(auto iter = request_queue.begin(); iter != request_queue.end();iter++){
			if(abs(iter->second.track_num - current_track) <= diff){
				track_to_service = iter->second.track_num;
				cv_to_service = iter->second.cv2;
				file_to_delete = iter->first;
			}
		}
		cout<<"service requester "<<file_to_delete<<" track "<<track_to_service<<endl;
		current_track = track_to_service;
		cv_to_service->signal();
		request_queue.erase(file_to_delete);
	}
}

void open(void *a[]){
	char **argv = (char **) a;
	max_disk_queue = atoi(argv[1]);
	active_requester = num_of_argv - 2;
	// for(int j = 0; j < num_of_argv - 2; j++){
	// 	cv j;
	// 	cv_list.add(j);
	// }
	thread service ((thread_startfunc_t) servicer, (void *) argv[1]);
	cout<<"how many worker thread"<<num_of_argv-2<<endl;
	for (int i = 2; i < num_of_argv; i++)
	{
		Worker worker;
		worker.disk_name = argv[i];
		worker.worker_id = i;
		Worker *worker_pointer;
		worker_pointer = &worker;
		thread thread_name ((thread_startfunc_t) requester, (void *) worker_pointer);
	}
}


int main(int argc, char *argv[])
{
	num_of_argv = argc;
	cpu::boot((thread_startfunc_t) open, (void **) argv, 0);
	return 0;
}