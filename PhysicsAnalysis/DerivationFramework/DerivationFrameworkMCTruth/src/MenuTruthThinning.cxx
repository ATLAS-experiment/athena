/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MenuTruthThinning.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Translated into xAOD by James Catmore (James.Catmore@cern.ch)
// from orignal D3PD code by Snyder, Marshall, Boonekamp et al:
// TruthD3PDAnalysis/src/TruthParticleFilterTool.cxx
// Provides user with a menu of options for thinning truth particles
// from xAOD. Tool sets up a mask which is then passed to the
// thinning service.
// DOES NOT PRESERVE GRAPH INTEGRITY

#include "DerivationFrameworkMCTruth/MenuTruthThinning.h"
#include "xAODTruth/TruthVertexContainer.h"
#include "AthenaKernel/errorcheck.h"
#include "TruthUtils/HepMCHelpers.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"
#include "AthContainers/ConstAccessor.h"
#include <vector>
#include <string>

using Gaudi::Units::GeV;

// Constructor
DerivationFramework::MenuTruthThinning::MenuTruthThinning(const std::string& t,
                                                          const std::string& n,
                                                          const IInterface* p ) :
base_class(t,n,p),
m_writeFirstN(-1),
m_totpart(0),
m_removedpart(0)
{
    declareProperty ("WritePartons",
                     m_writePartons = true,
                     "Keep partons?");
    
    declareProperty ("WriteHadrons",
                     m_writeHadrons = false,
                     "Keep hadrons?");
    
    declareProperty ("WriteBHadrons",
                     m_writeBHadrons = true,
                     "Keep b-hadrons?");

    declareProperty ("WriteCHadrons",
                     m_writeCHadrons = true,
                     "Keep c-hadrons?");

    declareProperty ("WriteGeant",
                     m_writeGeant = false,
                     "Keep Geant particles?");
    
    declareProperty ("GeantPhotonPtThresh",
                     m_geantPhotonPtThresh = 0.5*GeV,
                     "Write Geant photons with Pt above this threshold.  "
                     "Set to < 0 to not write any.");
    
    declareProperty ("WriteTauHad",
                     m_writeTauHad = false,
                     "Keep hadronic taus?");
    
    declareProperty ("PartonPtThresh",
                     m_partonPtThresh = -1,
                     "Write partons with Pt above this threshold.");
    
    declareProperty ("WriteBSM",
                     m_writeBSM = false,
                     "Keep BSM particles?");
    
    declareProperty ("WriteBosons",
                     m_writeBosons = false,
                     "Keep bosons?");
    
    declareProperty ("PhotonPtThresh",
                     m_photonPtCut = 3000.,
                     "Photon pt cut with WriteBosons");
    
    declareProperty ("WriteBSMProducts",
                     m_writeBSMProducts = false,
                     "Keep BSM particle decay products?");
    
    declareProperty ("WriteBosonProducts",
                     m_writeBosonProducts = false,
                     "Keep boson decay products?");
    
    declareProperty ("WriteTopAndDecays",
                     m_writeTopAndDecays = false,
                     "Keep top partons and immediate decay products?");
    
    declareProperty("WriteEverything",
                    m_writeEverything = false,
                    "Keep absolutely everything (overrides all other flags)");
    
    declareProperty("WriteAllLeptons",
                    m_writeAllLeptons = false,
                    "Keep absolutely all leptons");
    
    declareProperty("WriteLeptonsNotFromHadrons",
                    m_writeLeptonsNotFromHadrons = false,
                    "Keep leptons not from hadron decays");
    
    declareProperty("WriteAllStable",
                    m_writeAllStable = false,
                    "Keep all stable particles");
    
    declareProperty("WriteNotPhysical",
                    m_writeNotPhysical = false,
                    "Save also non-physical particles");
    
    declareProperty("WriteFirstN",
                    m_writeFirstN = -1,
                    "Keep first N particles in record");
    
    declareProperty("PreserveDescendants",
                    m_preserveDescendants = false,
                    "Preserve descendants of retained particles");
    
    declareProperty("PreserveGeneratorDescendants",
                    m_preserveGeneratorDescendants = false,
                    "Preserve descendants of retained particles excluding Geant particles");
    
    declareProperty("PreserveAncestors",
                    m_preserveAncestors = false,
                    "Preserve ancestors of retained particles");
    
    declareProperty ("PreserveParentsSiblingsChildren",
                     m_preserveImmediate = false,
                     "Preserve the parents, siblings and children of retained particles");
    
    declareProperty ("PreserveHadronizationVertices",
                     m_preserveHadVtx = false,
                     "Preserve hadronization vertices with parents/children.");
    
    declareProperty ("WritettHFHadrons",
                     m_writettHFHadrons = false,
                     "Keep tt+HF hadrons?");

    declareProperty ("PDGIDsToKeep",
                     m_pdgIdsToKeep={},
                     "List of PDG IDs to always keep");

    declareProperty ("LongLivedPDGIDs",
                     m_longLivedPdgIds={},
                     "List of PDG IDs of long lived particles so that one can store their children");
}

// Destructor
DerivationFramework::MenuTruthThinning::~MenuTruthThinning() {
}

// Athena initialize and finalize
StatusCode DerivationFramework::MenuTruthThinning::initialize()
{
    if (m_preserveDescendants && m_preserveGeneratorDescendants) {
        ATH_MSG_FATAL("You are asking to keep both all descendants, and only those from the event generator. Please check your job options.");
        return StatusCode::FAILURE;
    }
    if (m_preserveImmediate && (m_preserveDescendants || m_preserveGeneratorDescendants || m_preserveAncestors)) {
        ATH_MSG_FATAL("You are asking to preserve only parents/children/siblings of retained states, and also more distant relatives. Please check your job options.");
        return StatusCode::FAILURE;
    }
    m_totpart = 0;
    m_removedpart = 0;
    m_eventCount = 0;

    ATH_CHECK( m_particlesKey.initialize (m_streamName) );
    ATH_CHECK( m_verticesKey.initialize (m_streamName) );
    return StatusCode::SUCCESS;
}

StatusCode DerivationFramework::MenuTruthThinning::finalize()
{
    ATH_MSG_VERBOSE("finalize() ...");
    ATH_MSG_INFO("Total of " << m_totpart << " truth particles");
    ATH_MSG_INFO("Removed "<< m_removedpart << " truth particles");
    return StatusCode::SUCCESS;
}

// The thinning itself
StatusCode DerivationFramework::MenuTruthThinning::doThinning() const
{
    const EventContext& ctx = Gaudi::Hive::currentContext();

    // Retrieve the truth collections
    SG::ThinningHandle<xAOD::TruthParticleContainer> importedTruthParticles
      (m_particlesKey, ctx);
    m_totpart += importedTruthParticles->size();

    SG::ThinningHandle<xAOD::TruthVertexContainer> importedTruthVertices
      (m_verticesKey, ctx);

    // Print events
    //if( m_eventCount < 20 ){
    //    printxAODTruth(m_eventCount, importedTruthParticles);
    //}
    ++m_eventCount;
    
    // Set up a mask with the same number of entries as the full TruthParticle collection
    std::vector<bool> particleMask, vertexMask;
    int nTruthParticles = importedTruthParticles->size();
    int nTruthVertices = importedTruthVertices->size();
    particleMask.assign(nTruthParticles,true); // default: keep all particles
    vertexMask.assign(nTruthVertices,false); // throw all vertices: to be discussed
    
    // Standard particle loop
    for (int particleCounter = 0; particleCounter < nTruthParticles; ++particleCounter) {
        const xAOD::TruthParticle* particle = (*importedTruthParticles)[particleCounter];
        // Keep first N particles
        if ( (particleCounter > m_writeFirstN) && (!isAccepted(particle)) ){
            // Particle removal - note mask is true by default
            particleMask[particleCounter] = false;
            ++m_removedpart;
        }
    }
    
    // If user requested preservation of descendants/ancestors:
    // - loop over the masks and work out which particles need to be descended/ascended from
    // - do the recursive loop
    // - update the masks including the descendants/ancestors
    // To ensure graph completeness, this  over-rides anything set by the special treatment
    // of taus in the section above
    DerivationFramework::DecayGraphHelper decayHelper;
    std::unordered_set<int> encounteredUniqueIDs; // For loop handling
    if (m_preserveDescendants || m_preserveGeneratorDescendants || m_preserveAncestors) {
        for (int i=0; i<nTruthParticles; ++i) {
            bool toKeep = particleMask[i];
            if (!toKeep) continue;
            const xAOD::TruthParticle* particle = (*importedTruthParticles)[i];
            encounteredUniqueIDs.clear();
            if (m_preserveDescendants) decayHelper.descendants(particle,particleMask,vertexMask,encounteredUniqueIDs,true);
            encounteredUniqueIDs.clear();
            if (m_preserveGeneratorDescendants) decayHelper.descendants(particle,particleMask,vertexMask,encounteredUniqueIDs,false);
            encounteredUniqueIDs.clear();
            if (m_preserveAncestors) decayHelper.ancestors(particle,particleMask,vertexMask,encounteredUniqueIDs);
            encounteredUniqueIDs.clear();
        }
    }
    // User only wants to keep parents, siblings and children of retained states
    // Much simpler case - no recursion so no need for uniqueIDs etc
    if (m_preserveImmediate) {
        // Make a copy of the particle mask to avoid changes being propagated
        // down the chain
        std::vector<bool> particleMaskCopy = particleMask;
        for (int i=0; i<nTruthParticles; ++i) {
            bool toKeep = particleMask[i]; // still loop over the orginal list, not the copy
            if (!toKeep) continue;
            const xAOD::TruthParticle* particle = (*importedTruthParticles)[i];
            decayHelper.immediateRelatives(particle,particleMaskCopy,vertexMask,
                                           m_preserveHadVtx); // but only update the copy
        }
        particleMask = particleMaskCopy; // Now update the original list in one go
    }
    
    // Execute the thinning service based on the mask. Finish.
    importedTruthParticles.keep (particleMask);
    importedTruthVertices.keep  (vertexMask);
    
    return StatusCode::SUCCESS;
}

// Test to see if we want to keep a particle
bool DerivationFramework::MenuTruthThinning::isAccepted(const xAOD::TruthParticle* p) const
{
    bool ok = false;
    
    int pdg_id = p->absPdgId();

    // All explicitly requested PDG IDs of long lived particles, this is needed
    // because their childrens uniqueIDs can be above the cut off m_geantOffset
    if(!m_longLivedPdgIds.empty() && parentIsLongLived(p)) ok = true; 


    if (HepMC::is_simulation_particle(p) && !m_writeGeant && !m_writeEverything && !ok) {
        if (!(MC::isPhoton(pdg_id) && m_geantPhotonPtThresh >= 0 && p->pt() > m_geantPhotonPtThresh) )
            return false;
    }
    
    // Do we want to save everything?
    if (m_writeEverything) ok = true;
    
    // Save NotPhysical particles
    if (m_writeNotPhysical && !MC::isPhysical(p)) ok = true;
    
    // OK if we select partons and are at beginning of event record
    if( m_writePartons && !MC::isHadron(pdg_id) &&
       (m_partonPtThresh<0 || p->pt()>m_partonPtThresh) )
        ok = true;
    
    //  OK if we should select hadrons and are in hadron range
    // JRC: cut changed from PHOTOSMIN to m_geantOffset
    if( m_writeHadrons && MC::isHadron (pdg_id) && !HepMC::is_simulation_particle(p) ) ok = true;
    
    // OK if we should select b hadrons and are in hadron range
    // JRC: cut changed from PHOTOSMIN to m_geantOffset
    if( m_writeBHadrons &&  !HepMC::is_simulation_particle(p) && MC::isBottomHadron (pdg_id)) ok = true;

    // OK if we should select c hadrons and are in hadron range
    // JRC: cut changed from PHOTOSMIN to m_geantOffset
    if( m_writeCHadrons &&  !HepMC::is_simulation_particle(p) && MC::isCharmHadron (pdg_id)) ok = true;

    // PHOTOS range: check whether photons come from parton range or
    // hadron range
    int motherPDGID = 999999999;
    // AV: dropped the HepMC::PHOTOSMIN condition
    if ( !HepMC::is_simulation_particle(p) && p->hasProdVtx() )
    {
        const xAOD::TruthVertex* vprod = p->prodVtx();
        if (vprod->nIncomingParticles() > 0) {
            const xAOD::TruthParticle* mother = vprod->incomingParticle(0);
            if (mother) motherPDGID = mother->pdgId();
        }
        if( m_writePartons && !MC::isHadron( motherPDGID ) ) ok = true;
        if( m_writeHadrons && MC::isHadron( motherPDGID ) ) ok = true;
    }
    
    // OK if we should select G4 particles and are in G4 range
    if( m_writeGeant && HepMC::is_simulation_particle(p) ) ok = true;
    
    if(isLeptonFromTau(p)) ok = true;
    
    if(isFsrFromLepton(p)) ok = true;
    
    // Hadronic tau decays
    if(m_writeTauHad){
        std::unordered_set<int> uniqueID_trace;
        if (isFromTau(p, uniqueID_trace))
            ok = true;
    }
    
    // Bosons
    if(m_writeBosons && (MC::isZ(p)||MC::isW(p)||MC::isHiggs(p) || (MC::isPhoton(p) && p->pt()>m_photonPtCut) )) ok = true;
    
    // BSM particles
    if(m_writeBSM && MC::isBSM(p)) ok = true;
    
    // tt+HF hadrons
    if (m_writettHFHadrons && isttHFHadron(p)) ok = true;
    
    // Top quarks
    if(m_writeTopAndDecays && MC::isTop(p)) ok = true;
    
    // All leptons
    if(m_writeAllLeptons && MC::isLepton(p))ok = true; // Include 4th generation...

    // All stable
    if (m_writeAllStable && MC::isStable(p) && !HepMC::is_simulation_particle(p)) ok = true;
    
    // All leptons not from hadron decays
    if(!ok && m_writeLeptonsNotFromHadrons && MC::isLepton(p) && MC::isStable(p)) {// Include 4th generation...
        ok = !(matchHadronIncTau(p) || matchQuarkIncTau(p) || isOrphanIncTau(p));
    }
    
    if ((m_writeBSMProducts || m_writeBosonProducts || m_writeTopAndDecays) && p->hasProdVtx()){ // Either way we have to go through parents
        const xAOD::TruthVertex* prodVtx = p->prodVtx();
        unsigned int nIncoming = prodVtx->nIncomingParticles();
        for(unsigned int itr=0; itr<nIncoming; ++itr) {
            const xAOD::TruthParticle* incomingParticle = prodVtx->incomingParticle(itr);
            if (!incomingParticle) continue;
            if ((m_writeBSMProducts && MC::isBSM(incomingParticle)) ||
                (m_writeBosonProducts && (MC::isZ(incomingParticle)||MC::isW(incomingParticle)||MC::isHiggs(incomingParticle) || (MC::isPhoton(incomingParticle) && incomingParticle->pt()>m_photonPtCut) )) ||
                (m_writeTopAndDecays && MC::isTop(incomingParticle)) ){
                ok = true;
                break;
            }
        }
        
    }

    // All explicitly requested PDG IDs
    for (const auto id : m_pdgIdsToKeep){
      if (pdg_id==id) ok=true;
    } // Loop over PDG IDs

    return ok;
}

bool DerivationFramework::MenuTruthThinning::matchHadronIncTau(const xAOD::TruthParticle* part) const {
    //check if part descends from a hadron, possibly passing through a tau decay
    //ported from TopFiducial
    std::vector<int> hadrons = std::vector<int>{111};
    std::vector<int> taus = std::vector<int>{15,-15};
    return matchGenParticle(part, hadrons, taus, true);
}

bool DerivationFramework::MenuTruthThinning::matchQuarkIncTau(const xAOD::TruthParticle* part) const {
    //check if part descends from a hadron, possibly passing through a tau decay
    //ported from TopFiducial
    std::vector<int> quarks = std::vector<int>{1,6};
    std::vector<int> taus = std::vector<int>{15,-15};
    return matchGenParticle(part, quarks, taus, true);
}

bool DerivationFramework::MenuTruthThinning::isOrphanIncTau(const xAOD::TruthParticle* part) const {
    //check if part descends from nothing, possibly passing through a tau decay
    //(indicating a broken truth record)
    //ported from TopFiducial
    
    int pdgId = part->pdgId();
    
    if (!part->hasProdVtx()) return true;
    
    const xAOD::TruthVertex* prodVtx = part->prodVtx();
    unsigned int nIncoming = prodVtx->nIncomingParticles();
    unsigned int itrParent = 0;
    
    // Simple loop catch
    if (prodVtx==part->decayVtx()) return false;
    
    //0 incoming-> orphan
    if(nIncoming == 0) return true;
    
    unsigned int nParents = 0;
    while(nParents==0 && itrParent<nIncoming) {
        const xAOD::TruthParticle* incomingParticle = prodVtx->incomingParticle(itrParent);
        
        // If the parent is itself migrate up the generator record
        if(incomingParticle->pdgId() == pdgId) {
            if(!isOrphanIncTau(incomingParticle)) nParents++;
        }
        else if(MC::isTau(incomingParticle)) {
            if(!isOrphanIncTau(incomingParticle)) nParents++;
        }
        else {
            nParents++;
        }
        ++itrParent;
    }
    return (nParents==0);
}

bool DerivationFramework::MenuTruthThinning::matchGenParticle(const xAOD::TruthParticle* part, std::vector<int> &targetPdgIds,
                                                              std::vector<int> &intermediatePdgIds, bool targetsAreRange) const {
    //check if part descends from a particle, possibly passing through an intermediate decay
    //ported from TopFiducial
    
    int pdgId = part->pdgId();
    
    bool found = false;
    
    // Iterators for the target pdg or the intermediate pdg ids
    std::vector<int>::const_iterator itrPdgId, itrPdgIdEnd;
    
    if (!part->hasProdVtx()) return false;
    
    // Loop over the parents
    const xAOD::TruthVertex* prodVtx = part->prodVtx();
    unsigned int nIncoming = prodVtx->nIncomingParticles();
    
    // Simple loop catch
    if (prodVtx==part->decayVtx()) return false;
    
    unsigned int itrParent = 0;
    while(!found && itrParent<nIncoming) {
        const xAOD::TruthParticle* incomingParticle = prodVtx->incomingParticle(itrParent);
        
        if(!targetsAreRange) {
            
            // If the parent is one of the particles in the target pdg list
            itrPdgId = targetPdgIds.begin();
            itrPdgIdEnd = targetPdgIds.end();
            for(;itrPdgId != itrPdgIdEnd; ++itrPdgId) {
                if(incomingParticle->pdgId() == (*itrPdgId)) return true;
            }
        }
        else {
            int absPdgId = incomingParticle->absPdgId();
            
            // If the parent is within the range given in the target pdg list
            if(targetPdgIds.size() == 1) {
                if(absPdgId >= targetPdgIds.at(0)) return true;
            }
            else if(targetPdgIds.size() >= 2) {
                if(absPdgId >= targetPdgIds.at(0) &&
                   absPdgId <= targetPdgIds.at(1)) return true;
            }
        }
        
        // If the parent is itself migrate up the generator record
        if(incomingParticle->pdgId() == pdgId) {
            found = matchGenParticle(incomingParticle, targetPdgIds, intermediatePdgIds, targetsAreRange);
        }
        else {
            // If the parent is in the intermediatePdgIds list migrate up the generator record
            itrPdgId = intermediatePdgIds.begin();
            itrPdgIdEnd = intermediatePdgIds.end();
            bool foundIntermediate = false;
            while(!foundIntermediate && itrPdgId != itrPdgIdEnd) {
                if(incomingParticle->pdgId() == (*itrPdgId)) foundIntermediate = true;
                else ++itrPdgId;
            }
            if(foundIntermediate) {
                found = matchGenParticle(incomingParticle, targetPdgIds, intermediatePdgIds, targetsAreRange);
            }
        }
        
        ++itrParent;
    }
    return found;
}

bool DerivationFramework::MenuTruthThinning::isLeptonFromTau(const xAOD::TruthParticle* part) const{
    
    int pdg = part->pdgId();
    
    if(!MC::isSMLepton(part)) return false; // all leptons including tau.
    
    const xAOD::TruthVertex* prod = part->prodVtx();
    if(!prod) return false; // no parent.
    
    // Simple loop catch
    if (prod==part->decayVtx()) return false;
    
    // Loop over the parents of this particle.
    unsigned int nIncoming = prod->nIncomingParticles();
    for(unsigned int itr = 0; itr<nIncoming; ++itr){
        int parentId = prod->incomingParticle(itr)->pdgId();
        if( MC::isTau(parentId) ) {
            ATH_MSG_DEBUG("Particle with pdgId = " << pdg << ", matched to tau");
            return true; // Has tau parent
        }
        
        if(parentId == pdg) { // Same particle just a different MC status
            // Go up the generator record until a tau is found or not.
            // Note that this requires a connected *lepton* chain, while calling
            //  isFromTau would allow leptons from hadrons from taus
            if(isLeptonFromTau(prod->incomingParticle(itr))) {
                return true;
            }
        }
    }
    
    return false;
}

bool DerivationFramework::MenuTruthThinning::isFromTau(const xAOD::TruthParticle* part,
                                                       std::unordered_set<int>& uniqueID_trace) const {
    
    int pdg = part->pdgId();
    
    const xAOD::TruthVertex* prod = part->prodVtx();
    if(!prod) return false; // no parent.
    
    // Simple loop catch
    if (prod==part->decayVtx()) return false;
    
    // More complex loop catch
    std::unordered_set<int>::const_iterator foundVtx = uniqueID_trace.find( HepMC::uniqueID(prod) );
    if( foundVtx != uniqueID_trace.end() ) {
      ATH_MSG_DEBUG( "Found a loop (a la Sherpa sample).  Backing out." );
      return false;
    }
    uniqueID_trace.insert(HepMC::uniqueID(prod));
    
    // Loop over the parents of this particle.
    unsigned int nIncoming = prod->nIncomingParticles();
    for (unsigned int pitr = 0; pitr<nIncoming; ++pitr){
        const xAOD::TruthParticle* itrParent = prod->incomingParticle(pitr);
        if (!itrParent) continue;
        if (pitr>2) break; // No point in trying - this vertex does not have a quantum meaning...

        if( MC::isTau(itrParent) ) {
            // Check if one of the children of this parent was a tau - if it is, then it is
            //   photon radiation, and we already cover that under FSR
            bool has_fsr = false;
            if ( itrParent->hasDecayVtx() ){
                unsigned int nChildren = itrParent->decayVtx()->nOutgoingParticles();
                for (unsigned int citr = 0; citr<nChildren; ++citr) {
                    const xAOD::TruthParticle* itrChild = itrParent->decayVtx()->outgoingParticle(citr);
                    if (!itrChild) continue;
                    if (MC::isTau(itrChild)){
                        has_fsr = true;
                        break;
                    } // Caught FSR check
                } // loop over immediate children
            } // Parent has an end vertex
            if (has_fsr) return false;
            ATH_MSG_DEBUG("Particle with pdgId = " << pdg << ", matched to tau");
            return true; // Has tau parent
        }
        
        // Go up the generator record until a tau is found or not.
        if(isFromTau(itrParent, uniqueID_trace)) {
            return true;
        }
    }
    
    return false;
}



bool DerivationFramework::MenuTruthThinning::isttHFHadron(const xAOD::TruthParticle* part) {
    
    int ttHFClassification=6;

    static const SG::ConstAccessor<int> TopHadronOriginFlagAcc("TopHadronOriginFlag");
    if (TopHadronOriginFlagAcc.isAvailable(*part)){
        ttHFClassification = TopHadronOriginFlagAcc(*part);
    }
    else{
        return false;
    }
    
    if (ttHFClassification < 6 ) // useful Hadrons
        return true;
    
    return false;
}


bool DerivationFramework::MenuTruthThinning::isFsrFromLepton(const xAOD::TruthParticle* part) const {
    int pdg = part->pdgId();
    if(!MC::isPhoton(part)) return false;
    if(HepMC::is_simulation_particle(part) ) return false; // Geant photon
    const xAOD::TruthVertex* prod = part->prodVtx();
    if(!prod) return false; // no parent.
    // Simple loop check
    if (prod==part->decayVtx()) return false;
    // Loop over the parents of this particle.
    unsigned int nIncoming = prod->nIncomingParticles();
    for(unsigned int pitr=0; pitr<nIncoming; ++pitr){
        const xAOD::TruthParticle* itrParent = prod->incomingParticle(pitr);
        int parentId = itrParent->pdgId();
        if(MC::isElectron(parentId) ||
           MC::isMuon(parentId) ||
           MC::isTau(parentId)) {
          ATH_MSG_DEBUG("Photon with uniqueID " << HepMC::uniqueID(part) << " matched to particle with pdgId = " << parentId );
            return true; // Has lepton parent
        }
        if(parentId == pdg) { // Same particle just a different MC status
            // Go up the generator record until a tau is found or not.
            if(isFsrFromLepton(itrParent)) return true;
        }
    }
    return false;
}

bool DerivationFramework::MenuTruthThinning::parentIsLongLived(const xAOD::TruthParticle* part) const {
  //loop over the parents of the truth particle, if the parent is in the list of long lived particles
  //store this truth particle.
  for(size_t parent_itr = 0; parent_itr < part->nParents(); parent_itr++){
    if(!part->parent(parent_itr)) continue;
    const xAOD::TruthParticle* parent = part->parent(parent_itr);
    const int parent_abs_pdgid = abs(parent->pdgId());
    if(std::find(m_longLivedPdgIds.begin(), m_longLivedPdgIds.end(), parent_abs_pdgid) != m_longLivedPdgIds.end() ){
      return true;
    }
  }
  return false;
}
