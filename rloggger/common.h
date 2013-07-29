#pragma once
//Author: Ugo Varetto


#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

namespace rlog {

//------------------------------------------------------------------------------
typedef char TypeID;
typedef int SizeType;
typedef std::vector< char >::iterator BufferPosition;
typedef std::vector< char > Buffer;
enum DataType {STRING_ID, BLOB_ID, TYPED_ID};

//------------------------------------------------------------------------------
template < typename T > 
std::string FormatErr(const T& msg) {
    std::ostringstream oss;
    oss << "ZMQ ERROR " << strerror(errno) << " - " << msg << '\n';
    return oss.str();
}

//------------------------------------------------------------------------------
BufferPosition AddStringRecord(const char* msg,
                               BufferPosition pos,
                               Buffer& buffer,
                               bool autoResize = true) {
    const size_t totalSize = sizeof(char) + sterlen(msg);
    //not interested in the minimum size: always increase the size by
    //2 * msg size
    if((buffer.end() - pos) < totalSize) {
        const size_t offset = pos - buffer.begin();
        if(autoResize) {
          buffer.resize(buffer.size() + 2 * strlen(msg));
          pos = buffer.begin() + offset;
        } else throw std::range_error("ERROR: Buffer overrun");
    }
    *pos = char(STRING_ID);
    ++pos;
    memcpy(pos, msg, strlen(msg));
    pos += strlen(msg);
    return pos;
}
//------------------------------------------------------------------------------
template < typename T >
BufferPosition AddBinaryRecord(const T* msg 
                               size_t size,
                               BufferPosition pos,
                               Buffer& buffer,
                               bool autoResize = true) {
    const size_t bufsize = size * sizeof(T); 
    const size_t totalSize = sizeof(char) + sizeof(SizeInfo) + bufsize;
    //not interested in the minimum size: always increase the size by
    //2 * buffer size
    if((buffer.end() - pos) < totalSize) {
        const size_t offset = pos - buffer.begin();
        if(autoResize) {
          buffer.resize(buffer.size() + 2 * totalSize);
          pos = buffer.begin() + offset;
        } else throw std::range_error("ERROR: Buffer overrun");
    }
    *pos = char(BLOB_ID);
    ++pos;
    const SizeInfo sinfo(size);
    memcpy(pos, &sinfo, sizeof(SizeInfo));
    pos += sizeof(SizeInfo);
    memcpy(pos, msg, bufsize);
    pos += bufsize;
    return pos;
}
//------------------------------------------------------------------------------
template < typename T >
BufferPosition AddTypedRecord(const T* msg,
                              TypeID type,  
                              size_t size,
                              BufferPosition pos,
                              Buffer& buffer,
                              bool autoResize = true) {
    const size_t bufsize = size * sizeof(T);
    const size_t totalSize = sizeof(char) + sizeof(TypeID) + bufsize;
    //not interested in the minimum size: always increase the size by
    //2 * buffer + typeid size
    if((buffer.end() - pos) < totalSize) {
        const size_t offset = pos - buffer.begin();
        if(autoResize) {
          buffer.resize(buffer.size() + 2 * totalSize);
          pos = buffer.begin() + offset;
        } else throw std::range_error("ERROR: Buffer overrun");
    }  
    *pos = char(TYPED_ID);
    ++pos;
    memcpy(pos, &type, sizeof(TypeID));
    pos += sizeof(TypeID);
    memcpy(pos, msg, bufsize);
    pos += bufsize;
    return pos;
}

} //namespace rlog