/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"

// Define alias for the TEvent class 
// which is not the same for AnalysisBase and AthAnalysis
#ifdef XAOD_STANDALONE
// Those lines are only included if using AnalysisBase
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#else
// Those lines are only included if using AthAnalysis
#include "POOLRootAccess/TEvent.h"
#endif

#include <string>
#include <iomanip>

using CP::CorrectionCode;
ANA_MSG_HEADER(testBTagEfficiency)
ANA_MSG_SOURCE(testBTagEfficiency, "BtaggingEfficiencyToolTester")
using namespace testBTagEfficiency;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  const char* TEST_NAME = argv[0];
  if (argc < 4) {
    ANA_MSG_ERROR ( "No right inputs received!" );
    ANA_MSG_ERROR ( "Usage: " << TEST_NAME << "[CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  // Important to do this first!
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis
  POOL::Init();
  #endif

  std::string CDIPath = argv[1];
  std::string taggerName = argv[2];
  std::string workingPointName = argv[3];
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

  ANA_MSG_INFO( "-----------------------------------------------------");
  const std::map<CP::SystematicVariation, std::vector<std::string> > allowed_variations = tool->listSystematics();
  ANA_MSG_INFO("Allowed systematics variations for tool " << tool->name() << ":");
  for (auto var : allowed_variations) {
    ANA_MSG_INFO( std::setw(40) << std::left << var.first.name() << ":");
    for (auto flv : var.second) ANA_MSG_INFO( " " << flv);
  }
  ANA_MSG_DEBUG( "-----------------------------------------------------");
  

  ANA_MSG_DEBUG( "Creating a jet");
  xAOD::JetFourMom_t p4(50000.,0.7,0.3,1000.);
  xAOD::Jet * jet = new xAOD::Jet();
  jet->makePrivateStore();
  ANA_MSG_DEBUG("Setting jet 4 momentum");
  jet->setJetP4(p4);
  ANA_MSG_DEBUG("Setting jet attribute");
  jet->setAttribute("HadronConeExclTruthLabelID", 5);
  jet->setAttribute(taggerName+"_pb", 5.);
  jet->setAttribute(taggerName+"_pc", 5.);
  jet->setAttribute(taggerName+"_pu", 5.);
  jet->setAttribute(taggerName+"_ptau", 5.);
  float sf=0;
  float eff=0;
  CorrectionCode result;
  ANA_MSG_DEBUG( "Testing function calls without systematics...");

  result = tool->getEfficiency(*jet,eff);
  if( result!=CorrectionCode::Ok) { 
    ANA_MSG_ERROR("b jet get efficiency failed!"); 
    return 1;
  } else {
    ANA_MSG_DEBUG( "b jet get efficiency succeeded: " << eff );
  }
  result = tool->getScaleFactor(*jet,sf);

  if( result!=CorrectionCode::Ok) { 
    ANA_MSG_ERROR("b jet get scale factor failed"); 
    return 1;
  } else {
    ANA_MSG_INFO( "b jet get scale factor succeeded: " << sf );
  }

  ANA_MSG_DEBUG( "Testing function calls with systematics...");
  CP::SystematicSet systs = tool->affectingSystematics();
  for( CP::SystematicSet::const_iterator iter = systs.begin();
       iter!=systs.end(); ++iter) {
    CP::SystematicVariation var = *iter;
    CP::SystematicSet set;
    set.insert(var);
    StatusCode sresult = tool->applySystematicVariation(set);
    if( sresult !=StatusCode::SUCCESS) {
      ANA_MSG_ERROR( var.name() << " apply systematic variation FAILED ");
  }
    result = tool->getScaleFactor(*jet,sf);
    if( result!=CorrectionCode::Ok) {
      ANA_MSG_ERROR( var.name() << " getScaleFactor FAILED");
    } else {
      ANA_MSG_DEBUG( var.name() << " " << sf);
    }
  }

  // don't forget to switch back off the systematics...
  CP::SystematicSet defaultSet;
  StatusCode dummyResult = tool->applySystematicVariation(defaultSet);
  if (dummyResult != StatusCode::SUCCESS)
    ANA_MSG_ERROR( "problem disabling systematics setting!");

  return 0;
}
