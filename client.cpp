//1) start broker
//2) start clients
//3) start MPI processes


int main(int argc, char** argv) {

//subscribe to MPI id
//while true
//  receive data
//  process data    
    void* ctx = zmq_ctx_new(); 
    void* publisher = zmq_socket(ctx, ZMQ_SUB);
    const char* brokerURI = argv[1];
    const int mpiid = atoi(argv[2]);
    int rc = zmq_connect(req, brokerURI);
    assert(rc == 0);
    rc = zmq_setsockopt(publisher, ZMQ_SUBSCRIBE, &mpiid, sizeof(int));
    assert(rc == 0);
    while(1) {
        rc = zmq_recv(publisher, &buffer[0], buffer.size(), 0);
        assert(rc > 0);
    }
    rc = zmq_close(publisher);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

