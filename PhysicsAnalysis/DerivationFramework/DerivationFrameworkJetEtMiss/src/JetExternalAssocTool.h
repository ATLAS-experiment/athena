/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// JetExternalAssocTool.h

#ifndef DerivationFramework_JetExternalAssocTool_H
#define DerivationFramework_JetExternalAssocTool_H

/// Qi Zeng
/// Nov 2016
///
/// Tool to build link to third party objects through the intermediate object
/// e.g. b already has link to c
///      this tool build a link from a to c by matching b to a, assuming thereis a one-to-one correspondence between a and b

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKeyArray.h"

#include "xAODJet/JetContainer.h"

#include "TObjArray.h"
#include "TObjString.h"
#include <vector>
#include <string>

namespace DerivationFramework{

  class JetExternalAssocTool : public extends<AthAlgTool, IAugmentationTool> {

  public:
    JetExternalAssocTool(const std::string& t, const std::string& n, const IInterface* p);

    virtual StatusCode initialize() override final;
    virtual StatusCode addBranches() const override final;

  private:
    StatusCode TransferLink(const xAOD::Jet& jet, const xAOD::Jet& jet_external, const EventContext& ctx) const;

    /// Properties.
    SG::ReadHandleKey<xAOD::JetContainer> m_containerName{this, "InputJets", ""};
    SG::ReadHandleKey<xAOD::JetContainer> m_ExternalJetCollectionName{this, "ExternalJetCollectionName", ""};
    Gaudi::Property<std::string> m_momentPrefix{this, "MomentPrefix", ""};
    Gaudi::Property<std::vector<std::string>> m_VectorOfOldLinkNames{this, "ListOfOldLinkNames", {}};
    Gaudi::Property<std::vector<std::string>> m_VectorOfNewLinkNames{this, "ListOfNewLinkNames", {}};
    SG::WriteDecorHandleKeyArray<xAOD::JetContainer> m_dec_keys{this, "DecKeys", {}, "SG keys for external decorations"};

    Gaudi::Property<bool> m_dRMatch{this, "DeltaRMatch", false};
    Gaudi::Property<double> m_dRCut{this, "DeltaRCut", 0.01};

    /// decoration pointers
    typedef ElementLink<xAOD::IParticleContainer>            type_el;
    typedef std::vector<type_el>                             type_ghostlink;
  };

}
#endif
