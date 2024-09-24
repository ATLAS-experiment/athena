/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H
#define MUONR4_MUONPATTERNRECOGNTIONALGS_SEGMENTFITTINGALG__H

#include "MuonPatternEvent/MuonPatternContainer.h"


#include "MuonTruthHelpers/MuonSimHitHelpers.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonSpacePointCalibrator/ISpacePointCalibrator.h"

#include "MuonPatternEvent/SegmentFitterEventData.h"
#include "MuonPatternEvent/MuonHoughDefs.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include <TObject.h>

#include <set>


namespace ROOT {
  namespace Math{
    class Minimizer;
  }
}

class TCanvas;

namespace MuonR4{
    class CalibSegmentChi2Minimizer;
    
    /// @brief Algorithm to handle segment fits  
    /// 
    /// This is currently a placeholder to test ideas! 
    class SegmentFittingAlg: public AthReentrantAlgorithm{
        public:
            SegmentFittingAlg(const std::string& name, ISvcLocator* pSvcLocator);
            virtual ~SegmentFittingAlg();
            virtual StatusCode initialize() override;
            virtual StatusCode execute(const EventContext& ctx) const override;
            virtual StatusCode finalize() override;

            using HitVec = SegmentFitResult::HitVec;

        private:
            using Parameters = SegmentFit::Parameters;
            /// Helper method to fetch data from StoreGate. If the key is empty, a nullptr is assigned to the container ptr
            /// Failure is returned in cases, of non-empty keys and failed retrieval
            template <class ContainerType> StatusCode retrieveContainer(const EventContext& ctx,
                                                                        const SG::ReadHandleKey<ContainerType>& key,
                                                                        const ContainerType* & contToPush) const;
            /** @brief Executes the segment fit with start parameters. The returned fit result
             *         indicates whether the fit was a success and all relevant output parameters
             *  @brief ctx: Event context needed to access the calibration constants of the hits
             *  @brief gctx: Geometry context needed to place the segment globally within ATLAS to
             *               calculate the nominal time of arrival
             *  @brief startPars: Segment parameters preestimated from the SegmentSeed 
             *                    (either  hough pattern or two drift circle seed)
             *  @brief calibHits: Vector of strip & mdt hits to consider for the fit */
            SegmentFitResult fitSegmentHits(const EventContext& ctx,
                                            const ActsGeometryContext& gctx,
                                            const Parameters& startPars,
                                            SegmentFitResult::HitVec&& calibHits) const;
            /** @brief  */
            const xAOD::MuonSegment* getTruthSegment(const EventContext& ctx,
                                                     const std::unordered_set<const xAOD::MuonSimHit*>& matchedSeedHits) const;

            std::vector<std::unique_ptr<Segment>> fitSegmentSeed(const EventContext& ctx,
                                                                 const ActsGeometryContext& gctx,
                                                                 const SegmentSeed* seed) const;             
            /** @brief Spot hits with large discrepancy from the estimated parameters and remove them
             *         from the list.
             *  @param ctx: EventContext needed for hit calibration
             *  @param gctx: Geometry context needed if the beamspot constaint will be packed onto the segment
             *  @param fitResult: Data structure carrying the fit parameters & calibrated hits */
            bool removeOutliers(const EventContext& ctx,
                                const ActsGeometryContext& gctx,
                                const SegmentSeed& seed,
                                SegmentFitResult& fitResult) const;

            /** @brief Recovery of missed hits. Hits in the space point bucket  that are maximally
             *         <RecoveryPull> away from the fitted segment are put onto the segment candidate
             *         and the candidate is refitted. If the refitted candidate has a chi2/nDoF < <OutlierRemoval>
             *         the canidate is automatically choosen otherwise, its chi needs to be better. 
             *  @param ctx: EventContext needed for calibration of the hits
             *  @param gctx: Geometry context needed if the beamspot constaint will be packed onto the segment
             *  @param seed: Segment seed from the pattern recognition to access the underlying bucket
             *  @param toRecover: Fit result with parameters & hits to recover.  */
            bool plugHoles(const EventContext& ctx,
                           const ActsGeometryContext& gctx,
                           const SegmentSeed& seed,
                           SegmentFitResult& toRecover) const;
            /** @brief Removes all hits from the segment which are obvious outliers. E.g. tubes 
             *         which cannot be crossed by the segment. 
             *  @param gctx: Geometry context needed to calculate the nominal time of arrival,
             *               if the time fit is activated
             *  @param candidate: Reference of the segment candidate to prune. */
            void eraseWrongHits(const ActsGeometryContext& gctx, SegmentFitResult& candidate) const;            
            
            void resolveAmbiguities(const ActsGeometryContext& gctx,
                                    std::vector<std::unique_ptr<Segment>>& segmentCandidates) const;
            
            void visualizeFit(const EventContext& ctx,
                              const SegmentFitResult& fitResult,
                              const SpacePointBucket* bucket,
                              const std::string& extraLabel,
                              std::vector<std::unique_ptr<TObject>> primitives={}) const; 
            
            void visualizeFitPhi(const EventContext& ctx,
                                 const SegmentFitResult& fitResult,
                                 const SpacePointBucket* bucket,
                                 const std::string& extraLabel) const;

            /// ReadHandle of the seeds
            SG::ReadHandleKey<SegmentSeedContainer> m_seedKey{this, "ReadKey", "MuonHoughStationSegmentSeeds"};
            /// ReadHandle for the truth segments (For validation purposes)
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_truthSegKey{this, "TruthSegKey", "TruthSegmentsR4"};
            
            // write handle key for the output segment seeds 
            SG::WriteHandleKey<SegmentContainer> m_outSegments{this, "MuonSegmentContainer", "R4MuonSegments"};

            // access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"};

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};


            ToolHandle<ISpacePointCalibrator> m_calibTool{this, "Calibrator", "" };
            
            /// Toggle the fitter
            Gaudi::Property<bool> m_useMinuit{this, "useMinuit", false};

            Gaudi::Property<bool> m_doT0Fit{this, "fitSegmentT0", true};
            /// Add beamline constraint
            Gaudi::Property<bool> m_doBeamspotConstraint{this, "doBeamspotConstraint", false};
            Gaudi::Property<double> m_beamSpotR{this, "BeamSpotRadius", 30.* Gaudi::Units::cm};
            Gaudi::Property<double> m_beamSpotL{this, "BeamSpotLength", 20. * Gaudi::Units::m};

            
            /** @brief Two mdt seeds are the same if their defining parameters match wihin */
            Gaudi::Property<double> m_seedTanThetaReso{this, "ResoSeedTanTheta", 250. * Gaudi::Units::mrad};
            Gaudi::Property<double> m_seedY0Reso{this, "ResoSeedY0", 500.*Gaudi::Units::micrometer};
            Gaudi::Property<double> m_seedHitChi2{this, "ResoSeedHitAssoc", 5. };
            /** Cut on the segment chi2 / nDoF to launch the outlier removal */
            Gaudi::Property<double> m_outlierRemovalCut{this, "OutlierRemoval", 5.};
            Gaudi::Property<double> m_recoveryPull{this, "RecoveryPull", 5.};


            std::unique_ptr<TCanvas> m_allCan{};
            Gaudi::Property<std::string> m_allCanName{this, "AllCanName", "AllSegmentFits"};
            mutable std::atomic<unsigned int> m_canvCounter ATLAS_THREAD_SAFE{0};
            /// Draw maximally 5000 segment fit visualizations
            Gaudi::Property<unsigned int> m_nDrawCanvases{this, "MaxCanvases", 5000};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};
    };
}


#endif
