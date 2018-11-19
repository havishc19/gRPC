#include "threadpool.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
#include "clientServer.cc"

class store1 { 

};

int main(int argc, char** argv) {
	string IP_ADDR_HOST;
	int maxThreads;
	if (argc == 3) {
		IP_ADDR_HOST = std::string(argv[1]);
		maxThreads = std::min( 20, std::max(0,atoi(argv[2])));
	}
	else if (argc == 2) {
		IP_ADDR_HOST = std::string(argv[1]);
		maxThreads = 4;
	}
	else {
		IP_ADDR_HOST = "0.0.0.0:50060";
		maxThreads = 4;
	}
	string line;
  	ifstream myfile ("vendor_addresses.txt");
  	vector<string> vendorAddresses;
  	if (myfile.is_open())
  	{
    	while ( getline (myfile,line) )
    	{
    	  // cout << line << '\n';
    	  vendorAddresses.push_back(line);
    	}
    	myfile.close();
  	}
  	else{
  		 cout << "Unable to open file"; 
  		 return -1;	
  	}
	launchServer(vendorAddresses, IP_ADDR_HOST, maxThreads);
	return EXIT_SUCCESS;
}

