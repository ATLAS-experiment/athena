/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/MdtDriftCircleVariables.h"
#include "StoreGate/ReadHandle.h"
namespace MuonValR4{

    MdtDriftCircleVariables::MdtDriftCircleVariables(MuonTesterTree& tree,
                                                     const std::string& inContainer,
                                                     MSG::Level msgLvl,
                                                     const std::string& collName):
        TesterModuleBase{tree, inContainer + collName, msgLvl},
        m_key{inContainer},
        m_collName{collName}{
    }
    bool MdtDriftCircleVariables::declare_keys() {
        return declare_dependency(m_key);
    }
    bool MdtDriftCircleVariables::fill(const EventContext& ctx){
        const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

        SG::ReadHandle inContainer{m_key, ctx};
        if (!inContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        /// First dump the prds parsed externally
        for (const xAOD::MdtDriftCircle* dc : m_dumpedPRDS){
            dump(gctx, *dc);
        }
        /// Then parse the rest. If there's any
        for (const xAOD::MdtDriftCircle* dc : *inContainer) {
            const MuonGMR4::MdtReadoutElement* re = dc->readoutElement();
            const Identifier id{re->measurementId(dc->measurementHash())};
            if ((m_applyFilter && !m_filteredChamb.count(idHelperSvc()->chamberId(id))) ||
                m_idOutIdxMap.find(id) != m_idOutIdxMap.end()){
                ATH_MSG_VERBOSE("Skip "<<idHelperSvc()->toString(id));
                continue;
            }
            dump(gctx, *dc);
        }

        m_filteredChamb.clear();
        m_idOutIdxMap.clear();
        m_dumpedPRDS.clear();
        return true;
    }
    void MdtDriftCircleVariables::enableSeededDump() {
        m_applyFilter = true;
    }
    void MdtDriftCircleVariables::dumpAllHitsInChamber(const Identifier& chamberId){
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(chamberId));
    }
    unsigned int MdtDriftCircleVariables::push_back(const xAOD::MdtDriftCircle& dc){
        m_applyFilter = true;
        const MuonGMR4::MdtReadoutElement* re = dc.readoutElement();
        const Identifier id{re->measurementId(dc.measurementHash())};
        
        const auto insert_itr = m_idOutIdxMap.insert(std::make_pair(id, m_idOutIdxMap.size()));
        if (insert_itr.second) {
            m_dumpedPRDS.push_back(&dc);
        }
        return insert_itr.first->second; 
    }
    void MdtDriftCircleVariables::dump(const ActsTrk::GeometryContext& gctx,
                                    const xAOD::MdtDriftCircle& dc) {
        const MuonGMR4::MdtReadoutElement* re = dc.readoutElement();
        const Identifier id{re->measurementId(dc.measurementHash())};
    

        ATH_MSG_VERBOSE("Filling information for "<<idHelperSvc()->toString(id));
        

        const Amg::Vector3D tubePos{re->center(gctx, dc.measurementHash())};
        m_id.push_back(id);
        m_globPos.push_back(tubePos);
        m_driftRadius.push_back(dc.driftRadius());
        m_driftRadiusUncert.push_back(dc.driftRadiusUncert());
        m_tdcCounts.push_back(dc.tdc());
        m_adcCounts.push_back(dc.adc());
    }

}
