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

#include <PhotonEfficiencyCorrection/AsgPhotonEfficiencyCorrectionTool.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;

TEST_F (ColumnarMemoryTest, AsgPhotonEfficiencyCorrectionTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<AsgPhotonEfficiencyCorrectionTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2025/rel22.2/2024_FinalRun2_Recommendation_v1/map1.txt"));
  ASSERT_SUCCESS (tool->setProperty ("ForceDataType", unsigned (PATCore::ParticleDataType::Full)));
  ASSERT_SUCCESS (tool->initialize ());
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("EventInfo.RandomRunNumber", {284500});

  columnMap.addColumn ("Photons", {0, 1});
  columnMap.addColumn ("Photons.eta", {1});
  columnMap.addColumn ("Photons.caloClusterLinks.data", {0});
  columnMap.addColumn ("Photons.caloClusterLinks.offset", {0, columnMap.columnSize("Photons.caloClusterLinks.data")});
  columnMap.addColumn ("Photons.vertexLinks.data", {0});
  columnMap.addColumn ("Photons.vertexLinks.offset", {0, columnMap.columnSize("Photons.vertexLinks.data")});

  columnMap.addColumn ("egammaClusters", {0, 1});
  columnMap.addColumn ("egammaClusters.calE", {10e5 * cosh (1)});
  columnMap.addColumn ("egammaClusters.samplingPattern", {0x4});
  columnMap.addColumn ("egammaClusters.e_sampl.data", {10e5});
  columnMap.addColumn ("egammaClusters.e_sampl.offset", {0, columnMap.columnSize("egammaClusters.e_sampl.data")});
  columnMap.addColumn ("egammaClusters.eta_sampl.data", {1});
  columnMap.addColumn ("egammaClusters.eta_sampl.offset", {0, columnMap.columnSize("egammaClusters.eta_sampl.data")});

  columnMap.addColumn ("GSFConversionVertices", {0, 1});
  columnMap.addColumn ("GSFConversionVertices.trackParticleLinks.data", {0, 1});
  columnMap.addColumn ("GSFConversionVertices.trackParticleLinks.offset", {0, columnMap.columnSize("GSFConversionVertices.trackParticleLinks.data")});

  columnMap.addColumn ("GSFTrackParticles", {0, 2});
  columnMap.addColumn ("GSFTrackParticles.numberOfPixelHits", {1, 1});
  columnMap.addColumn ("GSFTrackParticles.numberOfSCTHits", {1, 1});

  columnMap.addColumn ("Photons.sfOut", {0});
  columnMap.addColumn ("Photons.validOut", {0});

  columnMap.setExpectation ("Photons.sfOut", {0.99605429});
  columnMap.setExpectation ("Photons.validOut", {1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}



void callXAOD (const AsgPhotonEfficiencyCorrectionTool& tool, bool isPrepCall, const std::string& name) {
  using namespace asg::msgUserCode;
  if (isPrepCall)
  {
    const xAOD::PhotonContainer *photons = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (photons, name));
    auto [photonsCopy, auxCopy] = xAOD::shallowCopyContainer (*photons);
    const xAOD::EventInfo *eventInfo = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (eventInfo, "EventInfo"));
    tool.callSingleEvent (*photonsCopy, *eventInfo);
    delete photonsCopy;
    delete auxCopy;
  }else
  {
    const xAOD::PhotonContainer *photons = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (photons, name));
    const xAOD::EventInfo *eventInfo = nullptr;
    ANA_CHECK_THROW (tool.evtStore()->retrieve (eventInfo, "EventInfo"));
    tool.callSingleEvent (*photons, *eventInfo);
  }
}



TEST_F (ColumnarPhysLiteTest, AsgPhotonEfficiencyCorrectionTool)
{
  auto tool = std::make_unique<AsgPhotonEfficiencyCorrectionTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("MapFilePath", "PhotonEfficiencyCorrection/2015_2025/rel22.2/2024_FinalRun2_Recommendation_v1/map1.txt"));
  ASSERT_SUCCESS (tool->setProperty ("ForceDataType", unsigned (PATCore::ParticleDataType::Full)));

  // add a dummy column for the test
  columnar::VertexAccessor<float> dummyColumn (*tool, "x");

  ASSERT_SUCCESS (tool->initialize ());

  doCall (*tool, "AsgPhotonEfficiencyCorrectionTool", "AnalysisPhotons", [&] (auto& args) {callXAOD (*tool, args.isPrepCall, args.inputContainer);}, {{"Photons", "AnalysisPhotons"}});
}

ATLAS_GOOGLE_TEST_MAIN
