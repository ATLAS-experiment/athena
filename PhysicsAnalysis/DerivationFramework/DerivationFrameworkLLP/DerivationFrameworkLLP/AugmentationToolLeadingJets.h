/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// AugmentationLeadingJets.h, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
#define DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace DerivationFramework {

  class AugmentationToolLeadingJets : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      AugmentationToolLeadingJets(const std::string& t, const std::string& n, const IInterface* p);
      virtual StatusCode initialize() override;
      virtual StatusCode addBranches(const EventContext& ctx) const override;


  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey
      { this, "JetKey", "AntiKt4EMTopoJets", "" };
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_decorationKey
      { this, "DecorationKey", m_jetKey, "DFDecoratorLeadingJets", "" };
  }; 
}

#endif // DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
