/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "ParticleJetTools/CopyTruthJetParticles.h"
#include "MCTruthClassifier/MCTruthClassifier.h"
#include "xAODTruth/TruthVertex.h"
#include "TruthUtils/HepMCHelpers.h"
#include "TruthUtils/MagicNumbers.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/TruthEventContainer.h"
#include "AthContainers/ConstDataVector.h"
#include "AsgDataHandles/ReadHandle.h"
#include "AsgDataHandles/WriteHandle.h"
#include "AsgMessaging/Check.h"
#include "CxxUtils/checker_macros.h"

#include <mutex>          // std::call_once, std::once_flag

#ifndef XAOD_STANDALONE
// Usage of metadata is for now only possible in Athena...
//#include "CoralBase/AttributeListException.h"
#include "AthAnalysisBaseComps/AthAnalysisHelper.h"
#endif

// For std::find in comesFrom()
#include <algorithm>

using namespace std;
using namespace MCTruthPartClassifier;

CopyTruthJetParticles::CopyTruthJetParticles(const std::string& name)
  : CopyTruthParticles(name) {}
StatusCode CopyTruthJetParticles::initialize() {
  ATH_CHECK(m_classif.retrieve());

  ATH_CHECK(m_truthEventKey.initialize());
  ATH_CHECK(m_outTruthPartKey.initialize());

  return StatusCode::SUCCESS;
}



bool CopyTruthJetParticles::classifyJetInput(const xAOD::TruthParticle* tp, 
                                             std::vector<const xAOD::TruthParticle*>& promptLeptons,
                                             std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const {

  // Check if this thing is a candidate to be in a truth jet
  //  First block is largely copied from isGenStable, which works on HepMC only
  if (HepMC::is_simulation_particle(tp)) return false; // Particle is from G4
  int pdgid = tp->pdgId();
  if (MC::isZeroEnergyPhoton(tp)) return false; // Work around for an old generator bug

  // -- changed for dark jet clustering -- //
  if ( !MC::isStable(tp) && !m_includeDark ) return false; // dark hadrons will not be status 1
  // ----------------------------------- //
  
  // Easy classifiers by PDG ID
  if(MC::isNeutrino(pdgid)) {
    if (!m_includeNu) return false;
  } else {
    if (!m_includeBSMNonInt && !MC::isInteracting(pdgid)) return false;
  }
  if (!m_includeMu && MC::isMuon(pdgid)) return false;

  // Already built a list of prompt leptons, just use it here
  if (!m_includePromptLeptons && std::find(promptLeptons.begin(),promptLeptons.end(),tp)!=promptLeptons.end()){
    ATH_MSG_VERBOSE("Veto prompt lepton (" << pdgid << ") with pt " << tp->pt());
    return false;
  }

  // Extra catch.  If we aren't supposed to include prompt leptons, we aren't supposed to include prompt neutrinos
  unsigned int tc_res = getTCresult(tp, tc_results);
  if (!m_includePromptLeptons && MC::isNeutrino(pdgid) && MCTruthClassifier::isPrompt(tc_res)) {
    return false;
  }

  // -- added for dark jet clustering -- //
  // new classifiers to account for dark particles
  // for dark jets: ignore SM particles; include only "stable" dark hadrons
  if (!m_includeSM && ((abs(tp->pdgId()) < 4.9e6) || (abs(tp->pdgId()) >= 5e6))) return false;
  if (m_includeDark) {
    if (abs(tp->pdgId()) <= 4900101) return false; // ignore Xd, qd, gd
    if (tp->hasDecayVtx() && (abs(tp->child()->pdgId()) >= 4.9e6)) return false; // ignore "non-stable" dark hadrons (decaying to dark sector) -- "stable" if decaying to SM
  }
  // for SM jets: ignore dark particles - probably unnecessary bc of status requirement above
  if (!m_includeDark && (std::abs(tp->pdgId()) >= 4.9e6) && (std::abs(tp->pdgId()) < 5e6)) return false;
  // ----------------------------------- //

  if (!m_includePromptPhotons && MC::isPhoton(pdgid) && tp->hasProdVtx()){
    //ParticleOrigin orig = getPartOrigin(tp, originMap);
    //if (orig==Higgs || orig==HiggsMSSM) return false;
    if (MCTruthClassifier::isPrompt(tc_res))  return false;
  }

  // If we want to remove photons via the dressing decoration
  if (!m_dressingName.empty()){
    // Accessor for the dressing decoration above
    const static SG::AuxElement::Accessor<char> dressAcc(m_dressingName);
    if (MC::isPhoton(pdgid) && dressAcc(*tp)) return false;
  } // End of removal via dressing decoration

  // Pseudo-rapidity cut
  if(std::abs(tp->eta())>m_maxAbsEta) return false;

  // Vetoes of specific origins.  Not fast, but if no particles are specified should not execute
  if (m_vetoPDG_IDs.size()>0){
    std::vector<int> used_vertices;
    for (int anID : m_vetoPDG_IDs){
      used_vertices.clear();
      if (comesFrom(tp,anID,used_vertices)) return false;
    }
  }

  // Made it! 
  return true;
}


unsigned int CopyTruthJetParticles::getTCresult(const xAOD::TruthParticle* tp,
                                                std::map<const xAOD::TruthParticle*,unsigned int>& tc_results) const
{
  if(tc_results.find(tp) == tc_results.end()) {
    const unsigned int result = tp ? std::get<0>(MCTruthPartClassifier::defOrigOfParticle(tp)) : 0;
    tc_results[tp] = result;
  }
  return tc_results[tp];
}

int CopyTruthJetParticles::setBarCodeFromMetaDataCheck() const {

    // Usage of metadata is only possible in Athena (not supported by dual-use tools yet)...
#ifndef XAOD_STANDALONE
    bool found = false;
    // retrieve the value for the current sample from metadata
    
    int barcodeOffset_tmp(0);
    ATH_MSG_INFO("Look for barcode offset in  metadata ... ");
    try {
      StatusCode sc= AthAnalysisHelper::retrieveMetadata("/Simulation/Parameters","SimBarcodeOffset",barcodeOffset_tmp) ;
      found = sc.isSuccess();
    } catch(std::exception &e) {
      ATH_MSG_DEBUG(" Could not retrieve barcode offset in metadata  : "<< e.what());
    }
    if (found) {    
      if (HepMC::SIM_BARCODE_THRESHOLD!=barcodeOffset_tmp) ATH_MSG_WARNING(" Barcode offset found in metadata. Its value is :  "<< barcodeOffset_tmp << "vs the used "<< HepMC::SIM_BARCODE_THRESHOLD);
    }
#else // standalone :
    ATH_MSG_WARNING("Can't retrieve automatically the truth barcode offset outside Athena.  Falling back to offset property: " << HepMC::SIM_BARCODE_THRESHOLD);
#endif
  return 0;
}

int CopyTruthJetParticles::execute() const {

  // retrieve barcode Offset for this event from metadata.
  // We'd need a cleaner solution where this offset is set only at 
  // each new file, but this requires some tool interface which does 
  // not exist in RootCore yet. 
  // So we use the less disruptive solution in Athena for now...

  // the function used below is 
   // std::call_once(metaDataFlag,basicMetaDataCheck(), this);
  //  std::call_once(metaDataFlag,this->basicMetaDataCheck());
  // the syntax is explained in http://stackoverflow.com/questions/23197333/why-is-this-pointer-needed-when-calling-stdcall-once
  // this syntax requires the call_once function to receive the object the function is called upon
  //": these are all non-static member functions , they act on objects 
  // they need the "this" pointer which always point to the object the function is working on
  //http://www.learncpp.com/cpp-tutorial/812-static-member-functions/ 


  //  std::call_once(metaDataFlag,&CopyTruthJetParticles::basicMetaDataCheck,this,barcodeOffset);
  // this call happens only once and it modifies m_barcodeOffset
  // Note that catching the return value of this is rather complicated, so it throws rather than returning errors
  static std::once_flag metaDataFlag;
  std::call_once(metaDataFlag,[&]() {setBarCodeFromMetaDataCheck();});

  std::vector<const xAOD::TruthParticle*> promptLeptons;
  promptLeptons.reserve(10);

  // Retrieve the xAOD truth objects
  auto truthEvents = SG::makeHandle(m_truthEventKey);
  if ( !truthEvents.isValid() ) {
    ATH_MSG_ERROR("Failed to retrieve truth event container " << m_truthEventKey.key());
    return 1;
  }
  
  // Classify particles for tagging and add to the TruthParticleContainer
  std::unique_ptr<ConstDataVector<xAOD::TruthParticleContainer> > ptruth(new ConstDataVector<xAOD::TruthParticleContainer>(SG::VIEW_ELEMENTS));
  std::map<const xAOD::TruthParticle*,unsigned int> tc_results;
  tc_results.clear();
  size_t numCopied = 0;
  const xAOD::TruthEvent* hsevt = truthEvents->front();
  if(!hsevt) {
    ATH_MSG_ERROR("Null pointer received for first truth event!");
    return 1;
  }

  for (unsigned int ip = 0; ip < hsevt->nTruthParticles(); ++ip) {
    const xAOD::TruthParticle* tp = hsevt->truthParticle(ip);
    if(!tp) continue;
    if (tp->pt() < m_ptmin)
        continue;
    // Cannot use the truth helper functions; they're written for HepMC
    // Last two switches only apply if the thing is a lepton and not a tau
    int pdgid = tp->pdgId();
    if ((std::abs(pdgid)==11 || std::abs(pdgid)==13) && tp->hasProdVtx()){
      // If this is a prompt, generator stable lepton, then we can use it
      if(MC::isStable(tp) && !HepMC::is_simulation_particle(tp) && MCTruthClassifier::isPrompt(getTCresult(tp, tc_results))) {
        promptLeptons.push_back(tp);
      }
    }
  }

  for (size_t itp(0); itp<hsevt->nTruthParticles(); ++itp) {
    const xAOD::TruthParticle* tp = hsevt->truthParticle(itp);
    if(!tp) continue;
    if (tp->pt() < m_ptmin)
        continue;

    if (classifyJetInput(tp, promptLeptons, tc_results)) { 
      ptruth->push_back(tp);
      numCopied += 1;
    }
  }

  ATH_MSG_DEBUG("Copied " << numCopied << " truth particles into " << m_outTruthPartKey.key() << " TruthParticle container");

  // record
  auto truthParticles_out = SG::makeHandle(m_outTruthPartKey);
  ATH_MSG_DEBUG("Recorded truth particle collection " << m_outTruthPartKey.key());
  // notify
  if (!truthParticles_out.put(std::move(ptruth))) {
    ATH_MSG_ERROR("Unable to write new TruthParticleContainer to event store: " 
                  << m_outTruthPartKey.key());
  } else {
    ATH_MSG_DEBUG("Created new TruthParticleContainer in event store: " 
                  << m_outTruthPartKey.key());
  }

  return 0;
}


bool CopyTruthJetParticles::comesFrom( const xAOD::TruthParticle* tp, const int pdgID, std::vector<int>& used_vertices ) const {
  // If it's not a particle, then it doesn't come from something...
  if (!tp) return false;
  // If it doesn't have a production vertex or has no parents, it doesn't come from much of anything
  if (!tp->prodVtx() || tp->nParents()==0) return false;
  // If we have seen it before, then skip this production vertex
  if (std::find(used_vertices.begin(),used_vertices.end(), tp->prodVtx()->barcode())!=used_vertices.end()) return false;
  // Add the production vertex to our used list
  used_vertices.push_back( tp->prodVtx()->barcode() );
  // Loop over the parents
  for (size_t par=0;par<tp->nParents();++par){
    // Check for null pointers in case of skimming
    if (!tp->parent(par)) continue;
    // Check for a match
    if (tp->parent(par)->absPdgId()==pdgID) return true;
    // Recurse on this parent
    if (comesFrom(tp->parent(par), pdgID, used_vertices)) return true;
  }
  // No hits -- all done with the checks!
  return false;
}

