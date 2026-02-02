
/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonIdHelpers/MdtIdHelper.h"
#include <MuonSpacePoint/SpacePointPerLayerSplitter.h>
#include <MuonIdHelpers/IMuonIdHelperSvc.h>
#include <MuonSpacePoint/SpacePointPerLayerSorter.h>

#include "Acts/Utilities/Helpers.hpp"
namespace MuonR4 {
    using HitVec = SpacePointPerLayerSplitter::HitVec;
    SpacePointPerLayerSplitter::SpacePointPerLayerSplitter(const SpacePointBucket& bucket):
        SpacePointPerLayerSplitter(Acts::unpackConstSmartPointers(bucket)){}

    SpacePointPerLayerSplitter::SpacePointPerLayerSplitter(const HitVec& hits) {
        if (hits.empty()) {
            return;
        }
        const Muon::IMuonIdHelperSvc* idHelperSvc {hits.front()->msSector()->idHelperSvc()};
        const MdtIdHelper& idHelper {idHelperSvc->mdtIdHelper()};
        const SpacePointPerLayerSorter laySorter{};

        HitVec::const_iterator itr = hits.begin();

        while (itr != hits.end()) {
            const std::size_t refLay = laySorter.sectorLayerNum(**itr);

            HitVec::const_iterator end_insert  = std::find_if(itr, hits.end(),[&refLay,&laySorter](const SpacePoint* testMe) {
                return refLay != laySorter.sectorLayerNum(*testMe);
            });

            const bool isMdt = (*itr)->type() == xAOD::UncalibMeasType::MdtDriftCircleType;
            HitLayVec& pushMe{isMdt ? m_mdtLayers : m_stripLayers};
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