/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FTagAnalysisInterfaces/IFTagScaleFactorTool.h"
#include <AsgTools/StandaloneToolHandle.h>
#include "xAODBase/IParticleContainer.h"

#ifdef XAOD_STANDALONE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
using TEVENT = xAOD::TEvent;
#else
#include "POOLRootAccess/TEvent.h"
using TEVENT = POOL::TEvent;
#endif

#include "TFile.h"

ANA_MSG_HEADER(testSFTool)
ANA_MSG_SOURCE(testSFTool, "ScaleFactorToolTester")
using namespace testSFTool;

int main ATLAS_NOT_THREAD_SAFE (int argc, char* argv[]) {

  if (argc < 5) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    return 1;
  }

  std::string inputDAOD = argv[1];
  std::string JsonConfigFile = argv[2];
  std::string taggerName = argv[3];
  std::string PCTName = argv[4];
  std::string objContainer = "AntiKt4EMPFlowJets";

  asg::StandaloneToolHandle<ftag::IFTagScaleFactorTool> sf_tool("ftag::FTagScaleFactorTool/SFToolTest");
  StatusCode sf_code1 = sf_tool.setProperty( "TaggerName", taggerName);
  StatusCode sf_code2 = sf_tool.setProperty( "JsonConfigFile", JsonConfigFile);
  StatusCode sf_code3 = sf_tool.setProperty( "ObjContainer", objContainer);
  StatusCode sf_code4 = sf_tool.setProperty( "PCTName", PCTName);
  StatusCode sf_code5 = sf_tool.initialize();
  std::vector<StatusCode> sf_codes = { sf_code1, sf_code2, sf_code3, sf_code4, sf_code5};

  for(const auto& code : sf_codes) {
    if(code.isFailure()) {
      ANA_MSG_ERROR("Failed to set property or initialize tool");
      return 1;
    }
  }

  CP::SystematicSet sysSet;
  sysSet = sf_tool->recommendedSystematics();

  TEVENT event(TEVENT::kClassAccess);
  gErrorIgnoreLevel = kError;
  std::unique_ptr<TFile> root_file {TFile::Open(inputDAOD.c_str(), "READ")};
  if(!event.readFrom(root_file.get()).isSuccess()) {
    ANA_MSG_ERROR ( "Accessing events in input file " << inputDAOD << "failed! " );
    return 1;
  }

  for(auto entry=0; entry < 20; ++entry) {
    event.getEntry(entry);

    const xAOD::IParticleContainer* particles = nullptr;
    if(!event.retrieve(particles, objContainer).isSuccess()) {
      ANA_MSG_ERROR("Failed to retrieve jets");
      return 1;
    }

    for (const auto particle : *particles) {
      ANA_MSG_INFO("====================================");
      ANA_MSG_INFO("Jet pt: " << particle->pt() << "eta: " << particle->eta() );
      float pb = SG::ConstAccessor<float>(taggerName+"_pb")(*particle);
      float pc = SG::ConstAccessor<float>(taggerName+"_pc")(*particle);
      float pu = SG::ConstAccessor<float>(taggerName+"_pu")(*particle);
      float ptau = SG::ConstAccessor<float>(taggerName+"_ptau")(*particle);
      ANA_MSG_INFO("pb : pc : pl : ptau = " << pb << ", " << pc << ", " << pu << ", " << ptau);
      std::unordered_map<std::string, int> wp_map = sf_tool->inferWPs(particle);
      ANA_MSG_INFO("PCT score: " << wp_map[PCTName]);

      float sf = 1.0f;
      for (const auto& var : sysSet) {
        CP::SystematicSet set;
        set.insert(var);
        CP::CorrectionCode code = sf_tool->getScaleFactor(*particle, sf, set);
        if (code == CP::CorrectionCode::Ok || code == CP::CorrectionCode::OutOfValidityRange) {
          ANA_MSG_INFO("Applied systematic: " << var.name());
          ANA_MSG_INFO("                   SF: " << sf);
        } else {
          ANA_MSG_ERROR("Failed to get scale factor for jet");
        }
      }
    }
  }

  root_file->Close();
  root_file.reset();
  return 0;
}


