/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
#define DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H

#include <string>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "xAODJet/JetContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

namespace DerivationFramework {

  class AugmentationToolLeadingJets : public AthReentrantAlgorithm { // FIXME RENAME
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetKey
      { this, "JetKey", "AntiKt4EMTopoJets", "" };
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_decorationKey
      { this, "DecorationKey", m_jetKey, "DFDecoratorLeadingJets", "" };
  };
}

#endif // DERIVATIONFRAMEWORK_AUGMENTATIONTOOLLEADINGJETS_H
