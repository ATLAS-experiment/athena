/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <MuonTruthHelpers/MuonSimHitHelpers.h>
#include <xAODMuonSimHit/MuonSimHitContainer.h>
#include <MuonSpacePoint/SpacePoint.h>
#include <MuonSpacePoint/CalibratedSpacePoint.h>
#include <MuonPatternEvent/Segment.h>
#include <AthLinks/ElementLink.h>

namespace MuonR4 {
    const xAOD::MuonSimHit* getTruthMatchedHit(const xAOD::UncalibratedMeasurement& prdHit) {
        static const SG::ConstAccessor<ElementLink<xAOD::MuonSimHitContainer>> acc("simHitLink");
        if (acc.isAvailable(prdHit)){
            const ElementLink<xAOD::MuonSimHitContainer>& link{acc(prdHit)};
            if (link.isValid()) {
                return (*link);
            }
        }
        return nullptr;
    }

    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const xAOD::MuonSegment& segment) {
        using SimHitLinkVec = std::vector<ElementLink<xAOD::MuonSimHitContainer>>; 
        static const SG::ConstAccessor<SimHitLinkVec> acc{"simHitLinks"};
        std::unordered_set<const xAOD::MuonSimHit*> hits{};
        if (!acc.isAvailable(segment)){
            return hits;
        }
        hits.reserve(acc(segment).size());
        for (const ElementLink<xAOD::MuonSimHitContainer>& link : acc(segment)) {
            if (link.isValid()) {
                hits.insert(*link);
            }
        }
        return hits;
    }

    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const std::vector<const SpacePoint*>& spacePoints) {
        std::unordered_set<const xAOD::MuonSimHit*> hits{};
        for (const SpacePoint* sp : spacePoints) {
            const xAOD::MuonSimHit* primHit{getTruthMatchedHit(*sp->primaryMeasurement())};
            const xAOD::MuonSimHit* secHit{sp->dimension() == 2 ? getTruthMatchedHit(*sp->secondaryMeasurement()) : nullptr};
            if(primHit){
                hits.insert(primHit);
            }
            if (secHit && secHit != primHit) {
                hits.insert(secHit);
            }
        }
        return hits;
    }

    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const std::vector<const CalibratedSpacePoint*>& measurements) {
        std::unordered_set<const xAOD::MuonSimHit*> hits{};
        for (const CalibratedSpacePoint* meas : measurements) {
            const SpacePoint* sp = meas->spacePoint();
            if (!sp) continue;
            const xAOD::MuonSimHit* primHit{getTruthMatchedHit(*sp->primaryMeasurement())};
            const xAOD::MuonSimHit* secHit{sp->dimension() == 2 ? getTruthMatchedHit(*sp->secondaryMeasurement()) : nullptr};
            if(primHit){
                hits.insert(primHit);
            }
            if (secHit && secHit != primHit) {
                hits.insert(secHit);
            }
        }
        return hits;
    }
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const Segment& segment) {
        std::vector<const CalibratedSpacePoint*> calibSps{};
        calibSps.reserve(segment.measurements().size());
        std::ranges::transform(segment.measurements(), std::back_inserter(calibSps), [](const auto& meas){return meas.get();});
        return getMatchingSimHits(calibSps);
    }
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const SpacePointBucket& bucket) {
        std::vector<const SpacePoint*> spacePoints{};
        spacePoints.reserve(bucket.size());
        std::ranges::transform(bucket, std::back_inserter(spacePoints), [](const SpacePointBucket::value_type& sp){return sp.get();});
        return getMatchingSimHits(spacePoints);
    }
    std::unordered_set<const xAOD::MuonSimHit*> getMatchingSimHits(const SegmentSeed& seed) {
        return getMatchingSimHits(seed.getHitsInMax());
    }
}


