/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROOTCOLLECTIONCOMMON_H
#define ROOTCOLLECTIONCOMMON_H

namespace pool {
   namespace RootCollection {

    // MN: recreate option is replaced by update to not overwrite the file
    constexpr const char* const poolOptToRootOpt[] = {"CREATE", "UPDATE", "UPDATE", "READ"};

    // Io flags are not constexpr 
    inline const Io::IoFlags poolOptToFileMgrOpt[] = { 
      Io::WRITE|Io::CREATE, Io::WRITE|Io::APPEND, Io::WRITE|Io::APPEND, Io::READ };

   }
}

#endif