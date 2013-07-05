#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <cstdlib>
#include <zmq.h>
//1) start broker
//2) start clients
//3) start MPI processes


typedef pid_t PID;

//------------------------------------------------------------------------------
void processdata(const unsigned char* data, size_t len) {
    PID pid;
    size_t datalen;
    memcpy(&pid, data, sizeof(pid));
    data += sizeof(pid);
    memcpy(&datalen, data, sizeof(datalen));
    data += sizeof(datalen);
    std::vector<char> databuf(datalen + 1, 0);
    memcpy(&databuf[0], data, datalen);
    std::cout << "PID: " << pid << ": " << &databuf[0] << std::endl;
}

//------------------------------------------------------------------------------
int main(int argc, char** argv) {

//subscribe to MPI id
//while true
//  receive data
//  process data    
    void* ctx = zmq_ctx_new(); 
    void* publisher = zmq_socket(ctx, ZMQ_SUB);
    const char* brokerURI = argv[1];
    const PID mpiid = atoi(argv[2]);
    int rc = zmq_connect(publisher, brokerURI);
    assert(rc == 0);
    rc = zmq_setsockopt(publisher, ZMQ_SUBSCRIBE, &mpiid, sizeof(mpiid));
    assert(rc == 0);
    unsigned char buffer[0x100];
    while(1) {
        rc = zmq_recv(publisher, buffer, 0x100, 0);
        assert(rc > 0);
        processdata(buffer, rc);
    }
    rc = zmq_close(publisher);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

