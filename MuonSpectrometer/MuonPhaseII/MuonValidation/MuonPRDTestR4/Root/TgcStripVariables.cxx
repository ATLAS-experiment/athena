/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/TgcStripVariables.h"
#include "StoreGate/ReadHandle.h"
namespace MuonValR4{

    TgcStripVariables::TgcStripVariables(MuonTesterTree& tree,
                                         const std::string& inContainer,
                                         MSG::Level msgLvl,
                                         const std::string& collName):
        TesterModuleBase{tree, inContainer + collName, msgLvl},
        m_key{inContainer},
        m_collName{collName}{
    }
    bool TgcStripVariables::declare_keys() {
        return declare_dependency(m_key);
    }
    bool TgcStripVariables::fill(const EventContext& ctx){
        const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

        SG::ReadHandle inContainer{m_key, ctx};
        if (!inContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        /// First dump the prds parsed externally
        for (const xAOD::TgcStrip* strip : m_dumpedPRDS){
            dump(gctx, *strip);
        }
        /// Then parse the rest. If there's any
        for (const xAOD::TgcStrip* strip : *inContainer) {
            const MuonGMR4::TgcReadoutElement* re = strip->readoutElement();
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
    void TgcStripVariables::enableSeededDump() {
        m_applyFilter = true;
    }
    void TgcStripVariables::dumpAllHitsInChamber(const Identifier& chamberId){
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(chamberId));
    }
    unsigned int TgcStripVariables::push_back(const xAOD::TgcStrip& strip){
        m_applyFilter = true;
        const MuonGMR4::TgcReadoutElement* re = strip.readoutElement();
        const Identifier id{re->measurementId(strip.measurementHash())};
        
        const auto insert_itr = m_idOutIdxMap.insert(std::make_pair(id, m_idOutIdxMap.size()));
        if (insert_itr.second) {
            m_dumpedPRDS.push_back(&strip);
        }
        return insert_itr.first->second; 
    }
    void TgcStripVariables::dump(const ActsTrk::GeometryContext& gctx,
                                 const xAOD::TgcStrip& strip) {
        const MuonGMR4::TgcReadoutElement* re = strip.readoutElement();
        const Identifier id{strip.identify()};
    

        ATH_MSG_VERBOSE("Filling information for "<<idHelperSvc()->toString(id));

        m_id.push_back(id);
        Amg::Vector3D locPos{Amg::Vector3D::Zero()};
        locPos = strip.localPosition<1>()[0] * Amg::Vector3D::UnitX();

        const Amg::Vector3D globPos{re->localToGlobalTrans(gctx, strip.layerHash()) *locPos};
        m_globPos.push_back(globPos);
        m_locPos.push_back(strip.localPosition<1>()[0]);
        m_locCov.push_back(strip.localCovariance<1>()(0,0));
   
    }

}
