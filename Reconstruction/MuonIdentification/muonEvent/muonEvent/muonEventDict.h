/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONEVENT_MUONEVENTDICT_H
#define MUONEVENT_MUONEVENTDICT_H

#include "Navigation/Navigable.h" // for i686/.../muonEventDict_gen.h
#include "muonEvent/DepositInCalo.h"
#include "muonEvent/CaloEnergy.h"
#include "muonEvent/MuonSpShowerContainer.h"
#include "muonEvent/MuonCaloEnergyContainer.h"

/** @struct MuonDictionary
    @brief  EDM dictionary providing the muon, calo energy and SpShower classes
*/

struct MuonDictionary {

  ElementLink<MuonCaloEnergyContainer> m_linkCal;
  std::pair<ElementLink<MuonCaloEnergyContainer>,double> m_pairCal;
  MuonCaloEnergyContainer mcalCont;
  CaloEnergy mcal;

  ElementLink<Rec::MuonSpShowerContainer> m_linkSho;
  std::pair<ElementLink<Rec::MuonSpShowerContainer>,double> m_pairSho;
  Rec::MuonSpShowerContainer shoCont;
  Rec::MuonSpShower sho;

  std::vector<DepositInCalo*> m_depositVecPointer;

  // Work around dictionary generation problem.
  DataVector<Rec::MuonSpShower> m_dummy3;
  DataVector<int> m_dummy5;
  DataVector<CaloEnergy> m_dummy6;
};


#endif 


