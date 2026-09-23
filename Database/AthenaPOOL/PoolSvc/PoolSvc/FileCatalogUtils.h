/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef POOLSVC_FILECATALOGUTILS_H
#define POOLSVC_FILECATALOGUTILS_H

#include "GaudiUtils/IFileCatalog.h"
#include "GaudiUtils/IFileCatalogMgr.h"

#include <string>

/// Free-standing helpers for operations on Gaudi's file catalog interfaces
/// (Gaudi::IFileCatalog / Gaudi::IFileCatalogMgr) that have no direct equivalent
/// there, so clients can use those interfaces directly instead of a wrapper class.
namespace FileCatalogUtils {

   /// Get the first PFN + filetype for the given FID
   void getFirstPFN( const Gaudi::IFileCatalog& fc,
                      const std::string& fid, std::string& pfn, std::string& tech );

   /// Get FID and filetype for a given PFN
   void lookupFileByPFN( const Gaudi::IFileCatalog& fc,
                          const std::string& pfn, std::string& fid, std::string& tech );

   /// Register PFN, assign new FID if not given. Throws if the PFN is already registered
   void registerPFN( Gaudi::IFileCatalog& fc,
                      const std::string& pfn, const std::string& ftype, std::string& fid );

   /// Add new catalog, identified by name, to the existing ones, optionally as the write catalog.
   /// Returns the full connect string of the newly added catalog, or an empty string if it
   /// already existed and nothing was added.
   std::string addCatalog( Gaudi::IFileCatalogMgr& mgr, const std::string& connect, bool forWriting = false );

}

#endif
