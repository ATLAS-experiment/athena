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
#include <AsgTools/AsgToolConfig.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>
#include <xAODCore/ShallowCopy.h>

#include <ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;

TEST_F (ColumnarMemoryTest, AsgElectronEfficiencyCorrectionTool)
{
  if (!checkMode())
    return;

  asg::AsgToolConfig toolConfig;
  toolConfig.setTypeAndName ("AsgElectronEfficiencyCorrectionTool/" + makeUniqueName());
  ASSERT_SUCCESS (toolConfig.setProperty ("MapFilePath", "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"));
  ASSERT_SUCCESS (toolConfig.setProperty ("ForceDataType", unsigned (PATCore::ParticleDataType::Full)));
  ASSERT_SUCCESS (toolConfig.setProperty ("CorrelationModel", "TOTAL"));
  ASSERT_SUCCESS (toolConfig.setProperty ("RecoKey", "Reconstruction"));
  ToolHandle<asg::AsgTool> myToolHandle;
  std::shared_ptr<void> cleanup;
  ASSERT_SUCCESS (toolConfig.makeTool (myToolHandle, cleanup));
  ColumnarTestToolHandle toolHandle (*myToolHandle);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("EventInfo.RandomRunNumber", {284500});

  columnMap.addColumn ("Electrons", {0, 1});
  columnMap.addColumn ("Electrons.pt", {10e5});
  columnMap.addColumn ("Electrons.eta", {1});
  columnMap.addColumn ("Electrons.author", {unsigned(xAOD::EgammaParameters::AuthorElectron)});
  columnMap.addColumn ("Electrons.caloClusterLinks.data", {0});
  columnMap.addColumn ("Electrons.caloClusterLinks.offset", {0, columnMap.columnSize("Electrons.caloClusterLinks.data")});

  columnMap.addColumn ("egammaClusters", {0, 1});
  columnMap.addColumn ("egammaClusters.calE", {10e5 * cosh (1)});
  columnMap.addColumn ("egammaClusters.calEta", {1});
  columnMap.addColumn ("egammaClusters.samplingPattern", {0x4});
  columnMap.addColumn ("egammaClusters.e_sampl.data", {10e5});
  columnMap.addColumn ("egammaClusters.e_sampl.offset", {0, columnMap.columnSize("egammaClusters.e_sampl.data")});
  columnMap.addColumn ("egammaClusters.eta_sampl.data", {1});
  columnMap.addColumn ("egammaClusters.eta_sampl.offset", {0, columnMap.columnSize("egammaClusters.eta_sampl.data")});

  columnMap.addColumn ("Electrons.sfOut", {0});
  columnMap.addColumn ("Electrons.validOut", {0});

  columnMap.setExpectation ("Electrons.sfOut", {0.999632});
  columnMap.setExpectation ("Electrons.validOut", {1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}



void callXAOD (const AsgElectronEfficiencyCorrectionTool& tool, bool isPrepCall, const std::string& name) {
  using namespace asg::msgUserCode;
  if (isPrepCall)
  {
    const xAOD::ElectronContainer *electrons = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (electrons, name));
    auto [electronsCopy, auxCopy] = xAOD::shallowCopyContainer (*electrons);
    const xAOD::EventInfo *eventInfo = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (eventInfo, "EventInfo"));
    tool.callSingleEvent (*electronsCopy, *eventInfo);
    delete electronsCopy;
    delete auxCopy;
  }else
  {
    const xAOD::ElectronContainer *electrons = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (electrons, name));
    const xAOD::EventInfo *eventInfo = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (eventInfo, "EventInfo"));
    tool.callSingleEvent (*electrons, *eventInfo);
  }
}



// temporarily disabled until we have the new PHYSLITE test file
TEST_F (ColumnarPhysLiteTest, DISABLED_AsgElectronEfficiencyCorrectionTool)
{
  asg::AsgToolConfig toolConfig;
  toolConfig.setTypeAndName ("AsgElectronEfficiencyCorrectionTool/" + makeUniqueName());
  ASSERT_SUCCESS (toolConfig.setProperty ("MapFilePath", "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"));
  ASSERT_SUCCESS (toolConfig.setProperty ("ForceDataType", unsigned (PATCore::ParticleDataType::Full)));
  ASSERT_SUCCESS (toolConfig.setProperty ("CorrelationModel", "TOTAL"));
  ASSERT_SUCCESS (toolConfig.setProperty ("RecoKey", "Reconstruction"));
  ToolHandle<AsgElectronEfficiencyCorrectionTool> myToolHandle;
  std::shared_ptr<void> cleanup;
  ASSERT_SUCCESS (toolConfig.makeTool (myToolHandle, cleanup));

  doCall (*myToolHandle, "AsgElectronEfficiencyCorrectionTool", "AnalysisElectrons", [&] (auto& args) {callXAOD (*myToolHandle, args.isPrepCall, args.inputContainer);}, {{"Electrons", "AnalysisElectrons"}});
}

ATLAS_GOOGLE_TEST_MAIN
