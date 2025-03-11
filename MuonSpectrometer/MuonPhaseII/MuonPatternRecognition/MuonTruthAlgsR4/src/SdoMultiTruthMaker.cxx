
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "SdoMultiTruthMaker.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "xAODMuonPrepData/UtilFunctions.h"
#include "MuonTruthHelpers/MuonSimHitHelpers.h" 
namespace MuonR4 {
   StatusCode SdoMultiTruthMaker::initialize() {
      
      ATH_CHECK(m_simHitKey.initialize());
      ATH_CHECK(m_writeKey.initialize());
      return StatusCode::SUCCESS;
   }
     
   StatusCode SdoMultiTruthMaker::execute(const EventContext& ctx) const {
       
      SG::WriteHandle prdTruth{m_writeKey, ctx};
      ATH_CHECK(prdTruth.record(std::make_unique<PRD_MultiTruthCollection>()));
        
      SG::ReadHandle readHandle{m_simHitKey, ctx};
      if (!readHandle.isPresent()) {
         ATH_MSG_FATAL("Failed to load container "<<m_simHitKey.fullKey());
         return StatusCode::FAILURE;
      }
      for (const xAOD::MuonSimHit* truthHit : *readHandle) {
         if (!truthHit->genParticleLink().isValid()){
            continue;
         }
         const auto& pl{truthHit->genParticleLink()};
         /// reduced set to the large multimap. But may be not for the typically small RDO/PRD ratio.
         using truthiter =  PRD_MultiTruthCollection::iterator;
         const Identifier prdId = truthHit->identify();
         std::pair<truthiter, truthiter> r = prdTruth->equal_range(prdId);
         if (r.second == std::find_if(r.first, r.second, 
            [pl](const PRD_MultiTruthCollection::value_type& prd_to_truth) {
               return prd_to_truth.second == pl;
         })) {
            prdTruth->insert(std::make_pair(prdId, pl)); 
         }
      }
      return StatusCode::SUCCESS;
   }
}