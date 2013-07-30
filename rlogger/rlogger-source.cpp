//Remote logger server: logs messages by publishing its process id and
//the message content; remote clients subscribe to log output through a
//broker
//Author: Ugo Varetto

//Note: UNIX only; for windows use DWORD type instead of pid_t and
//GetProcessId instead of getpid

#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
//for framework builds on Mac OS:
#ifdef __APPLE__
#include <ZeroMQ/zmq.h>
#else 
#include <zmq.h>
#endif

typedef pid_t PID;

//------------------------------------------------------------------------------
PID get_proc_id() {
    return getpid();
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "usage: " 
                  << argv[0] 
                  << " <broker URI>"
                  << std::endl;
        std::cout << "Example: logger \"tcp://logbroker:5555\"\n";          
        return 0;          
    }
    LogSource< int > log(argv[1], int(get_proc_id());
    std::cout << "PID: " << int(get_proc_id()) << std::endl;
    while(1) {
    	ls.Log("hello");
    	sleep(1);
    }
    return 0;
}



