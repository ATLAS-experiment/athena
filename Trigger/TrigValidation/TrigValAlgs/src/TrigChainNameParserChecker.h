/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGVALALGS_TRIGCHAINAMEPARSERCHECKER_H
#define TRIGVALALGS_TRIGCHAINAMEPARSERCHECKER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "TrigConfData/HLTMenu.h"

/**
 * @brief Checks the output of the ChainNameParser against the current menu.
 * Algorithm to be run in validation jobs where it will run the checks on the first event.
 * This validation preserves the functionality of the ChainNameParser when new chains are added to the menu.
 * This is important as the ChainNameParser is used in analysis contexts to ascertain trigger chain 
 * structure (i.e. the number of legs in the chain, their multiplicity requirement, and feature type).
 * when performing analysis level trigger matching. 
 **/
class TrigChainNameParserChecker : public AthReentrantAlgorithm {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual StatusCode initialize() override;
  virtual StatusCode execute (const EventContext& ctx) const override;

  SG::ReadHandleKey<TrigConf::HLTMenu> m_HLTMenuKey{this, "HLTTriggerMenu", "DetectorStore+HLTTriggerMenu", "HLT Menu key"};
};

#endif // TRIGVALALGS_TRIGCHAINAMEPARSERCHECKER_H
