/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include <AsgTools/StandaloneToolHandle.h>
#include "FTagAnalysisInterfaces/IBTaggingSelectionTool.h"
#include "xAODJet/JetContainer.h"
#include "xAODBTagging/BTagging.h"
#include "xAODBTagging/BTaggingUtilities.h"

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
ANA_MSG_SOURCE(testBTagSelection, "BtaggingSelectionToolTester")
using namespace testBTagSelection;

int main(int argc, char* argv[]) {

  const char* TEST_NAME = argv[0];

  if (argc < 4) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    ANA_MSG_ERROR (  "Usage: " << TEST_NAME << "[DAOD file name] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  std::string inputDAOD = argv[1];
  std::string CDIPath = argv[2];
  std::string taggerName = argv[3];
  std::string workingPointName = argv[4];
  std::string JetCollectionName = "AntiKt4EMPFlowJets";

  std::string EDMTaggerName;
  if (taggerName.find("GN2v00") != std::string::npos) {
    EDMTaggerName = "GN2v00";
  } else {
    EDMTaggerName = taggerName;
  }

  asg::StandaloneToolHandle<IBTaggingSelectionTool> tool("BTaggingSelectionTool/BTagSelecTest");
  StatusCode code1 = tool.setProperty( "FlvTagCutDefinitionsFileName", CDIPath);
  StatusCode code2 = tool.setProperty( "TaggerName",                   taggerName);
  StatusCode code3 = tool.setProperty( "OperatingPoint",               workingPointName);
  StatusCode code4 = tool.setProperty( "JetAuthor",                    JetCollectionName);
  StatusCode code5 = tool.setProperty( "MinPt",                        20000);
  StatusCode code6 = tool.initialize();
  std::vector<StatusCode> codes = {code1, code2, code3, code4, code5, code6};
  for (const auto& code : codes) {
    if (code != StatusCode::SUCCESS) {
      ANA_MSG_ERROR ( "Initialization of tool " << tool->name() << " failed! " );
      return 1;
    }
  }

  //load some jets to show how to use the tool
  gErrorIgnoreLevel = kError;
  TEVENT event(TEVENT::kClassAccess);

  TFile* m_file = TFile::Open(inputDAOD.c_str(),"read");
  if(!event.readFrom(m_file).isSuccess()) { 
    ANA_MSG_ERROR ( "failed to load file! " );
    return 1;
  }

  event.getEntry(0);

  const xAOD::JetContainer* jets = nullptr;
  if (!event.retrieve( jets, JetCollectionName ).isSuccess() ) { 
    ANA_MSG_ERROR ( " Failed to retrieve jet container! " );
    return 1;
  }

  int jet_index = 0;
  for (const xAOD::Jet* jet : *jets) {

    //getting a tagging decision, is the jet tagged or not
    bool tagged = static_cast<bool>(tool->accept(*jet));

    //you can get the tagger weight,
    double tagweight;
    if( tool->getTaggerWeight( *jet ,tagweight)!=CorrectionCode::Ok) { 
      ANA_MSG_ERROR (" error retrieving tagger weight! " );
      return 1;
    }

    if (workingPointName.find("Continuous") == std::string::npos) {
      ANA_MSG_INFO ( "jet " << jet_index << " " <<  taggerName  << "  " << workingPointName << " is tagged " << tagged << " tag weight " << tagweight );

      //if you have tagger weights, you can get the tagger weight this way
      const xAOD::BTagging *btag = xAOD::BTaggingUtilities::getBTagging( *jet );

      float jet_pb = btag->auxdata<float>(EDMTaggerName + "_pb");
      float jet_pc = btag->auxdata<float>(EDMTaggerName + "_pc");
      float jet_pu = btag->auxdata<float>(EDMTaggerName + "_pu");
      float jet_ptau = 0.;
      if (taggerName == "GN2v01") jet_ptau = btag->auxdata<float>(EDMTaggerName + "_ptau");

      if( tool->getTaggerWeight(jet_pb,jet_pc,jet_pu, tagweight, jet_ptau) != CorrectionCode::Ok ){
        ANA_MSG_ERROR (" error retrieving tagger weight! " );
        return 1;
      }

      ANA_MSG_INFO (" tagweight " << tagweight );

      double pT = jet->pt();
      double eta = jet->eta();

      // you can see if the jet is tagged using its pt/eta and tagweight

      tagged = static_cast<bool>(tool->accept(pT,eta,tagweight));
      //you can also extract the cut value (which may or may not depend on the jet pt)
      double cutval;
      //provide the pt in MeV
      if( tool->getCutValue( jet->pt() , cutval)!=CorrectionCode::Ok ) {
        ANA_MSG_ERROR ( " error retrieving cut value " );
        return 1;
      }
      ANA_MSG_INFO ( " tagged " << tagged << " cut value " << cutval );
    } else{
      int quantile = tool->getQuantile(*jet);
      ANA_MSG_INFO ("jet " << jet_index << " " <<  taggerName  << "  " << workingPointName << " tag weight " << tagweight <<  " quantile " << quantile );
    }

    jet_index++;
  }

  return 0;
}
