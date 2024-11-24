/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"

#include "xAODBTaggingEfficiency/ToolDefaults.h"

#include "xAODJet/JetContainer.h"
#include "AsgMessaging/MessageCheck.h"

#include "TFile.h"

// Define alias for the TEvent class 
// which is not the same for AnalysisBase and AthAnalysis
#ifdef XAOD_STANDALONE
// Those lines are only included if using AnalysisBase
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
using TEVENT = xAOD::TEvent;
#else
// Those lines are only included if using AthAnalysis
#include "POOLRootAccess/TEvent.h"
using TEVENT = POOL::TEvent;
#endif

using CP::CorrectionCode;
using namespace asg::msgUserCode;

int main(int argc, char* argv[]) {

  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  const char* TEST_NAME = argv[0];

  if (argc < 2) {
    std::cout <<  "Usage: " << TEST_NAME << " [DAOD file name]" << std::endl;
    return 1;
  }

  std::string inputDAOD = argv[1];
  std::string workingPointName =  "FixedCutBEff_77";

  // Important to do this first!
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis
  POOL::Init();
  #endif
  
  // this is needed for the so called MC/MC efficiency map, details
  // can be found here:
  // https://ftag.docs.cern.ch/algorithms/activities/mcmc/
  //
  // normally the generator info is stored in the metadata of DAOD
  // files, you can call the `MCMC_dsid_map` function in the following
  // link to convert the generator info into dsid we put in the CDI:
  // https://gitlab.cern.ch/atlas/athena/-/blob/main/PhysicsAnalysis/JetTagging/JetTagPerformanceCalibration/CalibrationDataInterface/python/MCMCGeneratorHelper.py?ref_type=heads
  unsigned int sample_dsid = 601414;

  // //---------------------------------------------------------------------------
  // // set up BTaggingSelectionTool which is needed to get the btagging decision
  // //---------------------------------------------------------------------------
  asg::StandaloneToolHandle<IBTaggingSelectionTool> btagSelTool("BTaggingSelectionTool/BTagselecTest");
  ANA_CHECK( btagSelTool.setProperty( "OperatingPoint", workingPointName) );
  ANA_CHECK( btagSelTool.setProperty( "OutputLevel", MSG::WARNING) );
  ANA_CHECK( btagSelTool.initialize() );
  
  // //------------------------------------------------------------------------------
  // // set up BTaggingEfficiencyTool which is needed to get the btagging SFs and systematics
  // //------------------------------------------------------------------------------
  asg::StandaloneToolHandle<IBTaggingEfficiencyTool> btagEffTool("BTaggingEfficiencyTool/BTagEffTest");
  ANA_CHECK( btagEffTool.setProperty( "EfficiencyCalibrations", sample_dsid) );
  ANA_CHECK( btagEffTool.setProperty( "OutputLevel", MSG::WARNING) );
  ANA_CHECK( btagEffTool.initialize() );
  
  // //------------------------------------------------------------------------------
  // // read in the input file and event loop
  // //------------------------------------------------------------------------------
  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> m_file {TFile::Open(inputDAOD.c_str(), "READ")};
  if(!event.readFrom(m_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << inputDAOD << "failed! " );
    return 1;
  }

  // loop over events, only looping over the first event as an example here
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
      std::cout << std::endl << "BTagging decision on this jet is " << tagged << std::endl;

      // to make sure to retrieve the nominal scale factor
      // apply empty set which corresponds to Nominal
      CP::SystematicSet emptySet;
      ANA_CHECK( btagEffTool->applySystematicVariation(emptySet) );

      // get the btagging SFs and systematics on a jet
      float sf = 0;
      CorrectionCode result = btagEffTool->getScaleFactor(*jet, sf);

      // If the retrieval of the scale factor failed and the jet is not out of range 
      // it implies there is an issue so raise error
      if (result != CorrectionCode::Ok && result != CorrectionCode::OutOfValidityRange) {
        ANA_MSG_ERROR ( "Failed to get SFs on this jet! " );
        return 1;
      }

      if (result != CorrectionCode::OutOfValidityRange){
        std::cout << "Nominal SF for this jet: " << sf << std::endl;
      }
      else {
        // Out of validity range jet print its pT and eta then skip
        std::cout << "Out of validity range jet: (pT, eta)=(" << jet->pt() << ","  << jet->eta() << ")" << std::endl;
        std::cout << "Nominal SF for this jet: " << sf << std::endl;
        std::cout << "Skip jet" << std::endl;
        // Pass to the next jet 
        continue;
      }

      const CP::SystematicSet& sysSet = btagEffTool->recommendedSystematics();
      for (const auto& var : sysSet) {
        CP::SystematicSet set;
        set.insert(var);

        ANA_CHECK( btagEffTool->applySystematicVariation(set) );
        
        result = btagEffTool->getScaleFactor(*jet, sf);
        if (result != CorrectionCode::Ok) {
          ANA_MSG_ERROR ( "Failed to get SFs on this jet! " );
          return 1;
        } else {
          std::cout << "SF for this jet with systematic variation " << var.name() << " is " << sf << std::endl;
        }
      }
      // After processing flavour tagging efficiency uncertainties
      // don't forget to switch back off the systematics
      ANA_CHECK( btagEffTool->applySystematicVariation(emptySet) );
    }
    
  }

  m_file->Close();
  return 0;
}
