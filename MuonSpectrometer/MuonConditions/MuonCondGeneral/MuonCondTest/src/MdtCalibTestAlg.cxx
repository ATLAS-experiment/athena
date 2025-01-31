/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtCalibTestAlg.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "StoreGate/ReadCondHandle.h"
namespace Muon{

    StatusCode MdtCalibTestAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode MdtCalibTestAlg::execute(const EventContext& ctx) const {
        SG::ReadCondHandle calibContainer{m_readKey, ctx};
        if (!calibContainer.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_readKey.fullKey());
            return StatusCode::FAILURE;
        }
        std::vector<const MuonGMR4::MdtReadoutElement*> mdtMls = m_detMgr->getAllMdtReadoutElements();
        std::set<MuonCalib::MdtFullCalibData::RtRelationPtr> testedRt{};
        for (const MuonGMR4::MdtReadoutElement* re : mdtMls){
            const MuonCalib::MdtFullCalibData* calibData = calibContainer->getCalibData(re->identify(), msgStream());
            if (!calibData) {
                ATH_MSG_FATAL("No calibration constants were defined for multilayer "<<m_idHelperSvc->toStringDetEl(re->identify()));
                return StatusCode::FAILURE;
            }
            if (false && testedRt.insert(calibData->rtRelation).second) {
                /// Test the rt relation
                double r =0.;
                while (r < re->innerTubeRadius()) {
                
                    const std::optional<double> t = calibData->rtRelation->tr()->driftTime(r);
                    if (!t) {
                        r+=m_stepR;
                        continue; 
                    }
                    const double backR = calibData->rtRelation->rt()->radius(*t);
                    if (std::abs(r - backR) > m_mapTolerance) {
                        ATH_MSG_FATAL("The drift radius "<<r<<" is mapped back to "<<backR<<". Difference: "<<std::abs(r -backR)
                                    <<" "<<m_idHelperSvc->toStringDetEl(re->identify()));
                        return StatusCode::FAILURE;
                    }
                    r+=m_stepR;
                }
            }
        }
        return StatusCode::SUCCESS;
    }
}