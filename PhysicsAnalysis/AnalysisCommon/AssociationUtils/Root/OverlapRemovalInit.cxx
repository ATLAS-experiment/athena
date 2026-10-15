/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System includes
#include <limits>

// ROOT includes
#include "TError.h"

// Local includes
#include "AssociationUtils/OverlapRemovalInit.h"
#include "AssociationUtils/DeltaROverlapTool.h"
#include "AssociationUtils/FEOverlapTool.h"
#include "AssociationUtils/EleMuSharedTrkOverlapTool.h"
#include "AssociationUtils/EleJetOverlapTool.h"
#include "AssociationUtils/MuJetOverlapTool.h"
#include "AssociationUtils/TauLooseEleOverlapTool.h"
#include "AssociationUtils/TauLooseMuOverlapTool.h"
#include "AssociationUtils/OverlapRemovalTool.h"

// Error checking macro
#define ORT_CHECK( ARG )                        \
  do {                                          \
    if(!static_cast<bool>(ARG)) {               \
      ::Error("OverlapRemovalInit",             \
              "Failed to execute: \"%s\"",      \
              #ARG );                           \
      return StatusCode::FAILURE;               \
    }                                           \
  } while( false )


namespace ADD_CMODE(ORUtils)
{

  //---------------------------------------------------------------------------
  // ORFlags constructor
  //---------------------------------------------------------------------------
  ORFlags::ORFlags(const std::string& masterToolName,
                   const std::string& theInputLabel,
                   const std::string& theOutputLabel)
    : masterName(masterToolName),
      inputLabel(theInputLabel),
      outputLabel(theOutputLabel)
  {}

  //---------------------------------------------------------------------------
  // Preconfigured recommended tools
  //---------------------------------------------------------------------------
  StatusCode recommendedTools(const ORFlags& flags, ToolBox& tbox)
  {
    // Master tool
    tbox.masterTool.setTypeAndName(columnar::add_cmode("ORUtils")+"::OverlapRemovalTool/" +
                                   flags.masterName);

    // El-el
    if(flags.doElectrons && flags.doEleEleOR && tbox.eleEleORT.empty()) {
      tbox.eleEleORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::EleEleOverlapTool/" +
                                    flags.masterName + ".EleEleORT");
    }

    // El-mu
    if(flags.doElectrons && flags.doMuons && tbox.eleMuORT.empty()) {
      tbox.eleMuORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::EleMuSharedTrkOverlapTool/" +
                                   flags.masterName + ".EleMuORT");
    }
    // El-jet
    if(flags.doElectrons && flags.doJets && tbox.eleJetORT.empty()) {
      tbox.eleJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::EleJetOverlapTool/" +
                                    flags.masterName + ".EleJetORT");
      ORT_CHECK( tbox.eleJetORT.setProperty("BJetLabel", flags.bJetLabel) );
      ORT_CHECK( tbox.eleJetORT.setProperty("MaxElePtForBJetAwareOR", flags.maxElePtForBJetAwareOR) );
      ORT_CHECK( tbox.eleJetORT.setProperty("UseSlidingDR", flags.boostedLeptons) );
      ORT_CHECK( tbox.eleJetORT.setProperty("DoFEOR", flags.doFEOR) );
    }
    // Mu-jet
    if(flags.doMuons && flags.doJets && tbox.muJetORT.empty()) {
      tbox.muJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::MuJetOverlapTool/" +
                                   flags.masterName + ".MuJetORT");
      ORT_CHECK( tbox.muJetORT.setProperty("BJetLabel", flags.bJetLabel) );
      ORT_CHECK( tbox.muJetORT.setProperty("UseSlidingDR", flags.boostedLeptons) );
      ORT_CHECK( tbox.eleJetORT.setProperty("DoFEOR", flags.doFEOR) );
    }

    // Tau-ele overlap tool
    if(flags.doTaus && flags.doElectrons && tbox.tauEleORT.empty()) {
      if (!flags.doFEOR) {
        tbox.tauEleORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                    flags.masterName + ".TauEleORT");
        ORT_CHECK( tbox.tauEleORT.setProperty("DR", 0.2) );
      }
      else if (flags.doFEOR) {
        tbox.tauEleORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                    flags.masterName + ".TauEleORT");
      }
    }
    // Tau-mu overlap tool
    if(flags.doTaus && flags.doMuons && tbox.tauMuORT.empty()) {
      if (!flags.doFEOR) {
        tbox.tauMuORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                   flags.masterName + ".TauMuORT");
        ORT_CHECK( tbox.tauMuORT.setProperty("DR", 0.2) );
      }
      else if (flags.doFEOR) {
        tbox.tauMuORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                   flags.masterName + ".TauMuORT");
      }
    }
    // Tau-jet overlap tool
    if(flags.doTaus && flags.doJets && tbox.tauJetORT.empty()) {
      if (!flags.doFEOR) {
        tbox.tauJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                    flags.masterName + ".TauJetORT");
        ORT_CHECK( tbox.tauJetORT.setProperty("DR", 0.2) );
      }
      else if (flags.doFEOR) {
        tbox.tauJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                    flags.masterName + ".TauJetORT");
      }
    }

    // Pho-ele overlap tool
    if(flags.doPhotons && flags.doElectrons && tbox.phoEleORT.empty()) {
      if (!flags.doFEOR) {
      tbox.phoEleORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                    flags.masterName + ".PhoEleORT");
      }
      else if (flags.doFEOR) {
        tbox.phoEleORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                    flags.masterName + ".PhoEleORT");
      }
    }
    // Pho-mu overlap tool
    if(flags.doPhotons && flags.doMuons && tbox.phoMuORT.empty()) {
      if (!flags.doFEOR) {
        tbox.phoMuORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                   flags.masterName + ".PhoMuORT");
      }
      else if (flags.doFEOR) {
        tbox.phoMuORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                    flags.masterName +".PhoMuORT");
      }
    }
    // Pho-jet overlap tool
    if(flags.doPhotons && flags.doJets && tbox.phoJetORT.empty()) {
      if (!flags.doFEOR) {
        tbox.phoJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                    flags.masterName + ".PhoJetORT");
      }
      else if (flags.doFEOR) {
        tbox.phoJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::FEOverlapTool/" + 
                                    flags.masterName + ".PhoJetORT");
      }
    }

    // Ele-fatjet overlap tool
    if(flags.doElectrons && flags.doFatJets && tbox.eleFatJetORT.empty()) {
      tbox.eleFatJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                       flags.masterName + ".EleFatJetORT");
      ORT_CHECK( tbox.eleFatJetORT.setProperty("DR", 1.0) );
    }

    // Jet-fatjet overlap tool
    if(flags.doJets && flags.doFatJets && tbox.jetFatJetORT.empty()) {
      tbox.jetFatJetORT.setTypeAndName(columnar::add_cmode("ORUtils")+"::DeltaROverlapTool/" +
                                       flags.masterName + ".JetFatJetORT");
      ORT_CHECK( tbox.jetFatJetORT.setProperty("DR", 1.0) );
    }

    // Overlap tool common properties
    for(auto hptr : tbox.getOverlapTools()) {
      ORT_CHECK( hptr->setProperty("LinkOverlapObjects", flags.linkOverlapObjects) );
    }

    // Global common properties
    ORT_CHECK( tbox.setGlobalProperty("InputLabel", flags.inputLabel) );
    ORT_CHECK( tbox.setGlobalProperty("OutputLabel", flags.outputLabel) );
    ORT_CHECK( tbox.setGlobalProperty("OutputPassValue", flags.outputPassValue) );

    return StatusCode::SUCCESS;
  }

}

#undef ORT_CHECK
