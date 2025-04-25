/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// Algorithm producing truth info for PrepRawData, keeping all MC particles contributed to a PRD.

#include "MuonTruthAlgs/MuonPRD_MultiTruthMaker.h"

#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

//================================================================
StatusCode MuonPRD_MultiTruthMaker::initialize() {
    ATH_MSG_DEBUG("MuonPRD_MultiTruthMaker::initialize()");
    ATH_CHECK(m_idHelperSvc.retrieve());
    ATH_CHECK(m_MdtPrdKey.initialize(!m_MdtPrdKey.empty()));
    ATH_CHECK(m_CscPrdKey.initialize(!m_CscPrdKey.empty()));
    ATH_CHECK(m_RpcPrdKey.initialize(!m_RpcPrdKey.empty()));
    ATH_CHECK(m_TgcPrdKey.initialize(!m_TgcPrdKey.empty()));
    ATH_CHECK(m_sTgcPrdKey.initialize(!m_sTgcPrdKey.empty()));
    ATH_CHECK(m_MmPrdKey.initialize(!m_MmPrdKey.empty()));

    ATH_CHECK(m_MdtSDOKey.initialize(!m_MdtPrdKey.empty()));
    ATH_CHECK(m_CscSDOKey.initialize(!m_CscPrdKey.empty()));
    ATH_CHECK(m_RpcSDOKey.initialize(!m_RpcPrdKey.empty()));
    ATH_CHECK(m_TgcSDOKey.initialize(!m_TgcPrdKey.empty()));
    ATH_CHECK(m_sTgcSDOKey.initialize(!m_sTgcPrdKey.empty()));
    ATH_CHECK(m_MmSDOKey.initialize(!m_MmPrdKey.empty()));

    ATH_CHECK(m_MdtTruthMapKey.initialize(!m_MdtPrdKey.empty()));
    ATH_CHECK(m_CscTruthMapKey.initialize(!m_CscPrdKey.empty()));
    ATH_CHECK(m_RpcTruthMapKey.initialize(!m_RpcPrdKey.empty()));
    ATH_CHECK(m_TgcTruthMapKey.initialize(!m_TgcPrdKey.empty()));
    ATH_CHECK(m_sTgcTruthMapKey.initialize(!m_sTgcPrdKey.empty()));
    ATH_CHECK(m_MmTruthMapKey.initialize(!m_MmPrdKey.empty()));
    return StatusCode::SUCCESS;
}

//================================================================
StatusCode MuonPRD_MultiTruthMaker::execute(const EventContext& ctx) const {
    ATH_MSG_DEBUG("MuonPRD_MultiTruthMaker::execute()");
    ATH_CHECK(buildPRD_Truth(ctx, m_MdtPrdKey, m_MdtSDOKey, m_MdtTruthMapKey));
    ATH_CHECK(buildPRD_Truth(ctx, m_CscPrdKey, m_CscSDOKey, m_CscTruthMapKey));
    ATH_CHECK(buildPRD_Truth(ctx, m_RpcPrdKey, m_RpcSDOKey, m_RpcTruthMapKey));
    ATH_CHECK(buildPRD_Truth(ctx, m_TgcPrdKey, m_TgcSDOKey, m_TgcTruthMapKey));
    ATH_CHECK(buildPRD_Truth(ctx, m_sTgcPrdKey, m_sTgcSDOKey, m_sTgcTruthMapKey));
    ATH_CHECK(buildPRD_Truth(ctx, m_MmPrdKey, m_MmSDOKey, m_MmTruthMapKey));
    return StatusCode::SUCCESS;
}

//================================================================
template <class PrdType, class SimCollection>
StatusCode MuonPRD_MultiTruthMaker::buildPRD_Truth(const EventContext& ctx,
                                                   const SG::ReadHandleKey<Muon::MuonPrepDataContainerT<PrdType>>& prdKey, 
                                                   const SG::ReadHandleKey<SimCollection>& sdoKey,
                                                   const SG::WriteHandleKey<PRD_MultiTruthCollection>& outputKey) const{
    
    if (prdKey.empty()) {
        ATH_MSG_DEBUG("No key has been defined for "<<typeid(PrdType).name()<<". Bail out silently");
        return StatusCode::SUCCESS;
    }
    SG::ReadHandle prdContainer{prdKey, ctx};
    if (!prdContainer.isPresent()) {
        ATH_MSG_ERROR("Could not read " << prdKey.key());
        return StatusCode::FAILURE;
    }

    SG::ReadHandle simDataMap{sdoKey, ctx};
    if (!simDataMap.isPresent()) {
        ATH_MSG_ERROR("SimDataCollection for key=" << sdoKey.key() << " not in storegate.");
        return StatusCode::FAILURE;
    }
    // Create and fill the PRD truth structure
    ATH_MSG_DEBUG("make PRD truth for " << outputKey.key());
    SG::WriteHandle prdTruth{outputKey, ctx};
    ATH_CHECK(prdTruth.record(std::make_unique<PRD_MultiTruthCollection>()));

    for (const Muon::MuonPrepDataCollection<PrdType>* coll : *prdContainer) {
        for (const PrdType* prd : *coll) {
            ATH_MSG_VERBOSE("addPrepRawDatum(): new PRD " << prd << ", id=" << prd->identify() << ", number of RDOs: " << prd->rdoList().size());
            bool gotSDO{false}, gotValidParticle{false};

            for (const auto& nextRDO : prd->rdoList()) {
                typename SimCollection::const_iterator iter(simDataMap->find(nextRDO));
                if (iter == simDataMap->end()) {
                    continue;
                }
                gotSDO = true;
                // Got an SDO.  Try to associate the PRD to MC particles we have info about.
                typedef typename SimCollection::mapped_type SIMDATA;
                const SIMDATA& sdo = iter->second;
                const std::vector<typename SIMDATA::Deposit>& deposits = sdo.getdeposits();
                if (deposits.empty()) { continue; }
                for (const auto& [particleLink, mcData] : deposits) {
                    ATH_MSG_VERBOSE("addPrepRawDatum(): particleLink.isValid() " << particleLink.isValid());
                    ATH_MSG_VERBOSE("addPrepRawDatum(): Barcode " << particleLink.barcode() << " evt " << particleLink.eventIndex());
                    if (!particleLink.isValid()) {
                        continue;
                    }               
                    gotValidParticle = true;
                    // Associate the particle to the PRD. But don't add duplicates.
                    // Note: it may be more efficient to filter out duplicates among particles for the current PRD, then check-and-add the
                    // reduced set to the large multimap. But may be not for the typically small RDO/PRD ratio.
                    typedef PRD_MultiTruthCollection::iterator truthiter;
                    std::pair<truthiter, truthiter> r = prdTruth->equal_range(prd->identify());
                    const auto& pl = particleLink;  // Work around problem with clang 6.0.1
                    if (r.second == std::find_if(r.first, r.second, [pl](const PRD_MultiTruthCollection::value_type& prd_to_truth) {
                            return prd_to_truth.second == pl;
                        })) {
                        prdTruth->insert(std::make_pair(prd->identify(), particleLink));
                    }
                }
            }
            if (gotSDO && !gotValidParticle) {
                // Looked at all the deposits from all the SDOs, but did not find any valid particle link.
                // prdTruth->insert(std::make_pair(prd, particleLinkUnknown));
                ATH_MSG_DEBUG("addPrepRawDatum(): got SDO but no particles");
            }
        }
    }
    return StatusCode::SUCCESS;
}

