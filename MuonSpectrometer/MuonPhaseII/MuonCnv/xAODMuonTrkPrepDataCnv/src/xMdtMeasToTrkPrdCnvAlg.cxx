/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/  

#include "xMdtMeasToTrkPrdCnvAlg.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "MuonReadoutGeometryR4/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "xAODMuonPrepData/MdtDriftCircleContainer.h"
#include "xAODMuonPrepData/MdtTwinDriftCircleContainer.h"

namespace MuonR4{
    StatusCode xMdtMeasToTrkPrdCnvAlg::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_detMgrKey.initialize());
        return StatusCode::SUCCESS;
    }
    StatusCode xMdtMeasToTrkPrdCnvAlg::execute(const EventContext& ctx) const {
        // getting both xAOD containers
        const xAOD::MdtDriftCircleContainer* mdtContainer{nullptr};
        ATH_CHECK(SG::get(mdtContainer, m_readKey, ctx));

        const MuonGM::MuonDetectorManager* detMgr{nullptr};
        ATH_CHECK(SG::get(detMgr, m_detMgrKey, ctx));

        std::vector<std::unique_ptr<Muon::MdtPrepDataCollection>> prdCollections{};
        const MdtIdHelper& idHelper{m_idHelperSvc->mdtIdHelper()};
        prdCollections.resize(idHelper.module_hash_max());
        // single drift circles 
        for (const xAOD::MdtDriftCircle* meas : *mdtContainer) {
            const Identifier measId = meas->identify();
            const IdentifierHash modHash{m_idHelperSvc->moduleHash(measId)};
            
            std::unique_ptr<Muon::MdtPrepDataCollection>& coll = prdCollections[modHash];
            if (!coll) {
                coll = std::make_unique<Muon::MdtPrepDataCollection>(modHash);
                coll->setIdentifier(m_idHelperSvc->chamberId(measId));
            }
            const MuonGM::MdtReadoutElement* outEle = detMgr->getMdtReadoutElement(measId);
            
            std::unique_ptr<Muon::MdtPrepData> prd{};
            std::vector<Identifier> rdo{measId};

            if (meas->numDimensions() == 1) {
                // covariance matrix manipulation
                Amg::MatrixX cov(1, 1);
                (cov)(0,0) = meas->driftRadiusCov();
                prd =  std::make_unique<Muon::MdtPrepData>(measId, meas->driftRadius() * Amg::Vector2D::UnitX(), 
                                                           cov, outEle, meas->tdc(), meas->adc(), meas->status());
    
            } else {
                 // drift measurement and covariance matrix manipulation
                const auto* twin = static_cast<const xAOD::MdtTwinDriftCircle*>(meas);
                Amg::Vector2D hitPos{twin->driftRadius(), twin->posAlongWire()};
                AmgSymMatrix(2) cov{AmgSymMatrix(2)::Identity()};
                (cov)(0,0) = twin->driftRadiusCov();
                (cov)(1,1) = twin->posAlongWireCov();

                prd =  std::make_unique<Muon::MdtTwinPrepData>(measId, std::move(hitPos), std::move(cov), 
                                                               outEle, twin->tdc(), twin->adc(),
                                                               twin->twinTdc(), twin->twinAdc(), twin->status());
            }
            prd->setHashAndIndex(coll->identifyHash(), coll->size());
            coll->push_back(std::move(prd));
        }
      
        /// Write everything to disk in the end
        auto outContainer = std::make_unique<Muon::MdtPrepDataContainer>(idHelper.module_hash_max());
        for (std::unique_ptr<Muon::MdtPrepDataCollection>& coll : prdCollections){
            if (!coll) continue;
            const IdentifierHash hash = coll->identifyHash();
            ATH_CHECK(outContainer->addCollection(coll.release(), hash));
        }
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::move(outContainer))); 
        
        return StatusCode::SUCCESS;
    }
}
