#pragma once
//Author: Ugo Varetto
#include <cstdlib>
#ifdef __APPLE__
#include <ZeroMQ/zmq.h>
#else
#include <zmq.h>
#endif
#include "common.h"

//==============================================================================
namespace rlog {

//------------------------------------------------------------------------------
struct DummyHandler {
    DummyHandler() {}
    template < typename T > DummyHandler(T&) {}
    template < typename T > DummyHandler(const T&) {}
    void operator()(void*, SizeType, char*, SizeType) {}
};

struct ParseInt {
    std::string operator()(const void* sid, SizeType sz) {
      std::ostringstream oss;
      oss << *reinterpret_cast< const int* >(sid);
      return oss.str();
    }
};

//------------------------------------------------------------------------------
template < typename SubIdParserT >
struct TextHandler {
    TextHandler(std::ostream& os) : os_(&os) {}
    void operator()(void* sid, SizeType sz, char* text, SizeType textSize) {
        *os_ << parser(sid, sz) << ": " << text << std::endl;
    }
    std::ostream* os_;
    SubIdParserT parser;
};


//------------------------------------------------------------------------------    
template < typename SubIdT = int,
           typename TextHandlerT = DummyHandler,
           typename BinaryHandlerT = DummyHandler,
           typename TypedHandlerT = DummyHandler >
class LogClient {
public:
    LogClient(SubIdT subId, 
              const char* brokerURI,
              size_t bufferSize = 0x10000, //64kB 
              bool connect = true) 
    : brokerURI_(brokerURI),
      subId_(subId), 
      context_(0),
      socket_(0),
      inBuffer_(bufferSize) {
        if(connect) Connect(brokerURI_.c_str(), subId);
    }
    LogClient(SubIdT subId, size_t bufferSize = 0x10000) 
    : subId_(subId),
      context_(0),
      socket_(0),
      inBuffer_(0x100)
    {}
    template < typename T >
    LogClient(T& config, //important to have a non const ref to support 
                         //non const data such as ostreams directly w/o a
                         //wrapper
              SubIdT subId,
              const char* brokerURI,
              size_t bufferSize = 0x10000, //64kB 
              bool connect = true) 
    : txtHandler_(config),
      binHandler_(config),
      typedHandler_(config),
      brokerURI_(brokerURI),
      subId_(subId),
      context_(0),
      socket_(0),
      inBuffer_(bufferSize) {
        if(connect) Connect(brokerURI_.c_str(), subId);
    }
    template < typename T >
    LogClient(const T& config,
              SubIdT subId,
              const char* brokerURI,
              size_t bufferSize = 0x10000, //64kB 
              bool connect = true) 
    : txtHandler_(config),
      binHandler_(config),
      typedHandler_(config),
      brokerURI_(brokerURI),
      subId_(subId),
      context_(0),
      socket_(0),
      inBuffer_(bufferSize) {
        if(connect) Connect(brokerURI_.c_str(), subId);
    }
    template < typename T >
    LogClient(const T& config,
              SubIdT subId,
              size_t bufferSize = 0x10000) 
    : txtHandler_(config),
      binHandler_(config),
      typedHandler_(config),
      context_(0),
      socket_(0),
      inBuffer_(bufferSize)
    {}
    void Connect(const char* brokerURI, SubIdT subId) {
        Clear();
        context_ = zmq_ctx_new();
        if(context_ == 0) {
            throw std::runtime_error(
                FormatErr("context creation failed"));
        }
        socket_ = zmq_socket(context_, ZMQ_SUB);
        if(socket_ == 0) {
            throw std::runtime_error(
                FormatErr("socket creation failed"));
        }
        //EmptySubId is a point of customization: must be supplied by client
        //code to determine if subId is valid, if not an empty id will be
        //specified to subscribe to all publishers.
        const int rc = !EmptySubId(subId) ? 
             zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, AddressOfSubId(subId),
                            SizeOfSubId(subId)) :
             zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, 0, 0);
        if(rc != 0) 
            throw std::runtime_error(FormatErr("cannot set socket operatio"));     
        if(zmq_connect(socket_, brokerURI) != 0) {
            throw std::runtime_error(FormatErr("connection failed")); 
        }
        memset(&items_[0], sizeof(zmq_pollitem_t), char(0));
        items_[0].socket = socket_;
        items_[0].events = ZMQ_POLLIN;
        brokerURI_ = brokerURI;
        subId_ = subId;
    }
    void Disconnect() {
        if(socket_ != 0)
            if(zmq_close(socket_) != 0) 
                throw std::runtime_error(FormatErr("closing socket failed"));
    }
    int Recv() {
        //receive id
        int rc = zmq_recv(socket_, &inBuffer_[0], inBuffer_.size(), 0);
        subIdBuffer_.resize(rc);
        std::copy(inBuffer_.begin(), inBuffer_.begin() + rc,
                  subIdBuffer_.begin());      
        //receive data
        rc = zmq_recv(socket_, &inBuffer_[0], inBuffer_.size(), 0);
        if(rc <= 0) return rc;
        switch(DataType(inBuffer_[0])) {
        case TEXT_ID: inBuffer_[rc] = '\0';
                      txtHandler_(&subIdBuffer_[0], subIdBuffer_.size(),
                        &inBuffer_[sizeof(char) + sizeof(SizeType)],
                        *reinterpret_cast< SizeType* >(
                            &inBuffer_[sizeof(char)]));
                      break;
        case BLOB_ID: binHandler_(&subIdBuffer_[0], subIdBuffer_.size(),
                                  &inBuffer_[sizeof(char) + sizeof(SizeType)],
                        *reinterpret_cast< SizeType* >(
                                                    &inBuffer_[sizeof(char)]));
                      break;
        default: throw std::logic_error("Unrecognized type");
                 break;                                                                 
        }
        return rc;
    }
    int Poll(int ms = 0) {
        const int rc = zmq_poll(&items_[0], 1, ms * 1000);
        if(rc <= 0) return rc;
        if(items_[0].revents && ZMQ_POLLIN) return Recv();
        return 0;        
    } 
private:    
    void Clear() {
        Disconnect();
        if(context_ != 0) 
            if(zmq_ctx_destroy(context_) != 0)
                throw std::runtime_error(
                    FormatErr("context destruction failed"));    
    }
private:
    TextHandlerT txtHandler_;
    BinaryHandlerT binHandler_;
    TypedHandlerT typedHandler_;
    std::string brokerURI_;
    SubIdT subId_;
    void* context_;
    void* socket_;
    std::vector< char > inBuffer_;
    zmq_pollitem_t items_[1];
    std::vector< char > subIdBuffer_;
};

} // namespace rlog