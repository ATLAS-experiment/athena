/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/
//#include <AsgTools/ToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyTool.h"
#include "CalibrationDataInterface/CDIReader.h"

#include "xAODJet/JetContainer.h"
#include <AsgTools/StandaloneToolHandle.h>
#include "xAODBTaggingEfficiency/ToolDefaults.h"
#include "AsgMessaging/MessageCheck.h"
#include "PathResolver/PathResolver.h"

#include <string>
#include <iomanip>

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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// This executable cycles through the different calibrated flavour taggers                               //
//// in a CDI file, and tests out both the SFEigen and SFGlobalEigen systematic strategies                 //
//// to compare their output and performance.                                                              //
//// Use this in unison with the 'validate_reduction' function in CalibrationDataEigenVariations.cxx       //
//// to plot the bin-to-bin correlations - both for the "true" covariance matrix, and the                  //
//// approximate covariance matrix constructed **after** the PCA reduction takes place                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Further information on usage:
// This tool can be configured to test also the response, given a CDI file and a DAOD file.
// In this case, the CDI file must contain information for taggers which can operate on the objects in the DAOD file, 
// outdated CDI files cannot be used on newer DAODs and vice versa! This code will fail otherwise - to avoid this, you
// can filter down what combinations to study but adding some simple statements of the form "if(...) continue;", below

ANA_MSG_HEADER(testSysStratComp)
ANA_MSG_SOURCE(testSysStratComp, "SystematicStrategyComparison")
using namespace testSysStratComp;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

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
  //std::string jetAuthorName = "AntiKt4EMPFlowJets";

  // Important to do this first!
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis
  POOL::Init();
  #endif

  ANA_MSG_INFO("Starting up the SystematicStrategyComparison . . .");
  // select your efficiency map based on the DSID of your sample:
  unsigned int sample_dsid = 601229;

  // systematic strategies to compare
  std::vector<std::string> strats;
  strats.push_back("SFEigen");
  strats.push_back("SFGlobalEigen");

  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> root_file {TFile::Open(DAODpath.c_str(), "READ")};
  if(!event.readFrom(root_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << DAODpath.c_str() << "failed! " );
    return 1;
  } else {
    long long int i = 4;
    if ( event.getEntry(i) < 0) {
      ANA_MSG_ERROR ( "Reading event " << i << " failed! " );
      return 1;
    }
    ANA_MSG_DEBUG("Successfully opened file "<<DAODpath.c_str());
  }

  Analysis::CDIReader Reader(PathResolverFindCalibFile(CDIPath));
  for(const std::string& strat : strats ){
    ANA_MSG_INFO("\n\n\n######################## Strat: " << strat << " ########################");

    asg::StandaloneToolHandle<IBTaggingEfficiencyTool> tool("BTaggingEfficiencyTool/SysStratTest");
    StatusCode code1 = tool.setProperty("ScaleFactorFileName", CDIPath);
    StatusCode code2 = tool.setProperty("TaggerName", taggerName);
    StatusCode code3 = tool.setProperty("OperatingPoint", workingPointName);
    //StatusCode code4 = tool.setProperty("JetAuthor", jetAuthorName);
    StatusCode code5 = tool.setProperty("MinPt", 20.);
    StatusCode code6 = tool.setProperty("SystematicsStrategy", strat);//either "SFEigen" or "SFGlobalEigen"
    StatusCode code7 = tool.setProperty("EfficiencyCalibrations", sample_dsid);
    StatusCode code8 = tool.setProperty("useFlexibleConfig", true);
    StatusCode code_init = tool.initialize();
    std::vector<StatusCode> codes = {code_init, code1, code2, code3, code5, code6, code7, code8};
    for (const auto& code : codes) {
      if (code != StatusCode::SUCCESS) {
        ANA_MSG_ERROR ("Initialization of tool " << tool->name() << " failed! ");
        return 1;
      }
    }

    std::cout << "-----------------------------------------------------" << std::endl;
    //const std::map<CP::SystematicVariation, std::vector<std::string> > allowed_variations = tool->listSystematics();
    //ANA_MSG_DEBUG("Allowed systematics variations for tool " << tool->name() << ":");
    //for (const auto& var : allowed_variations) {
      //std::cout << std::setw(40) << std::left << var.first.name() << ":";
      //for (const auto& flv : var.second) std::cout << " " << flv;
      //std::cout << std::endl;
    //}
    std::cout << "-----------------------------------------------------" << std::endl;

    // retrieve the "real jets" of the jet collection in question
    const xAOD::JetContainer* jets = nullptr;
    if (!event.retrieve(jets, ftag::defaults::jet_collection).isSuccess()){
      ANA_MSG_ERROR ( "Retrieving jet collection " << ftag::defaults::jet_collection << " failed! " );
      return 1;
    }

    // test with the jet!
    int jet_index = 0;
    for(const xAOD::Jet* jet : *jets){
      // skip jet as any lower than this and you start seeing failed SF/Eff retrieval
      if(jet->pt() < 20000 or std::abs(jet->eta()) > 2.4) break;
      int truthlabel = -999;
      jet->getAttribute("HadronConeExclTruthLabelID",truthlabel);
      ANA_MSG_INFO("\n- - - - - - - - - - -  Jet " << jet_index << " - - - - - - - - - - - -");
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
        ANA_MSG_INFO("Jet get efficiency succeeded: " << eff);
      }
      result = tool->getScaleFactor(*jet,sf);
      if( result!=CorrectionCode::Ok) {
        ANA_MSG_ERROR("Jet get scale factor failed");
        return 1;
      } else {
        ANA_MSG_INFO("Jet get scale factor succeeded: " << sf);
      }

      ANA_MSG_DEBUG("Testing function calls with systematics...");
      const CP::SystematicSet& systs = tool->affectingSystematics();
      for(const auto& var : systs){
        CP::SystematicSet set;
        set.insert(var);
        StatusCode sresult = tool->applySystematicVariation(set);
        if( sresult !=StatusCode::SUCCESS) {
          ANA_MSG_ERROR(var.name() << "apply systematic variation FAILED ");
          return 1;
        }
        result = tool->getScaleFactor(*jet,sf);
        if( result!=CorrectionCode::Ok) {
          ANA_MSG_ERROR(var.name() << "getScaleFactor FAILED");
          return 1;
        } else {
          ANA_MSG_INFO(var.name() << ": scale-factor = " << sf);
        }
      }
      jet_index += 1;
      // don't forget to switch back off the systematics...
      CP::SystematicSet defaultSet;
      StatusCode dummyResult = tool->applySystematicVariation(defaultSet);
      if (dummyResult != StatusCode::SUCCESS) { 
        ANA_MSG_ERROR("Problem disabling systematics setting!");
        return 1;
      }
    }
  } // end strats loop
  root_file->Close();

  return 0;
}
