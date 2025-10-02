/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTest/MDTPRDVariables.h"

#include "AthenaBaseComps/AthMsgStreamMacros.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
namespace MuonPRDTest {
    MDTPRDVariables::MDTPRDVariables(MuonTesterTree& tree, const std::string& container_name, MSG::Level msglvl) :
        PrdTesterModule(tree, "PRD_MDT", msglvl), m_key{container_name} {}
    bool MDTPRDVariables::declare_keys() { return declare_dependency(m_key); }

    bool MDTPRDVariables::fill(const EventContext& ctx) {
        ATH_MSG_DEBUG("do fillMDTPRDVariables()");

        SG::ReadHandle<Muon::MdtPrepDataContainer> mdtprdContainer{m_key, ctx};
        if (!mdtprdContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve prd container " << m_key.fullKey());
            return false;
        }

        ATH_MSG_DEBUG("retrieved MDT PRD Container with size " << mdtprdContainer->size());

        unsigned int n_PRD{0};
        for(const Muon::MdtPrepDataCollection* coll : *mdtprdContainer ) {
            for (const Muon::MdtPrepData* prd: *coll) {

                m_MDT_PRD_id.push_back(prd->identify());
                m_MDT_PRD_globalPos.push_back(prd->globalPosition());
                m_MDT_PRD_radius.push_back(prd->localPosition().x());                
                m_MDT_PRD_error.push_back(Amg::error(prd->localCovariance(), Trk::locX));
                m_MDT_PRD_adc.push_back(prd->adc());
                m_MDT_PRD_tdc.push_back(prd->tdc());
                m_MDT_PRD_status.push_back(prd->status());

                ++n_PRD;
            }
        }
        m_MDT_nPRD = n_PRD;
        ATH_MSG_DEBUG(" finished fillMDTPRDVariables()");
        return true;
    }
}