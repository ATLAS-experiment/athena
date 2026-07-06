/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_FASTRECONSTRUCTIONALGS_FASTMUONSABUILDER__H
#define MUONR4_FASTRECONSTRUCTIONALGS_FASTMUONSABUILDER__H

#include <xAODMuon/Muon.h>
#include <xAODMuon/MuonContainer.h>
#include "xAODMuon/MuonAuxContainerR4.h"
#include "xAODMuonViews/FillContainer.h"

#include <MuonFastRecoEvent/GlobalPattern.h>

#include <MuonRecToolInterfacesR4/ISpacePointCalibrator.h>
#include <MuonPatternHelpers/SegmentLineFitter.h>
#include <MuonPatternHelpers/MdtSegmentSeedGenerator.h>
#include <MuonTrackFindingTools/MsTrackSeeder.h>
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MagFieldConditions/AtlasFieldCacheCondObj.h"

#include <MuonRecToolInterfacesR4/IPatternVisualizationTool.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>


namespace MuonR4::FastReco{
    
    /// @brief Standalone module to handle fast segment fitting and momentum estimation
    ///        starting from global patterns.
    /// 
    /// This tool consists of the final stage of the Phase-2 fast reconstruction,
    /// processing the global patterns found in previous steps to build muon 
    /// candidates. It performs a straight line segment fit using the pattern hits
    /// in each station and use the fitted segments to estimate the momentum of 
    /// the muon candidate. The main method consumes the global patterns and produces
    /// the output muon candidates as ****to be defined FastReco::MuonSA objects****. 

    class FastMuonSABuilder : public AthMessaging {
        public:
            /** @brief Type alias for the station index */
            using StIndex = Muon::MuonStationIndex::StIndex;
            /** @brief Type alias for the segment fitting parameters */
            using Parameters = SegmentFit::Parameters;
            /** @brief Type alias for the line fitter */
            using LineFitter = MuonR4::SegmentFit::SegmentLineFitter;
            /** @brief Type alias for the L-R segment seeder */
            using MdtSegmentSeeder = MuonR4::SegmentFit::MdtSegmentSeedGenerator;
            /** @brief Define the muon container type */
            using MuonCont_t = xAOD::FillContainer<xAOD::MuonContainer, xAOD::MuonAuxContainerR4>;
            /** @brief Configuration object */           
            struct Config {
                /** @brief Whether to recalibrate hits during fitting */
                bool recalibInFit{false};
                /** @brief Whether to use Hessian residuals */
                bool useHessianResidual{false};
                /** @brief Two mdt seeds are the same if their defining parameters match within */
                double seedHitChi2{5.};
                /** @brief Toggle seed recalibration. The two seed circles are recalibrated using 
                 *         the initial seed */
                bool recalibSeed{false};
                /** @brief Reduced chi2 defining a good segment, stopping the fit of other segments in the same station */
                double goodSegmentCut{2.};
                /** @brief Cut on the segment chi2 / nDoF to launch the outlier removal */
                double outlierRemovalCut{5.};
                /** @brief Pull value for hit recovery */
                double recoveryPull{5.};
                /** @brief Minimum number of precision hits required for a segment */
                unsigned precHitCut{3};
                /** @brief Whether to use the fast fitter */
                bool useFastFitter{true};
                /** @brief Whether the fast fitter is used as a pre-fitter */
                bool fastPreFitter{false};
                /** @brief Whether to ignore failed pre-fits and try the full fit anyway */
                bool ignoreFailedPreFit{false};
                /** @brief Maximum number of iterations in the fit */
                unsigned maxIter{50};
                /** @brief Steps between two segments to integrate the magnetic field */
                unsigned nFieldSteps{30};
                /** @brief Pointer to the calibrator */
                const ISpacePointCalibrator* calibrator{nullptr};
                /** @brief Pointer to the visualization tool */
                const MuonValR4::IPatternVisualizationTool* visionTool{nullptr};
                /** @brief Pointer to the idHelperSvc */
                const Muon::IMuonIdHelperSvc* idHelperSvc{nullptr};
            };

            /** @brief Standard constructor
             *  @param name: Name to be printed in the messaging
             *  @param config: Configuration parameters */
            FastMuonSABuilder(const std::string& name,
                              Config&& config);

            /** @brief Main methods steering the muon candidate building. Given the global pattern,
             *         it fits segments in each station and use them to estimate the muon momentum.
             *  @param ctx: Event context
             *  @param gctx: Geometry context
             *  @param magField: Magnetic field
             *  @param pattern: Global pattern
             *  @param outMuons: Output muon container to be filled
             *  @return: Pointer to the built muon candidate if successful, otherwise nullptr */
            xAOD::Muon* buildMuonCandidate(const EventContext& ctx,
                                           const ActsTrk::GeometryContext& gctx,
                                           const AtlasFieldCacheCondObj& magField,
                                           const GlobalPattern& pattern,
                                           MuonCont_t& outMuons) const;
        private:
            /** @brief Type alias for the hit type & associated vector */
            using Hit_t = const SpacePoint*;
            using HitVec_t = std::vector<Hit_t>;
            /** @brief Type alias for the bucket type */
            using Bucket_t = const SpacePointBucket*;
            /** @brief Type alias for the segment type */
            using Segment_t = std::unique_ptr<Segment>;
            /** @brief Fit a segment using the provided seed
             *  @param ctx: Event context
             *  @param localToGlobal: Transformation from local to global coordinates
             *  @param patternSeed: Seed for the segment fit
             *  @return: Unique pointer to the fitted segment */
            Segment_t fitSegment(const EventContext& ctx,
                                 const Amg::Transform3D& localToGlobal,
                                 Bucket_t parentBucket,
                                 std::vector<Hit_t>&& hits) const;

            /** @brief Estimate the bending parameters of a segment through a weighted linear regression. 
             *         This method is used to estimate initial parameters when no straw measurements
             *         are available and the MDTseeder cannot be used, e.g. NSW.
             *  @param hits: Vector of hits
             *  @param pars: Parameters to be estimated
             *  @return: Vector of valid hits during the linear regression */
            HitVec_t estimateBendingPars(HitVec_t&& hits,
                                         Parameters& pars) const;
            /** @brief Global Pattern Recognition configuration */
            Config m_cfg;
            /** @brief Spacepoint sorter per logical measurement layer */
            SpacePointPerLayerSorter m_spSorter{};
            /** @brief Pointer to the actual segment fitter */
            std::unique_ptr<LineFitter> m_fitter{};
            /** @brief Pointer to the NSW segment fitter */
            std::unique_ptr<LineFitter> m_nswFitter{};
            /** @brief Pointer to the L-R segment seeder */
            std::unique_ptr<MdtSegmentSeeder> m_mdtSeeder{};
            /** @brief Pointer to the track seeder for momentum estimate */
            std::unique_ptr<MsTrackSeeder> m_trackSeeder{};
    };
}


#endif
