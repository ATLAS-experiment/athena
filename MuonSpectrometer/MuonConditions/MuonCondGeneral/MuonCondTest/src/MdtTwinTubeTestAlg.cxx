/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "MdtTwinTubeTestAlg.h"
#include "StoreGate/ReadCondHandle.h"
namespace Muon{

    StatusCode MdtTwinTubeTestAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode MdtTwinTubeTestAlg::execute(const EventContext& ctx) const {
        SG::ReadCondHandle twinMap{m_readKey, ctx};
        ATH_CHECK(twinMap.isValid());
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        const std::vector<const MuonGMR4::MdtReadoutElement*> readOutEles{m_detMgr->getAllMdtReadoutElements()};
        for (const MuonGMR4::MdtReadoutElement* re : readOutEles) {
            const Identifier detEl{re->identify()};
            if (!twinMap->isTwinTubeLayer(detEl)) {
                ATH_MSG_DEBUG("Multi layer "<<m_idHelperSvc->toStringDetEl(detEl)<<" does not contain a ML");
                continue;
            }
            ATH_MSG_INFO("Test twin tube mapping in "<<m_idHelperSvc->toStringDetEl(detEl));
            for (unsigned int layer = 1; layer<= re->numLayers(); ++layer) {
                for (unsigned tube = 1; tube <= re->numTubesInLay(); ++tube) {
                    const Identifier primId = idHelper.channelID(detEl, idHelper.multilayer(detEl), layer, tube);
                    const Identifier twinId = twinMap->twinId(primId);
                    const Identifier backId = twinMap->twinId(twinId);
                    if (backId != primId) {
                        ATH_MSG_FATAL("Back & forth mapping of "<<m_idHelperSvc->toString(primId)<<" -> "
                            <<m_idHelperSvc->toString(twinId)<<" -> "<<m_idHelperSvc->toString(backId)<<" does not close");
                        return StatusCode::FAILURE;
                    }
                    if (twinId == primId){
                        ATH_MSG_WARNING("Twin tube maps back onto itself "<<m_idHelperSvc->toString(primId));
                        return StatusCode::FAILURE;
                    }
                }
            }
        }        
        return StatusCode::SUCCESS;
    }
}
