/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHMETADATAWRITER_H
#define DERIVATIONFRAMEWORK_TRUTHMETADATAWRITER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CxxUtils/checker_macros.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Handles to services
#include "GaudiKernel/ServiceHandle.h"
#include "StoreGate/ReadHandleKey.h"

// Service for the metadata (tag info)
#include "EventInfoMgt/ITagInfoMgr.h"

// Service for the weights
#include "GenInterfaces/IHepMCWeightSvc.h"

// EDM classes - typedefs, so have to #include them
#include "xAODTruth/TruthMetaDataContainer.h"
#include "xAODEventInfo/EventInfo.h"

// Standard library includes
#include <string>
#include <unordered_set>

// Forward declarations
class IHepMCWeightSvc;

namespace DerivationFramework {

  class ATLAS_NOT_THREAD_SAFE TruthMetaDataWriter : public extends<AthAlgTool, IAugmentationTool> {
    //  ^ meta-data handling in addBranches
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    /// Connection to the metadata store
    ServiceHandle< StoreGateSvc > m_metaStore{this, "MetaDataStore", "MetaDataStore"};
    /// Service for retrieving the weight names
    ServiceHandle< IHepMCWeightSvc > m_weightSvc{this, "HepMCWeightSvc", "HepMCWeightSvc/HepMCWeightSvc"};
    /// The meta data container to be written out
    xAOD::TruthMetaDataContainer* m_tmd{};
    /// SG key and name for meta data
    Gaudi::Property<std::string> m_metaName{this, "MetaObjectName", "TruthMetaData"}; // FIXME WriteHandle???
    /// Set for tracking the mc channels for which we already added meta data
    mutable std::unordered_set<uint32_t> m_existingMetaDataChan;
    /// TagInfoMgr to get information out of /TagInfo
    ServiceHandle< ITagInfoMgr > m_tagInfoMgr{
      "TagInfoMgr", name()};
    // ReadHandle key for EventInfo
    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this, "EventInfoKey", "EventInfo", "EventInfo key"};

  };
}

#endif
