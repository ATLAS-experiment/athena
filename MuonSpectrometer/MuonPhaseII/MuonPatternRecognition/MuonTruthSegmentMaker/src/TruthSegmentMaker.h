/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TRUTHSEGMENTMAKER_H
#define MUONTRUTHSEGMENTMAKER_TRUTHSEGMENTMAKER_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
  class TruthSegmentMaker : public AthReentrantAlgorithm {
      public:
          using AthReentrantAlgorithm::AthReentrantAlgorithm;

          ~TruthSegmentMaker() = default;

          StatusCode initialize() override final;
          StatusCode execute(const EventContext& ctx) const override;
      
      private:
  
          /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
          /// Failure is returned in cases, of non-empty keys and failed retrieval
          template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                      const SG::ReadHandleKey<ContainerType>& key,
                                                                      const ContainerType* & contToPush) const;

          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

          SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_readKeys{this, "SimHitKeys", {}};

          SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

          SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "WriteKey", "TruthSegmentsR4"};

          using HitLinkVec = std::vector<ElementLink<xAOD::MuonSimHitContainer>>;
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_eleLinkKey{this, "SimHitLink", m_segmentKey, "simHitLinks"};

          const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
  };
}
#endif
