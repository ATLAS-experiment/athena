/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonFastRecoEvent/GlobalPattern.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"

namespace {
    double inDegrees(double angle) {
        return angle / Gaudi::Units::deg;
    }
}
namespace MuonR4 {

GlobalPattern::GlobalPattern(HitCollection&& hitPerStation,
                             std::vector<const SpacePointBucket*>&& bucketPerStation)
    : m_hitsInStation(std::move(hitPerStation)),
      m_parentBuckets(std::move(bucketPerStation)) {};

std::vector<GlobalPattern::StIndex> GlobalPattern::getStations() const {
    std::vector<StIndex> out{};
    out.reserve(m_hitsInStation.size());
    std::ranges::transform(m_hitsInStation, std::back_inserter(out), 
        [](const auto& pair){ return pair.first;});
    return out;
}

const std::vector<GlobalPattern::HitType>& GlobalPattern::hitsInStation(StIndex station) const {
    const auto& it = m_hitsInStation.find(station);
    if (it != m_hitsInStation.end()) {
        return it->second;
    }
    static const std::vector<HitType> empty{};
    return empty;
}

const std::vector<const SpacePointBucket*> GlobalPattern::bucketsInStation(StIndex station) const {
    std::vector<const SpacePointBucket*> buckets{};
    std::ranges::copy_if(m_parentBuckets, std::back_inserter(buckets), 
        [station](const SpacePointBucket* bucket) {
            return Muon::MuonStationIndex::toStationIndex(bucket->msSector()->chamberIndex()) == station; });
    return buckets;
}

double GlobalPattern::sectorPhi() const { 
    static const Muon::MuonSectorMapping sectorMap{};
    return sectorMap.sectorOverlapPhi(sector(), secondarySector()); 
}

void GlobalPattern::print(std::ostream& ostr) const {
    ostr<<"SpacePoint Pattern, Sector: "<< sector() << "  & " <<  (isSectorOverlap() ? std::to_string(secondarySector()) : "-")
        <<", theta: "<<inDegrees(theta()) << ", Phi: "<<inDegrees(phi())<< " Sector Phi: "<<inDegrees(sectorPhi())
        <<", nPrecisionLayers: "<<nPrecisionLayers()<<", nTriggerLayers: "<<nTriggerLayers()<<", nPhiLayers: "<<nPhiLayers()
        <<", mean normalized residual squared: "<<meanNormResidual2();    
    ostr<<", Hit per station: \n";
    for (const auto& [station, hits] : m_hitsInStation) {
        ostr<<"  Station "<<stName(station)<<": "<<hits.size()<<" hits\n";
        for (const auto& hit : hits) {
            ostr<<"    "<<*hit<<"\n";
        }
    }
}

}