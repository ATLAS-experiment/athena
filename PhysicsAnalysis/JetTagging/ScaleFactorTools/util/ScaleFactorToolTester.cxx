/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "FTagAnalysisInterfaces/IScaleFactorTool.h"
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

int main(int argc, char* argv[]) {

  if (argc < 3) {
    ANA_MSG_ERROR (  "No right inputs received!" );
    return 1;
  }

  std::string inputDAOD = argv[1];
  std::string JsonConfigFile = argv[2];
  std::string taggerName = argv[3];
  std::string objContainer = "AntiKt4EMPFlowJets";

  asg::StandaloneToolHandle<IScaleFactorTool> sel_tool("ScaleFactorTool/SFToolTest");
  StatusCode sel_code1 = sel_tool.setProperty( "TaggerName", taggerName);
  StatusCode sel_code2 = sel_tool.setProperty( "JsonConfigFile", JsonConfigFile);
  StatusCode sel_code3 = sel_tool.setProperty( "ObjContainer", objContainer);
  StatusCode sel_code4 = sel_tool.initialize();
  std::vector<StatusCode> sel_codes = { sel_code1, sel_code2, sel_code3, sel_code4};

  for(const auto& code : sel_codes) {
    if(code.isFailure()) {
      ANA_MSG_ERROR("Failed to set property or initialize tool");
      return 1;
    }
  }

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
      float pb = SG::AuxElement::ConstAccessor<float>(taggerName+"_pb")(*particle);
      float pc = SG::AuxElement::ConstAccessor<float>(taggerName+"_pc")(*particle);
      float pu = SG::AuxElement::ConstAccessor<float>(taggerName+"_pu")(*particle);
      float ptau = SG::AuxElement::ConstAccessor<float>(taggerName+"_ptau")(*particle);
      ANA_MSG_INFO("pb : pc : pl : ptau = " << pb << ", " << pc << ", " << pu << ", " << ptau);
      float sf = 1.;
      sf = sel_tool->getSF(particle);
      ANA_MSG_INFO("SF = " << sf );
    }
  }

  root_file->Close();
  root_file.reset();
  return 0;
}
