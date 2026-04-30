/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

// System include(s):
#include <memory>
#include <string>
// ROOT include(s):
#include <TFile.h>
#include <TString.h>
// Infrastructure include(s):
#ifdef XAOD_STANDALONE
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"
#endif // XAOD_STANDALONE
// Asg includes
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
// Local includes
#include "CreateDummyEl.h"
#include "SFHelpers.h"
//
#include <boost/program_options.hpp>

#include "AsgMessaging/MessageCheck.h"
#include "AsgMessaging/MsgStream.h"
#include "AsgTools/StandaloneToolHandle.h"

namespace asg {
ANA_MSG_HEADER(msgSelectorCheck)
ANA_MSG_SOURCE(msgSelectorCheck, "")
}

int
test1(int argc, char* argv[])
{
  StatusCode::enableFailure();
  using namespace asg::msgSelectorCheck;
  ANA_CHECK_SET_TYPE(int);
  setMsgLevel(MSG::INFO);
  /*
   * Parse the input from the command line
   */
  std::string fileName{};
  std::string mapfileName{};
  std::string idkey{};
  std::string recokey{};
  std::string isokey{};
  std::string triggerkey{};
  std::string model{};
  int runno{};
  float eta{};
  float pt{};
  std::string type{};
  int inputlevel{};

  using namespace boost::program_options;
  options_description desc{ "Options" };
  desc.add_options()("help,h", "Help screen")(
    "msgLevel,l",
    value<int>(&inputlevel)->default_value(static_cast<int>(MSG::INFO)),
    "message level")("file,f",
                     value<std::string>(&fileName)->default_value(""),
                     "scale factor file")(
    "runno,r", value<int>(&runno)->required(), "run number: Required")(
    "eta,e", value<float>(&eta)->required(), "eta: Required")(
    "pt,p", value<float>(&pt)->required(), "pt: Required")(
    "type,t",
    value<std::string>(&type)->required(),
    "Simulation type: Required")(
    "correlation,c",
    value<std::string>(&model)->default_value("FULL"),
    "Correlation Model FULL (default),SIMPLIFIED,TOTAL,COMBMCTOYS")(
    "keyreco,k",
    value<std::string>(&recokey)->default_value(""),
    "Reco working point Key")(
    "mapfile,m",
    value<std::string>(&mapfileName)->default_value(""),
    "Map file name")("keyid,d",
                     value<std::string>(&idkey)->default_value(""),
                     "Identification working point Key")(
    "keyiso,i",
    value<std::string>(&isokey)->default_value(""),
    "Isolation working point Key")(
    "keytrigger,g",
    value<std::string>(&triggerkey)->default_value(""),
    "Trigger working point Key");
  variables_map vm;
  try {
    store(parse_command_line(argc, argv, desc), vm);
    if (vm.count("help")) {
      ANA_MSG_INFO(desc << '\n');
      return 0;
    }
    notify(vm);
  } catch (const boost::program_options::error& ex) {
    ANA_MSG_ERROR(ex.what() << '\n');
    ANA_MSG_ERROR(desc << '\n');
    return 0;
  }

  if (!(type == "FullSim" || type == "AtlFast2")) {
    ANA_MSG_ERROR("No valid type given (FullSim or AtlFast2)");
    ANA_MSG_ERROR(desc << '\n');
    return 0;
  }
  PATCore::ParticleDataType::DataType SimType =
    (type == "FullSim" ? PATCore::ParticleDataType::Full
                       : PATCore::ParticleDataType::Fast);
  if (!(model == "COMBMCTOYS" || model == "FULL" || model == "SIMPLIFIED" ||
        model == "TOTAL")) {
    ANA_MSG_ERROR("No valid correlation model");
    ANA_MSG_ERROR(desc << '\n');
    return 0;
  }

  // Initialize the xAOD application
  const char* APP_NAME = argv[0];
  msg().setName(APP_NAME);
  MSG::Level mylevel = static_cast<MSG::Level>(inputlevel);
  setMsgLevel(mylevel);
  msg().setName(APP_NAME);
  ANA_CHECK(xAOD::Init(APP_NAME));

  // Initialize the store
  xAOD::TEvent event(xAOD::TEvent::kClassAccess);
  xAOD::TStore store;

  // Configure the tool based on the inputs
  asg::StandaloneToolHandle<IAsgElectronEfficiencyCorrectionTool>
    ElEffCorrectionTool(
      "AsgElectronEfficiencyCorrectionTool/ElEffCorrectionTool");
  if (!fileName.empty()) {
    std::vector<std::string> inputFiles{ fileName };
    ANA_CHECK(
      ElEffCorrectionTool.setProperty("CorrectionFileNameList", inputFiles));
  }
  if (!mapfileName.empty()) {
    ANA_CHECK(ElEffCorrectionTool.setProperty("MapFilePath", mapfileName));
  }
  if (!recokey.empty()) {
    ANA_CHECK(ElEffCorrectionTool.setProperty("RecoKey", recokey));
  }
  if (!idkey.empty()) {
    ANA_CHECK(ElEffCorrectionTool.setProperty("IdKey", idkey));
  }
  if (!isokey.empty()) {
    ANA_CHECK(ElEffCorrectionTool.setProperty("IsoKey", isokey));
  }
  if (!triggerkey.empty()) {
    ANA_CHECK(ElEffCorrectionTool.setProperty("TriggerKey", triggerkey));
  }
  ANA_CHECK(ElEffCorrectionTool.setProperty("ForceDataType", (int)SimType));
  ANA_CHECK(ElEffCorrectionTool.setProperty("OutputLevel", mylevel));
  ANA_CHECK(ElEffCorrectionTool.setProperty("CorrelationModel", model));
  ANA_CHECK(ElEffCorrectionTool.initialize());
  if (mylevel < MSG::INFO) {
    asg::ToolStore::dumpToolConfig();
  }
  /*
   * create and then retrieve the dummy electrons (in this case is 1)
   * This also setups the store,so for done before the tool init
   * it really is a pseudo "reconstruction"
   */
  std::vector<std::pair<double, double>> pt_eta{ { pt, eta } };
  ANA_CHECK(getElectrons(pt_eta, runno, store).isSuccess());
  const xAOD::ElectronContainer* electrons(nullptr);
  ANA_CHECK(store.retrieve(electrons, "MyElectrons").isSuccess());
  // Loop over electrons , here it is one
  xAOD::Electron el = *(electrons->at(0));
  /*
   * Potentiallly we can make this part more clever, for now since it is an
   * util I did not try to optimise too much.
   */
  int index = ElEffCorrectionTool->systUncorrVariationIndex(el);
  /*
   * Set up the systematic variations
   * 2 cases one for "continuous" one for "toys"
   */
  bool isToys = model.find("TOY") != std::string::npos;
  double nominalSF{};
  double totalNeg{};
  double totalPos{};
  ANA_CHECK(SFHelpers::result(
              ElEffCorrectionTool, el, nominalSF, totalPos, totalNeg, isToys) ==
            0);

  ANA_MSG_INFO("===> Model : " << model << "| electron : Pt = " << el.pt()
                               << " : eta = " << el.eta() << " : Bin index = "
                               << index << " : SF = " << nominalSF << " + "
                               << totalPos << " - " << totalNeg << " <===");

  return 0;
}


int main (int argc, char *argv[])
{
  try {
    return test1(argc, argv);
  } catch (const std::exception& e) {
    std::cerr << "exception: " << e.what() << "\n";
    return 1;
  }
}
