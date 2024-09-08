/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"

#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

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
ANA_MSG_SOURCE(testBTagSelection, "BtaggingToolTester")
using namespace testBTagSelection;

int main() {

  std::string inputDAOD = "/cvmfs/atlas-nightlies.cern.ch/repo/data/data-art/WorkflowReferences/main/mc_PHYS_Run3/v40/DAOD_PHYS.myOutput.pool.root";
  std::string CDIPath = "xAODBTaggingEfficiency/13p6TeV/2023-22-13TeV-MC21-CDI-2023-09-13_v1.root";
  std::string taggerName = "DL1dv01";
  std::string workingPointName = "FixedCutBEff_77";
  std::string JetCollectionName = "AntiKt4EMPFlowJets";
  std::string strat = "SFEigen"; // systematic strategy, see here for more info on which strategy to use: https://ftag.docs.cern.ch/calibrations/cdi/systematics/eigenvectordecomp/#which-method-should-i-use
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
  StatusCode code4 = btagSelTool.setProperty( "JetAuthor",                    JetCollectionName);
  StatusCode code5 = btagSelTool.setProperty( "MinPt",                        20000);
  StatusCode code6 = btagSelTool.setProperty( "OutputLevel",                  MSG::WARNING);
  StatusCode code7 = btagSelTool.initialize();
  std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6, code7};
  for (const auto& code : codes) {
    if (code != StatusCode::SUCCESS) {
      ANA_MSG_ERROR ( "Initialization of tool " << btagSelTool->name() << " failed! " );
      return 1;
    }
  }

  //------------------------------------------------------------------------------
  // set up BTaggingEfficiencyTool which is needed to get the btagging SFs and systematics
  //------------------------------------------------------------------------------
  asg::StandaloneToolHandle<IBTaggingEfficiencyTool> btagEffTool("BTaggingEfficiencyTool/BTagEffTest");
  code1 = btagEffTool.setProperty( "ScaleFactorFileName",  CDIPath); 
  code2 = btagEffTool.setProperty( "TaggerName",           taggerName);
  code3 = btagEffTool.setProperty( "OperatingPoint",       workingPointName);
  code4 = btagEffTool.setProperty( "JetAuthor",            JetCollectionName);
  code5 = btagEffTool.setProperty( "MinPt",                20000);
  code6 = btagEffTool.setProperty( "SystematicsStrategy",  strat);
  code7 = btagEffTool.setProperty( "IgnoreOutOfValidityRange", true);
  StatusCode code8 = btagEffTool.setProperty( "EfficiencyBCalibrations", sample_dsid);
  StatusCode code9 = btagEffTool.setProperty( "EfficiencyCCalibrations", sample_dsid);
  StatusCode code10 = btagEffTool.setProperty( "EfficiencyLightCalibrations", sample_dsid);
  StatusCode code11 = btagEffTool.setProperty( "EfficiencyTCalibrations", sample_dsid);
  StatusCode code12 = btagEffTool.setProperty( "OutputLevel", MSG::WARNING);
  StatusCode code13 = btagEffTool.initialize();
  std::vector<StatusCode> effcodes = {code1, code2, code3, code4, code5, code6, code7, code8, code9, code10, code11, code12, code13};
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
    if (!event.retrieve(jets, JetCollectionName).isSuccess()) {
      ANA_MSG_ERROR ( "Retrieving jet collection " << JetCollectionName << " failed! " );
      return 1;
    }

    // loop over jets
    for (const xAOD::Jet* jet : *jets ) {

      // get the btagging decision on a jet
      bool tagged = static_cast<bool>(btagSelTool->accept(*jet));
      std::cout << "BTagging decision on this jet is " << tagged << std::endl;

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
