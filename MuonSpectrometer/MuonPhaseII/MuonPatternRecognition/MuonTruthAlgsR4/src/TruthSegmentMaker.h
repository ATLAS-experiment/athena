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
  /*** @brief The TruthSegmentMaker collects hits inside the chamber stemming from the same HepMC::GenParticle. 
   *          The particle state at the first sim hit is propagated via a simple straight line onto the plane 
   *          z=0 in the sector frame. The parameters are then translated into the global frame and a xAOD::MuonSegment
   *          is created. The segment hit summary is written based on the sim hit type. If a gasGap has eta & phi measurements,
   *          the counters in the respective categories are each increased for the given sim hit. Finally, a vector of ElementLinks
   *          pointing to the sim hits is decorated to the segment. */
  class TruthSegmentMaker : public AthReentrantAlgorithm {
      public:
          using AthReentrantAlgorithm::AthReentrantAlgorithm;

          ~TruthSegmentMaker() = default;

          StatusCode initialize() override final;
          StatusCode execute(const EventContext& ctx) const override;
      
      private:
          /** @brief Returns the transform from the local simHit frame -> chamber frame
           *  @param gctx: Geometry context to align the chambers within ATLAS
           *  @param chanId: Identifier of the channel for which the transform shall be fetched */
          Amg::Transform3D toChamber(const ActsGeometryContext& gctx,
                                     const Identifier& chanId) const;
          /** @brief IdHelperSvc to decode the Identifiers */
          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
          /** @brief List of sim hit containers from which the truth segments shall be retrieved */
          SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_readKeys{this, "SimHitKeys", {}};
          /** @brief Key to the geometry context. Needed to align the hits inside ATLAS */
          SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
          /** @brief Key under which the segment Container will be recorded in StoreGate */
          SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "WriteKey", "TruthSegmentsR4"};
          /** @brief Decoration key of the associated sim hit links */
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_eleLinkKey{this, "SimHitLink", m_segmentKey, "simHitLinks"};
          /** @brief Decoration key of the associated particle pt */
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_ptKey{this, "PtKey", m_segmentKey, "pt"};
          /** @brief Decoration key of the local parameters */
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_locParKey{this, "LocParKey", m_segmentKey,"localSegPars"};
          /** @brief Decoration key of the muon charge  */
          SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer> m_qKey{this, "qKey", m_segmentKey, "charge"};
          /** @brief Build segments from muon hits only */
          Gaudi::Property<bool> m_useOnlyMuonHits{this, "useOnlyMuonHits", true};
          /** @brief Pointer to the muon readout geometry */
          const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
  };
}
#endif
