/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "utilityFunctions.h"
#include "PdgConditional.h"
#include <cmath>
#include <algorithm>

namespace{
  enum CountIndices{
    chi01, chi02, chi03, chi04, ch1plus, ch1minus, ch2plus, ch2minus, gluino, squark,
    antisquark, sbottom, stop, sbottom2, stop2, antisbottom, antistop, antisbottom2, 
    antistop2, selecRminus, selecRplus, selecLminus, selecLplus, selnuL, 
    smuonRminus, smuonRplus, smuonLminus, smuonLplus, smunuL, stau1minus, stau1plus,
    stau2minus, stau2plus, staunuL, unattributed, nParticleIndices
  };
}
  
namespace DerivationFramework{
  int
  gluinoSquarkClassification(const std::array<int, 35> & c){
    int result{};
    // gluino/squark + X
    if (c[gluino]==1 && (c[squark]==1 || c[antisquark] ==1)) return 1;
    else if (c[gluino]==2) return 2;
    else if (c[squark]==2 || c[antisquark]==2) return 3;
    else if (c[squark]==1 && c[antisquark]==1) return 4;

    else if (c[sbottom]==1 && c[antisbottom]==1) return 51;
    else if (c[sbottom2]==1 && c[antisbottom2]==1) return 52;
    else if (c[stop]==1 && c[antistop]==1) return 61;
    else if (c[stop2]==1 && c[antistop2]==1) return 62;

    else if (c[gluino]==1 && c[chi01]==1) return 71;
    else if (c[gluino]==1 && c[chi02]==1) return 72;
    else if (c[gluino]==1 && c[chi03]==1) return 73;
    else if (c[gluino]==1 && c[chi04]==1) return 74;

    else if (c[gluino]==1 && c[ch1plus]==1) return 75;
    else if (c[gluino]==1 && c[ch2plus]==1) return 76;
    else if (c[gluino]==1 && c[ch1minus]==1) return 77;
    else if (c[gluino]==1 && c[ch2minus]==1) return 78;

    else if ((c[squark]==1 || c[antisquark ]==1) && c[chi01]==1) return 81;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[chi02]==1) return 82;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[chi03]==1) return 83;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[chi04]==1) return 84;

    else if ((c[squark]==1 || c[antisquark ]==1) && c[ch1plus]==1) return 85;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[ch2plus]==1) return 86;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[ch1minus]==1) return 87;
    else if ((c[squark]==1 || c[antisquark ]==1) && c[ch2minus]==1) return 88;
    return result;
  }
  
  int
  gauginoPairProduction(const std::array<int, 35> & c){
  // Gaugino pair-production
    // chi^{0}_1 + X
    int result{};
    if (c[chi01]==2) return 111;
    else if (c[chi01]==1 && c[chi02]==1) return 112;
    else if (c[chi01]==1 && c[chi03]==1) return 113;
    else if (c[chi01]==1 && c[chi04]==1) return 114;
    else if (c[chi01]==1 && c[ch1plus]==1) return 115;
    else if (c[chi01]==1 && c[ch2plus]==1) return 116;
    else if (c[chi01]==1 && c[ch1minus]==1) return 117;
    else if (c[chi01]==1 && c[ch2minus]==1) return 118;

    // chi^{0}_2 + X
    else if (c[chi02]==2) return 122;
    else if (c[chi02]==1 && c[chi03]==1) return 123;
    else if (c[chi02]==1 && c[chi04]==1) return 124;
    else if (c[chi02]==1 && c[ch1plus]==1) return 125;
    else if (c[chi02]==1 && c[ch2plus]==1) return 126;
    else if (c[chi02]==1 && c[ch1minus]==1) return 127;
    else if (c[chi02]==1 && c[ch2minus]==1) return 128;

    // chi^{0}_3 + X
    else if (c[chi03]==2) return 133;
    else if (c[chi03]==1 && c[chi04]==1) return 134;
    else if (c[chi03]==1 && c[ch1plus]==1) return 135;
    else if (c[chi03]==1 && c[ch2plus]==1) return 136;
    else if (c[chi03]==1 && c[ch1minus]==1) return 137;
    else if (c[chi03]==1 && c[ch2minus]==1) return 138;

    // chi^{0}_4 + X
    else if (c[chi04]==2) return 144;
    else if (c[chi04]==1 && c[ch1plus]==1) return 145;
    else if (c[chi04]==1 && c[ch2plus]==1) return 146;
    else if (c[chi04]==1 && c[ch1minus]==1) return 147;
    else if (c[chi04]==1 && c[ch2minus]==1) return 148;

    // chi^{+}_1/2 + chi^{-}_1/2
    else if (c[ch1plus]==1 && c[ch1minus]==1) return 157;
    else if (c[ch1plus]==1 && c[ch2minus]==1) return 158;

    else if (c[ch2plus]==1 && c[ch1minus]==1) return 167;
    else if (c[ch2plus]==1 && c[ch2minus]==1) return 168;
    return result;
  }
  int
  slepton(const std::array<int, 35> & c){
    int result{};
     // slepton
    if (c[selecLplus]==1 && c[selecLminus]==1) return 201; // sElectronLPair
    else if (c[selecRplus]==1 && c[selecRminus]==1) return 202; // sElectronRPair
    else if (c[selnuL]==2) return 203; // sElectron neutrino pair
    else if (c[selecLplus]==1 && c[selnuL]==1) return 204;  // sElectron+ sNutrino
    else if (c[selecLminus]==1 && c[selnuL]==1) return 205; // sElectron- sNutrino
    else if (c[stau1plus]==1 && c[stau1minus]==1) return 206;
    else if (c[stau2plus]==1 && c[stau2minus]==1) return 207;
    else if ((c[stau1plus]==1 || c[stau1minus]==1) && (c[stau2plus]==1 || c[stau2minus]==1)) return 208;
    else if (c[staunuL]==2) return 209; // sTau neutrino pair
    else if (c[stau1plus]==1 && c[staunuL]==1) return 210;
    else if (c[stau1minus]==1 && c[staunuL]==1) return 211;
    else if (c[stau2plus]==1 && c[staunuL]==1) return 212;
    else if (c[stau2minus]==1 && c[staunuL]==1) return 213;
    return result;
  }
  int 
  smuon(const std::array<int, 35> & c){
    int result{};
    if (c[smuonLplus]==1 && c[smuonLminus]==1) return 216; // sMuonPair
    else if (c[smuonRplus]==1 && c[smuonRminus]==1) return 217; // sMuonPair
    else if (c[smunuL]==2) return 218; // sMuon neutrino pair
    else if (c[smuonLplus]==1 && c[smunuL]==1) return 219;  // sMuon+ sNutrino
    else if (c[smuonLminus]==1 && c[smunuL]==1) return 220; // sMuon- sNutrino
    return result;
  }
  
  
  unsigned int 
  finalStateID(const int SUSY_Spart1_pdgId, const int SUSY_Spart2_pdgId){
    std::array<int, nParticleIndices> particleCountByType{};
    std::array<PdgConditional,34> conditions{
      unsigned(1000022), //unsigned number does an std::abs before comparison
      unsigned(1000023),
      unsigned(1000025),
      unsigned(1000035),
      1000024, //signed int does a straight equality comparison
      -1000024,
      1000037,
      -1000037,
      1000021,
      //insert a lambda to do the comparison
      PdgConditional([](int pdgId)->bool{return ((std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004)) && (pdgId>0);}),
      PdgConditional([](int pdgId)->bool{return (std::abs(pdgId)>1000000 && std::abs(pdgId)<= 1000004) || (std::abs(pdgId)>2000000 && std::abs(pdgId)<=2000004);}),
      1000005,
      1000006,
      2000005,
      2000006,
      -1000005,
      -1000006,
      -2000005,
      -2000006,
      2000011,
      -2000011,
      1000011,
      -1000011,
      unsigned(1000012),
      2000013,
      -2000013,
      1000013,
      -1000013,
      unsigned(1000014),
      1000015,
      -1000015,
      2000015,
      -2000015,
      unsigned(1000016)
    };
    auto it = std::find(conditions.begin(),conditions.end(), SUSY_Spart1_pdgId);
    int idx = std::distance(conditions.begin(), it);
    ++particleCountByType[idx];
    //
    it = std::find(conditions.begin(),conditions.end(), SUSY_Spart2_pdgId);
    idx = std::distance(conditions.begin(), it);
    ++particleCountByType[idx];
    if (const auto & v=gluinoSquarkClassification(particleCountByType);v) return v;
    // Gaugino pair-production
    // chi^{0}_1 + X
    if (const auto & v=gauginoPairProduction(particleCountByType);v) return v;
    // slepton
    if (const auto & v=slepton(particleCountByType);v) return v;
    // smuon
    if (const auto & v=smuon(particleCountByType);v) return v;
    return 0;
  }






}
