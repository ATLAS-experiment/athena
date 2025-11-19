/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPRDTestR4/TrackContainerModule.h"
#include "MuonPatternHelpers/MatrixUtils.h"

#include "StoreGate/ReadHandle.h"
#include "ActsInterop/UnitConverters.h"

using namespace Acts;
using namespace MuonR4;
namespace MuonValR4{
    TrackContainerModule::TrackContainerModule(MuonTesterTree& tree,
                                               const std::string& inContainer,
                                               MSG::Level msgLvl,
                                               const std::string& collName):
            TesterModuleBase{tree, inContainer, msgLvl},
        m_key{inContainer},
        m_collName{collName} {}
        
    bool TrackContainerModule::declare_keys() {
        return declare_dependency(m_key);
    }
    bool TrackContainerModule::fill(const EventContext& ctx){
        const ActsTrk::TrackContainer* tracks{nullptr};
        if (!SG::get(tracks, m_key, ctx)) {
            return false;
        }
        for (const auto track : *tracks) {
            const Amg::Vector3D trkP4 = ActsTrk::convertMomFromActs(track.fourMomentum()).first;
            const double chi2 = track.chi2();
            const int q = sign(track.qOverP());
            const unsigned nDoF = track.nDoF();

            m_trackPt += trkP4.perp();
            m_trackEta += trkP4.eta();
            m_trackPhi += trkP4.phi();
            m_trackQ += q;
            m_trackNdoF += nDoF;
            m_trackChi2 += chi2;
            m_parentSeed += track.component<std::size_t, Acts::hashString("parentSeed")>();
        }
        return true;
    }
}
