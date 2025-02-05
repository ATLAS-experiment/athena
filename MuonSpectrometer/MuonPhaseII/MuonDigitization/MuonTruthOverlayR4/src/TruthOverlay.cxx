/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "TruthOverlay.h"

#include "MuonIdHelpers/IdentifierByDetElSorter.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadHandle.h"
#include "xAODMuonSimHit/MuonSimHitAuxContainer.h"
#include "xAODMuonViews/ChamberViewer.h"

namespace MuonR4{

    StatusCode TruthOverlay::initialize() {
        ATH_CHECK(m_sigKey.initialize());
        ATH_CHECK(m_bkgKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(m_idHelperSvc.retrieve());
        if (m_deadTime < m_mergeTime) {
            ATH_MSG_FATAL("The "<<m_deadTime<<" should be greater than "<<m_mergeTime);
            return StatusCode::FAILURE;
        }
        return StatusCode::SUCCESS;
    }
    StatusCode TruthOverlay::execute(const EventContext& ctx) const {
        

        /// Map that contains all hits sorted by identifier
        std::map<Identifier, std::vector<const xAOD::MuonSimHit*>, 
                 Muon::IdentifierByDetElSorter> overlayed{m_idHelperSvc.get()};

        /// Helper function to fill the hits to merge
        auto fillMap = [&overlayed, &ctx, this] (const SG::ReadHandleKey<xAOD::MuonSimHitContainer>& key) -> StatusCode{
            SG::ReadHandle hits{key, ctx};
            ATH_CHECK(hits.isPresent());
            xAOD::ChamberViewer viewer{*hits, m_idHelperSvc.get()};
            do {
                for (const xAOD::MuonSimHit* hit : viewer) {
                   overlayed[hit->identify()].push_back(hit);
                }            
            } while (viewer.next());

            return StatusCode::SUCCESS;
        };
        ATH_CHECK(fillMap(m_sigKey));
        ATH_CHECK(fillMap(m_bkgKey));

        /// Setup the container for merging        
        SG::WriteHandle writeHandle {m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::MuonSimHitContainer>(),
                                     std::make_unique<xAOD::MuonSimHitAuxContainer>()));

        for (auto& [mergeId, hits] : overlayed) {
            ATH_MSG_VERBOSE("Perform truth overlay for "<<m_idHelperSvc->toString(mergeId));
            std::ranges::sort(hits, [](const xAOD::MuonSimHit* a, const xAOD::MuonSimHit* b){
                return a->globalTime() < b->globalTime();
            });

            for (auto itr = hits.begin(); itr != hits.end(); ++itr){
                const xAOD::MuonSimHit* primHit{*itr};
                if (!m_includePileUpTruth && HepMC::ignoreTruthLink(primHit->genParticleLink(), m_vetoPileUpTruthLinks)){
                    ATH_MSG_VERBOSE("Reject background hit");
                    continue;
                }
                xAOD::MuonSimHit* merged = writeHandle->push_back(std::make_unique<xAOD::MuonSimHit>());
                (*merged) = (*primHit);
                auto mergeItr = itr +1;
                for ( ; mergeItr !=hits.end(); ++mergeItr) {
                    const xAOD::MuonSimHit* mergeMe{*itr};
                    if (!m_includePileUpTruth && HepMC::ignoreTruthLink(mergeMe->genParticleLink(), m_vetoPileUpTruthLinks)){
                        ATH_MSG_VERBOSE("Don't merge with a background hit.");
                        continue;
                    }
                    if(mergeMe->globalTime() - primHit->globalTime() < m_mergeTime) {
                        /// If a background hit overlays with a signal hit, take the signal
                        if (std::abs(mergeMe->pdgId()) == 13 || (mergeMe->genParticleLink() && !merged->genParticleLink())){
                            (*merged) =  (*mergeMe);
                        }
                        merged->setEnergyDeposit(mergeMe->energyDeposit() + primHit->energyDeposit());
                    } else if (mergeMe->globalTime() - primHit->globalTime() < m_deadTime) {
                        continue;
                    } else { 
                        break;
                    }
                }
                itr = mergeItr - 1;
            }
        }
        return StatusCode::SUCCESS;
    }
}