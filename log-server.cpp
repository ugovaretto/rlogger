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
#include <zmq.h> 

typedef pid_t PID;

//------------------------------------------------------------------------------
PID get_proc_id() {
    return getpid();
}

//------------------------------------------------------------------------------
template < typename T >
size_t formatdata(unsigned char* buffer, PID pid, const T* data, size_t len) {
    memcpy(buffer, &pid, sizeof(pid));
    buffer += sizeof(pid);
    const size_t datalength = len * sizeof(data);
    memcpy(buffer, &datalength, sizeof(datalength));
    buffer += sizeof(datalength);
    memcpy(buffer, data, datalength);    
    return sizeof(pid) + sizeof(datalength) + datalength; 
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
    if(argc < 2) {
        std::cout << "usage: " 
                  << argv[0] 
                  << " <broker URI>"
                  << std::endl;
        std::cout << "Example: logger \"tcp://logserver:5555\"\n";          
        return 0;          
    }
    void* ctx = zmq_ctx_new(); 
    void* req = zmq_socket(ctx, ZMQ_PUB);
    const char* brokerURI = argv[1];
    int rc = zmq_connect(req, brokerURI);
    assert(rc == 0);
    unsigned char buffer[0x100];
    size_t size = 0;
    std::cout << "PID: " << getpid() << std::endl;
    while(1) {
    	size = formatdata(buffer, getpid(), "hello", strlen("hello"));
    	rc = zmq_send(req, buffer, size, 0);
    	sleep(1);
    }
    rc = zmq_close(req);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

