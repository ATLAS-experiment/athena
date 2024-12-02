/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/SegmentAmbiSolver.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>
#include <Acts/Utilities/Enumerate.hpp>

namespace MuonR4 {
    using namespace SegmentFit;
    using SegmentVec = SegmentAmbiSolver::SegmentVec;

    SegmentAmbiSolver::SegmentAmbiSolver(const std::string& name, Config&& cfg):
        AthMessaging{name},
        m_cfg{std::move(cfg)} {}

    double SegmentAmbiSolver::redChi2(const Segment& segment) const {
        return segment.chi2() / segment.nDoF();
    }
    SegmentVec SegmentAmbiSolver::resolveAmbiguity(const ActsGeometryContext& gctx,
                                                   SegmentVec&& toResolve) const {
        
        std::ranges::stable_sort(toResolve,[this](const SegmentVec::value_type& a,
                                              const SegmentVec::value_type& b){
            const double redChi2A = redChi2(*a);
            const double redChi2B = redChi2(*b);
            if (redChi2A < m_cfg.selectByNDoFChi2 && redChi2B < m_cfg.selectByNDoFChi2){
                return a->nDoF() > b->nDoF();
            }
            return redChi2A < redChi2B;
        });
        SegmentVec resolved{};

        /// Mark the first object as resolved
        resolved.push_back(std::move(toResolve[0]));
        toResolve.erase(toResolve.begin());
        std::vector<std::vector<int>> segmentSigns{driftSigns(gctx, *resolved.front(), resolved.front()->measurements())};
        std::vector<MeasurementSet> segMeasurements{extractPrds(*resolved.front())};

        for (std::unique_ptr<Segment>& resolveMe : toResolve) {
            ATH_MSG_VERBOSE("Try to resolve new segment "<<toString(localSegmentPars(gctx, *resolveMe))
                          <<" redChi2: "<<redChi2(*resolveMe)<<" nDoF: "<<resolveMe->nDoF());
            /// Fetch first the Prds 
            MeasurementSet testMeas{extractPrds(*resolveMe)};
            Resolution reso{Resolution::noOverlap};
            unsigned int resolvedIdx{0};
            for (std::unique_ptr<Segment>& goodSeg : resolved) {
                ATH_MSG_VERBOSE("Test against segment "<<toString(localSegmentPars(gctx, *goodSeg))
                        <<" redChi2: "<<redChi2(*goodSeg)<<" nDoF: "<<goodSeg->nDoF());
                MeasurementSet& resolvedM = segMeasurements[resolvedIdx];
                std::vector<int>& existSigns{segmentSigns[resolvedIdx++]};
                /// Check whether the two segments share hits at all
                unsigned int shared = countShared(resolvedM, testMeas);
                if (shared < m_cfg.sharedPrecHits) {
                    ATH_MSG_VERBOSE("Too few shared measurements "<<shared<<" (Required: "<<m_cfg.sharedPrecHits<<").");
                    continue;
                }
                /// Re-evaluate the drift signs of the accepted measurement w.r.t. good one
                const std::vector<int> reEvaluatedSigns{driftSigns(gctx, *resolveMe, goodSeg->measurements())};

                unsigned int sameSides{0};
                for (unsigned int s =0 ; s < existSigns.size(); ++s) {
                    sameSides += (reEvaluatedSigns[s] == existSigns[s]);
                }
                /// Left-right solutions differ & the two segments have the same nDOF
                if (!m_cfg.remLeftRightAmbi && sameSides != existSigns.size() && resolveMe->nDoF() == goodSeg->nDoF()) {
                    ATH_MSG_VERBOSE("Reference signs: "<<existSigns<<" / re-evaluated: "<<reEvaluatedSigns);
                    continue;
                }
                const double resolvedChi2  = redChi2(*goodSeg);
                const double resolveMeChi2 = redChi2(*resolveMe);
                reso = resolveMeChi2 < resolvedChi2 ? Resolution::superSet : Resolution::subSet;

                ATH_MSG_VERBOSE("Chi2 good "<<resolvedChi2<<", candidate chi2: "<<resolveMeChi2);
                /// Segments below that threshold are not considered for outlier removal
                /// Take the one which has more degrees of freedom
                if (resolveMeChi2 < m_cfg.selectByNDoFChi2 && resolvedChi2 < m_cfg.selectByNDoFChi2) {
                    reso =  goodSeg->nDoF() > resolveMe->nDoF() ? Resolution::subSet : Resolution::superSet;
                }
                if (reso == Resolution::superSet) {
                    std::swap(goodSeg, resolveMe);
                    std::swap(resolvedM, testMeas);
                    existSigns = driftSigns(gctx, *resolveMe, resolveMe->measurements());
                } else if (reso == Resolution::subSet) {
                    break;
                }
            }
            /// No overlap detected thus far
            if (reso == Resolution::noOverlap) {
                segMeasurements.push_back(std::move(testMeas));
                segmentSigns.push_back(driftSigns(gctx, *resolveMe, resolveMe->measurements()));
                resolved.push_back(std::move(resolveMe));
            }
        }
        return resolved;
    }
    std::vector<int> SegmentAmbiSolver::driftSigns(const ActsGeometryContext& gctx,
                                                   const Segment& segment,
                                                   const Segment::MeasVec& measurements) const {
        const auto [locPos, locDir] = makeLine(localSegmentPars(gctx, segment));
        ATH_MSG_VERBOSE("Fetch drift signs for segment "<<segment.msSector()->identString()<<" -- "<<Amg::toString(locPos)
                        <<Amg::toString(locDir));
        return SegmentFitHelpers::driftSigns(locPos, locDir, measurements,  msg());
    }
    SegmentAmbiSolver::MeasurementSet 
        SegmentAmbiSolver::extractPrds(const Segment& segment) const {
        
        MeasurementSet meas{};
        for (const Segment::MeasVec::value_type& hit : segment.measurements()) {
            if (!hit->spacePoint() || hit->fitState() != CalibratedSpacePoint::State::Valid || !hit->measuresEta()) {
                continue;
            }
            meas.insert(hit->spacePoint()->primaryMeasurement());
            if (hit->spacePoint()->secondaryMeasurement()) {
                 meas.insert(hit->spacePoint()->secondaryMeasurement());
            }
        }
        return meas;
    }
    unsigned int SegmentAmbiSolver::countShared(const MeasurementSet& measSet1, 
                                                const MeasurementSet& measSet2) const {
        if (measSet1.size() > measSet2.size()) {
            return std::count_if(measSet2.begin(),measSet2.end(),[&measSet1](const xAOD::UncalibratedMeasurement* meas){
                return measSet1.count(meas);
            });
        }
        return std::count_if(measSet1.begin(),measSet1.end(),[&measSet2](const xAOD::UncalibratedMeasurement* meas){
                return measSet2.count(meas);
            }); 
    }
}