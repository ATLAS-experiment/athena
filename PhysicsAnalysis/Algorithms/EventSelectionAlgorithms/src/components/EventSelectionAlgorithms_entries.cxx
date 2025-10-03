/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina

#include <EventSelectionAlgorithms/ChargeSelectorAlg.h>
#include <EventSelectionAlgorithms/MissingETPlusTransverseMassSelectorAlg.h>
#include <EventSelectionAlgorithms/MissingETSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonInvariantMassSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonInvariantMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/TransverseMassSelectorAlg.h>
#include <EventSelectionAlgorithms/SaveFilterAlg.h>
#include <EventSelectionAlgorithms/NObjectPtSelectorAlg.h>
#include <EventSelectionAlgorithms/NObjectMassSelectorAlg.h>
#include <EventSelectionAlgorithms/NLargeRJetMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/DileptonOSSFInvariantMassWindowSelectorAlg.h>
#include <EventSelectionAlgorithms/SumNLeptonPtSelectorAlg.h>
#include <EventSelectionAlgorithms/JetNGhostSelectorAlg.h>
#include <EventSelectionAlgorithms/RunNumberSelectorAlg.h>

// Project include(s).
#include "AsgTools/AsgComponentFactories.h"

DECLARE_COMPONENT (CP::ChargeSelectorAlg)
DECLARE_COMPONENT (CP::MissingETPlusTransverseMassSelectorAlg)
DECLARE_COMPONENT (CP::MissingETSelectorAlg)
DECLARE_COMPONENT (CP::DileptonInvariantMassSelectorAlg)
DECLARE_COMPONENT (CP::DileptonInvariantMassWindowSelectorAlg)
DECLARE_COMPONENT (CP::TransverseMassSelectorAlg)
DECLARE_COMPONENT (CP::SaveFilterAlg)
DECLARE_COMPONENT (CP::NObjectPtSelectorAlg)
DECLARE_COMPONENT (CP::NObjectMassSelectorAlg)
DECLARE_COMPONENT (CP::NLargeRJetMassWindowSelectorAlg)
DECLARE_COMPONENT (CP::DileptonOSSFInvariantMassWindowSelectorAlg)
DECLARE_COMPONENT (CP::SumNLeptonPtSelectorAlg)
DECLARE_COMPONENT (CP::JetNGhostSelectorAlg)
DECLARE_COMPONENT (CP::RunNumberSelectorAlg)
