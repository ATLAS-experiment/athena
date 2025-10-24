/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PRDTESTERR4_TRACKCONTAINERMODULE_H
#define PRDTESTERR4_TRACKCONTAINERMODULE_H
#include "MuonPRDTestR4/TesterModuleBase.h"
#include "ActsEvent/TrackContainer.h"
namespace MuonValR4{

    class TrackContainerModule: public TesterModuleBase {
        public:
            TrackContainerModule(MuonTesterTree& tree,
                                const std::string& inContainer,
                                MSG::Level msgLvl = MSG::Level::INFO,
                                const std::string& collName = "ActsMsTracks");

            bool declare_keys() override final;

             bool fill(const EventContext& ctx) override final;
        private:
            SG::ReadHandleKey<ActsTrk::TrackContainer> m_key{};
            std::string m_collName{"ActsMsTracks"};

            MuonVal::VectorBranch<float>& m_trackPt{parent().newVector<float>(m_collName + "_pt")};
            MuonVal::VectorBranch<float>& m_trackEta{parent().newVector<float>(m_collName + "_eta")};
            MuonVal::VectorBranch<float>& m_trackPhi{parent().newVector<float>(m_collName + "_phi")};
            MuonVal::VectorBranch<int>& m_trackQ{parent().newVector<int>(m_collName + "_q")};
            MuonVal::VectorBranch<float>& m_trackChi2{parent().newVector<float>(m_collName + "_chi2")};
            MuonVal::VectorBranch<unsigned>& m_trackNdoF{parent().newVector<unsigned>(m_collName + "_nDoF")};
            
    };
}

#endif