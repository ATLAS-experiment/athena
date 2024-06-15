/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/MMPRDVariables.h"
#include "MuonPRDTest/MMRDOVariables.h"

#include "MuonReadoutGeometry/MMReadoutElement.h"

namespace MuonPRDTest {
    MMPRDVariables::MMPRDVariables(MuonTesterTree& tree, const std::string& prd_container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "PRD_MM", true, msglvl), m_key{prd_container_name} {}

    bool MMPRDVariables::declare_keys() { return declare_dependency(m_key); }

    bool MMPRDVariables::fill(const EventContext& ctx) {
        m_externalPush = false;
        ATH_MSG_DEBUG("do fillMMPRDVariables()");
        SG::ReadHandle<Muon::MMPrepDataContainer> mmprdContainer{m_key, ctx};
        if (!mmprdContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve prd container " << m_key.fullKey());
            return false;
        }
        ATH_MSG_DEBUG("retrieved MM PRD Container with size " << mmprdContainer->size());


        unsigned int n_PRD{0};
        for(const Muon::MMPrepDataCollection* coll : *mmprdContainer) {
            if (m_applyFilter && !m_filteredChamb.count(coll->identify())) {
                ATH_MSG_VERBOSE("Do not dump measurements from "<<idHelperSvc()->toStringChamber(coll->identify()));
                continue;
            }
            for (const Muon::MMPrepData* prd : *coll) {
                dump(*prd);
                ++n_PRD;
            }
        }
        m_NSWMM_nPRD = n_PRD;
        ATH_MSG_DEBUG(" finished fillMMPRDVariables()");
        m_filteredPRDs.clear();
        m_filteredChamb.clear();
        return true;
    }
    unsigned int MMPRDVariables::push_back(const Muon::MMPrepData& prd){
        m_externalPush = true;
        return dump(prd);
    }
    void MMPRDVariables::enableSeededDump() {
        m_applyFilter = true;
    }
    void MMPRDVariables::dumpAllHitsInChamber(const MuonGM::MMReadoutElement& detEle){
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(detEle.identify()));
    }

    unsigned int MMPRDVariables::dump(const Muon::MMPrepData& prd) {
        const Identifier Id = prd.identify();

        if (m_filteredPRDs.count(Id)) {
            ATH_MSG_VERBOSE("The hit has already been added "<<idHelperSvc()->toString(Id));
            return m_filteredPRDs.at(Id);
        }
        m_NSWMM_PRD_time.push_back(prd.time());

        const MuonGM::MMReadoutElement* det = prd.detectorElement();

        m_NSWMM_PRD_id.push_back(Id);
        Amg::Vector3D pos = prd.globalPosition();
        const Amg::MatrixX & cov = prd.localCovariance();
        Amg::Vector2D loc_pos{Amg::Vector2D::Zero()};
        det->surface(Id).globalToLocal(pos, Amg::Vector3D::Zero(), loc_pos);

        ATH_MSG_DEBUG("MicroMegas PRD local pos.:  "<<Amg::toString(loc_pos)
                                                    << ", ex=" << std::setw(6) << std::setprecision(2) << cov(0,0));

        m_NSWMM_PRD_globalPos.push_back(pos);
        m_NSWMM_PRD_localPos.push_back(loc_pos);
        m_NSWMM_PRD_covMatrix_1_1.push_back(cov(0,0));
        m_NSWMM_PRD_nStrips.push_back((prd.rdoList()).size());

        m_NSWMM_PRD_stripNumbers.push_back(prd.stripNumbers());
        m_NSWMM_PRD_stripTimes.push_back(prd.stripTimes());
        m_NSWMM_PRD_stripCharges.push_back(prd.stripCharges());

        m_NSWMM_PRD_author.push_back(static_cast<short>(prd.author()));
        m_NSWMM_PRD_quality.push_back(static_cast<int8_t>(prd.quality()));

        unsigned idx = m_filteredPRDs.size();
        if (m_externalPush) {
            m_filteredPRDs.insert(std::make_pair(Id, idx));
        }
        return idx;
    }
}
