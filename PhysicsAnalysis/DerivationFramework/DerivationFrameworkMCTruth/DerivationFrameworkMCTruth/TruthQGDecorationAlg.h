/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TRUTHQGDECORATIONALG_H
#define DERIVATIONFRAMEWORK_TRUTHQGDECORATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

// Read/decor handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

// xAOD containers
#include "xAODJet/JetContainer.h"

// STL includes
#include <string>

namespace DerivationFramework {

  class TruthQGDecorationAlg : public AthReentrantAlgorithm {
  public:

    using AthReentrantAlgorithm::AthReentrantAlgorithm;

    StatusCode initialize();
    virtual StatusCode execute(const EventContext& ctx) const;

  private:
    /// input collection key
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey
    {this, "JetCollection", "AntiKt4TruthWZJets", "Name of jet collection for decoration"};
    /// output decoration
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_decOutput
      {this, "TrueFlavor", m_jetsKey, "TrueFlavor", "Name of the output decoration on the jet"};
  };
}

#endif // DERIVATIONFRAMEWORK_TRUTHQGDECORATIONALG_H
