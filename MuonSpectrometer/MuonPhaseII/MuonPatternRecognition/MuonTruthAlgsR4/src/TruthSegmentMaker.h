/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRUTHSEGMENTMAKER_TRUTHSEGMENTMAKER_H
#define MUONTRUTHSEGMENTMAKER_TRUTHSEGMENTMAKER_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKeyArray.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/WriteDecorHandle.h"


#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonCondData/NswErrorCalibData.h"

#include "MuonPatternEvent/MuonHoughDefs.h"

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
          
          /** @brief Tuple consisting out of pointer to the sim hit and the position & direction
           *         expressed in the chamber's frame */
          using HitPosTuple_t = std::tuple<const xAOD::MuonSimHit*, Amg::Vector3D, Amg::Vector3D>;
          using SimHitVec_t = std::vector<HitPosTuple_t>;



          using EleLink_t = ElementLink<xAOD::MuonSimHitContainer>;
          using HitLinkVec_t = std::vector<EleLink_t>;
          using LinkDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, HitLinkVec_t>;
          using FloatDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, float>;

          using SegPars_t = xAOD::MeasVector<Acts::toUnderlying(SegmentFit::ParamDefs::nPars)>;
          using SegParDecor_t = SG::WriteDecorHandle<xAOD::MuonSegmentContainer, SegPars_t>;
        
          /** @brief Helper struct to ship the write DecorHandles and the reference to the output
           *         segment container through the class methods*/
          struct WriteDecorHolder{
             /** @brief Constructor taking the reference to the mutable output container storing
              *         the truth segments, the refefrence to the TruthSegmentMaker to initialize the
              *         WriteDecorHandles and the event context */
              WriteDecorHolder(xAOD::MuonSegmentContainer& outContainer,
                                const TruthSegmentMaker& parent,
                               const EventContext& ctx):
                  segments{outContainer},
                  paramDecor{parent.m_locParKey, ctx},
                  hitLinkDecor{parent.m_eleLinkKey, ctx},
                  chargeDecor{parent.m_qKey, ctx},
                  ptDecor{parent.m_ptKey, ctx} {}
              /** @brief Reference to the mutable segment container */
              xAOD::MuonSegmentContainer& segments;
              /** @brief Decorator to assign the local segment parameters */
              SegParDecor_t paramDecor;
              /** @brief Decorator to define the links to the sim hits building 
               *         the truth segment */
              LinkDecor_t hitLinkDecor;
              /** @brief Decorate the segment's charge */
              FloatDecor_t chargeDecor;
              /** @brief Decorate the particle's pt at that place */
              FloatDecor_t ptDecor;
          };
          /** @brief Takes a list of related sim hits and transforms them into a truth segment.
           *         If there're less than 3 precision hits, then no segment is created.
           * @param ctx: EventContext to fetch the hit uncertainties from store gate
           * @param locToGlob: Coordinate transform from sector frame -> global frame
           * @param hits: List of hits to be combined onto a single segment
           * @param writeShip: Helper struct carrying the output segment container &
           *                   the associated decorators */
          xAOD::MuonSegment* constructSegmentFromHits(const EventContext& ctx,
                                                      const Amg::Transform3D& locToGlob,
                                                      const SimHitVec_t& hits,
                                                      WriteDecorHolder& writeShip) const;
          /** @brief Attempts to assemble truth segments from a list of loose sim hits, i.e.,
           *         the hits are stemming from a muon but due to the pile-up truth record,
           *         no HepMCParticles are recorded. Hits are paired based on kinetic energy,
           *         and line compatibility
           * @param ctx: EventContext to access the uncertainty conditions data during segment building
           * @param locToGlob: Transform from the sector -> global frame
           * @param simHits: Collection of all sim hits in the sector without truth link
           * @param writeShip: Helper struct carrying the output segment container &
           *                   the associated decorators */
          void buildSegmentsFromBkg(const EventContext& ctx,
                                    const Amg::Transform3D& locToGlob,
                                    const SimHitVec_t& simHits,
                                    WriteDecorHolder& writeShip) const;

          /** @brief Returns the muon pt from the sim hit */
          float muonPt(const xAOD::MuonSimHit& hit, const Amg::Vector3D& globDir) const;
          /** @brief Evaluates the hit uncertainty. For the precision detectors, the uncertainties
           *         are fetched from the COOL DB. For the remaining detectors, the uncertainties are
           *         based on the strip pitch
           * @param ctx: EventContext to fetch the constants from store gate
           * @param hit: Reference to the sim hit */
          float hitUncertainty(const EventContext& ctx, const xAOD::MuonSimHit& hit) const;
          /** @brief IdHelperSvc to decode the Identifiers */
          ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
          /** @brief List of sim hit containers from which the truth segments shall be retrieved */
          SG::ReadHandleKeyArray<xAOD::MuonSimHitContainer> m_readKeys{this, "SimHitKeys", {}};
          /** @brief Key to the geometry context. Needed to align the hits inside ATLAS */
          SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
          /** @brief Key under which the segment Container will be recorded in StoreGate */
          SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "WriteKey", "MuonTruthSegments"};
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
          /** @brief Construct segments from pile-up hits without GenParticleLink */
          Gaudi::Property<bool> m_includePileUpHits{this, "includePileUpHits", true};
          /** @brief Minimum energy threshold for pile up hits to be converted  */
          Gaudi::Property<float> m_pileUpHitMinE{this, "energyThresholdPileUp", 1.*Gaudi::Units::GeV};
          /** @brief Maximum energy loss between two pile-up hits */
          Gaudi::Property<float> m_pileUpHitELoss{this, "pileUpHitELoss", 5.*Gaudi::Units::MeV};
          /** @brief Maximum scattering angle between two pile-up hits */
          Gaudi::Property<float> m_pileUpHitAngleCone{this, "pileUpHitAngleCone", 1.*Gaudi::Units::deg};
          /** @brief Maximum separation between two pile-up hits */
          Gaudi::Property<float> m_pileUpHitDistance{this, "pileUpHitDistance", 2.*Gaudi::Units::m};
          /** @brief ID / ITk cylinder radius */
          Gaudi::Property<float> m_idCylinderR{this, "IdCylinderR", 1.1*Gaudi::Units::m};
          /**  @brief ID / Itk cylinder half length */
          Gaudi::Property<float> m_idCylinderHalfZ{this, "IdCylinderHalfZ", 3.2*Gaudi::Units::m};
          /** @brief Pointer to the muon readout geometry */
          const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
          /** @brief Data dependency on the Mdt calibration container to calculate the uncertainty */
          SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_mdtCalibKey{this, "MdtCalibKey", "MdtCalibConstants",
                                                                                "Conditions object containing the calibrations"};
          /** @brief Data dependency on the Nsw calibration container to estimate the uncertaintys */
          SG::ReadCondHandleKey<NswErrorCalibData> m_nswUncertKey{this, "NswErrorKey", "NswUncertData", 
                                                                  "Key of the parametrized NSW uncertainties"};
    };
}
#endif
