#include <cassert>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <zmq.h> 

//1) start broker
//2) start clients
//3) start MPI processes

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
    const size_t datalen = len * sizeof(data);
    memcpy(buffer, &datalen, sizeof(datalen));
    buffer += sizeof(datalen);
    memcpy(buffer, data, datalen);    
    return sizeof(pid) + sizeof(datalen) + datalen; 
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {
//get id of process    
//connect to server and send

//start typical simulation loop:

//while condition not met:
    // exchange data
    // (log exchanged data)
    // compute
    // (log computed data)
//write output data
    void* ctx = zmq_ctx_new(); 
    void* req = zmq_socket(ctx, ZMQ_REQ);
    const char* brokerURI = argv[1];
    int rc = zmq_connect(req, brokerURI);
    assert(rc == 0);
    unsigned char buffer[0x100];
    size_t size = 0;
    std::cout << "PID: " << getpid() << std::endl;
    while(1) {
    	size = formatdata(buffer, getpid(), "hello", sizeof("hello"));
    	rc = zmq_send(req, buffer, size, 0);
    	assert(rc > 0);
    	sleep(1);
    }
    rc = zmq_close(req);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

