/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonInferenceInterfaces/LayerBucket.h"

#include "MuonSpacePoint/SpacePointPerLayerSorter.h"
#include "Acts/Utilities/Enumerate.hpp"
namespace MuonML{
    using HitVec = MuonR4::SpacePointPerLayerSorter::HitVec;
    LayerSpBucket::LayerSpBucket(const MuonR4::SpacePointBucket& bucket):
        m_min{bucket.coveredMin()},
        m_max{bucket.coveredMax()} {
        reserve(bucket.size());
        m_layNum.resize(bucket.size());
        std::vector<uint8_t>::iterator layItr{m_layNum.begin()};
        const MuonR4::SpacePointPerLayerSorter sorter{bucket};
        uint8_t globLayer{0};
        m_nMdtLay = sorter.mdtHits().size();
        m_nStripLay = sorter.stripHits().size();
        
        for (const HitVec& spInLay : sorter.mdtHits()) {
            for (const MuonR4::SpacePoint* spacePoint: spInLay) {
                push_back(spacePoint);
                (*layItr++) = globLayer;
            }
            ++globLayer;
        }
        for (const HitVec& spInLay : sorter.stripHits()) {
            for (const MuonR4::SpacePoint* spacePoint: spInLay) {
                push_back(spacePoint);
                (*layItr++) = globLayer;
            }
            ++globLayer;
        }
    }
}