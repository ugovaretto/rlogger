rlogger
=======

Remote MPI logger

Log reguests from MPI processes to clients through a zeromq-based broker.

Broker running on non-compute nodes receives messges from compute-node
MPI processes and dispatches messages to remote clients.

For the first version the following constraints are followed:

* start broker THEN clients THEN MPI processes
* one subscription per MPI process only; clients can receive data from
  multiple MPI processes by using one socket per process; note that
  subscribing to more than a single MPI process using a single socket with 
  the current features implemented in zmq ZMQ_SUB sockets it's not easy because
  only a simple match is performed; advanced (OR) filtering would have to be
  performed on the server side 



