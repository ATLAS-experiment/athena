/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/MmClusterVariables.h"
#include "StoreGate/ReadHandle.h"
namespace MuonValR4{

    MmClusterVariables::MmClusterVariables(MuonTesterTree& tree,
                                         const std::string& inContainer,
                                         MSG::Level msgLvl,
                                         const std::string& collName):
        TesterModuleBase{tree, inContainer + collName, msgLvl},
        m_key{inContainer},
        m_collName{collName}{
    }
    bool MmClusterVariables::declare_keys() {
        return declare_dependency(m_key);
    }
    bool MmClusterVariables::fill(const EventContext& ctx){
        const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

        SG::ReadHandle inContainer{m_key, ctx};
        if (!inContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        /// First dump the prds parsed externally
        for (const xAOD::MMCluster* strip : m_dumpedPRDS){
            dump(gctx, *strip);
        }
        /// Then parse the rest. If there's any
        for (const xAOD::MMCluster* strip : *inContainer) {
            const MuonGMR4::MmReadoutElement* re = strip->readoutElement();
            const Identifier id{re->measurementId(strip->measurementHash())};
            if ((m_applyFilter && !m_filteredChamb.count(idHelperSvc()->chamberId(id))) ||
                m_idOutIdxMap.find(id) != m_idOutIdxMap.end()){
                ATH_MSG_VERBOSE("Skip "<<idHelperSvc()->toString(id));
                continue;
            }
            dump(gctx, *strip);
        }

        m_filteredChamb.clear();
        m_idOutIdxMap.clear();
        m_dumpedPRDS.clear();
        return true;
    }
    void MmClusterVariables::enableSeededDump() {
        m_applyFilter = true;
    }
    void MmClusterVariables::dumpAllHitsInChamber(const Identifier& chamberId){
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(chamberId));
    }
    unsigned int MmClusterVariables::push_back(const xAOD::MMCluster& strip){
        m_applyFilter = true;
        const MuonGMR4::MmReadoutElement* re = strip.readoutElement();
        const Identifier id{re->measurementId(strip.measurementHash())};
        
        const auto insert_itr = m_idOutIdxMap.insert(std::make_pair(id, m_idOutIdxMap.size()));
        if (insert_itr.second) {
            m_dumpedPRDS.push_back(&strip);
        }
        return insert_itr.first->second; 
    }
    void MmClusterVariables::dump(const ActsTrk::GeometryContext& gctx,
                                 const xAOD::MMCluster& strip) {
        const MuonGMR4::MmReadoutElement* re = strip.readoutElement();
        const Identifier id{re->measurementId(strip.layerHash())};
    

        ATH_MSG_VERBOSE("Filling information for "<<idHelperSvc()->toString(id));

        m_id.push_back(id);
        Amg::Vector3D locPos{Amg::Vector3D::Zero()};
        locPos = strip.localPosition<1>()[0] * Amg::Vector3D::UnitX();

        const Amg::Vector3D globPos{re->localToGlobalTrans(gctx, strip.layerHash()) *locPos};
        m_globPos.push_back(globPos);
        m_locPos.push_back(strip.localPosition<1>()[0]);
        m_locCov.push_back(strip.localCovariance<1>()(0,0));
        m_charge.push_back(strip.charge());
        m_time.push_back(strip.time());
        m_author.push_back(static_cast<char>(strip.author()));
    }

}
