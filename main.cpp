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
map<string, bool> active_requester_list;
unsigned int active_requester;
int current_track = 0;
cv servicer_cv;

mutex write_lock;

void requester(void *a){
	//Worker *worker = (Worker *) a;
	// string disk_name_string = worker->disk_name;
	// int worker_id_int = worker->worker_id;
	char *disk_name = (char *) a;
	string disk_name_string = disk_name;
	// write_lock.lock();
	// cout<<"I am worker No. "<<disk_name_string<<endl;
	// //cout<<"I am worker No. "<<disk_name_string<<endl;
	// write_lock.unlock();
	//DiskRequester w(disk_name_string, worker_id_int);

	DiskRequester w(disk_name_string);
	int track;
	ifstream ds(disk_name_string);
	while(ds>>track){
		mutex1.lock();
		while((request_queue.size() == max_disk_queue) || (request_queue.find(disk_name_string) != request_queue.end())){
			w.cv1.wait(mutex1);
		}
		write_lock.lock();
		cout<<"requester "<<w.file_name<<" track "<<track<<endl;
		write_lock.unlock();
		Request_form track_and_cv;
		track_and_cv.track_num = track;
		track_and_cv.cv2 = &w.cv1;
		request_queue[disk_name_string] = track_and_cv;
		servicer_cv.signal();
		mutex1.unlock();
	}
	write_lock.lock();
	cout<<"worker "<<disk_name_string<<" end!!"<<endl;
	write_lock.unlock();
	mutex1.lock();
	active_requester_list[disk_name_string] = false;
	active_requester = active_requester - 1;
	mutex1.unlock();
}

void servicer(void *a){
	
	while(true){
		mutex1.lock();
		unsigned int largest_possible_request = min(max_disk_queue, active_requester);
		write_lock.lock();
		cout<<"largest_possible_request"<<largest_possible_request<<endl;
		cout<<"request_queue"<<request_queue.size()<<endl;
		cout<<"active_requester"<<active_requester<<endl;
		write_lock.unlock();
		if(largest_possible_request == 0 && request_queue.size() == 0){
			cout<<"largest_possible_request"<<largest_possible_request<<endl;
			cout<<"request_queue"<<request_queue.size()<<endl;
			cout<<"active_requester"<<active_requester<<endl;
			break;
		}
		while(request_queue.size() < largest_possible_request){
			write_lock.lock();
			cout<<"servicer waiting"<<endl;
			write_lock.unlock();
			servicer_cv.wait(mutex1);
		}
		// if(request_queue.size() != largest_possible_request){
		// 	continue;
		// }
		write_lock.lock();
		cout<<"servicer wake up!!!!!"<<endl;
		write_lock.unlock();
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
		write_lock.lock();
		cout<<"service requester "<<file_to_delete<<" track "<<track_to_service<<endl;
		write_lock.unlock();
		current_track = track_to_service;
		request_queue.erase(file_to_delete);
		//cout<<"I line 87 alive! "<<endl;
		if(active_requester_list[file_to_delete]){
			cv_to_service->signal();
		}
		//cout<<"I line 89 alive! "<<endl;
		mutex1.unlock();
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
	
	for (int i = 2; i < num_of_argv; i++)
	{
		Worker worker;
		worker.disk_name = argv[i];
		worker.worker_id = i;
		Worker *worker_pointer;
		worker_pointer = &worker;
		active_requester_list[argv[i]] = true;
		// write_lock.lock();
		// cout<<"creating worker "<<argv[i]<<endl;
		// write_lock.unlock();
		//thread thread_name ((thread_startfunc_t) requester, (void *) worker_pointer);
		thread thread_name ((thread_startfunc_t) requester, (void *) argv[i]);
	}
}


int main(int argc, char *argv[])
{
	num_of_argv = argc;
	cpu::boot((thread_startfunc_t) open, (void **) argv, 0);
	return 0;
}