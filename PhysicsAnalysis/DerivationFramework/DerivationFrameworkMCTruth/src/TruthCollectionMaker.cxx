/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthCollectionMaker.cxx
// Author: James Catmore (James.Catmore@cern.ch)
// Removes all truth particles/vertices which do not pass a user-defined cut

// Base class
#include "DerivationFrameworkMCTruth/TruthCollectionMaker.h"
// EDM includes
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "TruthUtils/MagicNumbers.h"
// Handles
#include "StoreGate/WriteDecorHandle.h"
// To look up which generator is being used
#include "StoreGate/StoreGateSvc.h"
#include "xAODTruth/TruthMetaDataContainer.h"
#include "TruthUtils/HepMCHelpers.h"
#include "AthContainers/ConstAccessor.h"
// STL Includes
#include <vector>
#include <string>
#include <numeric>

// Athena initialize and finalize
StatusCode DerivationFramework::TruthCollectionMaker::initialize()
{
    ATH_MSG_VERBOSE("initialize() ...");
    // Input truth particles
    ATH_CHECK( m_particlesKey.initialize() );
    ATH_MSG_INFO("Using " << m_particlesKey.key() << " as the input truth container key");

    // Output (new) truth particles
    ATH_CHECK(m_outputParticlesKey.initialize());
    ATH_MSG_INFO("New truth particles container key: " << m_outputParticlesKey.key() );

    if (m_partString.empty()) {
        ATH_MSG_FATAL("No selection string provided");
        return StatusCode::FAILURE;
    } else {ATH_MSG_INFO("Truth particle selection string: " << m_partString );}

    // Set up the text-parsing machinery for thinning the truth directly according to user cuts
    if (!m_partString.empty()) {
       ATH_CHECK( initializeParser(m_partString) );
    }

    // Initialise read handle keys
    ATH_CHECK(m_originReadDecorKey.initialize());
    ATH_CHECK(m_typeReadDecorKey.initialize());
    ATH_CHECK(m_outcomeReadDecorKey.initialize());
    ATH_CHECK(m_classificationReadDecorKey.initialize());

    // Initialise decorator handle keys
    ATH_CHECK(m_linkDecoratorKey.initialize());
    ATH_CHECK(m_originDecoratorKey.initialize());
    ATH_CHECK(m_typeDecoratorKey.initialize());
    ATH_CHECK(m_outcomeDecoratorKey.initialize());
    ATH_CHECK(m_classificationDecoratorKey.initialize());
    ATH_CHECK(m_motherIDDecoratorKey.initialize());
    ATH_CHECK(m_daughterIDDecoratorKey.initialize());
    ATH_CHECK(m_hadronOriginDecoratorKey.initialize());

    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::TruthCollectionMaker::finalize()
{
    ATH_MSG_VERBOSE("finalize() ...");
    //ATH_MSG_INFO("Processed "<< m_ntotvtx <<" truth vertices, "<< m_npassvtx << " were retained ");
    ATH_MSG_INFO("Processed "<< m_ntotpart <<" truth particles, "<< m_npasspart << " were retained ");
    ATH_CHECK( finalizeParser() );
    return StatusCode::SUCCESS;
}

// Selection and collection creation
StatusCode DerivationFramework::TruthCollectionMaker::addBranches(const EventContext& ctx) const
{
    // Event context for AthenaMT

    // Set up for some metadata handling
    // TODO: this isn't MT compliant. This information should go into the config level and avoid meta store
    static const bool is_sherpa = [this]() {
        bool is_sherpa = false;
        if (m_metaStore->contains<xAOD::TruthMetaDataContainer>("TruthMetaData")){
            // Note that I'd like to get this out of metadata in general, but it seems that the
            // metadata isn't fully available in initialize, and since this is a const function
            // I can only do the retrieve every event, rather than lazy-initializing, since this
            // metadata ought not change during a run
            const xAOD::TruthMetaDataContainer* truthMetaData(nullptr);
            // Shamelessly stolen from the file meta data tool
            if (m_metaStore->retrieve(truthMetaData).isSuccess() && !truthMetaData->empty()){
                // Let's just be super sure...
                const std::string gens = truthMetaData->at(0)->generators();
                is_sherpa = (gens.find("sherpa")==std::string::npos &&
                             gens.find("Sherpa")==std::string::npos &&
                             gens.find("SHERPA")==std::string::npos) ? false : true;
            } // Seems to be the only sure way...
            else {
                ATH_MSG_WARNING("Found xAODTruthMetaDataContainer empty! Configuring to be NOT Sherpa.");
            }
            ATH_MSG_INFO("From metadata configured: Sherpa? " << is_sherpa);
        } else {
            ATH_MSG_WARNING("Could not find metadata container in storegate; assuming NOT Sherpa");
        }
        return is_sherpa;
    }();

    // Retrieve truth collections
    SG::ReadHandle<xAOD::TruthParticleContainer> truthParticles(m_particlesKey,ctx);
    if (!truthParticles.isValid()) {
        ATH_MSG_ERROR("Couldn't retrieve TruthParticle collection with name " << m_particlesKey);
        return StatusCode::FAILURE;
    }

    // Create the new particle containers and WriteHandles
    SG::WriteHandle<xAOD::TruthParticleContainer> newParticlesWriteHandle(m_outputParticlesKey, ctx);
    ATH_CHECK(newParticlesWriteHandle.record(std::make_unique<xAOD::TruthParticleContainer>(),
                                             std::make_unique<xAOD::TruthParticleAuxContainer>()));
    ATH_MSG_DEBUG( "Recorded new TruthParticleContainer with key: " << (m_outputParticlesKey.key()));

    // Set up a mask with the same entries as the full collections
    unsigned int nParticles = truthParticles->size();
    m_ntotpart += nParticles;

    // Set up decor readers
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > originReadDecor(m_originReadDecorKey, ctx);
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeReadDecor(m_typeReadDecorKey, ctx);
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeReadDecor(m_outcomeReadDecorKey, ctx);
    SG::ReadDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationReadDecor(m_classificationReadDecorKey, ctx);

    // Set up decorators
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, ElementLink<xAOD::TruthParticleContainer> > linkDecorator(m_linkDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > originDecorator(m_originDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > typeDecorator(m_typeDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > outcomeDecorator(m_outcomeDecoratorKey, ctx);
    SG::WriteDecorHandle<xAOD::TruthParticleContainer, unsigned int > classificationDecorator(m_classificationDecoratorKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, int > motherIDDecorator(m_motherIDDecoratorKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, int > daughterIDDecorator(m_daughterIDDecoratorKey, ctx);
    SG::WriteDecorHandle< xAOD::TruthParticleContainer, int > hadronOriginDecorator(m_hadronOriginDecoratorKey, ctx);

    // Execute the text parsers and update the mask
    if (!m_partString.empty()) {
        std::vector<int> entries =  m_parser->evaluateAsVector();
        unsigned int nEntries = entries.size();
        // check the sizes are compatible
        if (nParticles != nEntries ) {
            ATH_MSG_ERROR("Sizes incompatible! Are you sure your selection string used TruthParticles?");
            return StatusCode::FAILURE;
        } else {
            // add relevant particles to new collection

            //---------------
            //This is some code to *add* new particles. Probably a good idea to break this off as a sub-function, but I'll let James C decide where that should go.
            //---------------

            //Let's check if we want to build W/Z bosons
            bool SherpaW = false;
            bool SherpaZ = false;
            if (m_partString.value().find("24") != std::string::npos && m_do_sherpa && is_sherpa) {
                SherpaW = true;
            }
            if (m_partString.value().find("23") != std::string::npos && m_do_sherpa && is_sherpa) {
                SherpaZ = true;
            }
            if ((SherpaW or SherpaZ) && is_sherpa){
                if (std::accumulate(entries.begin(),entries.end(),0) > 0){ //We actually have some W and Z bosons in there.
                    SherpaW = false;
                    SherpaZ = false;
                }
            }

            if ((SherpaW || SherpaZ) && is_sherpa){
                // Currently only handles un-ambiguous cases
                std::vector<const xAOD::TruthParticle*>  status_nonPhysical;
                for (unsigned int i=0; i<nParticles; ++i) {
                    // Nullptr check
                    if (!truthParticles->at(i)) continue;
                    // Only collect leptons
                    if (!MC::isSMLepton(truthParticles->at(i))) continue;
                    // Gather by status
                    if (!MC::isPhysical(truthParticles->at(i))) status_nonPhysical.push_back(  truthParticles->at(i) );
                } // Done with loop over truth particles
                // Boson cases that we can actually deal with -- generically up to VVV
                if ((status_nonPhysical.size()==2 || status_nonPhysical.size()==4 || status_nonPhysical.size()==6) && (SherpaZ || SherpaW)){
                    // Basic boson pairing...
                    int gens[3] = {0,0,0};
                    for (size_t i=0;i<status_nonPhysical.size();++i){
                        if (status_nonPhysical[i]->absPdgId()<13) gens[0]++;
                        else if (status_nonPhysical[i]->absPdgId()<15) gens[1]++;
                        else gens[2]++;
                    } // Loop over status_nonPhysical particles
                    // Should only have even numbers per generation.  Any number greater than 2 or ==1 and we're dead
                    if (gens[0]>2 || gens[0]==1 || gens[1]>2 || gens[1]==1 || gens[2]>2 || gens[2]==1){
                        // In agreeing with Sherpa authors, these are Q-M ambiguous states.  Do not let users be evil.
                        ATH_MSG_VERBOSE("Too many leptons of one generation.  Cannot make bosons.  Give up");
                        return StatusCode::SUCCESS;
                    }
                    std::vector<const xAOD::TruthParticle*> boson;
                    for (size_t i=0;i<status_nonPhysical.size();++i){
                        if (status_nonPhysical[i]->absPdgId()<13) boson.push_back(status_nonPhysical[i]);
                        else if (gens[0]==0 && status_nonPhysical[i]->absPdgId()<15) boson.push_back(status_nonPhysical[i]);
                        else if (gens[0]==0 && gens[1]==0) boson.push_back(status_nonPhysical[i]);
                        if (boson.size()==2){
                            // Make a boson!  Just have to figure out _which_ boson!
                            int pdg_id=0;
                            // Easy case: Z boson
                            if (boson[0]->pdgId()==-boson[1]->pdgId()) pdg_id=23;
                            else if (std::abs(boson[0]->pdgId()+boson[1]->pdgId())!=1){
                                // No idea what you were
                                ATH_MSG_WARNING("Do not know how to interpret as a boson: " << boson[0]->pdgId() << " " << boson[1]->pdgId());
                            } else {
                                // W boson
                                pdg_id=24*(boson[0]->pdgId()+boson[1]->pdgId());
                            }
                            if ( (SherpaW && MC::isW(pdg_id)) ||
                                 (SherpaZ && MC::isZ(pdg_id)) ){
                                // Make a Z or a W
                                xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
                                newParticlesWriteHandle->push_back( xTruthParticle );
                                if (m_keep_navigation_info){
                                    if (boson[0]->hasProdVtx()) {
                                        if ((boson[0]->prodVtx()->nIncomingParticles() > 0) && (boson[0]->prodVtx()->incomingParticle(0)!=nullptr)) {
                                            motherIDDecorator(*xTruthParticle) = boson[0]->prodVtx()->incomingParticle(0)->pdgId();
                                        } else {motherIDDecorator(*xTruthParticle) = 0;}
                                    } else {motherIDDecorator(*xTruthParticle) = 0;}
                                    if (boson[0]->hasDecayVtx()) {
                                        if ((boson[0]->decayVtx()->nOutgoingParticles() > 0) && (boson[0]->decayVtx()->outgoingParticle(0)!=nullptr)) {
                                            daughterIDDecorator(*xTruthParticle) = boson[0]->decayVtx()->outgoingParticle(0)->pdgId();
                                        } else {daughterIDDecorator(*xTruthParticle) = 0;}
                                    } else {daughterIDDecorator(*xTruthParticle) = 0;}
                                }
                                // Set with what makes sense here
                                xTruthParticle->setPdgId(pdg_id);
                                // Set dummy values
                                xTruthParticle->setUid(HepMC::INVALID_PARTICLE_ID);
                                xTruthParticle->setStatus(3);
                                // Use the sum of the momenta
                                xAOD::IParticle::FourMom_t new_mom = boson[0]->p4()+boson[1]->p4();
                                xTruthParticle->setM(new_mom.M());
                                xTruthParticle->setPx(new_mom.Px());
                                xTruthParticle->setPy(new_mom.Py());
                                xTruthParticle->setPz(new_mom.Pz());
                                xTruthParticle->setE(new_mom.E());
                            }
                            // Now clear the vectors
                            boson.clear();
                            // And move to the next generation
                            if (gens[0]==0 && gens[1]==0) gens[2]=0;
                            else if (gens[0]==0) gens[1]=0;
                            else gens[0]=0;
                        } // Done making a boson
                    } // Done looping over particles
                }
                if (status_nonPhysical.size()==1 || status_nonPhysical.size()==3 || status_nonPhysical.size()==5 || status_nonPhysical.size()>6){
                    ATH_MSG_WARNING(status_nonPhysical.size() << " leptons found in the Sherpa event record.  Not sure how to deal with this.");
                }
                return StatusCode::SUCCESS;
            }
            for (unsigned int i=0; i<nParticles; ++i) {
                ElementLink<xAOD::TruthParticleContainer> eltp(*truthParticles,i);
                if (entries[i]==1) {
                    //In TRUTH3, we want to remove all particles but the first and last in a decay chain.  This is off in TRUTH1.  The first and last particles in the decay chain are decorated as such.

                    const xAOD::TruthParticle* theParticle = (*truthParticles)[i];
                    if (m_do_compress){
                        bool same_as_mother = false;
                        bool same_as_daughter = false;
                        if (theParticle->hasProdVtx()){
                            if ((theParticle->prodVtx()->nIncomingParticles() > 0) && (theParticle->prodVtx()->incomingParticle(0)!=nullptr)) {
                                if (theParticle->prodVtx()->incomingParticle(0)->pdgId() == theParticle->pdgId()){
                                    same_as_mother = true;
                                }
                            }
                        }
                        if (theParticle->hasDecayVtx()){
                            if ((theParticle->decayVtx()->nOutgoingParticles() > 0) && (theParticle->decayVtx()->outgoingParticle(0)!=nullptr)) {
                                if (theParticle->decayVtx()->outgoingParticle(0)->pdgId() == theParticle->pdgId()){
                                    same_as_daughter = true;
                                }
                            }
                        }
                        if (same_as_mother && same_as_daughter){
                            entries[i]=0;
                            continue;
                        }
                    }
                    xAOD::TruthParticle* xTruthParticle = new xAOD::TruthParticle();
                    newParticlesWriteHandle->push_back( xTruthParticle );
                    if (m_keep_navigation_info){
                        if (theParticle->hasProdVtx()) {
                            if ((theParticle->prodVtx()->nIncomingParticles() > 0) && (theParticle->prodVtx()->incomingParticle(0)!=nullptr)) {
                                motherIDDecorator(*xTruthParticle) = theParticle->prodVtx()->incomingParticle(0)->pdgId();
                            } else {motherIDDecorator(*xTruthParticle) = 0;}
                        } else {motherIDDecorator(*xTruthParticle) = 0;}
                        if (theParticle->hasDecayVtx()) {
                            if ((theParticle->decayVtx()->nOutgoingParticles() > 0) && (theParticle->decayVtx()->outgoingParticle(0)!=nullptr)) {
                                daughterIDDecorator(*xTruthParticle) = theParticle->decayVtx()->outgoingParticle(0)->pdgId();
                            } else {daughterIDDecorator(*xTruthParticle) = 0;}
                        } else {daughterIDDecorator(*xTruthParticle) = 0;}
                    }
                    // Fill with numerical content
                    *xTruthParticle=*theParticle;
                    // Copy over the decorations if they are available
                    typeDecorator(*xTruthParticle) =
                      typeReadDecor(*theParticle);

                    originDecorator(*xTruthParticle) =
                      originReadDecor(*theParticle);

                    outcomeDecorator(*xTruthParticle) =
                      outcomeReadDecor(*theParticle);

                    classificationDecorator(*xTruthParticle) =
                      classificationReadDecor(*theParticle);

                    if (m_outputParticlesKey.key()=="TruthHFHadrons"){ // FIXME Cannot find any other reference to this container name in Athena...
                        static const SG::ConstAccessor<int> TopHadronOriginFlagAcc("TopHadronOriginFlag"); // FIXME This should be a ReadDecorHandle
                        hadronOriginDecorator(*xTruthParticle) =
                          TopHadronOriginFlagAcc.withDefault (*theParticle, 0); // FIXME avoid using withDefault as it masks configuration/scheduling issues?
                    }

                    if(m_keep_navigation_info) linkDecorator(*xTruthParticle) = eltp;
                }
            }
        }
        // Count the mask
        for (unsigned int i=0; i<nParticles; ++i) if (entries[i]) ++m_npasspart;
    }

    return StatusCode::SUCCESS;
}
