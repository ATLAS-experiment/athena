/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonTruthHelpers/MuonSimHitHelpers.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "xAODMuonSimHit/MuonSimHitContainer.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "MuonSpacePoint/SpacePoint.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonPatternEvent/Segment.h"
#include "AthLinks/ElementLink.h"
#include "xAODMuonPrepData/CombinedMuonStrip.h"

#include <ranges>

namespace {
    using SegLink_t = ElementLink<xAOD::MuonSegmentContainer>;
    using SimHitLink_t = ElementLink<xAOD::MuonSimHitContainer>;
    using PrdLink_t = ElementLink<xAOD::UncalibratedMeasurementContainer>;
 
    using PrdLinkVec_t = std::vector<PrdLink_t>;
    using SimHitLinkVec_t = std::vector<SimHitLink_t>;
    using SegLinkVec_t = std::vector<SegLink_t>;

    inline 
    void appendHit(const xAOD::MuonMeasurement* prdHit,
                   std::vector<const xAOD::MuonSimHit*>& appendTo) {
        if (!prdHit) {
            return;
        }
        const xAOD::MuonSimHit* hit = MuonR4::getTruthMatchedHit(*prdHit);
        if (hit && !std::ranges::contains(appendTo, hit)) {
            appendTo.push_back(hit);
        }
    }

    template <typename T>
    std::vector<typename T::element_type*> stripSmartPointer(const std::vector<T>& vec){
        std::vector<typename T::element_type*> ret{};
        ret.reserve(vec.size());
        std::ranges::transform(vec, std::back_inserter(ret), [](const T& obj){
            return obj.get();
        });
        return ret;
    }
}

namespace MuonR4 {
    const xAOD::MuonSimHit* getTruthMatchedHit(const xAOD::MuonMeasurement& prdHit) {
        static const SG::ConstAccessor<SimHitLink_t> acc("simHitLink");
        if (acc.isAvailable(prdHit)) {
            const SimHitLink_t& link{acc(prdHit)};
            if (link.isValid()) {
                return (*link);
            }
        }
        return nullptr;
    }

    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(const xAOD::MuonSegment& segment) {

        static const SG::ConstAccessor<SimHitLinkVec_t> acc_simLink{"simHitLinks"};
        static const SG::ConstAccessor<PrdLinkVec_t> acc_prdLink{"prdLinks"};
        std::vector<const xAOD::MuonSimHit*> hits{};
        if (acc_simLink.isAvailable(segment)){
            hits.reserve(acc_simLink(segment).size());
            for (const SimHitLink_t& link : acc_simLink(segment)) {
                if (link.isValid()) {
                    hits.emplace_back(*link);
                }
            }
        } else if (acc_prdLink.isAvailable(segment)){
            for (const PrdLink_t& link : acc_prdLink(segment)) {
                if(!link.isValid()){
                    continue;
                }
                const auto* prdHit = dynamic_cast<const xAOD::MuonMeasurement*>(*link);
                if (!prdHit) {
                    continue;
                }
                /// Combined muon strips don't have a truth link
                if (prdHit->numDimensions() == 0){
                    const auto* cmb = static_cast<const xAOD::CombinedMuonStrip*>(prdHit);
                    appendHit(cmb->primaryStrip(), hits);
                    appendHit(cmb->secondaryStrip(), hits);
                    continue;
                }
                appendHit(prdHit, hits);
            }
        }
        return hits;
    }

    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(std::span<const SpacePoint* const> spacePoints) {
        std::vector<const xAOD::MuonSimHit*> hits{};
        for (const SpacePoint* sp : spacePoints) {
            appendHit(sp->primaryMeasurement(), hits);
            appendHit(sp->secondaryMeasurement(), hits);
        }
        return hits;
    }

    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(std::span<const CalibratedSpacePoint* const> measurements) {
        std::vector<const xAOD::MuonSimHit*> hits{};
        for (const CalibratedSpacePoint* meas : measurements) {
            const SpacePoint* sp = meas->spacePoint();
            if (!sp) continue;
            hits.insert_range(hits.end(), getMatchingSimHits(std::array{sp}));
        }
        return hits;
    }
    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(const Segment& segment) {
        const auto measurements = stripSmartPointer(segment.measurements());
        return getMatchingSimHits(measurements);
    }
    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(const SpacePointBucket& bucket) {
        return getMatchingSimHits(stripSmartPointer(bucket));
    }
    std::vector<const xAOD::MuonSimHit*> getMatchingSimHits(const SegmentSeed& seed) {
        return getMatchingSimHits(seed.getHitsInMax());
    }
    const xAOD::TruthParticle* getTruthMatchedParticle(const xAOD::MuonSegment& segment){
        using TruthLink_t = ElementLink<xAOD::TruthParticleContainer>;
        static const SG::ConstAccessor<TruthLink_t> acc{"truthParticleLink"};
        if (acc.isAvailable(segment)){
            const TruthLink_t& link {acc(segment)};
            if (link.isValid()) {
                return *link;
            }
        }
        const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(segment);
        return truthSeg != nullptr ? getTruthMatchedParticle(*truthSeg) : nullptr;
    }
    const xAOD::MuonSegment* getMatchedTruthSegment(const xAOD::MuonSegment& segment) {
        static const SG::ConstAccessor<SegLink_t> acc{"truthSegmentLink"};
        if (acc.isAvailable(segment)) {
            const SegLink_t& link{acc(segment)};
            if (link.isValid()){
                return *link;
            }
        }
        return nullptr;
    }

    const xAOD::MuonSegment* getMatchedTruthSegment(const xAOD::MuonSimHit& hit) {
        static const SG::ConstAccessor<SegLink_t> acc{"truthSegmentLink"};
        if (acc.isAvailable(hit)) {
            const SegLink_t& link{acc(hit)};
            if (link.isValid()){
                return *link;
            }
        }
        return nullptr;
    }
    const xAOD::TruthParticle* getTruthMatchedParticle(const xAOD::MuonSimHit& hit) {
        const xAOD::MuonSegment* truthSeg = getMatchedTruthSegment(hit);
        return truthSeg ? getTruthMatchedParticle(*truthSeg) : nullptr;
    }

    std::vector<const xAOD::MuonSegment*> getTruthSegments(const xAOD::TruthParticle& truthMuon) {
        static const SG::ConstAccessor<SegLinkVec_t> acc{"truthSegmentLinks"};
        std::vector<const xAOD::MuonSegment*> segments{};

        if (!acc.isAvailable(truthMuon)) {
            return segments;
        }
        for (const SegLink_t& link : acc(truthMuon)) {
            segments.emplace_back(*link);
        }
        return segments;
    }
}


