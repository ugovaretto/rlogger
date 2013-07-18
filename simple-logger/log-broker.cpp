//Standard pub-sub middle layer: forwards subscriptions and messages
//between a frontend and a backend.
//Author: Ugo Varetto

#include <cassert>
#include <iostream>
//for framework builds on Mac OS:
#ifdef __APPLE__
#include <ZeroMQ/zmq.h>
#else 
#include <zmq.h>
#endif

int main(int argc, char** argv) {
    if(argc < 3) {
        std::cout << "usage: " 
                  << argv[0] 
                  << " <front-end URI> <back-end URI>"
                  << std::endl;
        std::cout << "Example: broker \"tcp://*:5555\" \"tcp://*:6666\"\n";          
        return 0;          
    }
    void* ctx = zmq_ctx_new(); 
    void* frontend = zmq_socket(ctx, ZMQ_XPUB);
    void* backend = zmq_socket(ctx, ZMQ_XSUB);
    const char* brokerURI = argv[1];
    const char* backendURI = argv[2];
    int rc = zmq_bind(frontend, brokerURI);
    assert(rc == 0);
    rc = zmq_bind(backend, backendURI);
    assert(rc == 0);
    while(1) {
        zmq_proxy(frontend, backend, 0);
    }
    rc = zmq_close(frontend);
    assert(rc == 0);
    rc = zmq_close(backend);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

