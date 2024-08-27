/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/SegmentAmbiSolver.h>
#include <MuonPatternHelpers/SegmentFitHelperFunctions.h>

namespace MuonR4 {
    using MeasByLayerMap = SegmentAmbiSolver::MeasByLayerMap;
    using SegmentVec = SegmentAmbiSolver::SegmentVec;

    SegmentAmbiSolver::SegmentAmbiSolver(const std::string& name):
        AthMessaging{name} {}

    
    SegmentVec SegmentAmbiSolver::resolveAmbiguity(const ActsGeometryContext& gctx,
                                                   SegmentVec&& toResolve) const {
        
        std::ranges::stable_sort(toResolve,[](const std::unique_ptr<Segment>& a,
                                              const std::unique_ptr<Segment>&b){
            const double redChi2A = a->chi2() / a->nDoF();
            const double redChi2B = b->chi2() / b->nDoF();
            if (redChi2A < 5. && redChi2B < 5.){
                return a->nDoF() > b->nDoF();
            }
            return redChi2A < redChi2B;
        });
        SegmentVec resolved{};
        /// Mark the first object as resolved
        resolved.push_back(std::move(toResolve[0]));
        std::vector<MeasByLayerMap> resolvedPrds{extractPrds(*resolved[0])};
        toResolve.erase(toResolve.begin());
        for (std::unique_ptr<Segment>& resolveMe : toResolve) {
            MeasByLayerMap testPrds{extractPrds(*resolveMe)};

            Resolution reso{Resolution::noOverlap};
            
            unsigned int prdPointer{0};
            for (std::unique_ptr<Segment>& reference : resolved) {
                MeasByLayerMap& refPrds{resolvedPrds[prdPointer++]};
                std::vector<const SpacePoint*> overlaps{};
                overlaps.reserve(testPrds.size());
                for (auto& [layerId, spacePoint] : testPrds) {
                    MeasByLayerMap::const_iterator ref_itr = refPrds.find(layerId);
                    if (ref_itr == refPrds.end() || ref_itr->second != spacePoint){
                        continue;
                    }
                    overlaps.push_back(spacePoint);
                }
                if (overlaps.empty()) {
                    continue;
                }
                std::vector<int> signRef{driftSigns(gctx, *reference, overlaps)}, 
                                 signTest{driftSigns(gctx, *resolveMe, overlaps)};
                /** Count in how many cases the signs are differing */
                unsigned int diffSites{0};
                for (unsigned sIdx = 0; sIdx < signRef.size(); ++sIdx) {
                    diffSites += signRef[sIdx] != signTest[sIdx];
                }
                ATH_MSG_VERBOSE("Signs reference: "<<signRef<<", signs test: "<<signTest);
                if (signRef.size() - diffSites <= 1 && testPrds.size() == refPrds.size()) {
                    ATH_MSG_VERBOSE("Both segments are describing different solutions.");
                    continue;
                }
                if (overlaps.size() == testPrds.size()) {
                    ATH_MSG_VERBOSE("The test segment is a subset of the reference.");
                    reso = Resolution::subSet;
                    break;
                } else if (overlaps.size() == refPrds.size()) {
                    ATH_MSG_VERBOSE("The test segment is a superset of the reference.");
                    reso = Resolution::superSet;
                    refPrds = std::move(testPrds);
                    reference = std::move(resolveMe);
                    break;
                }
            }
            if(reso == Resolution::noOverlap) {
                resolved.push_back(std::move(resolveMe));
                resolvedPrds.push_back(std::move(testPrds));
            }
           
        }
        return resolved;
    }
    MeasByLayerMap SegmentAmbiSolver::extractPrds(const Segment& segment) const{
        MeasByLayerMap prds{};
        const Muon::IMuonIdHelperSvc* idHelperSvc = segment.chamber()->idHelperSvc();
        for (const Segment::MeasType& meas : segment.measurements()) {
            if(meas->fitState() != CalibratedSpacePoint::State::Valid ||
               !meas->spacePoint()) {
                continue;
            }
            const Identifier layerId = idHelperSvc->layerId(meas->spacePoint()->identify());
            auto insert_itr = prds.insert(std::make_pair(layerId, meas->spacePoint()));
            if (!insert_itr.second) {
                ATH_MSG_WARNING("Layer "<<idHelperSvc->toString(layerId)
                             <<" has already meaasurement "<<idHelperSvc->toString(insert_itr.first->second->identify())
                             <<". Cannot add "<<idHelperSvc->toString(meas->spacePoint()->identify())<<" for ambiguity resolution.");
            }
        }
        return prds;
    }

    std::vector<int> SegmentAmbiSolver::driftSigns(const ActsGeometryContext& gctx,
                                                   const Segment& segment,
                                                   const std::vector<const SpacePoint*>& measurements) const {
        
        const Amg::Transform3D globToLoc{segment.chamber()->globalToLocalTrans(gctx)};
        return SegmentFitHelpers::driftSigns(globToLoc*segment.position(), 
                                             globToLoc.linear() * segment.direction(), measurements, msg());
    }
}