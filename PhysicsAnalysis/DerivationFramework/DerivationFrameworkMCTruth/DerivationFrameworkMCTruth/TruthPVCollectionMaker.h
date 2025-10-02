/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H
#define DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODTruth/TruthEventContainer.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"

namespace DerivationFramework {

  class TruthPVCollectionMaker : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      TruthPVCollectionMaker(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthPVCollectionMaker();
      StatusCode initialize();
      virtual StatusCode addBranches() const;

    private:
      SG::ReadHandleKey<xAOD::TruthEventContainer> m_eventsKey{this, "EventsKey", "TruthEvents"}; //!< Input event collection (navigates to the vertices)
      SG::WriteHandleKey<xAOD::TruthVertexContainer> m_outVtxKey{this, "NewCollectionName", ""}; //!< Output collection name
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHPVCOLLECTIONMAKER_H
