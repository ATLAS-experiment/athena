/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/



#include "xAODTruth/TruthEvent.h"
#include "xAODTruth/TruthEventContainer.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

#include "RedoTruthLinksAlg.h"


using namespace std;

namespace xAODMaker {
    
    StatusCode RedoTruthLinksAlg::initialize() {
        
        // initialize handles
        ATH_CHECK(m_truthLinkContainerKey.initialize());
        ATH_CHECK(m_linksOnlyTruthEventContainerKey.initialize());
        ATH_MSG_DEBUG("xAOD linksOnlyTruthEventContainer name = " << m_linksOnlyTruthEventContainerKey.key() );
        ATH_MSG_DEBUG("xAOD TruthParticleLinksContainer name = " << m_truthLinkContainerKey.key());
        return StatusCode::SUCCESS;
    }
    
    
    StatusCode RedoTruthLinksAlg::execute(const EventContext& ctx) const {
        
        SG::WriteHandle truthLinkVec{m_truthLinkContainerKey, ctx};
        ATH_CHECK(truthLinkVec.record(std::make_unique<xAODTruthParticleLinkVector>()));
            
                
        SG::ReadHandle xTruthEventContainer {m_linksOnlyTruthEventContainerKey, ctx};
        // validity check is only really needed for serial running. Remove when MT is only way.
        ATH_CHECK(xTruthEventContainer.isValid());

        // Loop over events and particles
        for (const xAOD::TruthEvent* evt : *xTruthEventContainer) {
            for (const auto& par : evt->truthParticleLinks()) {
                if ( !par.isValid() ) {
                    // This can happen if particles have been thinned.
                    ATH_MSG_VERBOSE("Found invalid particle element link in TruthEvent"); //< @todo Use HepMC evt number?
                    continue;
                }
                // Create link between HepMC and xAOD truth
                /// @todo AB: Truth particle links should only be made to the signal event... hence the 0. Right?
                truthLinkVec->push_back(std::make_unique<xAODTruthParticleLink>(HepMcParticleLink(HepMC::barcode(*par), 0, HepMcParticleLink::IS_POSITION, HepMcParticleLink::IS_BARCODE), par)); // FIXME is barcode-based - using barcodes until xAOD::TruthParticle::id() is available
            }
        }
            
        std::stable_sort(truthLinkVec->begin(), truthLinkVec->end(), SortTruthParticleLink());
        ATH_MSG_VERBOSE("Summarizing truth link size: " << truthLinkVec->size() );
        
        return StatusCode::SUCCESS;
    }        
} // namespace xAODMaker
