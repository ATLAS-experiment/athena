/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file MetaDataToCondAlg.cxx
 * @brief Implementation of MetaDataToCondAlg
 */

#include "MetaDataToCondAlg.h"

#include "StoreGate/StoreGateSvc.h"
#include "StoreGate/WriteCondHandle.h"
#include "IOVDbDataModel/IOVMetaDataContainer.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "AthenaKernel/IOVInfiniteRange.h"

StatusCode MetaDataToCondAlg::initialize() {
  ATH_MSG_DEBUG("Initializing MetaDataToCondAlg for folder " << m_folderName.value());

  if (m_folderName.value().empty()) {
    ATH_MSG_ERROR("FolderName property must be set");
    return StatusCode::FAILURE;
  }

  // Use folder name as output key if not explicitly set
  if (m_outputKey.key().empty()) {
    m_outputKey = m_folderName.value();
  }

  ATH_CHECK(m_outputKey.initialize());
  ATH_CHECK(m_metaDataStore.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode MetaDataToCondAlg::execute(const EventContext& ctx) const {
  SG::WriteCondHandle<AthenaAttributeList> writeHandle(m_outputKey, ctx);

  if (writeHandle.isValid()) {
    ATH_MSG_DEBUG("CondHandle " << m_outputKey.fullKey() << " is already valid");
    return StatusCode::SUCCESS;
  }

  // Read from MetaDataStore
  const IOVMetaDataContainer* metaContainer = nullptr;
  if (!m_metaDataStore->retrieve(metaContainer, m_folderName.value()).isSuccess()) {
    ATH_MSG_ERROR("Could not retrieve IOVMetaDataContainer for folder " << m_folderName.value()
                  << " from MetaDataStore");
    return StatusCode::FAILURE;
  }

  const IOVPayloadContainer* payloadContainer = metaContainer->payloadContainer();
  if (!payloadContainer || payloadContainer->size() == 0) {
    ATH_MSG_ERROR("No payloads in IOVMetaDataContainer for folder " << m_folderName.value());
    return StatusCode::FAILURE;
  }

  // Get the first payload (parameter folders should have only one)
  const CondAttrListCollection* coll = *(payloadContainer->begin());
  if (!coll) {
    ATH_MSG_ERROR("Could not get CondAttrListCollection from payload container");
    return StatusCode::FAILURE;
  }

  // Extract AthenaAttributeList from the first channel
  auto itr = coll->begin();
  if (itr == coll->end()) {
    ATH_MSG_ERROR("CondAttrListCollection is empty");
    return StatusCode::FAILURE;
  }

  const coral::AttributeList& attrList = itr->second;
  auto athAttrList = std::make_unique<AthenaAttributeList>(attrList);

  ATH_MSG_DEBUG("Read " << athAttrList->size() << " attributes from folder " << m_folderName.value());

  // Use infinite range - the metadata is valid for the entire job
  writeHandle.addDependency(IOVInfiniteRange::infiniteRunLB());

  ATH_CHECK(writeHandle.record(std::move(athAttrList)));

  ATH_MSG_DEBUG("Recorded AthenaAttributeList for " << m_outputKey.fullKey());

  return StatusCode::SUCCESS;
}
