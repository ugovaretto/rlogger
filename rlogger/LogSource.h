#pragma once
//Author: Ugo Varetto
#ifdef __APPLE__
#include <ZeroMQ/zmq.h>
#else
#include <zmq.h>
#endif
#include "common.h"

//==============================================================================
namespace rlog {

//------------------------------------------------------------------------------    
template < typename SubIdT >
class LogSource {
public:
    LogSource(const char* brokerURI,
              SubIdT subId, 
              size_t st = 1, //always send
              bool connect = true) 
    : brokerURI_(brokerURI),
      subId_(subId),
      context_(0),
      socket_(0),
      outBuffer_(0x1000),
      sendTrigger_(st),
      outBufferPos_(outBuffer_.begin()) {
        if(connect) Connect(brokerURI_.c_str());
    }
    LogSource(size_t st = 1) 
    : context_(0),
      socket_(0),
      outBuffer_(0x1000),
      sendTrigger_(st),
      outBufferPos_(outBuffer_.begin())
    {}
    void Connect(const char* brokerURI) {
        Clear();
        context_ = zmq_ctx_new();
        if(context_ == 0) {
            throw std::runtime_error(
                FormatErr("context creation failed"));
        }
        socket_ = zmq_socket(context_, ZMQ_PUB);
        if(socket_ == 0) {
            throw std::runtime_error(
                FormatErr("socket creation failed"));
        }
        if(zmq_connect(socket_, brokerURI) != 0) {
            throw std::runtime_error(FormatErr("connection failed")); 
        }
        brokerURI_ = brokerURI;
    }
    void Disconnect() {
        if(socket_ != 0)
            if(zmq_close(socket_) != 0) 
                throw std::runtime_error(FormatErr("closing socket failed"));
    }
    void Log(const char* msg,
             bool forceFlush = false,
             bool autoResizeBuffer = true) {
        outBufferPos_ = AddStringRecord(msg, 
                                        outBufferPos_,
                                        outBuffer_,
                                        autoResizeBuffer);
        CheckAndFlush(forceFlush);
    }
    template < typename T >
    void Log(const T* msg,
             size_t size,
             bool forceFlush = false,
             bool autoResizeBuffer = true) {
        outBufferPos_ = AddBinaryRecord(msg,
                                        size,
                                        outBufferPos_,
                                        outBuffer_,
                                        autoResizeBuffer);
        CheckAndFlush(forceFlush);
    }
    void Flush() {
        if(outBufferPos_ == outBuffer_.begin()) return;
        //Addressof and SizeOf SubId are points of customization provided by
        //client code
        if(zmq_send(socket_, AddressOfSubId(subId_),
           SizeOfSubId(subId_), ZMQ_SNDMORE) < 0)
            throw std::runtime_error(FormatErr("id send failure"));
        if(zmq_send(socket_, &outBuffer_[0],
                    outBufferPos_ - outBuffer_.begin(), 0) < 0 )
            throw std::runtime_error(FormatErr("send failure"));
        outBufferPos_ = outBuffer_.begin();
        oss_.str("");
    }
    void CheckAndFlush(bool forceFlush) {
        //this should never happen because bounds are checked at each
        //record insertion
        if(outBufferPos_ > outBuffer_.end())
            throw std::range_error(
                FormatErr("out buffer pointer out of bound"));
        if(forceFlush
           || (outBufferPos_ - outBuffer_.begin() >= sendTrigger_))
            Flush();
    }
private:    
    void Clear() {
        Disconnect();
        ResetBuffers();    
        if(context_ != 0) 
            if(zmq_ctx_destroy(context_) != 0)
                throw std::runtime_error(
                    FormatErr("context destruction failed"));
    }
    void ResetBuffers() {
        oss_.str("");
        outBufferPos_ = outBuffer_.begin();
    }
private:
    std::string brokerURI_;
    SubIdT subId_;
    void* context_;
    void* socket_;
   
    std::vector< char > outBuffer_;
    size_t sendTrigger_; //sends if size of outBuffer > sendTrigger
    std::vector< char >::iterator outBufferPos_; //current position in 
                                                 //the out buffer
    //need a place to store stringstream used by operator <<
    std::ostringstream oss_;
    template < typename T >
    friend LogSource& operator<<(LogSource& ls, const T& data) {
        ls.oss_ << data;
        ls.Log(ls.oss_.str().c_str());
        return ls;
    }

};


//------------------------------------------------------------------------------
template < typename T, typename D >
void  Log(LogSource< T >& ls, const D& data) {
    ls << data;
}

} // namespace rlog