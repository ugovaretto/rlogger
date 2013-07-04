//1) start broker
//2) start clients
//3) start MPI processes


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
    while(1) {
    	formatdata(buffer, procid, "hello");
    	rc = zmq_send(req, &buffer[0], buffer.size(), 0);
    	assert(rc > 0);
    	sleep(1);
    }
    rc = zmq_close(req);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
}

