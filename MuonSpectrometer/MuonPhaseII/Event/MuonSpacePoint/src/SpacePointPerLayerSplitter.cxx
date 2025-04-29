
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonIdHelpers/MdtIdHelper.h"
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>

namespace MuonR4 {
    using HitVec = SpacePointPerLayerSplitter::HitVec;
    inline HitVec stripSmartPtr(const SpacePointBucket& bucket) {
        HitVec hits{};
        hits.reserve(bucket.size());
        std::transform(bucket.begin(),bucket.end(),std::back_inserter(hits), 
                      [](const SpacePointBucket::value_type& hit){return hit.get();});
        return hits;
    }
    SpacePointPerLayerSplitter::SpacePointPerLayerSplitter(const SpacePointBucket& bucket):
        SpacePointPerLayerSplitter(stripSmartPtr(bucket)){}

    SpacePointPerLayerSplitter::SpacePointPerLayerSplitter(const HitVec& hits) {
        if (hits.empty()) return;

        const Muon::IMuonIdHelperSvc* idHelperSvc {hits.front()->msSector()->idHelperSvc()};
        const MdtIdHelper& idHelper {idHelperSvc->mdtIdHelper()};
        SpacePointPerLayerSorter laySorter{idHelperSvc};

        HitVec::const_iterator itr = hits.begin();

        while (itr != hits.end()) {
            const Identifier refId = laySorter.detectorLayerId((*itr)->identify());

            HitVec::const_iterator end_insert  = std::find_if(itr, hits.end(),[&refId,&laySorter](const SpacePoint* testMe) {
                 return refId != laySorter.detectorLayerId(testMe->identify());
            });

            const bool isMdt = (*itr)->type() == xAOD::UncalibMeasType::MdtDriftCircleType;
            HitLayVec& pushMe{ isMdt ? m_mdtLayers : m_stripLayers};
            if (isMdt && !pushMe.empty()){
                if (idHelper.multilayer((*itr)->identify()) != idHelper.multilayer(pushMe.back().front()->identify())) {
                    m_tubeLaySwitch = pushMe.size();
                }
            } 

            (isMdt ? m_nMdtHits : m_nStripHits) += pushMe.emplace_back(itr, end_insert).size();
            itr = end_insert;
        }
    }
}