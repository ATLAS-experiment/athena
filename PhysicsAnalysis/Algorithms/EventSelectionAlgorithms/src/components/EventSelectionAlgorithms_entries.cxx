/*
   Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

DECLARE_COMPONENT (CP::ChargeSelectorAlg)
DECLARE_COMPONENT (CP::MissingETPlusTransverseMassSelectorAlg)
DECLARE_COMPONENT (CP::MissingETSelectorAlg)
DECLARE_COMPONENT (CP::DileptonInvariantMassSelectorAlg)
DECLARE_COMPONENT (CP::DileptonInvariantMassWindowSelectorAlg)
DECLARE_COMPONENT (CP::TransverseMassSelectorAlg)
DECLARE_COMPONENT (CP::SaveFilterAlg)
DECLARE_COMPONENT (CP::NObjectPtSelectorAlg)
