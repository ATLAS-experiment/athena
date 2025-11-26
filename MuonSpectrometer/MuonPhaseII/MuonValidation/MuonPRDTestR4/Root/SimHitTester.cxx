/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonPRDTestR4/SimHitTester.h"
#include "StoreGate/ReadHandle.h"
namespace MuonValR4{
    SimHitTester::SimHitTester(MuonTesterTree& tree,
                               const std::string& inContainer,
                               const ActsTrk::DetectorType detType,
                               MSG::Level msgLvl):
        TesterModuleBase{tree, inContainer, msgLvl},
        m_key{inContainer},
        m_collName{ActsTrk::to_string(detType)+"SimHits"} {
      switch (detType) {
         case ActsTrk::DetectorType::Mdt:
            m_identifier = std::make_shared<MdtIdentifierBranch>(tree, m_collName);
            break;
        case ActsTrk::DetectorType::Rpc:
            m_identifier = std::make_shared<RpcIdentifierBranch>(tree, m_collName);
            break;
        case ActsTrk::DetectorType::Tgc:
            m_identifier = std::make_shared<TgcIdentifierBranch>(tree, m_collName);
            break;
        case ActsTrk::DetectorType::sTgc:
            m_identifier = std::make_shared<sTgcIdentifierBranch>(tree, m_collName);
            break;
        case ActsTrk::DetectorType::Mm:
            m_identifier = std::make_shared<MmIdentifierBranch>(tree, m_collName);
            break;
        default:
            ATH_MSG_WARNING("Unsupported detector type "<<ActsTrk::to_string(detType));
      };

    }
    bool SimHitTester::declare_keys() {
        return declare_dependency(m_key) && parent().addBranch(m_identifier);
    }
    bool SimHitTester::fill(const EventContext& ctx){

        SG::ReadHandle inContainer{m_key, ctx};
        if (!inContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve "<<m_key.fullKey());
            return false;
        }
        
        for (const xAOD::MuonSimHit* simHit : *inContainer) {
            fillHit(ctx, *simHit);
        }  
        m_idxLookUp.clear();      
        return true;
    }
    unsigned SimHitTester::push_back(const xAOD::MuonSimHit& hit) {
        return fillHit(Gaudi::Hive::currentContext(), hit);
    } 
    unsigned int SimHitTester::fillHit(const EventContext& ctx,
                                       const xAOD::MuonSimHit& simHit) {

        const auto insertItr = m_idxLookUp.insert(std::make_pair(&simHit, m_idxLookUp.size()));
        if (!insertItr.second) {
            return insertItr.first->second;
        }
        const ActsTrk::GeometryContext& gctx{getGeoCtx(ctx)};

        const MuonGMR4::MuonDetectorManager* detMgr = getDetMgr();

        const Identifier id = simHit.identify();
        ATH_MSG_VERBOSE("Filling information for "<<idHelperSvc()->toString(id));
        
        const MuonGMR4::MuonReadoutElement* re = detMgr->getReadoutElement(id);

        const IdentifierHash trfHash{idHelperSvc()->isMdt(id) ? re->measurementHash(id)
                                                              : re->layerHash(id)};

        const Amg::Vector3D locPos{xAOD::toEigen(simHit.localPosition())};
        const Amg::Vector3D locDir{xAOD::toEigen(simHit.localDirection())};

        const Amg::Transform3D& locToGlobal{re->localToGlobalTrans(gctx, trfHash)};
        m_globPos.push_back(locToGlobal*locPos);
        m_globDir.push_back(Amg::Vector3D(locToGlobal.linear()* locDir));
        m_locPos.push_back(locPos);
        m_locDir.push_back(locDir);
        m_globTime.push_back(simHit.globalTime());
        m_beta.push_back(simHit.beta());
        m_pdgId.push_back(simHit.pdgId());
        m_energyDep.push_back(simHit.energyDeposit());
        m_kinE.push_back(simHit.kineticEnergy());
        m_mass.push_back(simHit.mass());
        m_identifier->push_back(id);

        return insertItr.first->second;
    }


}
