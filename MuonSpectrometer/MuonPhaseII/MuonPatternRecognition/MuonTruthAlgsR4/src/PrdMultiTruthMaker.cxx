
/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#include "PrdMultiTruthMaker.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "xAODMuonViews/ChamberViewer.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h" 
namespace MuonR4 {
     StatusCode PrdMultiTruthMaker::initialize() {
        
        ATH_CHECK(m_xAODPrdKeys.initialize());
        for (const xAODPrdKey_t& key : m_xAODPrdKeys) {
            m_simHitDecorKeys.emplace_back(key, m_simLinkDecor);
        }
        ATH_CHECK(m_simHitDecorKeys.initialize());
        ATH_CHECK(m_MdtPrdKey.initialize(!m_MdtPrdKey.empty()));
        ATH_CHECK(m_RpcPrdKey.initialize(!m_RpcPrdKey.empty()));
        ATH_CHECK(m_TgcPrdKey.initialize(!m_TgcPrdKey.empty()));
        ATH_CHECK(m_sTgcPrdKey.initialize(!m_sTgcPrdKey.empty()));
        ATH_CHECK(m_MmPrdKey.initialize(!m_MmPrdKey.empty()));

        ATH_CHECK(m_MdtTruthMapKey.initialize(!m_MdtPrdKey.empty()));
        ATH_CHECK(m_RpcTruthMapKey.initialize(!m_RpcPrdKey.empty()));
        ATH_CHECK(m_TgcTruthMapKey.initialize(!m_TgcPrdKey.empty()));
        ATH_CHECK(m_sTgcTruthMapKey.initialize(!m_sTgcPrdKey.empty()));
        ATH_CHECK(m_MmTruthMapKey.initialize(!m_MmPrdKey.empty()));
        
        return StatusCode::SUCCESS;
     }
     StatusCode PrdMultiTruthMaker::execute(const EventContext& ctx) const {
        using TechIdx_t = Muon::MuonStationIndex::TechnologyIndex;
        using TechMap_t = std::unordered_map<TechIdx_t, std::vector<const xAODPrdCont_t*>>;
        TechMap_t prdContByTech{};
        for (const xAODPrdKey_t& key : m_xAODPrdKeys) {
            SG::ReadHandle readHandle{key, ctx};
            if (!readHandle.isPresent()) {
               ATH_MSG_FATAL("Failed to load container "<<key.fullKey());
               return StatusCode::FAILURE;
            }
            /// Container empty nothing todo
            if (readHandle->empty()){
               continue;
            }
            /// Fetch the first measurement to determine the technology
            const Identifier id = xAOD::identify(readHandle->front());
            prdContByTech[m_idHelperSvc->technologyIndex(id)].push_back(readHandle.cptr());
        }
        ATH_CHECK(buildMultiCollection(ctx, m_MdtPrdKey, m_MdtTruthMapKey, prdContByTech[TechIdx_t::MDT]));
        ATH_CHECK(buildMultiCollection(ctx, m_RpcPrdKey, m_RpcTruthMapKey, prdContByTech[TechIdx_t::RPC]));
        ATH_CHECK(buildMultiCollection(ctx, m_TgcPrdKey, m_TgcTruthMapKey, prdContByTech[TechIdx_t::TGC]));
        ATH_CHECK(buildMultiCollection(ctx, m_sTgcPrdKey, m_sTgcTruthMapKey, prdContByTech[TechIdx_t::STGC]));
        ATH_CHECK(buildMultiCollection(ctx, m_MmPrdKey, m_MmTruthMapKey, prdContByTech[TechIdx_t::MM]));
        return StatusCode::SUCCESS;
      }
      template<typename PrdType>
         StatusCode PrdMultiTruthMaker::buildMultiCollection(const EventContext& ctx,
                                                             const SG::ReadHandleKey<Muon::MuonPrepDataContainerT<PrdType>>& prdKey,
                                                             const SG::WriteHandleKey<PRD_MultiTruthCollection>& writeKey,
                                                             const std::vector<const xAODPrdCont_t*>& measContainers) const {

         if (prdKey.empty()) {
            ATH_MSG_VERBOSE("Do not process "<<typeid(PrdType).name()<<" as the key is empty");
            return StatusCode::SUCCESS;
         }
         SG::ReadHandle prdContainer{prdKey,ctx};
         if (!prdContainer.isPresent()) {
            ATH_MSG_FATAL("Failed to retrieve container "<<prdKey.fullKey()<<".");
            return StatusCode::FAILURE;
         }
         SG::WriteHandle prdTruth{writeKey, ctx};
         ATH_CHECK(prdTruth.record(std::make_unique<PRD_MultiTruthCollection>()));

         for (const Muon::MuonPrepDataCollection<PrdType>* prdColl : *prdContainer){
             for (const xAODPrdCont_t* measCont : measContainers) {
                  xAOD::ChamberViewer contViewer{*measCont};
                  if (!contViewer.loadView(prdColl->identifyHash())){
                      continue;
                  }
                  for (const PrdType* prd : *prdColl) {
                        xAODPrdCont_t::const_iterator meas_itr = 
                              std::ranges::find_if(contViewer,[&prd](const xAOD::UncalibratedMeasurement* meas){ 
                                    return xAOD::identify(meas) == prd->identify();});
                        if (meas_itr == contViewer.end()) {
                           ATH_MSG_WARNING("Strange there should be a measurement associated with "<<
                                       m_idHelperSvc->toString(prd->identify()));
                           continue;
                        }
                        const xAOD::MuonSimHit* truthHit{getTruthMatchedHit(**meas_itr)};
                        if (!truthHit || !truthHit->genParticleLink().isValid()){
                           continue;
                        }
                        const auto& pl{truthHit->genParticleLink()};
                        /// reduced set to the large multimap. But may be not for the typically small RDO/PRD ratio.
                        using truthiter =  PRD_MultiTruthCollection::iterator;
                        
                        std::pair<truthiter, truthiter> r = prdTruth->equal_range(prd->identify());
                        if (r.second == std::find_if(r.first, r.second, 
                                 [pl](const PRD_MultiTruthCollection::value_type& prd_to_truth) {
                                       return prd_to_truth.second == pl;
                                 })) {
                           prdTruth->insert(std::make_pair(prd->identify(), pl)); 
                        }  
                  }
              }
         }
         return StatusCode::SUCCESS;
      }
}