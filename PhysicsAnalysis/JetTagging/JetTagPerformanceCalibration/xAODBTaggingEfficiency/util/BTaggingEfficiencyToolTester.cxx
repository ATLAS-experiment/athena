/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "TFile.h"
#include "CalibrationDataInterface/CDIReader.h"
#include "xAODJet/JetContainer.h"

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

#include <string>
#include <iomanip>

using CP::CorrectionCode;
ANA_MSG_HEADER(testBTagEfficiency)
ANA_MSG_SOURCE(testBTagEfficiency, "BtaggingEfficiencyToolTester")
using namespace testBTagEfficiency;

int test1 ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  // Important to do this first!
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis
  POOL::Init();
  #endif

  const char* TEST_NAME = argv[0];
  if (argc < 5) {
    ANA_MSG_ERROR ( "No right inputs received!" );
    ANA_MSG_ERROR ( "Usage: " << TEST_NAME << "[DAOD] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  std::string DAODpath   = argv[1];
  std::string CDIPath    = argv[2];
  std::string taggerName = argv[3];
  std::string workingPointName = argv[4];
  std::string JetCollectionName = "AntiKt4EMPFlowJets";
  // select your efficiency map based on the DSID of your sample:
  unsigned int sample_dsid = 601229;


  asg::StandaloneToolHandle<IBTaggingEfficiencyTool> tool("BTaggingEfficiencyTool/BTagEffTest");
  StatusCode code1 = tool.setProperty("ScaleFactorFileName", CDIPath);
  StatusCode code2 = tool.setProperty("TaggerName",    taggerName);
  StatusCode code3 = tool.setProperty("OperatingPoint", workingPointName);
  StatusCode code4 = tool.setProperty("JetAuthor", JetCollectionName);
  StatusCode code5 = tool.setProperty("MinPt", 20. );
  StatusCode code6 = tool.setProperty("EfficiencyCalibrations", sample_dsid);
  StatusCode code7 = tool.initialize();
  std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6, code7};
  for (const auto& code : codes) {
    if (code != StatusCode::SUCCESS) {
      ANA_MSG_ERROR ( "Initialization of tool " << tool->name() << " failed! ");
      return 1;
    }
  }
  ANA_MSG_INFO("Initialization of tool " << tool->name() << " finished.");

  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> root_file {TFile::Open(DAODpath.c_str(), "READ")};
  if(!event.readFrom(root_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << DAODpath.c_str() << "failed! " );
    return 1;
  } else {
    long long int imax = 5;
    for(long long int i = 0; i < imax; i++){
      ANA_MSG_DEBUG("Successfully opened file "<<DAODpath.c_str());

        event.getEntry(i);
      ANA_MSG_INFO("\n--- Reading Event: "<<i<<" ---");

        // retrieve the "real jets" of the jet collection in question
      const xAOD::JetContainer* jets = nullptr;
      if (!event.retrieve(jets, JetCollectionName).isSuccess()){
        ANA_MSG_ERROR ( "Retrieving jet collection " << JetCollectionName << " failed! " );
        return 1;
      }

          int jet_index = 0;
      for(const xAOD::Jet* jet : *jets){
        // skip jet as any lower than this and you start seeing failed SF/Eff retrieval
        if(jet->pt() < 20000 or std::abs(jet->eta()) > 2.4) continue;
        int truthlabel = -999;
        jet->getAttribute("HadronConeExclTruthLabelID",truthlabel);
        ANA_MSG_INFO("--- Jet " << jet_index << " ---");
        ANA_MSG_INFO("pt = " << jet->pt() << " eta = " << jet->eta() << " truthlabel = " << truthlabel);
        // Storage for sf/eff values
        float sf=0;
        float eff=0;
        CorrectionCode result;
        ANA_MSG_DEBUG("Testing function calls without systematics...");
        result = tool->getEfficiency(*jet,eff);
        if(result!=CorrectionCode::Ok){
          ANA_MSG_ERROR("Jet get efficiency failed");
          return 1;
        } else {
          ANA_MSG_INFO("Jet Efficiency: " << eff);
        }
        result = tool->getScaleFactor(*jet,sf);
        if( result!=CorrectionCode::Ok) {
          ANA_MSG_ERROR("Jet get scale factor failed");
          return 1;
        } else {
          ANA_MSG_INFO("Jet get scale factor succeeded: " << sf);
        }
        jet_index++;
      }
    }
  }
  root_file->Close();

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
