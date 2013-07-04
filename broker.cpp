//1) start broker
//2) start clients
//3) start MPI processes


int main(int argc, char** argv) {

//create PUB socket
//start receiving messages from log servers
//while true
//  receive MPI id and data on incoming socket
//  publish to <MPI id, data>

//consider using ZMQ_SWAP: Set disk offload size to
//temporary store messages 
    void* ctx = zmq_ctx_new(); 
    void* subscriber = zmq_socket(ctx, ZMQ_PUB);
    void* mpi = zmq_socket(ctx, ZMQ_REP);
    int rc = zmq_bind(subscriber, brokerURI);
    assert(rc == 0);
    rc = zmq_bind(mpi, mpiURI);
    assert(rc == 0);
    while(1) {
        rc = zmq_recv(mpi, &buffer[0], buffer.size(), 0);
        assert(rc > 0);
        rc = zmq_send(subscriber, &buffer[0], rc, 0);
        assert(rc > 0);
    }
    rc = zmq_close(subscriber);
    assert(rc == 0);
    rc = zmq_close(mpi);
    assert(rc == 0);
    rc = zmq_ctx_destroy(ctx);
    assert(rc == 0);
    return 0;
    return 0;
}

