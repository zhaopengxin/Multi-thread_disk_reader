#include <iostream>
#include "thread.h"
#include <fstream>

using namespace std;
int max_disk_queue;

void requester(void *a){
	char *disk_file_name = (char *) a;
	int track;
	ifstream ds(disk_file_name);
	while(ds>>track){
		cout<<track<<endl;
	}

}

void servicer(void *a){
	cout<<max_disk_queue<<endl;
}
void open(void *a[]){
	char **argv = (char **) a;
	max_disk_queue = atoi(argv[1]);
	thread service ((thread_startfunc_t) servicer, (void *) argv[1]);
	cout<<"sizeof(argv)/sizeof(argv[1])"<<sizeof(argv)/sizeof(argv[1])<<endl;
	cout<<"sizeof(argv)/sizeof(char *)"<<sizeof(argv)/sizeof(char *)<<endl;
	cout<<"sizeof(argv"<<sizeof(argv)<<"sizeof(argv[1]"<<sizeof(argv[1])<<"sizeof(char *)"<<sizeof(char *)<<endl;
	for (int i = 2; i < sizeof(argv)/sizeof(argv[1]); ++i)
	{
		//char thread_name[] = {'t', (char) i};
		thread thread_name ((thread_startfunc_t) requester, (void *) argv[i]);
	}
}


int main(int argc, char *argv[])
{
	cpu::boot((thread_startfunc_t) open, (void **) argv, 0);
	return 0;
}