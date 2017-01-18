#include <string>
#include "thread.h"

using namespace std;
class DiskRequester
{
public:

	DiskRequester(string diskname, int worker_id){
		file_name = diskname;
		id = worker_id;
	};
	//~DiskRequester(){};
	string file_name;
	int id;
	cv cv1;
	/* data */
};