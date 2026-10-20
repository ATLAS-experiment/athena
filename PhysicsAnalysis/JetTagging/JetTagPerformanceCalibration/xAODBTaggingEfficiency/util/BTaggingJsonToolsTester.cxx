/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h"
#include "xAODJet/JetContainer.h"
#include <AsgTools/StandaloneToolHandle.h>

// Define alias for the TEvent class 
// which is not the same for AnalysisBase and AthAnalysis
#ifdef XAOD_STANDALONE
// Those lines are only included if using AnalysisBase
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
using TEVENT = xAOD::TEvent;
#else
// Those lines are only included if using AthAnalysis or Athena
#include "POOLRootAccess/TEvent.h"
using TEVENT = POOL::TEvent;
#endif

#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"
#include "TFile.h"

ANA_MSG_HEADER(testBTaggingJsonToolTester)
ANA_MSG_SOURCE(testBTaggingJsonToolTester, "BTaggingJsonToolTester")
using namespace testBTaggingJsonToolTester;

bool containNoSF(const std::string& str) {
  std::string target = "nosf";

  auto it = std::search(
      str.begin(), str.end(),
      target.begin(), target.end(),
      [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
  );

  return (it != str.end());
}

int test1 ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  const char* TEST_NAME = argv[0];

  if (argc < 4) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    ANA_MSG_ERROR (  "Usage: " << TEST_NAME << " [DAOD file name] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  // Important to do this first!
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis or Athena 
  POOL::Init();
  #endif

  std::string inputDAOD = argv[1];
  std::string JsonConfigFile = argv[2];
  std::string OutputName = argv[3];
  std::string OperatingPoint = argv[4];
  std::string mcGenerator = (argc > 5) ? argv[5] : "";
  std::string JetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";

  asg::StandaloneToolHandle<IBTaggingSelectionJsonTool> selTool("BTaggingSelectionJsonTool/BTagSelTest");
  ANA_CHECK(selTool.setProperty( "MaxEta", 2.5 ));
  ANA_CHECK(selTool.setProperty( "MinPt", 0. ));
  ANA_CHECK(selTool.setProperty( "OutputName", OutputName ));
  ANA_CHECK(selTool.setProperty( "JetAuthor", JetCollection ));
  ANA_CHECK(selTool.setProperty( "OperatingPoint", OperatingPoint ));
  ANA_CHECK(selTool.setProperty( "JsonConfigFile", JsonConfigFile ));
  ANA_CHECK(selTool.setProperty( "OutputLevel", MSG::ERROR));
  ANA_CHECK(selTool.initialize());

  asg::StandaloneToolHandle<IBTaggingEfficiencyJsonTool> effTool("BTaggingEfficiencyJsonTool/BTagEffTest");
  CP::SystematicSet sysSet;
  if (!containNoSF(JsonConfigFile)) {
    ANA_CHECK(effTool.setProperty( "MaxEta", 2.5 ));
    ANA_CHECK(effTool.setProperty( "MinPt",  0 ));
    ANA_CHECK(effTool.setProperty( "OutputName", OutputName ));
    ANA_CHECK(effTool.setProperty( "JetAuthor", JetCollection ));
    ANA_CHECK(effTool.setProperty( "OperatingPoint", OperatingPoint ));
    ANA_CHECK(effTool.setProperty( "JsonConfigFile", JsonConfigFile ));
    ANA_CHECK(effTool.setProperty( "MCGenerator", mcGenerator ));
    ANA_CHECK(effTool.setProperty( "btagSelectionJsonTool", selTool ));
    ANA_CHECK(effTool.initialize());
    
    sysSet = effTool->recommendedSystematics();
    ANA_MSG_INFO("Recommended systematics: " << sysSet.name());
  }

  ANA_MSG_INFO("succesfully initialized");

  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> root_file {TFile::Open(inputDAOD.c_str(), "READ")};
  if(!event.readFrom(root_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << inputDAOD << "failed! " );
    return 1;
  }
  
  
  // Here it is meanlingless to loop over event we should select 
  // large-R jets with pT > 250 GeV, mass > 50 GeV and |eta| < 2.0 
  // as this is the calibration range 
  int nValidLargeRJet = 0;
  int NMaxValidLargeRJet = 500;

  // Just for sanity do not process more than 5000 event or the number of events in the file
  // as this is CI so we don't want to use too much ressources
  // Since we are processing a ttbar sample in CI not many event passing Xbb working point 
  long long int NMaxEvent = std::min((long long int) 5000, (long long int) event.getEntries()); 
  long long int entry = -1;

  // Values required for a jet to be considered as valid 
  float minpTGeV = 250; 
  float minMassGeV = 50;
  float maxEta = 2.0; 

  while (nValidLargeRJet < NMaxValidLargeRJet && entry < NMaxEvent ){ 
    // Increase entry 
    entry+=1;

    // Read event 
    event.getEntry(entry);

    const xAOD::JetContainer* jets = nullptr;
    if(!event.retrieve(jets, JetCollection).isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve jets");
      return 1;
    }
    // Loop over jets of the event 
    for(const auto jet : *jets) {
      // Invalid jet with either too low pT or mass or too high eta value 
      // Hence skip jet 
      if (jet->pt()/1000. < minpTGeV || jet->m()/1000. < minMassGeV || std::abs(jet->eta()) > maxEta){ continue; }
      // If jet not skipped then it is a valid large-R jet 
      nValidLargeRJet+=1;

      ANA_MSG_INFO("====================================");
      ANA_MSG_INFO("Jet(pT=" << jet->pt() << ", mass="<< jet->m()<< ", eta=" << jet->eta() <<")");

      bool tagged = static_cast<bool>(selTool->accept(*jet));
      ANA_MSG_INFO("Tagged: " << tagged);

      if (!containNoSF(JsonConfigFile)) {
        float sf = 1.;
        for (const auto& var : sysSet) {
          CP::SystematicSet set;
          set.insert(var);
          CP::CorrectionCode code = effTool->getScaleFactor(*jet, sf, set);

          if (code != CP::CorrectionCode::Ok) {
            ANA_MSG_ERROR("Failed to get scale factor for jet");
          } else {
            ANA_MSG_INFO("Applied systematic: " << var.name());
            ANA_MSG_INFO("                   SF: " << sf);
          }
        }
      } 
    }
  }

  root_file->Close();
  root_file.reset();
  return 0;
}


int main ATLAS_NOT_THREAD_SAFE (int argc, char *argv[])
{
  try {
    return test1(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}

