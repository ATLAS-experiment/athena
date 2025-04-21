/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// TruthQGDecorationTool.cxx
// Create a single decoration for flavor tagging of truth jets

#include "DerivationFrameworkMCTruth/TruthQGDecorationTool.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteDecorHandle.h"
#include "AthContainers/ConstAccessor.h"
#include "xAODJet/JetContainer.h"
#include <string>

// Constructor
DerivationFramework::TruthQGDecorationTool::TruthQGDecorationTool(const std::string& t,
        const std::string& n,
        const IInterface* p ) :
    base_class(t,n,p)
{
}

// Destructor
DerivationFramework::TruthQGDecorationTool::~TruthQGDecorationTool() {
}

// Initialize
StatusCode DerivationFramework::TruthQGDecorationTool::initialize() {

  ATH_CHECK(m_jetsKey.initialize());
  ATH_CHECK(m_decOutput.initialize());
  return StatusCode::SUCCESS; 

}

// Function to do dressing, implements interface in IAugmentationTool
StatusCode DerivationFramework::TruthQGDecorationTool::addBranches() const
{
  // Event context
  const EventContext& ctx = Gaudi::Hive::currentContext();

  // Retrieve the jet container
  SG::ReadHandle<xAOD::JetContainer> inputJets(m_jetsKey, ctx);
  if (!inputJets.isValid()) {
    ATH_MSG_ERROR("Couldn't retrieve container with name " << m_jetsKey);
    return StatusCode::FAILURE;
  }

  SG::WriteDecorHandle<xAOD::JetContainer,int> output_decorator(m_decOutput, ctx);
  static const SG::ConstAccessor<int> PartonTruthLabelIDAcc("PartonTruthLabelID");
  static const SG::ConstAccessor<int> HadronConeExclTruthLabelIDAcc("HadronConeExclTruthLabelID");

  for (const auto *ajet : *inputJets){
    if (!PartonTruthLabelIDAcc.isAvailable(*ajet) ){
      ATH_MSG_ERROR("Did not have input PartonTruthLabelID decorations available");
      return StatusCode::FAILURE;
    }
    else if (!HadronConeExclTruthLabelIDAcc.isAvailable(*ajet) ){
      ATH_MSG_ERROR("Did not have input HadronConeExclTruthLabelID decorations available");
      return StatusCode::FAILURE;
    } // Now we have the input decorations
    /* Agreement from the HF-tagging and Jet/MET group:
        - If it is non-zero, use the label from the HF-tagging group (b, c, tau)
        - If it is zero, use the label from the Jet/MET group (q/g)
        - In the case that the two disagree (e.g. Jet/MET says b and HF says light),
           multiply the Jet/MET label by 100 to ensure this case is kept separate
    */
    if (HadronConeExclTruthLabelIDAcc(*ajet)!=0){
      output_decorator(*ajet) = HadronConeExclTruthLabelIDAcc(*ajet);
    } else {
      int labelid = PartonTruthLabelIDAcc(*ajet);
      if (std::abs(labelid)!=5 &&
          std::abs(labelid)!=4 &&
          std::abs(labelid)!=15){
        output_decorator(*ajet) = labelid;
      } else {
        output_decorator(*ajet) = labelid*100;
      }
    }
  } // Loop over jets

  return StatusCode::SUCCESS;
}
