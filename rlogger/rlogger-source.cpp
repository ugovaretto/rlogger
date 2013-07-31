//Remote logger server: logs messages by publishing its process id and
//the message content; remote clients subscribe to log output through a
//broker
//Author: Ugo Varetto

//Note: UNIX only; for windows use DWORD type instead of pid_t and
//GetProcessId instead of getpid


#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "LogSource.h"

typedef pid_t PID;

//------------------------------------------------------------------------------
PID get_proc_id() {
    return PID(getpid());
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    using namespace rlog;
    if(argc < 2) {
        std::cout << "usage: " 
                  << argv[0] 
                  << " <broker URI>"
                  << std::endl;
        std::cout << "Example: logger \"tcp://logbroker:5555\"\n";          
        return 0;          
    }
    RloggerCatchSignals();
    try {
        LogSource< int > ls(argv[1], int(get_proc_id()));
        std::cout << "PID: " << int(get_proc_id()) << std::endl;
        while(!Interrupted()) {
    	   ls.Log("hello");
    	   sleep(1);
        }
        std::cout << "\rInterrupted" << std::endl;
        ls.Clear();
    } catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}



