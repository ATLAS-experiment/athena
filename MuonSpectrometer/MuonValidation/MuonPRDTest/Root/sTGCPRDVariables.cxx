/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/sTGCPRDVariables.h"

#include "MuonReadoutGeometry/sTgcReadoutElement.h"

namespace MuonPRDTest {
    sTGCPRDVariables::sTGCPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "PRD_sTGC", msglvl), m_key{container_name} {}
    bool sTGCPRDVariables::declare_keys() { return declare_dependency(m_key); }

    bool sTGCPRDVariables::fill(const EventContext& ctx) {
        m_externalPush = false;
        ATH_MSG_DEBUG("do fillsTGCPRDVariables()");
        const MuonGM::MuonDetectorManager* MuonDetMgr = getDetMgr(ctx);
        if (!MuonDetMgr) { return false; }
        SG::ReadHandle<Muon::sTgcPrepDataContainer> stgcprdContainer{m_key, ctx};
        if (!stgcprdContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve prd container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved sTGC PRD Container with size " << stgcprdContainer->size());

        if (stgcprdContainer->size() == 0) ATH_MSG_DEBUG(" sTGC PRD Container empty ");
        unsigned int n_PRD{0};
        for(const Muon::sTgcPrepDataCollection* coll : *stgcprdContainer ) {
            for (unsigned int item=0; item<coll->size(); item++) {
                const Muon::sTgcPrepData* prd = coll->at(item);
                Identifier Id = prd->identify();

                m_NSWsTGC_PRD_id.push_back(Id);
                m_NSWsTGC_PRD_charge.push_back(prd->charge());

                const MuonGM::sTgcReadoutElement* det = prd->detectorElement();
                if (!det) {
                   ATH_MSG_ERROR("The sTGC hit "<<idHelperSvc()->toString(Id)<<" does not have a detector element attached. That should actually never happen");
                   return false;
                }

                Amg::Vector3D pos = prd->globalPosition();
                Amg::Vector2D loc_pos(0., 0.);
                det->surface(Id).globalToLocal(pos, Amg::Vector3D(0., 0., 0.), loc_pos);

                double err_x = prd->localCovariance()(0,0);
                double err_y = ( prd->localCovariance().rows()==2)? prd->localCovariance()(1,1) : 0.;

                ATH_MSG_DEBUG( "sTgc PRD local pos.:  x=" << std::setw(6) << std::setprecision(2) << loc_pos[0]
                                               << ", ex=" << std::setw(6) << std::setprecision(2) << err_x
                                               << ",  y=" << std::setw(6) << std::setprecision(2) << loc_pos[1] 
                                               << ", ey=" << std::setw(6) << std::setprecision(2) << err_y );

                m_NSWsTGC_PRD_globalPos.push_back(pos);

                m_NSWsTGC_PRD_localPosX.push_back(loc_pos[0]);
                m_NSWsTGC_PRD_localPosY.push_back(loc_pos[1]);
                m_NSWsTGC_PRD_covMatrix_1_1.push_back(err_x);
                m_NSWsTGC_PRD_covMatrix_2_2.push_back(err_y);

            if(m_applyFilter && !m_filteredChamb.count(coll->identify())){
                ATH_MSG_VERBOSE("Do not dump measurements from " << idHelperSvc()->toStringChamber(coll->identify()));
                continue;
            }
            for(const Muon::sTgcPrepData* prd: *coll){
                dump(*prd);
                ++n_PRD;
            }
        }
        }
        m_NSWsTGC_nPRD = n_PRD;
        ATH_MSG_DEBUG("finished fillsTGCPRDVariables()");
        m_filteredPRDs.clear();
        m_filteredChamb.clear();
        return true;
    } 

    unsigned int sTGCPRDVariables::push_back(const Muon::sTgcPrepData& prd){
        m_externalPush=true;
        return dump(prd);
    }

    void sTGCPRDVariables::enableSeededDump(){
        m_applyFilter = true;
    }

    void sTGCPRDVariables::dumpAllHitsInChamber(const MuonGM::sTgcReadoutElement& detEle){
        m_applyFilter=true;
        m_filteredChamb.insert(idHelperSvc()->chamberId(detEle.identify()));
    }

    unsigned int sTGCPRDVariables::dump(const Muon::sTgcPrepData& prd){
        const Identifier Id = prd.identify();


        if (m_filteredPRDs.count(Id)) {
            ATH_MSG_VERBOSE("The hit has already been added "<<idHelperSvc()->toString(Id));
            return m_filteredPRDs.at(Id);
        }

        m_NSWsTGC_PRD_id.push_back(Id);
        m_NSWsTGC_PRD_charge.push_back(prd.charge());

        const MuonGM::sTgcReadoutElement* det = prd.detectorElement();

        Amg::Vector3D pos = prd.globalPosition();
        Amg::Vector2D loc_pos{Amg::Vector2D::Zero()};
        det->surface(Id).globalToLocal(pos, Amg::Vector3D::Zero(), loc_pos);

        double err_x = prd.localCovariance()(0,0);
        double err_y = prd.localCovariance().rows()==2 ? prd.localCovariance()(1,1) : 0.;

        ATH_MSG_DEBUG( "sTgc PRD local pos.:  x=" << std::setw(6) << std::setprecision(2) << loc_pos[0]
                                       << ", ex=" << std::setw(6) << std::setprecision(2) << err_x
                                       << ",  y=" << std::setw(6) << std::setprecision(2) << loc_pos[1] 
                                       << ", ey=" << std::setw(6) << std::setprecision(2) << err_y );

        m_NSWsTGC_PRD_globalPos.push_back(pos);

        m_NSWsTGC_PRD_covMatrix_1_1.push_back(err_x);
        m_NSWsTGC_PRD_covMatrix_2_2.push_back(err_y);

        
        m_NSWsTGC_PRD_charge.push_back(prd.charge());
        
        // determine the time of the strip with the highest charge
        short int prdTime = prd.time();
        if(idHelperSvc()->stgcIdHelper().channelType(Id) == sTgcIdHelper::sTgcChannelTypes::Strip) {
            int maxCharge{-SHRT_MAX};
            std::vector<int> stripCharges = prd.stripCharges();
            for (int i = 0; i < static_cast<int>(stripCharges.size()); ++i) {
                if (stripCharges.at(i) > maxCharge) {
                    maxCharge = stripCharges.at(i);
                    prdTime = prd.stripTimes().at(i);
                }
            }
        }
        m_NSWsTGC_PRD_time.push_back(prdTime);              
        m_NSWsTGC_PRD_nStrips.push_back(prd.rdoList().size());

        m_NSWsTGC_PRD_stripCharge.push_back(prd.stripCharges());
        m_NSWsTGC_PRD_stripTime.push_back(prd.stripTimes());
        m_NSWsTGC_PRD_stripChannel.push_back(prd.stripNumbers());

        m_stgcAuthor.push_back(static_cast<uint8_t>(prd.author()));
        m_stgcQuality.push_back(static_cast<uint8_t>(prd.quality()));
        
        
        unsigned idx = m_filteredPRDs.size();
        if (m_externalPush) {
            m_filteredPRDs.insert(std::make_pair(Id, idx));
        }

        return idx;
    }
}