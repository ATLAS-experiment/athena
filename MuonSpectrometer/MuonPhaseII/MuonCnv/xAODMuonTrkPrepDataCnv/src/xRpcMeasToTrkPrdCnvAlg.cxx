/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/  

#include "xRpcMeasToTrkPrdCnvAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "MuonReadoutGeometryR4/RpcReadoutElement.h"
#include "MuonReadoutGeometry/RpcReadoutElement.h"

namespace MuonR4{
    StatusCode xRpcMeasToRpcTrkPrdCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_detMgrKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode xRpcMeasToRpcTrkPrdCnvAlg::execute(const EventContext& ctx) const {
        const xAOD::RpcMeasurementContainer* measContainer{nullptr};
        ATH_CHECK(SG::get(measContainer, m_readKey, ctx));

        const MuonGM::MuonDetectorManager* detMgr{nullptr};
        ATH_CHECK(SG::get(detMgr, m_detMgrKey, ctx));

        std::vector<std::unique_ptr<Muon::RpcPrepDataCollection>> prdCollections{};
        const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
        prdCollections.resize(idHelper.module_hash_max());
        for (const xAOD::RpcMeasurement* meas : *measContainer) {
            const Identifier measId = meas->identify();
            const IdentifierHash modHash{m_idHelperSvc->moduleHash(measId)};
            
            std::unique_ptr<Muon::RpcPrepDataCollection>& coll = prdCollections[modHash];
            if (!coll) {
                coll = std::make_unique<Muon::RpcPrepDataCollection>(modHash);
                coll->setIdentifier(m_idHelperSvc->chamberId(measId));
            }
            const MuonGM::RpcReadoutElement* outEle = detMgr->getRpcReadoutElement(measId);

            
            std::unique_ptr<Muon::RpcPrepData> prd{};
            std::vector<Identifier> rdo{measId};

            if (meas->numDimensions() == 1) {
                prd =  std::make_unique<Muon::RpcPrepData>(measId, coll->identifyHash(),
                                                           meas->localPosition<1>().x() * Amg::Vector2D::UnitX(),
                                                           std::move(rdo),
                                                           AmgSymMatrix(1){meas->localCovariance<1>()(0,0)}, 
                                                           outEle, meas->time(), meas->timeOverThreshold(), 0, 0);
            } else{
                prd =  std::make_unique<Muon::RpcPrepData>(measId, coll->identifyHash(),
                                                           xAOD::toEigen(meas->localPosition<2>()),
                                                           std::move(rdo),
                                                           xAOD::toEigen(meas->localCovariance<2>()), 
                                                           outEle, meas->time(), meas->timeOverThreshold(), 0, 0);
            
            }
            prd->setHashAndIndex(coll->identifyHash(), coll->size());
            coll->push_back(std::move(prd));
        }
        /// Write everything to disk in the end
        auto outContainer = std::make_unique<Muon::RpcPrepDataContainer>(idHelper.module_hash_max());
        for (std::unique_ptr<Muon::RpcPrepDataCollection>& coll : prdCollections){
            if (!coll) continue;
            const IdentifierHash hash = coll->identifyHash();
            ATH_CHECK(outContainer->addCollection(coll.release(), hash));
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(outContainer))); 
        
        return StatusCode::SUCCESS;
    }
}
