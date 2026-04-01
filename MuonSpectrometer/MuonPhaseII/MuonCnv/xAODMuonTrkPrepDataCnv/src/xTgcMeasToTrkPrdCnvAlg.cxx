/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  

#include "xTgcMeasToTrkPrdCnvAlg.h"

#include "StoreGate/WriteHandle.h"
namespace MuonR4 {
    StatusCode xTgcMeasToTrkPrdCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_detMgrKey.initialize());
        return StatusCode::SUCCESS;
    }

    StatusCode xTgcMeasToTrkPrdCnvAlg::execute(const EventContext& ctx) const {
        const MuonGM::MuonDetectorManager* detMgr{};
        const xAOD::TgcStripContainer* strips{nullptr};
        ATH_CHECK(SG::get(detMgr, m_detMgrKey, ctx));
        ATH_CHECK(SG::get(strips, m_readKey, ctx));
        std::vector<std::unique_ptr<Muon::TgcPrepDataCollection>> prdCollections{};
        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        prdCollections.resize(idHelper.module_hash_max());

        for (const xAOD::TgcStrip* meas : *strips) {
            const Identifier measId = meas->identify();
            const IdentifierHash modHash{m_idHelperSvc->moduleHash(measId)};
            
            std::unique_ptr<Muon::TgcPrepDataCollection>& coll = prdCollections[modHash];
            if (!coll) {
                coll = std::make_unique<Muon::TgcPrepDataCollection>(modHash);
                coll->setIdentifier(m_idHelperSvc->chamberId(measId));
            }
            const MuonGM::TgcReadoutElement* outEle = detMgr->getTgcReadoutElement(measId);
            Amg::Vector2D locPos = meas->localMeasurementPos().block<2,1>(0,0);
            auto outPrd = std::make_unique<Muon::TgcPrepData>(meas->identify(),
                                                              outEle->identifyHash(),
                                                              std::move(locPos),
                                                              std::vector<Identifier>{meas->identify()},
                                                              xAOD::toEigen(meas->localCovariance<1>()),
                                                              outEle,
                                                              meas->bcBitMap());
            outPrd->setHashAndIndex(coll->identifyHash(), coll->size());
            coll->push_back(std::move(outPrd));

        }


        /// Write everything to disk in the end
        auto outContainer = std::make_unique<Muon::TgcPrepDataContainer>(idHelper.module_hash_max());
        for (std::unique_ptr<Muon::TgcPrepDataCollection>& coll : prdCollections){
            if (!coll) continue;
            const IdentifierHash hash = coll->identifyHash();
            ATH_CHECK(outContainer->addCollection(coll.release(), hash));
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(outContainer))); 
        return StatusCode::SUCCESS;
    } 
}
