#pragma once
//Author: Ugo Varetto

#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <algorithm>
#include <signal.h>
#include <cassert>


#include <iostream>

//------------------------------------------------------------------------------

namespace rlog {
extern "C" void RloggerSignalHandler(int sig_value);
class Interrupt {
    static int interrupted_;
friend void RloggerSignalHandler(int sig_value);    
public:
    static bool interrupted() { return interrupted_ != 0; }
};

inline bool Interrupted() { return Interrupt::interrupted(); }

inline void RloggerCatchSignals() {
  signal(SIGABRT, &RloggerSignalHandler);
  signal(SIGTERM, &RloggerSignalHandler);
  signal(SIGINT, &RloggerSignalHandler);
  // alternate implementation:
  // struct sigaction action;
  // action.sa_handler = s_signal_handler;
  // action.sa_flags = 0;
  // sigemptyset (&action.sa_mask);
  // sigaction (SIGINT, &action, NULL);
  // sigaction (SIGTERM, &action, NULL);
}

//------------------------------------------------------------------------------
typedef char TypeID;
typedef int SizeType;
typedef std::vector< char >::iterator BufferPosition;
typedef std::vector< char > Buffer;
enum DataType {TEXT_ID = 1, BLOB_ID = 2};

//------------------------------------------------------------------------------
template < typename T > 
std::string FormatErr(const T& msg) {
    std::ostringstream oss;
    oss << "ZMQ ERROR: " << strerror(errno) << " - " << msg << '\n';
    return oss.str();
}

//Default implementations of SubId functions; thanks to Koenig lookup the
//selected implementation depends on the namespace the parameter type resides
//in, so this declarations do not interfere with custom data types defined in
//other namespaces
template < typename T > bool EmptySubId(T);

template <> inline bool EmptySubId<int>(int i) { return i < 0; }
template <> inline bool EmptySubId<unsigned int>(unsigned int i) { 
  return i == 0;
}  
template <> inline bool EmptySubId<char*>(char* id) { return strlen(id) == 0; }
template <> inline bool EmptySubId<const char*>(const char* id) { 
  return strlen(id) == 0;
}

//------------------------------------------------------------------------------
template < typename T > size_t SizeOfSubId(T) { return sizeof(T); }
template <> inline size_t SizeOfSubId< char* >(char* str) { 
  return strlen(str);
}
template <> inline size_t SizeOfSubId< const char* >(const char* str) { 
    return strlen(str);
}
inline size_t SizeOfSubId(const std::string& str) { 
    return str.size();
}
//------------------------------------------------------------------------------
template < typename T > void* AddressOfSubId(T& d) { return &d; }
template < typename T > const void* AddressOfSubId(const T& d) { return &d; }
template < typename T > void* AddressOfSubId(T* pd) { return pd; }
template < typename T > const void* AddressOfSubId(const T* pd) { return pd; }
//DANGEROUS but required so pass data to send/recv which require
//non-const pointers
template <> inline void* AddressOfSubId(std::string& s) { 
    return const_cast< char* >(s.c_str());
}
template <> inline const void* AddressOfSubId(const std::string& s) {
    return s.c_str();
}
//------------------------------------------------------------------------------
inline
BufferPosition AddStringRecord(const char* msg,
                               BufferPosition pos,
                               Buffer& buffer,
                               bool autoResize = true) {
    const SizeType sz = strlen(msg);
    const size_t totalSize = sizeof(char) + sizeof(SizeType) + sz;
    //not interested in the minimum size: always increase the size by
    //2 * msg size
    if((buffer.end() - pos) < totalSize) {
        const size_t offset = pos - buffer.begin();
        if(autoResize) {
          buffer.resize(buffer.size() + 2 * strlen(msg));
          pos = buffer.begin() + offset;
        } else throw std::range_error("ERROR: Buffer overrun");
    }
    *pos = char(TEXT_ID);
    ++pos;
    memcpy(&(*pos), &sz, sizeof(SizeType));
    pos += sizeof(SizeType);
    memcpy(&(*pos), msg, sz);
    pos += sz;
    return pos;
}
//------------------------------------------------------------------------------
template < typename T >
BufferPosition AddBinaryRecord(const T* msg, 
                               size_t size,
                               BufferPosition pos,
                               Buffer& buffer,
                               bool autoResize = true) {
    const size_t bufsize = size * sizeof(T); 
    const size_t totalSize = sizeof(char) + sizeof(SizeType) + bufsize;
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
    const SizeType sinfo(size);
    memcpy(&(*pos), &sinfo, sizeof(SizeType));
    pos += sizeof(SizeType);
    memcpy(&(*pos), msg, bufsize);
    pos += bufsize;
    return pos;
}

template < typename T >
struct ToString {
    std::string operator()(const void* sid, SizeType sz) {
      std::ostringstream oss;
      oss << *reinterpret_cast< const T* >(sid);
      return oss.str();
    }
};

template <>
struct ToString < const char* > {
    std::string operator()(const void* sid, SizeType sz) {
      return reinterpret_cast< const char* >(sid);
    }
};

//------------------------------------------------------------------------------
inline std::string ExtractStrings(const char* b, SizeType sz) {
    const char* e = b + sz;
    std::string ret;
    while(b < e) {
        assert(*b == TEXT_ID);
        ++b; 
        const SizeType s = *reinterpret_cast< const SizeType* >(b);
        assert(s > 0);
        b += sizeof(SizeType);
        ret += std::string(b, s);
        b += s;
    }
    return ret;
}

} //namespace rlog