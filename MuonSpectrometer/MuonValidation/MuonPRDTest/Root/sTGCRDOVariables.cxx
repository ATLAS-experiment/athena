/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/sTGCRDOVariables.h"

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

using namespace Muon;
namespace MuonPRDTest {
    sTGCRDOVariables::sTGCRDOVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "RDO_sTGC", true, msglvl), m_key{container_name} {}
    bool sTGCRDOVariables::declare_keys() { return declare_dependency(m_key); }

    bool sTGCRDOVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillsTGCRDOVariables()");
        SG::ReadHandle<STGC_RawDataContainer> stgcrdoContainer{m_key, ctx};
        if (!stgcrdoContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve stgc rdo container " << m_key.fullKey());
            return false;
        }
        ATH_MSG_DEBUG("retrieved sTGC rdo Container with size " << stgcrdoContainer->size());

        if (stgcrdoContainer->size() == 0) ATH_MSG_DEBUG(" sTGC rdo Container empty ");
        for (const STGC_RawDataCollection* coll : *stgcrdoContainer) {
            if(m_applyFilter && !m_filteredChamb.count(coll->identifyHash())){
                continue;
            }
            for (const STGC_RawData* rdo : *coll) {
                dump(ctx, *rdo);
            }
        }
        m_NSWsTGC_nRDO = m_NSWsTGC_rdo_charge.size();
        ATH_MSG_DEBUG(" finished fillsTGCRDOVariables()");
        return true;
    }



    unsigned int sTGCRDOVariables::push_back(const EventContext& ctx, const  Muon::STGC_RawData& rdo) {
            m_externalPush = true;
            return dump(ctx, rdo);
    }

    void sTGCRDOVariables::enableSeededDump() { m_applyFilter = true; }

    void sTGCRDOVariables::dumpAllHitsInChamber(const MuonGM::sTgcReadoutElement& detEle) {
        m_applyFilter = true;
        m_filteredChamb.insert(idHelperSvc()->moduleHash(detEle.identify()));
    }


    unsigned int sTGCRDOVariables::push_back(const EventContext& ctx, const Identifier& id) {
        SG::ReadHandle<STGC_RawDataContainer> stgcrdoContainer{m_key, ctx};
        if (!stgcrdoContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve sTGC rdo container " << m_key.fullKey());
            return -1;
        }
        ATH_MSG_DEBUG("retrieved sTGC rdo Container with size " << stgcrdoContainer->size());

        const STGC_RawDataCollection* coll =  stgcrdoContainer->indexFindPtr(idHelperSvc()->moduleHash(id));
        for (const STGC_RawData* rdo : *coll) {
            if (rdo->identify() == id) {
                return push_back(ctx, *rdo);
            }
        }
        ATH_MSG_ERROR("The requested RDO " << idHelperSvc()->toString(id) << " was not found in the container");
        return -1;
    }

    unsigned int sTGCRDOVariables::dump(const EventContext& ctx,const Muon::STGC_RawData& rdo) {
            const Identifier Id = rdo.identify();

            if (m_filteredRDOs.count(Id)) {
                ATH_MSG_VERBOSE("The hit has already been added " << idHelperSvc()->toString(Id));
                return m_filteredRDOs.at(Id);
            }

            const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
            if (!MuonDetMgr) { 
                ATH_MSG_ERROR("MuonDetMgr not found");
                return -1; 
            }

            const MuonGM::sTgcReadoutElement* rdoEl = MuonDetMgr->getsTgcReadoutElement(Id);
            if (!rdoEl) {
               ATH_MSG_ERROR("The sTGC hit "<<idHelperSvc()->toString(Id)<<" does not have a detector element attached. That should actually never happen");
               return -1;
            }
            Amg::Vector2D localStripPos(0.,0.);
            if ( rdoEl->stripPosition(Id,localStripPos) )  {
                ATH_MSG_WARNING("The sTGC hit "<<idHelperSvc()->toString(Id)<<" does not have a valid strip position.");
                return -1;
            }

            m_NSWsTGC_rdo_time.push_back(rdo.time());
            m_NSWsTGC_rdo_tdo.push_back(rdo.tdo());
            m_NSWsTGC_rdo_charge.push_back(rdo.charge());
            m_NSWsTGC_rdo_bcTag.push_back(rdo.bcTag());
            m_NSWsTGC_rdo_isDead.push_back(rdo.isDead());
            m_NSWsTGC_rdo_id.push_back(Id);

            m_NSWsTGC_rdo_localPosX.push_back(localStripPos.x());
            m_NSWsTGC_rdo_localPosY.push_back(localStripPos.y());

            Amg::Vector3D globalStripPos(0., 0., 0.);
            rdoEl->surface(Id).localToGlobal(localStripPos,Amg::Vector3D(0.,0.,0.),globalStripPos);
            m_NSWsTGC_rdo_globalPos.push_back(globalStripPos);
            unsigned idx = m_filteredRDOs.size();
            if (m_externalPush) {
                m_filteredRDOs.insert(std::make_pair(Id, idx));
            }
            return idx;
    }

} // namespace MuonPRDTest