/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
  */
#ifndef ROOTUTILS_NORMALIZEDTYPENAMEUTIL_H
#define ROOTUTILS_NORMALIZEDTYPENAMEUTIL_H

#include "TClassEdit.h"

#include <typeinfo>
#include <string>
#include <memory>

namespace RootUtils {
   struct FreeDeleter {
      void operator()(char *ptr) { free(ptr); }
   };

   class ManagedCStr : protected std::unique_ptr<char,FreeDeleter > {
   public:
      using std::unique_ptr<char,FreeDeleter>::unique_ptr;
      std::string str() { return (this->get() ? std::string(this->get()) : std::string()); }
   };

   // Convenience method to get ROOT's normalized type name for a type_info
   std::string getNormalizedTypeNameFromId(const std::type_info &a_type_info) {
      int errorCode{};
      std::string ret = ManagedCStr(TClassEdit::DemangleTypeIdName(a_type_info, errorCode)).str();
      if (errorCode != 0) {
         throw std::runtime_error(std::string("Failed to get normalized type for ") + a_type_info.name() );
      }
      return ret;
   }
   // Convenience method to get ROOT's normalized type name for a type
   template <typename T>
   std::string getNormalizedTypeName() {
      return getNormalizedTypeNameFromId(typeid(T));
   }
}
#endif
