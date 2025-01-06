/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GeneratorPhysVal_GeneratorSelector_H
#define GeneratorPhysVal_GeneratorSelector_H


#include <vector>
#include <TLorentzVector.h>
#include "xAODTruth/TruthParticleContainer.h"
#include "TruthUtils/HepMCHelpers.h"

class GeneratorSelector
{
  public: 

  GeneratorSelector() = default;
  ~GeneratorSelector() = default;
  
  enum class CUT_STATUS {YES,NO};
  const std::vector< TLorentzVector > GetGeneratorLevel(const xAOD::TruthParticleContainer *cont)
  {
  std::vector< TLorentzVector > generic_stable_parton;
    
    for(const auto  vcont: *cont){
      if (!(MC::isGenStable(vcont))) continue; 
      TLorentzVector tlv (vcont->px(), vcont->py(), vcont->pz(), vcont->e()); 
      generic_stable_parton.push_back(tlv);
    }

  return generic_stable_parton;
  }

  const std::vector< TLorentzVector > GetSimulationLevel(const xAOD::TruthParticleContainer *cont)
  {
  std::vector< TLorentzVector > generic_stable_parton;
  
    for(const auto  vcont: *cont){
      if (!(MC::isSimStable(vcont))) continue;
      TLorentzVector tlv (vcont->px(), vcont->py(), vcont->pz(), vcont->e());
      generic_stable_parton.push_back(tlv);

    }

  return generic_stable_parton;
  }

};

#endif
