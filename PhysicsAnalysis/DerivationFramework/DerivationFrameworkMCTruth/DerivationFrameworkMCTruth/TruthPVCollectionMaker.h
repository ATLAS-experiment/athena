/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H

// Base classes
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

namespace DerivationFramework {

  class TruthPVCollectionMaker : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::TruthEventContainer> m_eventsKey{this, "EventsKey", "TruthEvents"}; //!< Input event collection (navigates to the vertices)
    SG::WriteHandleKey<xAOD::TruthVertexContainer> m_outVtxKey{this, "NewCollectionName", ""}; //!< Output collection name
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H
