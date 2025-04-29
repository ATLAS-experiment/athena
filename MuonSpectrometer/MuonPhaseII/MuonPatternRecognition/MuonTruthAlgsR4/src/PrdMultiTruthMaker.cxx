
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "PrdMultiTruthMaker.h"

#include "StoreGate/WriteHandle.h"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h" 
namespace MuonR4 {
   StatusCode PrdMultiTruthMaker::initialize() {
      
      ATH_CHECK(m_xAODPrdKeys.initialize());
      for (const xAODPrdKey_t& key : m_xAODPrdKeys) {
         m_simHitDecorKeys.emplace_back(key, m_simLinkDecor);
      }
      if (m_xAODPrdKeys.empty()) {
         ATH_MSG_FATAL("No input containers were defined "<<m_xAODPrdKeys);
         return StatusCode::FAILURE;
      }
      ATH_CHECK(m_simHitDecorKeys.initialize());
      ATH_CHECK(m_writeKey.initialize());
      return StatusCode::SUCCESS;
   }
     
   StatusCode PrdMultiTruthMaker::execute(const EventContext& ctx) const {
       
      SG::WriteHandle prdTruth{m_writeKey, ctx};
      ATH_CHECK(prdTruth.record(std::make_unique<PRD_MultiTruthCollection>()));
        
      for (const xAODPrdKey_t& key : m_xAODPrdKeys) {
         const xAODPrdCont_t* unCalibMeasCont{nullptr};
         ATH_CHECK(SG::get(unCalibMeasCont, key, ctx));
         for (const xAOD::UncalibratedMeasurement* meas : *unCalibMeasCont) {
            const xAOD::MuonSimHit* truthHit{getTruthMatchedHit(*meas)};
            if (!truthHit || !truthHit->genParticleLink().isValid()){
               continue;
            }
            const auto& pl{truthHit->genParticleLink()};
            /// reduced set to the large multimap. But may be not for the typically small RDO/PRD ratio.
            using truthiter =  PRD_MultiTruthCollection::iterator;
            const Identifier prdId = xAOD::identify(meas);
            std::pair<truthiter, truthiter> r = prdTruth->equal_range(prdId);
            if (r.second == std::find_if(r.first, r.second, 
                  [pl](const PRD_MultiTruthCollection::value_type& prd_to_truth) {
                        return prd_to_truth.second == pl;
                  })) {
               prdTruth->insert(std::make_pair(prdId, pl)); 
            }
         }
      }
      return StatusCode::SUCCESS;
   }
}