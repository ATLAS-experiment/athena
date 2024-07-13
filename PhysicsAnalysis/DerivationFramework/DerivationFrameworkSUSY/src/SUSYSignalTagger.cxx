/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "DerivationFrameworkSUSY/SUSYSignalTagger.h"

#include "xAODEventInfo/EventInfo.h"

#include "TruthUtils/MagicNumbers.h"
#include "PdgConditional.h"
#include "utilityFunctions.h"
#include <array>
#include <algorithm>



namespace DerivationFramework {

  static const SG::AuxElement::Decorator<int> dec_procID("SUSY_procID");
  static const SG::AuxElement::Decorator<int> dec_pdgId1("SUSY_pid1");
  static const SG::AuxElement::Decorator<int> dec_pdgId2("SUSY_pid2");
    
  SUSYSignalTagger::SUSYSignalTagger(const std::string& t, const std::string& n, const IInterface* p):
    AthAlgTool(t,n,p){
    declareInterface<DerivationFramework::IAugmentationTool>(this);
  }  
  
  StatusCode SUSYSignalTagger::addBranches() const{
    const xAOD::EventInfo* eventInfo;
    if (evtStore()->retrieve(eventInfo,m_eventInfoName).isFailure()) {
      ATH_MSG_ERROR("could not retrieve event info " <<m_eventInfoName);
      return StatusCode::FAILURE;
    }
    
    const xAOD::TruthParticleContainer* truthPC = 0;
    if (evtStore()->retrieve(truthPC,m_mcName).isFailure()) {
      ATH_MSG_DEBUG("WARNING could not retrieve TruthParticleContainer " <<m_mcName);
      return StatusCode::FAILURE;
    }
    //Identify SUSY hard proc
    int pdgId1(0);
    int pdgId2(0);
    bool found = FindSusyHardProc( truthPC, pdgId1, pdgId2);
    if (!found) {
      ATH_MSG_WARNING("could not identify SUSY process! ");
      dec_procID(*eventInfo) = 0;
      dec_pdgId1(*eventInfo) = -99;
      dec_pdgId2(*eventInfo) = -99;
      return StatusCode::SUCCESS;
    }
    //Get SUSY proc ID 
    unsigned int procID = finalStateID(pdgId1, pdgId2);
    if (procID == 0)  ATH_MSG_WARNING("could not identify SUSY procID! ");
    dec_procID(*eventInfo) = procID;
    dec_pdgId1(*eventInfo) = pdgId1;
    dec_pdgId2(*eventInfo) = pdgId2;
    return StatusCode::SUCCESS;
  }


  bool 
  SUSYSignalTagger::FindSusyHardProc(const xAOD::TruthParticleContainer* truthP
  , int& pdgid1, int& pdgid2) const{
    pdgid1 = 0;
    pdgid2 = 0;
    const xAOD::TruthParticle* firstsp(0);
    const xAOD::TruthParticle* secondsp(0);
    if (!truthP || truthP->empty()) {
      return false;
    }
    for (const auto tp : *truthP) {
      //check ifSUSY particle
      if ((std::abs(tp->pdgId()) > 1000000 && std::abs(tp->pdgId()) < 1000007) || // squarkL
          (std::abs(tp->pdgId()) > 1000010 && std::abs(tp->pdgId()) < 1000017) || // sleptonL
          (std::abs(tp->pdgId()) > 2000000 && std::abs(tp->pdgId()) < 2000007) || // squarkR
          (std::abs(tp->pdgId()) > 2000010 && std::abs(tp->pdgId()) < 2000017) || // sleptonR
          (std::abs(tp->pdgId()) > 1000020 && std::abs(tp->pdgId()) < 1000040)) { // gauginos
        if (tp->nParents() != 0) {
          if ( tp->parent(0)->absPdgId()  < 1000000) {
            if (!firstsp) {
              firstsp = tp;
            } else if (!secondsp) {
              secondsp = tp;
            } else {
              if (firstsp->nChildren() != 0 && HepMC::uniqueID(tp) == HepMC::uniqueID(firstsp->child(0))) {
                firstsp = tp;
              }
              else if (secondsp->nChildren() != 0 && HepMC::uniqueID(tp) == HepMC::uniqueID(secondsp->child(0))) {
                secondsp = tp;
              }
              else if (firstsp->nChildren() != 0 && HepMC::uniqueID(firstsp->child(0)) == HepMC::uniqueID(secondsp)) {
                firstsp = secondsp;
                secondsp = tp;
              }
              else if (secondsp->nChildren() != 0 && HepMC::uniqueID(secondsp->child(0)) == HepMC::uniqueID(firstsp)) {
                secondsp = firstsp;
                firstsp = tp;
              }
            }
          }
        }
      }
    }
    // quit if no sparticles found
    if (!firstsp && !secondsp) return false; // should find none or two

    if (firstsp && firstsp->nChildren() == 1) {
      for (const auto tp : *truthP) {
        if (HepMC::uniqueID(tp) == HepMC::uniqueID(firstsp->child(0)) && tp->pdgId() != firstsp->pdgId()) {
          firstsp = tp;
          break;
        }
      }
    }
    if (secondsp && secondsp->nChildren() == 1) {
      for (const auto tp : *truthP) {
        if (HepMC::uniqueID(tp) == HepMC::uniqueID(secondsp->child(0)) && tp->pdgId() != secondsp->pdgId()) {
          secondsp = tp;
          break;
        }
      }
    }
    if (firstsp && abs(firstsp->pdgId()) > 1000000) pdgid1 = firstsp->pdgId();
    if (secondsp && abs(secondsp->pdgId()) > 1000000) pdgid2 = secondsp->pdgId();
    // Return gracefully:
    return true;
  }

} /// namespace
