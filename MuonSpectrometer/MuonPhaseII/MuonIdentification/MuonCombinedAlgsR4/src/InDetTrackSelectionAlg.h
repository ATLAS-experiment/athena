/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_INDETTRACKSELECTIONAlG_H
#define MUONCOMBINEDALGSR4_INDETTRACKSELECTIONAlG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODTracking/TrackParticleContainer.h"
#include "ActsEvent/TrackContainer.h"

#include "MuonTrackEvent/MuonTag.h"
#include "InDetTrackSelectionTool/IInDetTrackSelectionTool.h"

#include "ActsGeometryInterfaces/ITrackingGeometryTool.h"
#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "GaudiKernel/SystemOfUnits.h"

#include <span>

namespace MuonCombinedR4 {
    /** @brief Algorithm to select ID / ITk track candidates which may be 
               suitable for the combined muon reconstruction chain. Apart from basic
               kinematic requirements, the tracks need to be dR cones around MS tracks 
               or can be loosely matched to a standalone segment which
               has not been yet associated with a muon track. Successfully matched
               candidates need  */
    class InDetTrackSelectionAlg : public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final;
        private:
            /** @brief Extrapolate the ID track to the MS entrance in order to 
                       match it later with spectrometer activity
                @param ctx: EventContext to retrieve the alignment and 
                            magnetic field or the calo extensions
                @param idTrack: The track to be extrapolated */
            std::optional<Acts::BoundTrackParameters>
                extrapolateToMsEntrance(const EventContext& ctx,
                                        const xAOD::TrackParticle& idTrack) const;
            /** @brief Returns whether the ID track expressed on the calorimeter exit volume
                       can be loosely matched with a reconsturcted MS track. The matching is
                       based on the angular cone between the MS & the ID track + separations 
                       on the local parameters on the cylinder
                @param caloExitPars: The ID track expressed at the  calo exit
                @param msTrack: List of reconstructed MS track particles  */
            bool compatibleWithMsTrk(const Acts::BoundTrackParameters& caloExitPars,
                                     const MuonR4::MuonTagContainer& msTracks) const;
            /** @brief Checks whether the ID track is compatible with a reconstructed 
                       segment which is not part of a reconstructed MS track. Matching
                       is based on straight line extrapolations and sector correspondence  */
            bool compatibleWithSegment(const EventContext& ctx,
                                       const Acts::BoundTrackParameters& caloExitPars,
                                       const std::span<const xAOD::MuonSegment*> candidateSegs) const;
            /** @brief The input key for the ID / ITk track particles */
            SG::ReadHandleKey<xAOD::TrackParticleContainer> m_idTrkKey{this, "IdTrackKey", "InDetTrackParticles"};
            /** @brief Optional dependency on the calo extension container. */
            SG::ReadDecorHandleKey<xAOD::TrackParticleContainer> m_extensionDecorKey{this, "CaloExtensionDecorKey", m_idTrkKey, "caloExtensionLink"};
            /** @brief Input key for the MS track particles. ID tracks are only considered if they can be
                      roughly matched to a MS track */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_msTrkKey{this, "MsTrackKey", "MuonTagsSA"};
            /** @brief To pass the selection criteria, ITk tracks can alternatively be matched to  */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
            /** @brief The output key for the selected track candidates */           
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_writeKey{this, "writeKey", "MuonInDetCandidates"};
            /** @brief Detector manager to retrieve the sector envelope surfaces */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Track quality selection tool (optional) */
            ToolHandle<InDet::IInDetTrackSelectionTool> m_selectionTool{this, "TackSelectionTool" , ""};
            /** @brief Tracking geometry tool */
            PublicToolHandle<ActsTrk::ITrackingGeometryTool> m_trackingGeometryTool{this, "TrackingGeometryTool", ""};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };
            /** @brief Flag toggling whether the last track parameters shall be retrieved 
             *          from the calo extension linked to the ID tracks */
            Gaudi::Property<bool> m_useCaloExtension{this, "useCaloExtension", true};
            /** @brief The minimum momentum cut applied on the ID tracks to be considered */
            Gaudi::Property<float> m_trackPt{this, "minPt", 2.5*Gaudi::Units::GeV};
            /** @brief Apply a maximum eta cut to stay within the MS acceptance */
            Gaudi::Property<float> m_trackEta{this, "maxEta", 2.8};
            /** @brief Selection cuts for the MS tracks */
            Gaudi::Property <float> m_dEtaCutMsTrk{this, "dEtaMaxMsTrk", 0.2};
            Gaudi::Property <float> m_dPhiCutMsTrk{this, "dPhiMsTrk", 5*Gaudi::Units::deg};
            /** @brief Selection cuts for the Muon segments */
            Gaudi::Property <float> m_dEtaCutMsSeg{this, "dEtaMaxMsSegment",  0.3};
            Gaudi::Property <float> m_dY0CutMsSeg{this, "dY0MaxMsSegment", 40 * Gaudi::Units::cm};
    };
}

#endif