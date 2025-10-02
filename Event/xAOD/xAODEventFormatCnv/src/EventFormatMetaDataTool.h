// Dear emacs, this is -*- c++ -*-
/* Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration */

#ifndef XAODEVENTFORMATCNV_EVENTFORMATMETADATATOOL
#define XAODEVENTFORMATCNV_EVENTFORMATMETADATATOOL

// System include(s):
#include <string>
#include <memory>
#include <mutex>

// Gaudi/Athena include(s):
#include "Gaudi/Property.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthMetaDataSvc.h"
#include "AthenaKernel/IMetaDataTool.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "StoreGate/StoreGateSvc.h"

// EDM include(s):
#include "xAODEventFormat/EventFormat.h"

namespace xAODMaker {

/// Tool taking care of copying the event format object from file to file
///
/// This tool does the heavy lifting when fast-merging DxAOD files to
/// make sure that the xAOD::EventFormat metadata object is propagated
/// correctly from the input files to the output.
///
/// @author Jack Cranshaw <cranshaw@anl.gov>
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
/// @author Frank Berghaus <fberghaus@anl.gov>
///
class EventFormatMetaDataTool : public extends<::AthAlgTool, IMetaDataTool> {
 public:
  /// Regular AlgTool constructor
  EventFormatMetaDataTool(const std::string& type,
                          const std::string& name,
                          const IInterface* parent);

  /// Function initialising the tool
  virtual StatusCode initialize() override;

  /// Function collecting the metadata from a new input file
  virtual StatusCode beginInputFile(const SG::SourceID&) override {return beginInputFile();}

  /// Function collecting the metadata from a new input file
  virtual StatusCode endInputFile(const SG::SourceID&) override {return endInputFile();}

  /// Wait for metadata write operations to finish, then returns SUCCESS
  virtual StatusCode metaDataStop(const SG::SourceID&) {return metaDataStop();}

  /// Function called when a new input file is opened
  virtual StatusCode beginInputFile();

  /// Function called when the currently open input file got completely
  /// processed
  virtual StatusCode endInputFile() {return StatusCode::SUCCESS;}

  /// Wait for metadata write operations to finish, then return SUCCESS
  virtual StatusCode metaDataStop() override;

 private:
  /// Function collecting the event format metadata from the input file
  StatusCode collectMetaData();

  /// Connection to the input metadata store
  ServiceHandle< ::StoreGateSvc > m_inputMetaStore{this, "InputMetaStore",
    "StoreGateSvc/InputMetaDataStore", name()};

  /// Connection to the output metadata store
  ServiceHandle< IAthMetaDataSvc > m_outputMetaStore{this, "MetaDataSvc",
    "MetaDataSvc", name()};

  /// (optional) list of keys to copy, all if empty, default: empty
  Gaudi::Property<std::vector<std::string> > m_keys{ this, "Keys", {},
      "(optional) list of keys to copy, all if empty. default: empty"};

  /// MetaDataStop need to wait for ongoing writes
  std::mutex m_outputMutex;
};  // class EventFormatMetaDataTool
}  // namespace xAODMaker

#endif  // XAODEVENTFORMATCNV_EVENTFORMATMETADATATOOL
