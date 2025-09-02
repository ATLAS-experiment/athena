/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// HardTruthThinning.cxx, (c) ATLAS Detector software
// Author: Frank Paige
// Based on MenuTruthThinning and should perhaps be merged with it.
// Intended for use with CompactHardTruth.
// Preserves graph only for decays of selected particles.
//
// No treatment of Geant particles (yet).
//
///////////////////////////////////////////////////////////////////

#include "DerivationFrameworkMCTruth/HardTruthThinning.h"
#include "xAODJet/JetContainer.h"
#include "xAODBase/IParticle.h"
#include "AthenaKernel/errorcheck.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "StoreGate/ThinningHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "TruthUtils/HepMCHelpers.h"
#include <vector>
#include <string>

using Gaudi::Units::GeV;

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////

// Required parameters: 
// EventInfo, TruthParticles, TruthVertices, HardParticles
// Parameters that turn on features:
// JetName      If not empty, save constituents with cuts
// KeepIds      If pdgId list not empty, save particles and decay chains
// IsolRadius   If positive, save stable particles in isolation cones

DerivationFramework::HardTruthThinning::HardTruthThinning(
  const std::string& t, const std::string& n, const IInterface* p ) :
  base_class(t,n,p),
  m_jetPtCut(0.),
  m_jetEtaCut(5.0),
  m_jetConstPtCut(0.),
  m_jetPhotonPtCut(0.),
  m_isolR(-1),
  m_isolPtCut(0),  
  m_maxCount(0)
{
  declareProperty("JetPtCut", m_jetPtCut,
                  "truth jet minumum pt");

  declareProperty("JetEtaCut", m_jetEtaCut,
                  "truth jet maximum abs(eta)");

  declareProperty("JetConstPtCut", m_jetConstPtCut,
                  "jet constituent minimum pt");

  declareProperty("JetPhotonPtCut", m_jetPhotonPtCut,
                  "jet constituent photon minimum pt");

  declareProperty("KeepIds", m_keepIds,
                  "list of abs(pdgID) to keep");

  declareProperty("IsolRadius", m_isolR,
                  "isolation radius for leptons and photons");

  declareProperty("IsolPtCut", m_isolPtCut,
                  "isolation particle minimum pt");

  declareProperty("MaxCount",
                  m_maxCount,
                  "maximum number of events to print");
}


////////////////////////////////////////////////////////////
// Destructor
////////////////////////////////////////////////////////////

DerivationFramework::HardTruthThinning::~HardTruthThinning() {
}


//////////////////////////////////////////////////
// Athena initialize and finalize
//////////////////////////////////////////////////

StatusCode DerivationFramework::HardTruthThinning::initialize()
{
  ATH_CHECK( m_eventInfoKey.initialize() );
  ATH_CHECK( m_hardParticleKey.initialize() );
  ATH_CHECK( m_truthJetsKey.initialize(SG::AllowEmpty) );
  ATH_CHECK( m_truthParticleName.initialize (m_streamName) );
  ATH_CHECK( m_truthVertexName.initialize (m_streamName) );

  m_evtCount = -1;
  m_errCount = 0;

  // Check for unset photon cut
  if( m_jetPhotonPtCut < m_jetConstPtCut ) m_jetPhotonPtCut =m_jetConstPtCut;

  return StatusCode::SUCCESS;
}


StatusCode DerivationFramework::HardTruthThinning::finalize()
{
    ATH_MSG_INFO("finalize() ...");
    if( m_errCount > 0 ){
      ATH_MSG_WARNING("TruthHard/TruthEvent pdgId mismatches " <<m_errCount);
    } else {
      ATH_MSG_INFO("No TruthHard/TruthEvent pdgId mismatches");
    }

    return StatusCode::SUCCESS;
}


////////////////////////////////////////////////////////////
// doThinning
////////////////////////////////////////////////////////////

StatusCode DerivationFramework::HardTruthThinning::doThinning() const
{
  const EventContext& ctx = Gaudi::Hive::currentContext();

  ++m_evtCount;
  bool doPrint = m_evtCount < m_maxCount;

  // Retrieve truth particles and vertices
  SG::ThinningHandle<xAOD::TruthParticleContainer> inTruthParts
    (m_truthParticleName, ctx);
  SG::ThinningHandle<xAOD::TruthVertexContainer> inTruthVerts
    (m_truthVertexName, ctx);

  SG::ReadHandle<xAOD::TruthParticleContainer> inHardParts(m_hardParticleKey, ctx);
  if (!inHardParts.isValid()) {
    ATH_MSG_ERROR("Cannot retrieve TruthParticleContainer " << m_hardParticleKey);
    return StatusCode::FAILURE;
  }

  // Hard particles are different objects but have matching unique IDs.
  // Find hard particles with status==1.
  // Save 4vectors of leptons and photons for matching if requested.
  // Do we need a photon pt cut for soft photons from quarks?

  std::vector<const xAOD::TruthParticle*> hardPart;
  std::vector<TLorentzVector> pLepGam;

  for (const auto* pItr : *inHardParts) {
    if( MC::isStable(pItr) ){
      hardPart.push_back(pItr);
      if( m_isolR > 0 ){
        int ida = pItr->absPdgId();
        if( MC::isElectron(ida) || MC::isMuon(ida) || MC::isTau(ida) || MC::isPhoton(ida) ){
          pLepGam.push_back( pItr->p4() );
        }
      }
    }
  }

  // Print full input event
  long long int evtNum{};
  if( doPrint ){
    SG::ReadHandle<xAOD::EventInfo> evt(m_eventInfoKey, ctx);
    if(!evt.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve EventInfo");
      return StatusCode::FAILURE;
    }
    evtNum = evt->eventNumber();

    printxAODTruth(evtNum, inTruthParts.cptr());
  }


  // Set up masks for particles/vertices
  // Default is false for all

  std::vector<bool> partMask;
  std::vector<bool> vertMask;
  unsigned int inPartNum = inTruthParts->size();
  unsigned int inVertNum = inTruthVerts->size();
  partMask.assign(inPartNum, false);
  vertMask.assign(inVertNum, false);

  ATH_MSG_DEBUG("Initial particles/vertices = " <<inPartNum <<" " <<inVertNum);


  // Particles from hard event are stable => keep no parent/child vertices.
  // No descendants (except Geant).
  // Particles from keep list need descendants and their parent vertices.
  // There could be overlap, e.g. for taus. 
  // Expect no pdgId mismatches.

  std::vector<const xAOD::TruthParticle*> kids;

  for (const auto* pItr : *inTruthParts) {
    // Stable hard particle matches
    int uid = HepMC::uniqueID(pItr);
    bool isHard = false;
    for(unsigned int i=0; i<hardPart.size(); ++i){
      if( uid == HepMC::uniqueID(hardPart[i]) ){
        isHard = true;
        if( pItr->pdgId() != (hardPart[i])->pdgId() ){
          ATH_MSG_WARNING("pdgID mismatch, TruthParticle uid/pdgid "
                          <<HepMC::uniqueID(pItr) <<" " <<pItr->pdgId()
                          <<"   Hard uid/pdgid "  <<HepMC::uniqueID(hardPart[i])
                          <<" " <<(hardPart[i])->pdgId());
          ++m_errCount;
          break;
        }
      }
    }
    if( isHard ){
      if( doPrint ) ATH_MSG_DEBUG("ParticleMask isHard " <<uid);
      partMask[ pItr->index() ] = true;
    }

    // Keep particles
    int ida = pItr->absPdgId();
    bool isKeep = false;
    for(unsigned int i=0; i<m_keepIds.size(); ++i){
      if( ida == m_keepIds[i] ){
        isKeep = true;
        break;
      }
    }
    if( isKeep ){
      if( doPrint ) ATH_MSG_DEBUG("ParticleMask isKeep " <<uid);
      partMask[pItr->index()] = true;
      int nkids =  getDescendants( pItr, kids );
      for(int i=0; i<nkids; ++i){
        if( doPrint ) ATH_MSG_DEBUG("ParticleMask isKeep kid " 
                                    <<uid <<" " <<HepMC::uniqueID(kids[i]));
        partMask[ (kids[i])->index() ] = true;
        const xAOD::TruthVertex* v = (kids[i])->prodVtx();
        if( v ) vertMask[ v->index() ] = true;
      }
    }

    // Delta(R) matches to hard truth leptons/photons
    if( !pLepGam.empty() && pItr->pt() > m_isolPtCut ){
      TLorentzVector pp4 = pItr->p4();
      for(unsigned int lep=0; lep<pLepGam.size(); ++lep){
        double r = pp4.DeltaR( pLepGam[lep] );
        if( r < m_isolR ){
          if( doPrint ) ATH_MSG_DEBUG("ParticleMask isol " <<uid);
          partMask[ pItr->index() ] = true;
        }
      }
    }
  }


  // Retrieve optional jets
  // Add particles that are constituents of selected jets using unique IDs.
  // Is index() for JetConstituentVector or TruthParticleContainer or??

  if( !m_truthJetsKey.empty() ){

    SG::ReadHandle<xAOD::JetContainer> inJets(m_truthJetsKey, ctx);
    if (!inJets.isValid()) {
      ATH_MSG_ERROR("Cannot retrieve JetContainer " << m_truthJetsKey);
      return StatusCode::FAILURE;
    }

    std::vector<int> uidJetConst;

    for(const auto* ajet : *inJets){
      if( ajet->pt() < m_jetPtCut ) continue;
      if( std::abs(ajet->eta()) > m_jetEtaCut ) continue;

      xAOD::JetConstituentVector aconst = ajet->getConstituents();
      xAOD::JetConstituentVector::iterator aItr = aconst.begin();
      xAOD::JetConstituentVector::iterator aItrE = aconst.end();
      for( ; aItr != aItrE; ++aItr){
        const xAOD::JetConstituent* aip = (*aItr);
        const xAOD::IParticle* aipraw = aip->rawConstituent();
        const xAOD::TruthParticle* pp = 
        dynamic_cast<const xAOD::TruthParticle*>(aipraw);
        if( pp ) {
          if( pp->pt()>m_jetConstPtCut && !MC::isPhoton(pp) ){
            uidJetConst.push_back( HepMC::uniqueID(pp) );
          }
          if( pp->pt()>m_jetPhotonPtCut && !MC::isPhoton(pp) ){
            uidJetConst.push_back( HepMC::uniqueID(pp) );
          }
        } else {
          ATH_MSG_WARNING("Bad cast for particle in jet " <<ajet->index());
        }
      }
    }

    for (const auto* pItr : *inTruthParts) {
      int uid = HepMC::uniqueID(pItr);
      bool isJet = false;
      for(unsigned int i=0; i<uidJetConst.size(); ++i){
        if( uid == uidJetConst[i] ){
          isJet = true;
          break;
        }
      }
      if( isJet ){
        if( doPrint ) ATH_MSG_DEBUG("ParticleMask isJet " <<uid);
        partMask[ pItr->index() ] = true;
      }
    }

  } //end optional jets


  // Execute the thinning service based on the mask. Finish.
  inTruthParts.keep (partMask);
  inTruthVerts.keep (vertMask);

  // Final statistics
  int outPartNum = 0;
  for(unsigned int i=0; i<partMask.size(); ++i){
    if( partMask[i] ) ++outPartNum;
  }
  int outVertNum = 0;
  for(unsigned int i=0; i<vertMask.size(); ++i){
    if( vertMask[i] ) ++outVertNum;
  }

  ATH_MSG_DEBUG("Final particles/vertices = " <<outPartNum <<" " <<outVertNum);

  if( doPrint ){
    std::cout <<"======================================================================================" <<std::endl;
    std::cout <<"HardTruthThinning complete for event " <<evtNum <<std::endl;
    std::cout <<"Saved " <<outPartNum <<" particles" <<std::endl;
    std::cout <<"Particle unique IDs = ";
    for(unsigned int i=0; i<partMask.size(); ++i){
      if( partMask[i] ) std::cout << HepMC::uniqueID((*inTruthParts)[i]) <<" ";
    }
    std::cout <<std::endl;

    std::cout <<"Saved " <<outVertNum <<" vertices" <<std::endl;
    std::cout <<"Vertex unique IDs = ";
    for(unsigned int i=0; i<vertMask.size(); ++i){
      if( vertMask[i] ) std::cout << HepMC::uniqueID((*inTruthVerts)[i]) <<" ";
    }
    std::cout <<std::endl;
    std::cout <<"======================================================================================" <<std::endl;
  }

  return StatusCode::SUCCESS;

}


////////////////////////////////////////////////////////////
// Utility functions
////////////////////////////////////////////////////////////

// Emulate HepMC descendant iterator
// Multiple particles can give same descendant (string/cluster)
// Remove Geant descendants
// MUST check ElementLink validity with isValid() for thinned samples

int DerivationFramework::HardTruthThinning::getDescendants(
              const xAOD::TruthParticle* p,
              std::vector<const xAOD::TruthParticle*>& descendants ) {
  descendants.clear();
  if( ! (p->hasDecayVtx()) ) return 0;
  const xAOD::TruthVertex* dvtx = p->decayVtx();
  if( !dvtx ) return 0;
  if( dvtx->nOutgoingParticles() == 0 ) return 0;
  if(HepMC::is_simulation_vertex(dvtx)) return 0;
  const std::vector< ElementLink< xAOD::TruthParticleContainer > >& outPart =
  dvtx->outgoingParticleLinks();
  for(unsigned int k=0; k<outPart.size(); ++k){
    if( ! (outPart[k]).isValid() ) continue;
    const xAOD::TruthParticle* kid = *(outPart[k]);
    descendants.push_back(kid);
  }

  int nstart = 0;
  int nstop = descendants.size();

  while( nstop > nstart ){
    for(int i=nstart; i<nstop; ++i){
      const xAOD::TruthParticle* pp = descendants[i];
      if( ! (pp->hasDecayVtx()) ) continue;
      const xAOD::TruthVertex* vpp = pp->decayVtx();
      if( !vpp ) continue;
      if( vpp->nOutgoingParticles() == 0 ) continue;
      if( HepMC::is_simulation_vertex(vpp)) continue;
      const std::vector< ElementLink< xAOD::TruthParticleContainer > >&
      outPart2 = vpp->outgoingParticleLinks();
      for(unsigned int k=0; k<outPart2.size(); ++k){
        if( ! (outPart2[k]).isValid() ) continue;
        const xAOD::TruthParticle* kpp = *(outPart2[k]);
        if( HepMC::is_simulation_particle(kpp)) continue;
        bool isIn = false;
        for(unsigned int kk=0; kk<descendants.size(); ++kk){
          if(kpp==descendants[kk]) isIn = true;
        }
        if( !isIn ) descendants.push_back(kpp);
      }
    }
    nstart = nstop;
    nstop = descendants.size();
  }

  return nstop;
}

// Print xAODTruth Event. The printout is particle oriented, unlike
// the HepMC particle/vertex printout. Geant and pileup particles are
// omitted.

void DerivationFramework::HardTruthThinning::printxAODTruth(long long evnum,
                          const xAOD::TruthParticleContainer* truths) {

  std::vector<int> uidPars;
  std::vector<int> uidKids;

  std::cout <<"======================================================================================" <<std::endl;
  std::cout <<"xAODTruth Event " <<evnum <<std::endl;
  std::cout <<"   Unique ID    PDG Id  Status   px(GeV)   py(GeV)   pz(GeV)    E(GeV)   Parent: Decay" <<std::endl;
  std::cout <<"   -----------------------------------------------------------------------------------" <<std::endl;

  for (const auto* tpItr : *truths) {
    if (HepMC::is_simulation_particle(tpItr)) continue;
    int uid = HepMC::uniqueID(tpItr);
    int id = tpItr->pdgId();
    int stat = tpItr->status();
    float px = tpItr->px()/GeV;
    float py = tpItr->py()/GeV;
    float pz = tpItr->pz()/GeV;
    float e = tpItr->e()/GeV;
    uidPars.clear();
    uidKids.clear();

    if( tpItr->hasProdVtx() ){
      const xAOD::TruthVertex* pvtx = tpItr->prodVtx();
      if( pvtx ){
        const std::vector< ElementLink< xAOD::TruthParticleContainer > >& pars =
        pvtx->incomingParticleLinks();
        for(unsigned int k=0; k<pars.size(); ++k){
          if( ! (pars[k]).isValid() ) continue;
          const xAOD::TruthParticle* par = *(pars[k]);
          uidPars.push_back(HepMC::uniqueID(par));
        }
      }
    }
    if( tpItr->hasDecayVtx() ){
      const xAOD::TruthVertex* dvtx = tpItr->decayVtx();
      if( dvtx ){
        const std::vector< ElementLink< xAOD::TruthParticleContainer > >& kids =
        dvtx->outgoingParticleLinks();
        for(unsigned int k=0; k<kids.size(); ++k){
          if( ! (kids[k]).isValid() ) continue;
          const xAOD::TruthParticle* kid = *(kids[k]);
          uidKids.push_back(HepMC::uniqueID(kid));
        }
      }
    }

    std::cout <<std::setw(10)<<uid <<std::setw(12)<<id
              <<std::setw(8)<<stat
              <<std::setprecision(2)<<std::fixed
              <<std::setw(10)<<px <<std::setw(10)<<py
              <<std::setw(10)<<pz <<std::setw(10)<<e <<"   ";
    std::cout <<"P: ";
    for(unsigned int k=0; k<uidPars.size(); ++k){
      std::cout <<uidPars[k] <<" ";
    }
    std::cout <<"  D: ";
    for(unsigned int k=0; k<uidKids.size(); ++k){
      std::cout <<uidKids[k] <<" ";
    }
    std::cout <<std::endl;
  }
  std::cout <<"======================================================================================" <<std::endl;
}
