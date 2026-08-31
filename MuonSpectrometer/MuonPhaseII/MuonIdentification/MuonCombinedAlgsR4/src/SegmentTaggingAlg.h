/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_SEGMENTTAGGINGALG_H
#define MUONCOMBINEDALGSR4_SEGMENTTAGGINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonTrackEvent/MuonTag.h"
#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "ActsGeometryInterfaces/IExtrapolationTool.h"
#include "ActsEvent/ContextUtility.h"

namespace MuonCombinedR4 {
    class SegmentTaggingAlg: public AthReentrantAlgorithm {
        public:
            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute(const EventContext& ctx) const override final; 

        private:
            /** @brief Calculates the chi2 deviation between the segment parameters and
             *         the extrapolated ID track parameters
             * @param segment: Segment for which the chi2 shall be evaluated
             * @param extpIdPars: The ID track parameters on the same surface as the
             *                    segment's surface */
            double matchingScore(const Acts::GeometryContext& tgContext,
                                 const xAOD::MuonSegment& segment,
                                 const Acts::BoundTrackParameters& extpIdPars) const;
            /** @brief Returns the surface on which the local paramters of the segment
             *         are expressed
             * @param segment: The segment of interest*/
            const Acts::Surface& getSurface(const xAOD::MuonSegment& segment) const;
            /** @brief Retrieves the MS segment container from store gate and picks all
             *         segments that are not associated with a combined track in the reconstruction
             *         stage upstream
             * @param ctx: EventContext to access store gate */
            std::vector<const xAOD::MuonSegment*> prepareSegments(const EventContext& ctx) const;
            /** @brief First step of the segment tagging. The ID parameters at the calorimeter
             *         exit are roughly compared with each segment's sector and eta direction. 
             *         Suitable segments are appended to the list. At the end the list is sorted
             *         by the path -length of the straight line extrpolation onto the spectrometer
             *         sector surface.
             *  @param tgCotnext: The geometry context to align the calorimeter parameters in space
             *  @param caloExitPars: The ID track parameters expressed on the calo exit surface
             *  @param segments: Sollection of segments obtained from the @ref prepareSegments method */
            std::vector<const xAOD::MuonSegment*> findPotentialMatches(const Acts::GeometryContext& tgContext,
                                                                       const Acts::BoundTrackParameters& caloExitPars,
                                                                       const std::vector<const xAOD::MuonSegment*>& segmentCont) const;
            
            /** @brief Extrpolates the ID track through the MS and attempts to match the preselected candidates based
             *         on the chi2 between the segment parameters and the extrapolated ID track parameters. If no
             *         segment satisfies the chi2 cut, then the tag is destroyed and a nullptr is returned
             *  @param ctx: EventContext to access the alignment and the conditions
             *  @param selectedCandidates: List of pre-selected segment candidates to be matched
             *  @param idTag: The inner detector tag which is going to be extrapolated through the MS */
            std::unique_ptr<MuonR4::MuonTag> tagSegments(const EventContext& ctx,
                                                         std::vector<const xAOD::MuonSegment*>&& selectedCandidates,
                                                         std::unique_ptr<MuonR4::MuonTag>&& idTag) const;
            /** @brief The key of the selected ID track candidates extrapolated to the CaloExit  */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_idTrkKey{this, "IdTrackKey", "MuonInDetCandidates"};
            /** @brief List of successfully outside -> in combined muon track candidates. Segments associated
                       with combined tracks are not considered for the SegmenTagging chain */
            SG::ReadHandleKey<MuonR4::MuonTagContainer> m_cmbTrkKey{this, "CombedTrackKey", ""};
            /** @brief To pass the selection criteria, ITk tracks can alternatively be matched to  */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_segmentKey{this, "SegmentKey", "MuonSegmentsFromR4"};
            /** @brief The output key for the selected track candidates */           
            SG::WriteHandleKey<MuonR4::MuonTagContainer> m_writeKey{this, "writeKey", "SegmentTags"};
            /** @brief Detector manager to retrieve the sector envelope surfaces */
            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
            /** @brief Context provider for geometry, magnetic field and calibration contexts */
            ActsTrk::ContextUtility m_ctxProvider{this};
            /** @brief Track extrapolation tool */
            ToolHandle<ActsTrk::IExtrapolationTool> m_extrapolationTool{this, "ExtrapolationTool" ,"" };

            /** @brief Extra tolerance applied on the non-bending intercept when calculating
             *         the matching score */
            Gaudi::Property<double> m_toleranceX0{this, "toleranceX0", 20.*Gaudi::Units::cm};
            /** @brief Extra tolerance applied on the bending intercept when calculating
             *         the matching score */
            Gaudi::Property<double> m_toleranceY0{this, "toleranceY0", 5.*Gaudi::Units::cm};
            /** @brief Extra tolerance applied on the bending direction when calculating the
             *         matching score */
            Gaudi::Property<double> m_toleranceTheta{this, "toleranceTheta", 1.*Gaudi::Units::deg};
            /** @brief Extra tolerance applied on the bending direction when calculating the
             *         matching score */
            Gaudi::Property<double> m_tolerancePhi{this, "tolerancePhi", 2.*Gaudi::Units::deg};
            /** @brief Selection cut to match a segment to the ID track */
            Gaudi::Property<double> m_matchChi2{this, "matchChi2", 10.};
    };
}
#endif