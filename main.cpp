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
};
unsigned int max_disk_queue;
int num_of_argv;
map<string, Request_form> request_queue;
mutex mutex1;
map<string, int> active_requester_list;
unsigned int active_requester;
int current_track = 0;
cv servicer_cv;
cv requester_cv;
unsigned int largest_possible_request;

mutex write_lock;

void requester(void *a){
	mutex1.lock();
	char *disk_name = (char *) a;
	string disk_name_string = disk_name;

	DiskRequester w(disk_name_string);
	int track;
	ifstream ds(disk_name_string);
	while(ds>>track){
		largest_possible_request = min(max_disk_queue, active_requester);
		while((request_queue.size() == largest_possible_request) || (request_queue.find(disk_name_string) != request_queue.end())){
			requester_cv.wait(mutex1);
		}

		write_lock.lock();
		cout<<"requester "<<active_requester_list[w.file_name]<<" track "<<track<<endl;
		write_lock.unlock();
		Request_form track_and_cv;
		track_and_cv.track_num = track;
		request_queue[disk_name_string] = track_and_cv;
		servicer_cv.signal();
	}

	active_requester = active_requester - 1;
	servicer_cv.signal();
	mutex1.unlock();
	return;
}

void servicer(void *a){
	mutex1.lock();
	while(true){
		
		largest_possible_request = min(max_disk_queue, active_requester);
		write_lock.lock();
		write_lock.unlock();
		if(largest_possible_request == 0 && request_queue.size() == 0){
			break;
		}
		while(request_queue.size() < largest_possible_request){
			servicer_cv.wait(mutex1);
			largest_possible_request = min(max_disk_queue, active_requester);
		}
		string file_to_delete;
		int track_to_service;
		int diff = abs(request_queue.begin()->second.track_num - current_track);
		for(auto iter = request_queue.begin(); iter != request_queue.end();iter++){
			//cout<<"The difference of "<<file_to_delete<<" to "<<current_track<<" is "<<diff<<endl;
			if(abs(iter->second.track_num - current_track) <= diff){
				diff = abs(iter->second.track_num - current_track);
				track_to_service = iter->second.track_num;
				file_to_delete = iter->first;
			}
		}
		write_lock.lock();
		cout<<"service requester "<<active_requester_list[file_to_delete]<<" track "<<track_to_service<<endl;
		write_lock.unlock();
		current_track = track_to_service;
		request_queue.erase(file_to_delete);
		requester_cv.broadcast();
	}
	mutex1.unlock();
	return;
}

void open(void *a[]){
	char **argv = (char **) a;
	max_disk_queue = atoi(argv[1]);
	active_requester = num_of_argv - 2;
	largest_possible_request = max_disk_queue;
	thread service ((thread_startfunc_t) servicer, (void *) argv[1]);
	
	for (int i = 2; i < num_of_argv; i++)
	{
		active_requester_list[argv[i]] = i-2;
		thread thread_name ((thread_startfunc_t) requester, (void *) argv[i]);
	}
}


int main(int argc, char *argv[])
{
	num_of_argv = argc;
	cpu::boot((thread_startfunc_t) open, (void **) argv, 0);
	return 0;
}