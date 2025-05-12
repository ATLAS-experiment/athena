/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/MMRDOVariables.h"

#include "MuonReadoutGeometry/MMReadoutElement.h"

using namespace Muon;
namespace MuonPRDTest {
    MMRDOVariables::MMRDOVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "RDO_MM", true, msglvl), m_rdokey{container_name} {}
    bool MMRDOVariables::declare_keys() { return declare_dependency(m_rdokey); }

    bool MMRDOVariables::fill(const EventContext& ctx) {
        m_externalPush = false;
        ATH_MSG_DEBUG("do fillMMRDOVariables()");
        SG::ReadHandle<MM_RawDataContainer> mmrdoContainer{m_rdokey, ctx};
        if (!mmrdoContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve MM rdo container " << m_rdokey.fullKey());
            return false;
        }
        ATH_MSG_DEBUG("retrieved MM rdo Container with size " << mmrdoContainer->size());

        if (mmrdoContainer->size() == 0) ATH_MSG_DEBUG(" MM rdo Container empty ");
        for (const MM_RawDataCollection* coll : *mmrdoContainer) {
            if(m_applyFilter && !m_filteredChamb.count(coll->identifierHash())){
                continue;
            }


            for (const MM_RawData* rdo : *coll) {
                dump(ctx, *rdo);
            }
        }
        m_NSWMM_nRDO = m_NSWMM_rdo_charge.size();
        ATH_MSG_DEBUG(" finished fillMMRDOVariables()");
        return true;
    }
    
    unsigned int MMRDOVariables::push_back(const EventContext& ctx, const  Muon::MM_RawData& rdo) {
            m_externalPush = true;
            return dump(ctx, rdo);
    }
    
    void MMRDOVariables::enableSeededDump() { m_applyFilter = true; }
    
    void MMRDOVariables::dumpAllHitsInChamber(const MuonGM::MMReadoutElement& detEle) {
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->moduleHash(detEle.identify()));
    }
    
    
    unsigned int MMRDOVariables::push_back(const EventContext& ctx, const Identifier& id) {
        SG::ReadHandle<MM_RawDataContainer> mmrdoContainer{m_rdokey, ctx};
        if (!mmrdoContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve MM rdo container " << m_rdokey.fullKey());
            return -1;
        }
        ATH_MSG_DEBUG("retrieved MM rdo Container with size " << mmrdoContainer->size());
        const MM_RawDataCollection* coll =  mmrdoContainer->indexFindPtr(idHelperSvc()->moduleHash(id));
        for (const MM_RawData* rdo : *coll) {
            if (rdo->identify()== id) {
                return push_back(ctx, *rdo);
            }
        }
        ATH_MSG_ERROR("The requested RDO " << idHelperSvc()->toString(id) << " was not found in the container");
        return -1;
    }
    
    
    unsigned int MMRDOVariables::dump(const EventContext& ctx,const Muon::MM_RawData& rdo) {
            const Identifier Id = rdo.identify();
    
            if (m_filteredRDOs.count(Id)) {
                ATH_MSG_VERBOSE("The hit has already been added " << idHelperSvc()->toString(Id));
                return m_filteredRDOs.at(Id);
            }
        
            const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
            if (!MuonDetMgr) { 
                ATH_MSG_ERROR("MuonDetMgr not found");
                return  -1; 
            }
        
            const MuonGM::MMReadoutElement* rdoEl = MuonDetMgr->getMMReadoutElement(Id);
            if (!rdoEl) {
               ATH_MSG_ERROR("The micromega hit "<<idHelperSvc()->toString(Id)<<" does not have a detector element attached. That should actually never happen");
               return -1;
            }
            Amg::Vector2D localStripPos{Amg::Vector2D::Zero()};
            if ( !rdoEl->stripPosition(Id,localStripPos) )  {
                ATH_MSG_WARNING("The MM hit "<<idHelperSvc()->toString(Id)<<" does not have a valid strip position.");
                return -1; 
                
            }
            
            m_NSWMM_rdo_time.push_back(rdo.time());
            m_NSWMM_rdo_relBcid.push_back(rdo.relBcid());
            m_NSWMM_rdo_charge.push_back(rdo.charge());
            
        
            m_NSWMM_rdo_id.push_back(Id);
            m_NSWMM_rdo_localPosX.push_back(localStripPos.x());
            m_NSWMM_rdo_localPosY.push_back(localStripPos.y());
        
            Amg::Vector3D globalStripPos(0., 0., 0.);
            rdoEl->surface(Id).localToGlobal(localStripPos,Amg::Vector3D(0.,0.,0.),globalStripPos);
            m_NSWMM_rdo_globalPos.push_back(globalStripPos);
        
            unsigned idx = m_filteredRDOs.size();
            if (m_externalPush) {
                m_filteredRDOs.insert(std::make_pair(Id, idx));
            }
            return idx;
    }
}    