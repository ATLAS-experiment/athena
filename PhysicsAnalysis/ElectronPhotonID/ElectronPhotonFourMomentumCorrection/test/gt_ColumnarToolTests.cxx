/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgTesting/UnitTest.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>
#include <xAODCore/ShallowCopy.h>

#include <ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h>
#include <TruthUtils/ParticleConstants.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;

TEST_F (ColumnarMemoryTest, EgammaCalibrationAndSmearingTool)
{
  if (!checkMode())
    return;

  auto tool = new CP::EgammaCalibrationAndSmearingTool (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ESModel", "es2022_R22_PRE"));
  ASSERT_SUCCESS (tool->setProperty ("decorrelationModel", "1NP_v1"));
  ASSERT_SUCCESS (tool->setProperty ("useFastSim", 0));
  ASSERT_SUCCESS (tool->setProperty ("useMVACalibration", 0));
  ASSERT_SUCCESS (tool->setProperty ("useLayerCorrection", 0));
  ASSERT_SUCCESS (tool->setProperty ("onlyElectrons", 1));
  ASSERT_SUCCESS (tool->initialize ());
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.renameContainers ({{"EGamma", "Electrons"}});
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("EventInfo.runNumber", {284500});
  columnMap.addColumn ("EventInfo.RandomRunNumber", {284500});
  columnMap.addColumn ("EventInfo.eventNumber", {1234});
  columnMap.addColumn ("EventInfo.eventTypeBitmask", {unsigned (xAOD::EventInfo::IS_SIMULATION)});

  columnMap.addColumn ("Electrons", {0, 1});
  columnMap.addColumn ("Electrons.pt", {10e5});
  columnMap.addColumn ("Electrons.eta", {1});
  columnMap.addColumn ("Electrons.phi", {1});
  columnMap.addColumn ("Electrons.m", {ParticleConstants::electronMassInMeV});
  columnMap.addColumn ("Electrons.author", {unsigned (xAOD::EgammaParameters::AuthorElectron)});
  columnMap.addColumn ("Electrons.caloClusterLinks.data", {0});
  columnMap.addColumn ("Electrons.caloClusterLinks.offset", {0, columnMap.columnSize("Electrons.caloClusterLinks.data")});

  columnMap.addColumn ("egammaClusters", {0, 1});
  columnMap.addColumn ("egammaClusters.calEta", {1});
  columnMap.addColumn ("egammaClusters.calPhi", {1});
  columnMap.addColumn ("egammaClusters.ETACALOFRAME", {1});
  columnMap.addColumn ("egammaClusters.PHICALOFRAME", {1});
  columnMap.addColumn ("egammaClusters.samplingPattern", {0});
  columnMap.addColumn ("egammaClusters.e_sampl.data", {10e5});
  columnMap.addColumn ("egammaClusters.e_sampl.offset", {0, columnMap.columnSize("egammaClusters.e_sampl.data")});
  columnMap.addColumn ("egammaClusters.eta_sampl.data", {1});
  columnMap.addColumn ("egammaClusters.eta_sampl.offset", {0, columnMap.columnSize("egammaClusters.eta_sampl.data")});

  columnMap.addColumn ("Electrons.ptOut", {0});

  // there is no special reason for this value, it is just what came
  // out of my first test run.  if the tool changes, feel free to
  // update this value.
  columnMap.setExpectation ("Electrons.ptOut", {986918});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}

void callXAOD (const CP::EgammaCalibrationAndSmearingTool& tool, const std::string& name, const std::string& outputName) {
  using namespace asg::msgUserCode;
  const xAOD::ElectronContainer *egammas = nullptr;
  ANA_CHECK_THROW (tool.evtStore()->retrieve (egammas, name));
  auto [egammasCopy, auxCopy] = xAOD::shallowCopyContainer (*egammas);
  const xAOD::EventInfo *eventInfo = nullptr;
  ANA_CHECK_THROW (tool.evtStore()->retrieve (eventInfo, "EventInfo"));
  tool.callSingleEvent (*egammasCopy, *eventInfo);
  ANA_CHECK_THROW (tool.evtStore()->record (egammasCopy, outputName));
  ANA_CHECK_THROW (tool.evtStore()->record (auxCopy, outputName + "Aux."));
}

TEST_F (ColumnarPhysLiteTest, EgammaCalibrationAndSmearingTool)
{
  auto tool = new CP::EgammaCalibrationAndSmearingTool (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("ESModel", "es2022_R22_PRE"));
  ASSERT_SUCCESS (tool->setProperty ("decorrelationModel", "1NP_v1"));
  ASSERT_SUCCESS (tool->setProperty ("useFastSim", 0));
  ASSERT_SUCCESS (tool->setProperty ("useMVACalibration", 0));
  ASSERT_SUCCESS (tool->setProperty ("useLayerCorrection", 0));
  ASSERT_SUCCESS (tool->setProperty ("onlyElectrons", 1));
  ASSERT_SUCCESS (tool->initialize ());

  doCall (*tool, "EgammaCalibrationAndSmearingTool", "AnalysisElectrons", [&] (auto& args) {callXAOD (*tool, args.inputContainer, args.outputContainer);}, {{"EGamma", "AnalysisElectrons"}});
}

ATLAS_GOOGLE_TEST_MAIN
