/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/RpcMeasurementVariables.h"
#include "StoreGate/ReadHandle.h"
namespace MuonValR4{

    RpcMeasurementVariables::RpcMeasurementVariables(MuonTesterTree& tree,
                                                     const std::string& inContainer,
                                                     MSG::Level msgLvl,
                                                     const std::string& collName):
        TesterModuleBase{tree, inContainer + collName, msgLvl},
        m_key{inContainer},
        m_collName{collName}{
    }
    bool RpcMeasurementVariables::declare_keys() {
        return declare_dependency(m_key);
    }
    bool RpcMeasurementVariables::fill(const EventContext& ctx){
        const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

        SG::ReadHandle inContainer{m_key, ctx};
        if (!inContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        /// First dump the prds parsed externally
        for (const xAOD::RpcMeasurement* strip : m_dumpedPRDS){
            dump(gctx, *strip);
        }
        /// Then parse the rest. If there's any
        for (const xAOD::RpcMeasurement* strip : *inContainer) {
            const MuonGMR4::RpcReadoutElement* re = strip->readoutElement();
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
    void RpcMeasurementVariables::enableSeededDump() {
        m_applyFilter = true;
    }
    void RpcMeasurementVariables::dumpAllHitsInChamber(const Identifier& chamberId){
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(chamberId));
    }
    unsigned int RpcMeasurementVariables::push_back(const xAOD::RpcMeasurement& strip){
        m_applyFilter = true;
        const MuonGMR4::RpcReadoutElement* re = strip.readoutElement();
        const Identifier id{re->measurementId(strip.measurementHash())};
        
        const auto insert_itr = m_idOutIdxMap.insert(std::make_pair(id, m_idOutIdxMap.size()));
        if (insert_itr.second) {
            m_dumpedPRDS.push_back(&strip);
        }
        return insert_itr.first->second; 
    }
    void RpcMeasurementVariables::dump(const ActsTrk::GeometryContext& gctx,
                                       const xAOD::RpcMeasurement& strip) {
        const MuonGMR4::RpcReadoutElement* re = strip.readoutElement();
        const Identifier id{re->measurementId(strip.measurementHash())};
    

        ATH_MSG_VERBOSE("Filling information for "<<idHelperSvc()->toString(id));
        

        m_id.push_back(id);
        Amg::Vector3D locPos{strip.localMeasurementPos()};
        AmgSymMatrix(2) locCov{AmgSymMatrix(2)::Identity()};
        if (strip.numDimensions() == 1) {
            locCov(0,0) = strip.localCovariance<1>()(0,0);
        } else {
            locCov = xAOD::toEigen(strip.localCovariance<2>());
        }
        const Amg::Vector3D globPos{re->localToGlobalTrans(gctx, strip.layerHash()) *locPos};
        m_globPos.push_back(globPos);
        m_locPos.push_back(locPos.block<2,1>(0,0));
        m_locCov.push_back(locCov(0,0), locCov(1,1));
        m_time.push_back(strip.time());
        m_toT.push_back(strip.timeOverThreshold());
   
    }

}
