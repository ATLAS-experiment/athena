/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "../GepCellsHandlerAlg.h"
DECLARE_COMPONENT( GepCellsHandlerAlg )

#include "../GepClusteringAlg.h"
DECLARE_COMPONENT( GepClusteringAlg )

#include "../GepJetAlg.h"
DECLARE_COMPONENT( GepJetAlg )

#include "../GepMETAlg.h"
DECLARE_COMPONENT( GepMETAlg )

// Only the algorithm is a component. The bitwise MET core (Gep::TotalMETMaker) is a
// plain helper class it owns -- the same split as GepJetAlg and Gep::JetTaggerLRJMaker,
// where the maker is likewise not declared here.
#include "../TotalMETAlg.h"
DECLARE_COMPONENT( TotalMETAlg )

#include "../GepMETPufitAlg.h"
DECLARE_COMPONENT( GepMETPufitAlg )

#include "../GepClusterTimingAlg.h"
DECLARE_COMPONENT( GepClusterTimingAlg )

#include "../GepPi0Alg.h"
DECLARE_COMPONENT(GepPi0Alg)

#include "../EMB1CellsFromCaloCells.h"
DECLARE_COMPONENT(EMB1CellsFromCaloCells)

#include "../EMB1CellsFromCaloClusters.h"
DECLARE_COMPONENT(EMB1CellsFromCaloClusters)

#include "../GepCellTowerAlg.h"
DECLARE_COMPONENT(GepCellTowerAlg)

#include "../GepTowersAlg.h"
DECLARE_COMPONENT(GepTowersAlg)

#include "../GepEratioAlg.h"
DECLARE_COMPONENT(GepEMEratioAlg)
DECLARE_COMPONENT(GepTauEratioAlg)

#include "../GepEtaSoftKillerAlg.h"
DECLARE_COMPONENT(GepEtaSoftKillerAlg)