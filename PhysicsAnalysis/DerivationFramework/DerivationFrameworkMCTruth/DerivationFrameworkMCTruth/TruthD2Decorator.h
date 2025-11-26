/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHD2DECORATOR_H
#define DERIVATIONFRAMEWORK_TRUTHD2DECORATOR_H

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODJet/JetContainer.h"

namespace DerivationFramework {

  class TruthD2Decorator : public extends<AthAlgTool, IAugmentationTool> {
  public:

    using base_class::base_class;

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainerKey
    {this, "JetContainerKey", "AntiKt10TruthTrimmedPtFrac5SmallR20Jets", "Name of jet container key for input"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_decorationName
      {this, "DecorationName", m_jetContainerKey, "D2", "Decoration Name"};
  };
}

#endif // DERIVATIONFRAMEWORK_TruthD2Decorator_H
