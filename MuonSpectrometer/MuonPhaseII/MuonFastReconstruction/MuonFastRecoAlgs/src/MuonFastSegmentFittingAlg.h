/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONR4_MUONFASTRECOALGS_MUONFASTSEGMENTFITTINGALG__H
#define MUONR4_MUONFASTRECOALGS_MUONFASTSEGMENTFITTINGALG__H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/WriteHandleKey.h"

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "xAODMuon/MuonSegmentContainer.h"

#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"
#include "MuonRecToolInterfacesR4/IxAODSegmentCnvTool.h"
#include "MuonPatternHelpers/SegmentLineFitter.h"
#include "MuonPatternHelpers/MdtSegmentSeedGenerator.h"
#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

namespace MuonR4{
    /// @brief Algorithm handling fast segment fitting starting from global patterns.
    /// 
    /// This algorithm processes the global patterns found in previous steps to build 
    /// segments, needed later to estimate the momentum. It performs a straight line
    /// segment fit using the pattern hits in each station. The main method consumes 
    /// the global patterns and produces the xAOD::segment container decorated with 
    /// links to the original patterns. 

    class MuonFastSegmentFittingAlg: public AthReentrantAlgorithm{
        public:

            using AthReentrantAlgorithm::AthReentrantAlgorithm;
            virtual ~MuonFastSegmentFittingAlg() = default;

            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;

        private:
            /** @brief Type alias for the station index */
            using StIndex = Muon::MuonStationIndex::StIndex;
            /** @brief Type alias for the hit type & associated vector */
            using Hit_t = const SpacePoint*;
            using HitVec_t = std::vector<Hit_t>;
            /** @brief Type alias for the bucket type */
            using Bucket_t = const SpacePointBucket*;
            /** @brief Type alias for the segment seed type */
            using Seed_t = std::unique_ptr<SegmentSeed>;
            /** @brief Type alias for the segment type */
            using Segment_t = std::unique_ptr<Segment>;
            /** @brief Type alias for the segment fitting parameters */
            using Parameters = SegmentFit::Parameters;
            /** @brief Type alias for the line fitter */
            using LineFitter = MuonR4::SegmentFit::SegmentLineFitter;
            /** @brief Type alias for the L-R segment seeder */
            using MdtSegmentSeeder = MuonR4::SegmentFit::MdtSegmentSeedGenerator;

            /** @brief Struct to hold the segment and the seed for the needed lifetime */
            using SegmentSeedPair_t = std::pair<Seed_t, Segment_t>;
            /** @brief Type alias for the segment-seed optional */
            using SegmentSeedOpt_t = std::optional<SegmentSeedPair_t>;
            
            /** @brief Main methods steering the segment fitting. Given the global pattern,
             *         it fits segments in each station, computing just the minimum amount
             *         to allow muon candidate building, i.e. 3 segments.
             *  @param ctx: Event context
             *  @param gctx: Geometry context
             *  @param pattern: Global pattern
             *  @return: Vector of fitted segments */
            std::vector<SegmentSeedPair_t> processPattern(const EventContext& ctx,
                                                          const ActsTrk::GeometryContext& gctx,
                                                          const GlobalPattern& pattern) const;
            /** @brief Fit a segment in a station given the hits & their parent bucket.
             *  @param ctx: Event context
             *  @param localToGlobal: Transformation from local to global coordinates
             *  @param parentBucket: Pointer to the parent bucket of the hits, needed for hole recovery
             *  @param hits: Vector of hits to be fitted
             *  @return: Unique pointer to the fitted segment */
            SegmentSeedOpt_t fitSegment(const EventContext& ctx,
                                        const Amg::Transform3D& localToGlobal,
                                        Bucket_t parentBucket,
                                        HitVec_t&& hits) const;
            /** @brief Estimate the initial parameters for the segment fitting.
             *  @param localToGlobal: Transformation from local to global coordinates
             *  @param hits: Vector of hits
             *  @return: Estimated initial parameters */
            std::pair<HitVec_t, Parameters> initializePars(const Amg::Transform3D& localToGlobal,
                                                           const HitVec_t& hits) const;
            /** Define the coordinate planes */
            enum class CoordPlane : std::uint8_t {   
                /** Bending plane */            
                etaPlane = 0,
                /** Phi, e.g. non-bending, plane */
                phiPlane = 1
            };
            /** Define the line parameters */
            enum class ParamDefs2D : std::uint8_t {
                /** Tangent of the angle in the plane, defined as dy/dz */
                tanTheta = 0,
                /** Intercept in the plane */
                y0 = 1,
                /** Number of parameters */
                nParams = 2
            };
            /** Define simplified beamspot measurement in a defined plane */
            struct Beamspot {
                /** @brief Coordinates of the beamspot in the plane */
                double y{0.};
                double z{0.};
                /** @brief Covariance of the beamspot in the y-direction */
                double cov_yy{0.};
            };
            /** @brief Type alias for the line representation */
            using Line2D_t = std::array<double, Acts::toUnderlying(ParamDefs2D::nParams)>;
            /** @brief Type alias for the result of the linear regression, consisting of the
             *         valid hits and line parameters */
            using RegressionRes_t = std::pair<HitVec_t, std::optional<Line2D_t>>;
            /** @brief Estimate the segment parameters in the plane defined by the CoordPlane template 
             *         parameter using a weighted linear regression. This method is used to estimate 
             *         initial parameters either in the phi plane or in the bending plane when no 
             *         straw measurements are available and the MDTseeder cannot be used, e.g. NSW.
             *  @param Plane: Coordinate plane to be used for the regression
             *  @param hits: Vector of hits
             *  @return: Result of the linear regression */
            RegressionRes_t linearRegression(const CoordPlane Plane,
                                             const HitVec_t& hits,
                                             const std::optional<Beamspot>& beamspot = std::nullopt) const;
            /** @brief Find the segment to which increase the phi measurement count. This is needed to 
             *         avoid the segment selector to discard the candidate during the candidate building.
             *  @param segs: Vector of segment-seed pairs
             *  @return: Pointer to the segment to add phi measurements to, if any */
            const Segment* findSegmentToAddPhi(std::vector<SegmentSeedPair_t>& segs) const;
            /** @brief Helper method to compute the beamspot covariance in one local coordinate
             *  @param Plane: Coordinate plane to be used
             *  @param localToGlobal: Transform from local to global coordinates
             *  @return: The beamspot covariance in the specified local coordinate */
            double beamspotCov(const CoordPlane Plane, 
                               const Amg::Transform3D& localToGlobal) const;

            /** @brief Write handle key for the output global patterns */ 
            SG::ReadHandleKey<GlobalPatternContainer> m_inPatterns{this, "InPatterns", "MuonR4GlobalPatterns", "Global patterns to read"};
            /** @brief Write handle key for the output segments */ 
            SG::WriteHandleKey<xAOD::MuonSegmentContainer> m_outSegments{this, "OutSegments", "FastMuonSegments", "Segments to write"};

            /** @brief Abrivation of the extra declared auxVariables  */
            using DecorKey_t = SG::WriteDecorHandleKey<xAOD::MuonSegmentContainer>;
            /** @brief Parent global pattern decoration of the segments */
            DecorKey_t m_patternKey{this, "ParentPatternKey", m_outSegments, "GlobalPatternLink", "Parent global pattern for the segments"};
            /** @brief Decoration of the local segment parameters */
            DecorKey_t m_localSegParKey{this, "LocalSegParKey", m_outSegments, "localSegPars"};
            /** @brief Decoration of the local fit covariance parameters */
            DecorKey_t m_localSegCovKey{this, "LocalCovParKey", m_outSegments, "localSegCov"};
            /** @brief Decoration to the links to the associated Uncalibrated measurements */
            DecorKey_t m_prdLinkKey{this, "PrdLinkKey",  m_outSegments, "prdLinks" };
            /** @brief Decoration to the PrdLink state (I.e. outlier or valid) */
            DecorKey_t m_prdStateKey{this, "PrdStateKey", m_outSegments, "prdState"};
            /** @brief Auxiliary container to model two measurements in the same gas gap as a single track state */
            SG::WriteHandleKey<xAOD::CombinedMuonStripContainer> m_combMeasKey{this, "combinedPrdKey", "FastCombinedPrdKey"};

            /** @brief Geometry context key */              
            ActsTrk::GeoContextReadKey_t m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};
            /** @brief Handle to the MuonIdHelper service */        
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
            /** @brief Handle to the space point calibrator tool */
            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            /** @brief Segment converter tool */
            ToolHandle<IxAODSegmentCnvTool> m_segmentCnvTool{this, "SegmentCnvTool", ""};
            /** @brief Handle to the visualization tool for segments */
            ToolHandle<MuonValR4::IPatternVisualizationTool> m_segVisionTool{this, "VisualizationTool", ""};

            /** @brief Toggle the recalibration of hits during the segment fit */
            BooleanProperty m_recalibInFit{this, "recalibInFit" , false};
            /// @brief Use the expliciit Hessian in the residual calculation
            BooleanProperty m_hessianResidual{this, "useHessianResidual", false};
            /** @brief Two mdt seeds are the same if their defining parameters match wihin */
            DoubleProperty m_seedHitChi2{this, "ResoSeedHitAssoc", 5. };
            /** @brief Toggle seed recalibration. The two seed circles are recalibrated using 
             *         the initial seed */
            BooleanProperty m_recalibSeed{this, "SeedRecalibrate", false};
            /** @brief Reduced chi2 defining a good segment, stopping the fit of other segments in the same station */
            DoubleProperty m_goodSegmentCut{this, "GoodSegmentCut", 2.};
            /** @brief Number of degrees of freedom defining a good segment, stopping the fit of other segments in the same station */
            UnsignedIntegerProperty m_goodSegmentDoF{this, "GoodSegmentDoF", 5};
            /** @brief Cut on the segment chi2 / nDoF to launch the outlier removal */
            DoubleProperty m_outlierRemovalCut{this, "OutlierRemoval", 3.};
            /** @brief Pull value for hit recovery */
            DoubleProperty m_recoveryPull{this, "RecoveryPull", 3.};
            /** @brief Minimum number of precision hits to accept the segment */
            UnsignedIntegerProperty m_precHitCut{this, "PrecHitCut" , 3};
            /** @brief Use the fast Mdt fitter where possible */
            BooleanProperty m_useFastFitter{this, "useFastFitter", false};
            /** @brief The fast fitter is treated as a pre fitter */
            BooleanProperty m_fastPreFitter{this, "useFastPreFitter", false};
            /** @brief Switch to try the full fit when the fast pre-fitter fails */
            BooleanProperty m_ignoreFailedPreFit{this, "ignoreFailedPreFit", false};
            /** @brief Tune the number of iterations */
            UnsignedIntegerProperty m_maxIter{this, "maxIterations", 50};
            /** @brief Beam spot radius */
            BooleanProperty m_beamSpotRadius{this, "BeamSpotRadius", 30.*Gaudi::Units::cm};
            /** @brief Beam spot length */
            BooleanProperty m_beamSpotLength{this, "BeamSpotLength", 2.*Gaudi::Units::m};

            /** @brief Spacepoint sorter per logical measurement layer */
            SpacePointPerLayerSorter m_spSorter{};
            /** @brief Pointer to the actual segment fitter */
            std::unique_ptr<LineFitter> m_fitter{};
            /** @brief Pointer to the NSW segment fitter */
            std::unique_ptr<LineFitter> m_nswFitter{};
            /** @brief Pointer to the L-R segment seeder */
            std::unique_ptr<MdtSegmentSeeder> m_mdtSeeder{};
            /** @brief Covariance matrix of the beam spot */
            Acts::SquareMatrix<3> m_beamspotCov {Acts::SquareMatrix<3>::Zero()};
    };

}

#endif