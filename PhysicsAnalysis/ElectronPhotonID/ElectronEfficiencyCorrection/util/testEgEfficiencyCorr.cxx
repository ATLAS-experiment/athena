/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

// Local includes
#include "ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h"
#include "SFHelpers.h"
// System include(s):
#include <memory>
// ROOT include(s):
#include <TFile.h>
// Infrastructure include(s):
#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/TStore.h"

// EDM include(s):
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"

#include "AsgMessaging/MessageCheck.h"
#include "AsgMessaging/MsgStream.h"
#include "AsgTools/StandaloneToolHandle.h"
// To disable sending data
#include "xAODRootAccess/tools/TFileAccessTracer.h"

namespace asg {
ANA_MSG_HEADER(testEgEfficiencyCorr)
ANA_MSG_SOURCE(testEgEfficiencyCorr, "")
}

int
main(int argc, char* argv[])
{

  xAOD::TFileAccessTracer::enableDataSubmission(false);
  // The application's name:
  const char* APP_NAME = argv[0];

  using namespace asg::testEgEfficiencyCorr;
  ANA_CHECK_SET_TYPE(int);
  MSG::Level mylevel = MSG::INFO;
  setMsgLevel(mylevel);
  msg().setName(APP_NAME);

  // Check if we received a file name:
  if (argc < 2) {
    ANA_MSG_ERROR(APP_NAME << "No file name received!");
    ANA_MSG_ERROR(
      APP_NAME
      << "  Usage: <<APP_NAME <<  [xAOD file name] [Num of events to use]");
    return 1;
  }

  // Initialise the application:
  ANA_CHECK(xAOD::Init(APP_NAME));

  // Open the input file:
  const TString fileName = argv[1];
  ANA_MSG_INFO("Opening file: " << fileName.Data());
  std::unique_ptr<TFile> ifile(TFile::Open(fileName, "READ"));
  ANA_CHECK(ifile.get());

  // Create a TEvent object:
  xAOD::TEvent event;

  // Then the tools
  asg::StandaloneToolHandle<IAsgElectronEfficiencyCorrectionTool>
    ElEffCorrectionTool(
      "AsgElectronEfficiencyCorrectionTool/ElEffCorrectionTool");
  ANA_CHECK(ElEffCorrectionTool.setProperty("IdKey", "Medium"));
  ANA_CHECK(ElEffCorrectionTool.setProperty("ForceDataType", 1));
  ANA_CHECK(ElEffCorrectionTool.setProperty("OutputLevel", mylevel));
  ANA_CHECK(ElEffCorrectionTool.setProperty("CorrelationModel", "FULL"));
  ANA_CHECK(ElEffCorrectionTool.setProperty("UseRandomRunNumber", false));
  ANA_CHECK(ElEffCorrectionTool.initialize());

  // Then open the file(s)
  ANA_CHECK(event.readFrom(ifile.get()));
  ANA_MSG_INFO("Number of available events to read in:  "
               << static_cast<long long int>(event.getEntries()));

  // Decide how many events to run over:
  long long int entries = event.getEntries();
  if (argc > 2) {
    const long long int e = atoll(argv[2]);
    if (e < entries) {
      entries = e;
    }
  }
  ANA_MSG_INFO("Number actual events to read in:  " << entries);

  // Loop over the events:
  for (long long int entry = 0; entry < entries; ++entry) {
    event.getEntry(entry);
    ANA_MSG_INFO(" \n ==> Event " << entry);

    const xAOD::ElectronContainer* electrons = nullptr;
    ANA_CHECK(event.retrieve(electrons, "Electrons"));

    for (const xAOD::Electron* el : *electrons) {
      const xAOD::CaloCluster* cluster = el->caloCluster();
      if (!cluster) {
        ANA_MSG_ERROR("ERROR no cluster associated to the Electron \n");
        return CP::CorrectionCode::Error;
      }
      if (el->pt() < 7000)
        continue; // skip electrons outside of recommendations
      if (std::abs(cluster->etaBE(2)) >= 2.47) {
        continue;
      }

      int index = ElEffCorrectionTool->systUncorrVariationIndex(*el);
      /*
       * Set up the systematic variations
       */
      bool isToys = false;
      double nominalSF{};
      double totalNeg{};
      double totalPos{};
      ANA_CHECK(
        SFHelpers::result(
          ElEffCorrectionTool, *el, nominalSF, totalPos, totalNeg, isToys) ==
        0);

      ANA_MSG_INFO("===> electron : Pt = "
                   << el->pt() << " : eta = " << el->eta()
                   << " : Bin index = " << index << " : SF = " << nominalSF
                   << " + " << totalPos << " - " << totalNeg << " <===");
    }
  }

  ANA_MSG_INFO("===> DONE <===\n");
  return 0;
}

