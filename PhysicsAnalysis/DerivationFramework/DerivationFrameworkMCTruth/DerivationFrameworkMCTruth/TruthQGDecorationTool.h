/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DERIVATIONFRAMEWORK_TruthQGDecorationTool_H
#define DERIVATIONFRAMEWORK_TruthQGDecorationTool_H

// Interface classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"

// Read/decor handle keys
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"

// xAOD containers
#include "xAODJet/JetContainer.h"

// STL includes
#include <string>

namespace DerivationFramework {

  class TruthQGDecorationTool : public extends<AthAlgTool, IAugmentationTool> {
    public: 
      TruthQGDecorationTool(const std::string& t, const std::string& n, const IInterface* p);
      ~TruthQGDecorationTool();
      StatusCode initialize();
      virtual StatusCode addBranches(const EventContext& ctx) const;

    private:
      /// input collection key
      SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey
        {this, "JetCollection", "AntiKt4TruthWZJets", "Name of jet collection for decoration"};
      /// output decoration
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decOutput
        {this, "TrueFlavor", m_jetsKey, "TrueFlavor", "Name of the output decoration on the jet"}; 
  }; 
}

#endif // DERIVATIONFRAMEWORK_TRUTHDRESSINGTool_H
