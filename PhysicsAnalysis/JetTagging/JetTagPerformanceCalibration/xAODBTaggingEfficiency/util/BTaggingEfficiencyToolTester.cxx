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

// For setting formatting of ANA_MSG_INFO etc 
#include "AsgMessaging/MessagePrinter.h"
#include "AsgMessaging/MessagePrinterOverlay.h"
#else
// Those lines are only included if using AthAnalysis
#include "POOLRootAccess/TEvent.h"
using TEVENT = POOL::TEvent;
// For setting formatting of ANA_MSG_INFO etc 
#include "GaudiKernel/IMessageSvc.h"
#endif

#include "TTree.h"
#include <string>
#include <iomanip>

using CP::CorrectionCode;
ANA_MSG_HEADER(testBTagEfficiency)
ANA_MSG_SOURCE(testBTagEfficiency, "BtaggingEfficiencyToolTester")
using namespace testBTagEfficiency;

int test1 ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  #ifdef XAOD_STANDALONE
  // Change type returned by the ANA_CHECK function in case of error 
  // NB: this is needed here because the main() function should return an integer
  // In principle you should NOT call this line for your regular code 
  ANA_CHECK_SET_TYPE (int);

  // Important to do this first!
  // Those lines are only included if using AnalysisBase
  ANA_CHECK (xAOD::Init()) ;
  #else
  // Those lines are only included if using AthAnalysis
  POOL::Init();
  #endif

  // Adopt same print formatting for AnalysisBase and AthAnalysis/Athena 
  // Do not remove those lines as otherwise 
  // the formatting of printed messages with ANA_MSG_INFO etc 
  // is not the same 
  #ifdef XAOD_STANDALONE
  // Those lines are only included if using AnalysisBase

  // See MsgStream::doOutput function and MessagePrinterOverlay class 
  // https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthToolSupport/AsgMessaging/Root/MsgStream.cxx
  // https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthToolSupport/AsgMessaging/AsgMessaging/MessagePrinterOverlay.h
  // https://gitlab.cern.ch/atlas/athena/-/blob/main/Control/AthToolSupport/AsgMessaging/Root/MessagePrinterOverlay.cxx
  asg::MessagePrinter *msgPrinter = new asg::MessagePrinter(25);
  asg::MessagePrinterOverlay msgPrinterOverlay(msgPrinter);
  
  #else 
  // Retrieve/Initialize message service 
  IMessageSvc* msgSvc = Athena::getMessageSvc();
  if (msgSvc) {
    IProperty* msgSvcProp = dynamic_cast<IProperty*>(msgSvc);
    if (msgSvcProp) {
      // See MsgStream::doOutput function and Message class 
      // https://gitlab.cern.ch/atlas/Gaudi/-/blob/master/GaudiKernel/src/Lib/MsgStream.cpp
      // https://gitlab.cern.ch/atlas/Gaudi/-/blob/master/GaudiKernel/include/GaudiKernel/Message.h
      msgSvcProp->setProperty("Format", "% F%25W%S%0W%L%T    %0W%M").ignore(); 
    }
  }
  #endif

  const char* TEST_NAME = argv[0];
  if (argc < 5) {
    ANA_MSG_ERROR ( "Incorrect inputs received!" );
    ANA_MSG_ERROR ( "Usage: " << TEST_NAME << "[DAOD] [CDI path] [b-tagger name] [WP name]" );
    return 1;
  }

  std::string DAODpath   = argv[1];
  std::string CDIPath    = argv[2];
  std::string taggerName = argv[3];
  std::string workingPointName = argv[4];
  std::string JetCollectionName = "AntiKt4EMPFlowJets";

  std::unique_ptr<TFile> root_file {TFile::Open(DAODpath.c_str(), "READ")};
  if (!root_file || root_file->IsZombie()) {
    ANA_MSG_ERROR("Failed to open input file: " << DAODpath);
    return 1;
  }
  
  //Importing MetaData to access DSID and get the correct SF
  TTree* metaTree = nullptr;
  root_file->GetObject("MetaData", metaTree);
  if (!metaTree) {
    ANA_MSG_ERROR("Could not find TTree 'MetaData' in file: " << DAODpath);
    return 1;
  }
 
  if (!metaTree->GetBranch("FileMetaDataAuxDyn.mcProcID")) {
    ANA_MSG_ERROR("Branch 'mcProcID' not found in MetaData tree.");
    return 1;
  }
  
  Float_t mcProcID = 0.0f;
  metaTree->SetBranchAddress("FileMetaDataAuxDyn.mcProcID", &mcProcID);

  if (metaTree->GetEntries() <= 0) {
    ANA_MSG_ERROR("MetaData tree has no entries.");
    return 1;
  }
  metaTree->GetEntry(0);
  
  unsigned int sample_dsid = static_cast<unsigned int>(mcProcID);
  ANA_MSG_INFO("Read sample DSID (mcProcID) = " << sample_dsid);

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
