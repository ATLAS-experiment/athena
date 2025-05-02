/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef COPYEVENTSTREAMINFO_H
#define COPYEVENTSTREAMINFO_H

/** @file CopyEventStreamInfo.h
 *  @brief This file contains the class definition for the CopyEventStreamInfo class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "GaudiKernel/ServiceHandle.h"

#include "AthenaBaseComps/AthAlgTool.h"

#include "AthenaKernel/IMetaDataTool.h"
#include "AthenaKernel/IAthMetaDataSvc.h"

#include <string>

class StoreGateSvc;

/** @class CopyEventStreamInfo 
 *  @brief This class provides an algorithm to make the EventStreamInfo object and update it.
 **/
class CopyEventStreamInfo : public extends<::AthAlgTool, IMetaDataTool> {
public:
   /// Standard AlgTool Constructor
   CopyEventStreamInfo(const std::string& type, const std::string& name, const IInterface* parent);
   /// Destructor
   virtual ~CopyEventStreamInfo();

   /// AthAlgTool Interface method implementations:
   virtual StatusCode initialize() override final;

   /// Function called when a new input file is opened
   virtual StatusCode beginInputFile(const SG::SourceID& = "Serial") override final;
 
   /// Function called when the currently open input file got completely
   /// processed
   virtual StatusCode endInputFile(const SG::SourceID& = "Serial") override final;

   /// Function called when the tool should write out its metadata
   virtual StatusCode metaDataStop() override final;

private:
   /// (optional) list of keys to copy, all if empty, default: empty
   Gaudi::Property<std::vector<std::string> > m_keys{this, "Keys", {},
      "(optional) list of keys to copy, all if empty. default: empty"};

   /// Access to output MetaDataStore through MetaDataSvc (using MetaContainers)
   ServiceHandle<IAthMetaDataSvc> m_metaDataSvc;
   /// MetaDataStore for input
   ServiceHandle<StoreGateSvc> m_inputMetaDataStore;
};
#endif
