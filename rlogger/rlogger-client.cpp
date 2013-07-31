//Remote logger client: subscribe to specific process ids to receive
//log messages.
//Author: Ugo Varetto

//Note: UNIX only; for windows use DWORD type instead of pid_t

#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <cstdlib>

#include "LogClient.h"

typedef int PID;

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    using namespace rlog;
    if(argc < 2) {
        std::cout << "usage: " 
                  << argv[0] 
                  << " <server URI> [process id]"
                  << std::endl;
        std::cout << "Example: client \"tcp://logbroker:5555\" 27852\n";
        std::cout << "To receive notifications from ALL processes omit the"
                     " process id parameter\n";          
        return 0;          
    }
    const char* brokerURI = argv[1];
    const PID pid = argc > 2 ? atoi(argv[2]) : -1;
    LogClient< PID, TextHandler< ToString< int > > > 
        lc(std::cout, pid, brokerURI);
    while(1) lc.Recv();
    return 0;
}

