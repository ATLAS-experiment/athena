/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonSimHitSortingAlg.h"

#include <StoreGate/ReadHandle.h>
#include <StoreGate/WriteHandle.h>
#include <xAODMuonSimHit/MuonSimHitAuxContainer.h>
#include <MuonReadoutGeometryR4/MuonDetectorDefs.h>
#include <AthContainers/ConstDataVector.h>
#include <GaudiKernel/SystemOfUnits.h>
namespace {
    constexpr double tolerance = 1. * Gaudi::Units::micrometer;
}

StatusCode MuonSimHitSortingAlg::initialize() {
    if (m_readKeys.empty()) {
        ATH_MSG_FATAL("Please provide at least one container to sort");
        return StatusCode::FAILURE;
    }
    ATH_CHECK(m_readKeys.initialize());
    ATH_CHECK(m_writeKey.initialize());
    ATH_CHECK(m_idHelperSvc.retrieve());
    return StatusCode::SUCCESS;
}
StatusCode MuonSimHitSortingAlg::execute(const EventContext& ctx) const {
    ConstDataVector<xAOD::MuonSimHitContainer> allSimHits{SG::VIEW_ELEMENTS};
    for (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& inKey : m_readKeys) {
         SG::ReadHandle readHandle{inKey, ctx};
         if(!readHandle.isValid()) {
            ATH_MSG_FATAL("Failed to retrieve "<<inKey.fullKey());
            return StatusCode::FAILURE;
         }
         std::ranges::copy(*readHandle, std::back_inserter(allSimHits));
    }
    std::stable_sort(allSimHits.begin(), allSimHits.end(),
                    [this](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                        /// Sort by detector element
                        const IdentifierHash hashA = m_idHelperSvc->detElementHash(a->identify());
                        const IdentifierHash hashB = m_idHelperSvc->detElementHash(a->identify());
                        if (hashA != hashB) {
                            return hashA < hashB;
                        }
                        /// Inside the detector element sort them by channel
                        if (a->identify() != b->identify()) {
                            return a->identify() < b->identify();
                        }
                        /// In each, detector put the earliest hit first.
                        const float dT = a->globalTime() - b->globalTime();
                        if (std::abs(dT) >  0.1 * Gaudi::Units::picosecond) {
                            return dT < 0.;
                        }
                        /// Emitted brems electrons are sorted after the primary muons
                        if (std::abs(a->pdgId()) != std::abs(b->pdgId())){
                            return a->pdgId() > b->pdgId();
                        }
                        /// If Geant has undertaken multiple steps in the sensitive volume sort them
                        /// by barcode
                        return a->genParticleLink().barcode() < b->genParticleLink().barcode();
                    });
    if (m_removeDuplicates) {
        std::vector<const xAOD::MuonSimHit*> dupFreeHits{};
        dupFreeHits.reserve(allSimHits.size());
        std::ranges::copy_if(allSimHits, std::back_inserter(dupFreeHits), 
            [&dupFreeHits, this] (const xAOD::MuonSimHit* hit) {
                const int barcode = hit->genParticleLink().barcode();
                const Identifier hitId = hit->identify();
                const Amg::Vector3D lPos{xAOD::toEigen(hit->localPosition())};
                const Amg::Vector3D lDir{xAOD::toEigen(hit->localDirection())};
                ATH_MSG_VERBOSE("Check sim hit "<<m_idHelperSvc->toString(hitId)<<", pdgId:"<<hit->pdgId()
                                <<", barcode: "<<barcode
                                <<" at "<<Amg::toString(lPos, 2)<<"direction: "<<Amg::toString(lDir, 2));
                return std::ranges::find_if(dupFreeHits, 
                                    [&](const xAOD::MuonSimHit* selHit) {
                            if (selHit->identify() != hitId || 
                                barcode != selHit->genParticleLink().barcode()) return false;
                            if (barcode) return true;
                            const Amg::Vector3D dPos = lPos - xAOD::toEigen(selHit->localPosition());
                            const Amg::Vector3D dDir = lDir - xAOD::toEigen(selHit->localDirection());
                            return dPos.mag() < tolerance && dDir.mag() < tolerance;
                        }) == dupFreeHits.end();
            });
        allSimHits.clear();
        std::ranges::copy(dupFreeHits, std::back_inserter(allSimHits));
    }
    SG::WriteHandle writeHandle{m_writeKey, ctx};
    if (m_writeDeepCopy) {
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSimHitContainer>(),
                                     std::make_unique<xAOD::MuonSimHitAuxContainer>()));
        for (const xAOD::MuonSimHit* copy_me : allSimHits) {
            xAOD::MuonSimHit* newHit = writeHandle->push_back(std::make_unique<xAOD::MuonSimHit>());
            (*newHit) = (*copy_me);
        }
    } else {
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSimHitContainer>(*allSimHits.asDataVector())));
     }
    return StatusCode::SUCCESS;    
}
