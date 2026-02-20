/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IOVDBMETADATATOOLS_METADATATOCONDALG_H
#define IOVDBMETADATATOOLS_METADATATOCONDALG_H

/**
 * @file MetaDataToCondAlg.h
 * @brief Condition algorithm to populate ConditionStore from MetaDataStore
 *
 * This algorithm reads IOVMetaDataContainer objects from the MetaDataStore
 * and creates corresponding AthenaAttributeList objects in the ConditionStore.
 * This is needed for the direct in-file metadata mode where simulation and
 * digitization parameters are written directly to file metadata without
 * intermediate sqlite files.
 */

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/WriteCondHandleKey.h"
#include "PersistentDataModel/AthenaAttributeList.h"
#include "GaudiKernel/ServiceHandle.h"

class StoreGateSvc;

class MetaDataToCondAlg : public AthCondAlgorithm {
public:
  using AthCondAlgorithm::AthCondAlgorithm;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  /// Folder name to read from MetaDataStore
  Gaudi::Property<std::string> m_folderName{this, "FolderName", "",
    "Folder name to read from MetaDataStore (e.g., /Digitization/Parameters)"};

  /// Output condition key
  SG::WriteCondHandleKey<AthenaAttributeList> m_outputKey{this, "OutputKey", "",
    "Output condition key (defaults to FolderName if not set)"};

  /// Handle to MetaDataStore
  ServiceHandle<StoreGateSvc> m_metaDataStore{"StoreGateSvc/MetaDataStore", name()};
};

#endif // IOVDBMETADATATOOLS_METADATATOCONDALG_H
