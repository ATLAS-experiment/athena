/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMMETADATATOOL_H
#define BYTESTREAMMETADATATOOL_H

/** @file ByteStreamMetadataTool.h
 *  @brief This file contains the class definition for the ByteStreamMetadataTool class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 *  @author Frank Berghaus <fberghaus@anl.gov>
 **/

#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaKernel/SourceID.h"
#include "AthenaKernel/IMetaDataTool.h"

#include <string>
#include <set>

class StoreGateSvc;


/** @class ByteStreamMetadataTool
 *  @brief This class provides the MetaDataTool for ByteStreamMetadata objects
 **/
class ByteStreamMetadataTool
:         public extends<::AthAlgTool, IMetaDataTool>
{
public: 
  /// Standard Service Constructor
  ByteStreamMetadataTool(const std::string& type, const std::string& name,
      const IInterface* parent);

  /// Destructor
  virtual ~ByteStreamMetadataTool();

  /// Gaudi Service Interface method implementations:
  virtual StatusCode initialize() override;

  /// Incident service handle listening for BeginInputFile and EndInputFile.
  virtual StatusCode beginInputFile(const SG::SourceID&) override;
  virtual StatusCode metaDataStop() override;
  virtual StatusCode endInputFile(const SG::SourceID&) override;

private:
  ServiceHandle<StoreGateSvc> m_metadataStore;
  ServiceHandle<StoreGateSvc> m_inputStore;

  std::set<std::string> keysFromInput() const;
};

#endif // BYTESTREAMMETADATATOOL_H
