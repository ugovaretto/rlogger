rlogger - Remote logger
=======================

Log reguests from processes to remote clients through a zeromq-based broker.

When used in conjunction with MPI: Brokers running on non-compute node receive
messges from compute-node MPI processes and dispatch messages to remote
clients.

Broker, client and logger services can be started in any order; if clients
want subscribe to messages from specific process ids logger processes need
to be started first to read the process id to be passed to clients.

For the first version of the logger the following constraint is obeyed:

* one subscription per remote process only; clients can receive data from
  multiple processes by using one socket per process; or they can receive
  data from all process at once. Subscription to a specific subset of processes
  is not supported; note that subscribing to more than a single process using a
  single socket with the current features implemented in zmq sockets it's not
  easy because only a simple match is performed; advanced (OR) filtering would
  have to be performed on the server side for this to work. 

Flow:

* log source: publish to broker
* broker: subscribe to log sources; publish to clients
* client: subscribe to broker 

The same architecture can be reversed to allow for communication from clients
to servers to e.g. steer computation or select which information to log:

* compute process (usually same as log source): subscribe to control broker
* control broker: subscribe to control client; publish to compute process
* control client: publish to control broker

Use XPUB, XSUB sockets with zmq_proxy in the broker to allow subscriptions
to be dispatched to remote publishers (log sources and compute processes).







