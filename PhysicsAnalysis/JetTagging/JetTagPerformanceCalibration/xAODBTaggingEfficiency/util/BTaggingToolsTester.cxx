/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"

#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

#include "xAODBTaggingEfficiency/ToolDefaults.h"

#include "xAODJet/JetContainer.h"

#include "AsgMessaging/MessageCheck.h"

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/TEvent.h"
#define TEVENT xAOD::TEvent
#else
#include "POOLRootAccess/TEvent.h"
#define TEVENT POOL::TEvent
#endif

#include <string>
#include <iomanip>
#include "TFile.h"

using CP::CorrectionCode;
ANA_MSG_HEADER(testBTagSelection)
ANA_MSG_SOURCE(testBTagSelection, "BtaggingToolsTester")
using namespace testBTagSelection;

int main(int argc, char* argv[]) {

  const char* TEST_NAME = argv[0];

  if (argc < 4) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    ANA_MSG_ERROR (  "Usage: " << TEST_NAME << " [DAOD file name] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  std::string inputDAOD = argv[1];
  std::string CDIPath = argv[2];
  std::string taggerName = argv[3];
  std::string workingPointName = argv[4];
  std::string tagScheme = argv[5];
  
unsigned int sample_dsid = 601414; // this is needed for the so called MC/MC efficiency map, details can be found here: https://ftag.docs.cern.ch/algorithms/activities/mcmc/
                                     // normally the generator info is stored in the metadata of DAOD files, you can call the `MCMC_dsid_map` function in the following link to convert the generator info into dsid we put in the CDI:
                                     // https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/JetTagging/JetTagPerformanceCalibration/CalibrationDataInterface/python/MCMCGeneratorHelper.py?ref_type=heads
  //---------------------------------------------------------------------------
  // set up BTaggingSelectionTool which is needed to get the btagging decision
  //---------------------------------------------------------------------------
  asg::StandaloneToolHandle<IBTaggingSelectionTool> btagSelTool("BTaggingSelectionTool/BTagselecTest");
  StatusCode code1 = btagSelTool.setProperty( "FlvTagCutDefinitionsFileName", CDIPath);
  StatusCode code2 = btagSelTool.setProperty( "TaggerName",                   taggerName);
  StatusCode code3 = btagSelTool.setProperty( "OperatingPoint",               workingPointName);
  StatusCode code4 = btagSelTool.setProperty( "MinPt",                        20000);
  StatusCode code5 = btagSelTool.setProperty( "OutputLevel",                  MSG::WARNING);
  StatusCode code6 = btagSelTool.initialize();
  std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6};
  for (const auto& code : codes) {
    if (code != StatusCode::SUCCESS) {
      ANA_MSG_ERROR ( "Initialization of tool " << btagSelTool->name() << " failed! " );
      return 1;
    }
  }
  
  //---------------------------------------------------------------------------
  // set up a second BTaggingSelectionTool for the 2D 77% b-jet efficiency veto
  //---------------------------------------------------------------------------
  
  asg::StandaloneToolHandle<IBTaggingSelectionTool> btagSelTool_bveto("BTaggingSelectionTool/BTagselecTest");
  code1 = btagSelTool_bveto.setProperty( "FlvTagCutDefinitionsFileName", CDIPath);
  code2 = btagSelTool_bveto.setProperty( "TaggerName",                   taggerName);
  code3 = btagSelTool_bveto.setProperty( "OperatingPoint", "FixedCutBEff_77");
  code4 = btagSelTool_bveto.setProperty( "MinPt",                        20000);
  code5 = btagSelTool_bveto.setProperty( "OutputLevel",                  MSG::WARNING);
  code6 = btagSelTool_bveto.initialize();
  std::vector<StatusCode> codes_veto = {code1, code2, code3, code4, code5, code6};
  for (const auto& code : codes_veto) {
    if (code != StatusCode::SUCCESS) {
     ANA_MSG_ERROR ( "Initialization of tool " << btagSelTool_bveto->name() << " for b-veto failed! " );
      return 1;
    }
  }
  

  //------------------------------------------------------------------------------
  // set up BTaggingEfficiencyTool which is needed to get the btagging SFs and systematics
  //------------------------------------------------------------------------------
  asg::StandaloneToolHandle<IBTaggingEfficiencyTool> btagEffTool("BTaggingEfficiencyTool/BTagEffTest");
  code1 = btagEffTool.setProperty( "ScaleFactorFileName",  CDIPath); 
  code2 = btagEffTool.setProperty( "TaggerName",           taggerName);
  if (tagScheme == "1d") {
    code3 = btagEffTool.setProperty( "OperatingPoint",       "Continuous");
  } else {
    code3 = btagEffTool.setProperty( "OperatingPoint",       "Continuous2D");
  }
  code4 = btagEffTool.setProperty( "MinPt",                20000);
  code5 = btagEffTool.setProperty( "IgnoreOutOfValidityRange", true);
  code6 = btagEffTool.setProperty( "EfficiencyCalibrations", sample_dsid);
  StatusCode code7 = btagEffTool.setProperty( "OutputLevel", MSG::WARNING);
  StatusCode code8 = btagEffTool.initialize();
  std::vector<StatusCode> effcodes = {code1, code2, code3, code4, code5, code6, code7, code8};
  for (const auto& code : effcodes) { 
    if ( code != StatusCode::SUCCESS ) {
      ANA_MSG_ERROR ( "Initialization of tool " << btagEffTool->name() << " failed! " );
      return 1;
    }
  }
  
 

  //------------------------------------------------------------------------------
  // read in the input file and event loop
  //------------------------------------------------------------------------------
  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> m_file {TFile::Open(inputDAOD.c_str(), "READ")};
  if(!event.readFrom(m_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << inputDAOD << "failed! " );
    return 1;
  }

  // loop over events, i'm only looping over the first event as an exmaple here
  for (unsigned int i = 0; i < 1; i++) {
    if (event.getEntry(i) < 0) {
      ANA_MSG_ERROR ( "Reading event " << i << " failed! " );
      return 1;
    }
  
    // retrieve jets
    const xAOD::JetContainer* jets = nullptr;
    if (!event.retrieve(jets, ftag::defaults::jet_collection).isSuccess()) {
      ANA_MSG_ERROR ( "Retrieving jet collection " << ftag::defaults::jet_collection << " failed! " );
      return 1;
    }

    // loop over jets
    for (const xAOD::Jet* jet : *jets ) {

      // get the btagging decision on a jet
      bool tagged = static_cast<bool>(btagSelTool->accept(*jet));
      bool veto = true;
      if (tagScheme == "2d" && workingPointName.find("CEff") != std::string::npos) {
        veto = !static_cast<bool>(btagSelTool_bveto->accept(*jet));
      }
      std::cout << "BTagging decision on this jet is " << ( tagged && veto ) << std::endl;
     

      // get the btagging SFs and systematics on a jet
      float sf = 0;
      CorrectionCode result = btagEffTool->getScaleFactor(*jet, sf);
      if (result != CorrectionCode::Ok) {
        ANA_MSG_ERROR ( "Failed to get SFs on this jet! " );
        return 1;
      } else {
        std::cout << "Nominal SF for this jet: " << sf << std::endl;
      }

      const CP::SystematicSet& sysSet = btagEffTool->recommendedSystematics();
      for (const auto& var : sysSet) {
        CP::SystematicSet set;
        set.insert(var);
        StatusCode sresult = btagEffTool->applySystematicVariation(set);
        if (sresult != StatusCode::SUCCESS) {
          ANA_MSG_ERROR ( "Failed to apply systematic variation! " );
          return 1;
        }
        result = btagEffTool->getScaleFactor(*jet, sf);
        if (result != CorrectionCode::Ok) {
          ANA_MSG_ERROR ( "Failed to get SFs on this jet! " );
          return 1;
        } else {
          std::cout << "SF for this jet with systematic variation " << var.name() << " is " << sf << std::endl;
        }
      }

      // don't forget to switch back off the systematics
      CP::SystematicSet emptySet;
      if (btagEffTool->applySystematicVariation(emptySet) != StatusCode::SUCCESS) {
        ANA_MSG_ERROR ( "Failed to switch back off systematic setting! " );
        return 1;
      }
    }
  }

  m_file->Close();
  return 0;
}
