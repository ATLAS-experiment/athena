/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef HISTUTIL_H
#define HISTUTIL_H
#include "TVirtualMutex.h"
#include "TH1.h"
#include <string>
#include <string_view>

namespace HistUtil {
// Helper class to avoid that newly created histograms are added to random
// root TDirectories
class ProtectHistogramCreation {
public:
   ProtectHistogramCreation() {
      TVirtualMutex *root_global_mutex ATLAS_THREAD_SAFE = gGlobalMutex;
      if (root_global_mutex) {
         root_global_mutex->Lock();
      }
      m_addDirectory=TH1::AddDirectoryStatus();
      if (m_addDirectory) {
         TH1::AddDirectory(kFALSE);
      }
   }
   ~ProtectHistogramCreation() {
      if (m_addDirectory) {
         TH1::AddDirectory(m_addDirectory);
      }
      TVirtualMutex *root_global_mutex ATLAS_THREAD_SAFE = gGlobalMutex;
      if (root_global_mutex) {
         root_global_mutex->UnLock();
      }
   }
private:
   bool m_addDirectory = false;
};

// Helper class to compose strings in a stringstream-way but only with the overhead of
// string concatenation
class StringCat {
public:
   StringCat(const std::string &a) :m_str(a) {}
   StringCat(std::string &&a) :m_str(std::move(a)) {}
   StringCat() = default;

   const std::string &str() const                   { return m_str; }
   std::string release()                            { return std::move(m_str); }
   StringCat &operator<<(const std::string &a)      { m_str += a; return *this;}
   StringCat &operator<<(const char *a)             { m_str += a; return *this;}
   StringCat &operator<<(const std::string_view &a) { m_str += a; return *this;}
   template <typename T>
   StringCat &operator<<(const T &a)                { m_str += std::to_string(a); return *this;}
private:
   std::string m_str;
};

}
#endif
