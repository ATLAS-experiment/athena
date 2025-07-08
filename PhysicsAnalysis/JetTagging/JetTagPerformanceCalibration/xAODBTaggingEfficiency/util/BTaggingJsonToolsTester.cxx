/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FTagAnalysisInterfaces/IBTaggingSelectionJsonTool.h"
#include "FTagAnalysisInterfaces/IBTaggingEfficiencyJsonTool.h"
#include "xAODJet/JetContainer.h"
#include <AsgTools/StandaloneToolHandle.h>

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/TEvent.h"
#define TEVENT xAOD::TEvent
#else
#include "POOLRootAccess/TEvent.h"
#define TEVENT POOL::TEvent
#endif

#include "PATInterfaces/SystematicSet.h"
#include "PATInterfaces/SystematicVariation.h"
#include "TFile.h"

ANA_MSG_HEADER(testBTagJson)
ANA_MSG_SOURCE(testBTagJson, "BtaggingJsonToolTester")
using namespace testBTagJson;

bool containNoSF(const std::string& str) {
  std::string target = "nosf";

  auto it = std::search(
      str.begin(), str.end(),
      target.begin(), target.end(),
      [](char ch1, char ch2) { return std::tolower(ch1) == std::tolower(ch2); }
  );

  return (it != str.end());
}

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  const char* TEST_NAME = argv[0];

  if (argc < 4) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    ANA_MSG_ERROR (  "Usage: " << TEST_NAME << " [DAOD file name] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  POOL::Init();

  std::string inputDAOD = argv[1];
  std::string JsonConfigFile = argv[2];
  std::string TaggerName = argv[3];
  std::string OperatingPoint = argv[4];
  std::string JetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";

  asg::StandaloneToolHandle<IBTaggingSelectionJsonTool> sel_tool("BTaggingSelectionJsonTool/BTagSelTest");
  StatusCode sel_code1 = sel_tool.setProperty( "MaxEta", 2.5 );
  StatusCode sel_code2 = sel_tool.setProperty( "MinPt",  0 );
  StatusCode sel_code3 = sel_tool.setProperty( "TaggerName", TaggerName );
  StatusCode sel_code4 = sel_tool.setProperty( "JetAuthor", JetCollection );
  StatusCode sel_code5 = sel_tool.setProperty( "OperatingPoint", OperatingPoint );
  StatusCode sel_code6 = sel_tool.setProperty( "JsonConfigFile", JsonConfigFile );
  StatusCode sel_code7 = sel_tool.setProperty( "OutputLevel",    MSG::ERROR);
  StatusCode sel_code8 = sel_tool.initialize();
  std::vector<StatusCode> sel_codes = {sel_code1, sel_code2, sel_code3, sel_code4, sel_code5, sel_code6, sel_code7, sel_code8};
  for(const auto& code : sel_codes) {
    if(code.isFailure()) {
      ANA_MSG_ERROR("Failed to set property or initialize tool");
      return 1;
    }
  }

  asg::StandaloneToolHandle<IBTaggingEfficiencyJsonTool> tool("BTaggingEfficiencyJsonTool/BTagEffTest");
  CP::SystematicSet sysSet;
  if (!containNoSF(JsonConfigFile)) {
    StatusCode code1 = tool.setProperty( "MaxEta", 2.5 );
    StatusCode code2 = tool.setProperty( "MinPt",  0 );
    StatusCode code3 = tool.setProperty( "TaggerName", TaggerName );
    StatusCode code4 = tool.setProperty( "JetAuthor", JetCollection );
    StatusCode code5 = tool.setProperty( "OperatingPoint", OperatingPoint );
    StatusCode code6 = tool.setProperty( "JsonConfigFile", JsonConfigFile );
    StatusCode code7 = tool.initialize();
    std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6, code7};
    for(const auto& code : codes) {
      if(code.isFailure()) {
        ANA_MSG_ERROR("Failed to set property or initialize tool");
        return 1;
      }
    }

    sysSet = tool->recommendedSystematics();
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

  for(auto entry=0; entry < 20; ++entry) {
    event.getEntry(entry);

    const xAOD::JetContainer* jets = nullptr;
    if(!event.retrieve(jets, JetCollection).isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve jets");
      return 1;
    }

    for(const auto jet : *jets) {
      ANA_MSG_INFO("====================================");
      ANA_MSG_INFO("Jet pt: " << jet->pt() << " mass: " << jet->m() );
      bool tagged = static_cast<bool>(sel_tool->accept(*jet));
      ANA_MSG_INFO("Tagged: " << tagged);

      if (!containNoSF(JsonConfigFile)) {
        float sf = 1.;
        for (const auto& var : sysSet) {
          CP::SystematicSet set;
          set.insert(var);
          if (tool->getScaleFactor(*jet, sf, set) != CP::CorrectionCode::Ok) {
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

