/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
//  ConfigException.h
//  TopoCore
//  Created by Joerg Stelzer on 11/18/12.

#ifndef __TopoCore__Exception__
#define __TopoCore__Exception__

#include <iostream>
#include <sstream>
#include <string>

#define TCS_EXCEPTION(MSG) \
  do { std::ostringstream o; o << MSG; TCS::raise(o.str()); } while(0)
  
namespace TCS {
  
  
   
   class Exception : virtual public std::exception {
   public:
      enum type_t {
         CONFIG,
         RUNTIME
      };
      
      Exception(const std::string& msg) :
         m_msg(msg)
      {}
      
			virtual ~Exception() = default;

      virtual char const* what() const noexcept { return m_msg.data(); }
      
   private:
      std::string m_msg;
   };
   
   [[noreturn]] inline void raise(const std::string & msg) {
    throw Exception(msg);
  }
   
}

#endif /* defined(__TopoCore__ConfigException__) */
