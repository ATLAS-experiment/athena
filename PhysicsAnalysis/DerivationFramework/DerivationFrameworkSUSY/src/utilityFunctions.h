/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef DerivationFramework_utilityFunctions_h
#define DerivationFramework_utilityFunctions_h

#include <array>
namespace DerivationFramework{

  enum SUSYCountIndices{
    chi01, chi02, chi03, chi04, ch1plus, ch1minus, ch2plus, ch2minus, gluino, squark,
    antisquark, sbottom, stop, sbottom2, stop2, antisbottom, antistop, antisbottom2, 
    antistop2, selecRminus, selecRplus, selecLminus, selecLplus, selnuL, 
    smuonRminus, smuonRplus, smuonLminus, smuonLplus, smunuL, stau1minus, stau1plus,
    stau2minus, stau2plus, staunuL, unattributed, nParticleIndices
  };

  
  unsigned int 
  finalStateID(const int SUSY_Spart1_pdgId, const int SUSY_Spart2_pdgId);
  
  int
  gluinoSquarkClassification(const std::array<int, nParticleIndices> & c);
  
  int
  gauginoPairProduction(const std::array<int, nParticleIndices> & c);
  
  int
  slepton(const std::array<int, nParticleIndices> & c);
  
  int 
  smuon(const std::array<int, nParticleIndices> & c);

}

#endif
