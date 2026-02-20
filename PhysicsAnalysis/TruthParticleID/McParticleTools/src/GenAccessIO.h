/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRUTHHELPER_GENACCESSIO_H
#define TRUTHHELPER_GENACCESSIO_H

#include "GeneratorObjects/McEventCollection.h"
#include "AtlasHepMC/GenParticle.h"
#include "TruthUtils/HepMCHelpers.h"

#include "GaudiKernel/Bootstrap.h"
#include "GaudiKernel/ISvcLocator.h"
#include "StoreGate/StoreGateSvc.h"

#include <vector>
#include <string>

typedef std::vector<HepMC::ConstGenParticlePtr> MCParticleCollection;
typedef std::vector<HepMC::ConstGenParticlePtr>::const_iterator MCParticleCollectionCIter;

namespace TruthHelper {

class GenAccessIO {
public:

    GenAccessIO() {
        if (m_sgSvc.retrieve().isFailure()) {
            throw StatusCode::FAILURE;
        }
    }
    StatusCode getMC(MCParticleCollection& mcParticles, const bool ifgen=false, const std::string& key="GEN_EVENT") const {
        MsgStream log(Athena::getMessageSvc(), "GenAccessIO");

        // Retrieve iterators for McEventCollections objects
        SG::ConstIterator<McEventCollection> firstMEC;
        SG::ConstIterator<McEventCollection> lastMEC;
        if ( (m_sgSvc->retrieve(firstMEC, lastMEC)).isFailure() ) {
            log << MSG::ERROR << "Could not retrieve iterators for McEventCollections" << endmsg;
        }
        const McEventCollection& mcColl = *firstMEC;
        int icount = 0;
        for ( ; firstMEC!= lastMEC; ++firstMEC) icount++;
        log << MSG::DEBUG << "Number of McEventCollections=  "<< icount << endmsg;

        // If there is more than one then do the retrieve with the key
        if (icount > 1) {
            log << MSG::DEBUG << "Key = " << key << endmsg;
            const McEventCollection* mcCollptr = nullptr;
            return this->getDH(mcCollptr, key);
        }

        if (icount > 0) {
            // Iterate over all McEvent records
            McEventCollection::const_iterator itr;
            for (itr = mcColl.begin(); itr!=mcColl.end(); ++itr) {
                // Access the HepMC record which is wrapped within McEvent
                const HepMC::GenEvent* genEvt = (*itr);
                if (genEvt == 0) return StatusCode::FAILURE;
                if (ifgen) {
                    for (auto it: *genEvt) {
                        if (!HepMC::is_simulation_particle(it)) mcParticles.push_back(std::move(it));
                    }
                } else {
                    for (auto it: *genEvt) {
                        mcParticles.push_back(std::move(it));
                    }
                }
            }
        }
        return StatusCode::SUCCESS;
    }
    StatusCode getDH(const McEventCollection*& dh) const {
        return m_sgSvc->retrieve(dh);
    }
    StatusCode getDH(const McEventCollection*& dh, const std::string& key) const {
        return m_sgSvc->retrieve(dh, key);
    }
    StatusCode store (McEventCollection* storee, const std::string& key) const {
        return m_sgSvc->record(storee, key);
    }
private:
    ServiceHandle<StoreGateSvc> m_sgSvc{"StoreGateSvc", "GenAccessIO"};
};
}

#endif
