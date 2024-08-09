/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include <MuonPatternHelpers/SegmentAmbiSolver.h>

namespace MuonR4 {
    using MeasByLayerMap = SegmentAmbiSolver::MeasByLayerMap;
    using SegmentVec = SegmentAmbiSolver::SegmentVec;

    SegmentAmbiSolver::SegmentAmbiSolver(const std::string& name):
        AthMessaging{name} {}

    
    SegmentVec SegmentAmbiSolver::resolveAmbiguity(const ActsGeometryContext& gctx,
                                                    SegmentVec&& toResolve) const {
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
                if (signRef.size() - diffSites <= 1) {
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
            // Skip the auxillary beamspot constraint
            const SpacePoint* sp = meas->spacePoint();
            if (!sp) {
                continue;
            }
            const Identifier layerId = idHelperSvc->layerId(sp->identify());
            auto insert_itr = prds.insert(std::make_pair(layerId, sp));
            if (!insert_itr.second) {
                ATH_MSG_WARNING("Layer "<<idHelperSvc->toString(layerId)
                             <<" has already meaasurement "<<idHelperSvc->toString(insert_itr.first->second->identify())
                             <<". Cannot add "<<idHelperSvc->toString(sp->identify())<<" for ambiguity resolution.");
            }
        }
        return prds;
    }

    std::vector<int> SegmentAmbiSolver::driftSigns(const ActsGeometryContext& gctx,
                                                   const Segment& segment,
                                                   const std::vector<const SpacePoint*>& measurements) const {
        std::vector<int> signs{};
        signs.reserve(measurements.size());
        
        const Amg::Transform3D globToLoc{segment.chamber()->globalToLocalTrans(gctx)};
        /** Direction of the segment */
        const Amg::Vector3D segPos{globToLoc*segment.position()};
        const Amg::Vector3D segDir{globToLoc.linear()* segment.direction()};

        const Muon::IMuonIdHelperSvc* idHelperSvc{segment.chamber()->idHelperSvc()};

        for (const SpacePoint* sp : measurements) {
            if (sp->type() != xAOD::UncalibMeasType::MdtDriftCircleType) {
                continue;
            }
            const Amg::Vector3D deltaPos{segPos - sp->positionInChamber()};
            const double signedDist = deltaPos.y()*segDir.z() - segDir.y() * deltaPos.z(); 
            ATH_MSG_VERBOSE("Hit "<<idHelperSvc->toString(sp->identify())<<" drift radius "<<sp->driftRadius()
                           <<", signed distance: "<<signedDist<<", unsigned distance: "<<
                           Amg::lineDistance<3>(segPos, segDir, sp->positionInChamber(), sp->directionInChamber()));

            signs.push_back(signedDist > 0 ? 1 : -1);
        }
        return signs;
    }
}