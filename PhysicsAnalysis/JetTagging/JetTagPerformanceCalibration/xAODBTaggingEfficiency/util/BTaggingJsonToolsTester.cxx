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

int main() {

  std::string TaggerName = "GN2Xv01";
  std::string OperatingPoint = "FlatMassQCDEff_0p25";
  std::string JetCollection = "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets";
  std::string JsonConfigFile = "/afs/cern.ch/work/b/bdong/CDIMagic/csv_to_json_xbb/Xbb_lookup_table.json";

  asg::StandaloneToolHandle<IBTaggingSelectionJsonTool> sel_tool("BTaggingSelectionJsonTool/BTagSelTest");
  StatusCode sel_code1 = sel_tool.setProperty( "MaxEta", 2.5 );
  StatusCode sel_code2 = sel_tool.setProperty( "MinPt",  20000 );
  StatusCode sel_code3 = sel_tool.setProperty( "TaggerName", TaggerName );
  StatusCode sel_code4 = sel_tool.setProperty( "JetAuthor", JetCollection );
  StatusCode sel_code5 = sel_tool.setProperty( "OperatingPoint", OperatingPoint );
  StatusCode sel_code6 = sel_tool.setProperty( "JsonConfigFile", JsonConfigFile );
  StatusCode sel_code7 = sel_tool.initialize();
  std::vector<StatusCode> sel_codes = {sel_code1, sel_code2, sel_code3, sel_code4, sel_code5, sel_code6, sel_code7};
  for(const auto& code : sel_codes) {
    if(code.isFailure()) {
      ANA_MSG_ERROR("Failed to set property or initialize tool");
      return 1;
    }
  }

  asg::StandaloneToolHandle<IBTaggingEfficiencyJsonTool> tool("BTaggingEfficiencyJsonTool/BTagEffTest");
  StatusCode code1 = tool.setProperty( "MaxEta", 2.5 );
  StatusCode code2 = tool.setProperty( "MinPt",  20000 );
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

  ANA_MSG_INFO("succesfully initialized");

  TEVENT event(TEVENT::kClassAccess);

  std::unique_ptr<TFile> m_file {TFile::Open("/eos/user/b/bdong/AODs/DAOD_FTAG1.601230.e8514_s4162_r14622_p6057.small.pool.root","read")};
  if(!event.readFrom(m_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Initialization of tool " << tool->name() << " failed! " );
    return 1;
  }

  CP::SystematicSet sysSet = tool->recommendedSystematics();
  ANA_MSG_INFO("Recommended systematics: " << sysSet.name());

  for(auto entry=0; entry < 10; ++entry) {
    event.getEntry(entry);

    const xAOD::JetContainer* jets = nullptr;
    if(!event.retrieve(jets, "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets").isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve jets");
      return 1;
    }

    for(const auto jet : *jets) {
      ANA_MSG_INFO("====================================");
      ANA_MSG_INFO("Jet pt: " << jet->pt() << " mass: " << jet->m() );
      bool tagged = static_cast<bool>(sel_tool->accept(*jet));
      ANA_MSG_INFO("Tagged: " << tagged);

      float sf = 1.;

      // Loop over each systematic variation
      for( const auto& var : sysSet) {
        CP::SystematicSet set;
        set.insert(var);

        if (tool->getScaleFactor(*jet, sf, set) != CP::CorrectionCode::Ok) {
          ANA_MSG_ERROR("Failed to get scale factor for jet");
        } else {
          ANA_MSG_INFO("Applied systematic: " << var.name());
          ANA_MSG_INFO("                   SF: " << sf );
        }
      }
    }
  }

  m_file->Close();
  m_file.reset();
  return 0;

}

