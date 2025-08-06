/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigBjetHypo/safeLogRatio.h"

#include <cmath>

float safeLogRatio(float num, float denom) {
  float ratio = (denom == 0 ? INFINITY : num / denom);
  return ratio == 0 ? -INFINITY : std::log( ratio );
}

const xAOD::Jet* getJetFromBTagLink( const SG::AuxElement& btag ) {
   const std::string jetLinkName = "jetLink";
   SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> acc_jetLink(jetLinkName);
   auto jetLink = acc_jetLink(btag);
   if (!jetLink.isValid()) {
   std::cerr << "No jet link found in btagging object" << std::endl;
   return nullptr;
   }
   auto jet = *jetLink;
   return jet;
}