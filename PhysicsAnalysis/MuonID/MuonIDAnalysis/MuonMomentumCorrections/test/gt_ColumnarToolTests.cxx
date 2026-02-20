/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



//
// includes
//

#include "CxxUtils/checker_macros.h"
ATLAS_NO_CHECK_FILE_THREAD_SAFETY;

#include <AsgMessaging/AsgMessaging.h>
#include <AsgTesting/UnitTest.h>
#include <AsgTools/AsgToolConfig.h>
#include <ColumnarTestFixtures/ColumnarMemoryTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>
#include <xAODCore/ShallowCopy.h>

#include <MuonMomentumCorrections/MuonCalibTool.h>
#include <MuonAnalysisInterfaces/IMuonSelectionTool.h>
#include <xAODEventInfo/EventInfo.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarMemoryTest;
using columnar::ColumnarPhysLiteTest;
using columnar::TestUtils::IXAODToolCaller;

TEST_F (ColumnarMemoryTest, MuonCalibTool)
{
  if (!checkMode())
    return;

  auto tool = std::make_unique<CP::MuonCalibTool> (makeUniqueName());
  ASSERT_SUCCESS (tool->setProperty ("IsRun3Geo", false));
  ASSERT_SUCCESS (tool->setProperty ("calibMode", 0));
  ASSERT_SUCCESS (tool->setProperty ("ExcludeNSWFromPrecisionLayers", false));
  ASSERT_SUCCESS (tool->setProperty ("skipResolutionCategory", true));
  ASSERT_SUCCESS (tool->initialize());
  ColumnarTestToolHandle toolHandle (*tool);
  toolHandle.initialize ();

  for (auto& name : toolHandle.getColumnNames())
    std::cout << "requested column: " << name << std::endl;
  std::cout << "recommended systematics size: " << toolHandle.getRecommendedSystematics().size() << std::endl;

  ColumnMapType columnMap {toolHandle};

  columnMap.addColumn ("EventInfo", {0, 1});
  columnMap.addColumn ("EventInfo.runNumber", {284500}); // no specific reason, from ASG test file
  columnMap.addColumn ("EventInfo.RandomRunNumber", {284500}); // copy from above
  columnMap.addColumn ("EventInfo.eventNumber", {1234});
  columnMap.addColumn ("EventInfo.eventTypeBitmask", {unsigned(xAOD::EventInfo::IS_SIMULATION)});

  columnMap.addColumn ("Muons", {0, 1});
  columnMap.addColumn ("Muons.pt", {10e5});
  columnMap.addColumn ("Muons.eta", {1});
  columnMap.addColumn ("Muons.phi", {1});
  columnMap.addColumn ("Muons.charge", {1});
  columnMap.addColumn ("Muons.muonType", {unsigned(xAOD::Muon::MuonType::Combined)});
  columnMap.addColumn ("Muons.author", {unsigned(xAOD::Muon::Author::CaloTag)});
  columnMap.addColumn ("Muons.extrapolatedMuonSpectrometerTrackParticleLink", {0});
  columnMap.addColumn ("Muons.combinedTrackParticleLink", {0});
  columnMap.addColumn ("Muons.inDetTrackParticleLink", {0});
  columnMap.addColumn ("Muons.inDetTrackParticleLink.keys", {0, 1, 2, 3});

  columnMap.addColumn ("InDetTrackParticles", {0, 1});
  columnMap.addColumn ("InDetTrackParticles.d0", {0});
  columnMap.addColumn ("InDetTrackParticles.z0", {0});
  columnMap.addColumn ("InDetTrackParticles.phi", {0});
  columnMap.addColumn ("InDetTrackParticles.theta", {0});
  columnMap.addColumn ("InDetTrackParticles.qOverP", {0});
  columnMap.addColumn ("InDetTrackParticles.definingParametersCovMatrixDiag.data", {0, 0, 0, 0, 0});
  columnMap.addColumn ("InDetTrackParticles.definingParametersCovMatrixDiag.offset", {0, 5});
  columnMap.addColumn ("InDetTrackParticles.definingParametersCovMatrixOffDiag.data", {0, 0, 0, 0, 0, 0});
  columnMap.addColumn ("InDetTrackParticles.definingParametersCovMatrixOffDiag.offset", {0, 6});
  columnMap.addColumn ("InDetForwardTrackParticles", {0, 1});
  columnMap.addColumn ("InDetForwardTrackParticles.d0", {0});
  columnMap.addColumn ("InDetForwardTrackParticles.z0", {0});
  columnMap.addColumn ("InDetForwardTrackParticles.phi", {0});
  columnMap.addColumn ("InDetForwardTrackParticles.theta", {0});
  columnMap.addColumn ("InDetForwardTrackParticles.qOverP", {0});
  columnMap.addColumn ("InDetForwardTrackParticles.definingParametersCovMatrixDiag.data", {0, 0, 0, 0, 0});
  columnMap.addColumn ("InDetForwardTrackParticles.definingParametersCovMatrixDiag.offset", {0, 5});
  columnMap.addColumn ("InDetForwardTrackParticles.definingParametersCovMatrixOffDiag.data", {0, 0, 0, 0, 0, 0});
  columnMap.addColumn ("InDetForwardTrackParticles.definingParametersCovMatrixOffDiag.offset", {0, 6});
  columnMap.addColumn ("CombinedMuonTrackParticles", {0, 1});
  columnMap.addColumn ("CombinedMuonTrackParticles.d0", {0});
  columnMap.addColumn ("CombinedMuonTrackParticles.z0", {0});
  columnMap.addColumn ("CombinedMuonTrackParticles.phi", {0});
  columnMap.addColumn ("CombinedMuonTrackParticles.theta", {0});
  columnMap.addColumn ("CombinedMuonTrackParticles.qOverP", {0});
  columnMap.addColumn ("CombinedMuonTrackParticles.definingParametersCovMatrixDiag.data", {0, 0, 0, 0, 0});
  columnMap.addColumn ("CombinedMuonTrackParticles.definingParametersCovMatrixDiag.offset", {0, 5});
  columnMap.addColumn ("CombinedMuonTrackParticles.definingParametersCovMatrixOffDiag.data", {0, 0, 0, 0, 0, 0});
  columnMap.addColumn ("CombinedMuonTrackParticles.definingParametersCovMatrixOffDiag.offset", {0, 6});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles", {0, 1});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.d0", {0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.z0", {0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.phi", {0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.theta", {0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.qOverP", {0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixDiag.data", {0, 0, 0, 0, 0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixDiag.offset", {0, 5});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixOffDiag.data", {0, 0, 0, 0, 0, 0});
  columnMap.addColumn ("ExtrapolatedMuonTrackParticles.definingParametersCovMatrixOffDiag.offset", {0, 6});

  columnMap.addColumn ("Muons.ptOut", {0});
  columnMap.addColumn ("Muons.chargeOut", {0});
  columnMap.addColumn ("Muons.InnerDetectorPt", {0});
  columnMap.addColumn ("Muons.InnerDetectorCharge", {0});
  columnMap.addColumn ("Muons.MuonSpectrometerPt", {0});
  columnMap.addColumn ("Muons.MuonSpectrometerCharge", {0});

  columnMap.setExpectation ("Muons.ptOut", {0});
  columnMap.setExpectation ("Muons.chargeOut", {1});
  columnMap.setExpectation ("Muons.InnerDetectorPt", {0});
  columnMap.setExpectation ("Muons.InnerDetectorCharge", {1});
  columnMap.setExpectation ("Muons.MuonSpectrometerPt", {0});
  columnMap.setExpectation ("Muons.MuonSpectrometerCharge", {1});

  columnMap.connectColumnsToTool ();

  columnMap.call ();

  columnMap.checkExpectations ();
}

class XAODTestToolCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODTestToolCaller (const CP::MuonCalibTool& tool, const std::string& name)
    : AsgMessaging ("XAODTestToolCaller"), m_tool (tool), m_name (name)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_muons, m_name));
    ANA_CHECK (evtStore.retrieve (m_eventInfo, "EventInfo"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    auto [muonsCopy, muonsAuxCopy] = xAOD::shallowCopyContainer (*m_muons);
    m_muons = muonsCopy;
    ANA_CHECK (evtStore.record (muonsCopy, m_name + postfix));
    ANA_CHECK (evtStore.record (muonsAuxCopy, m_name + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_muons, *m_eventInfo);
    return StatusCode::SUCCESS;
  }

private:
  const CP::MuonCalibTool& m_tool;
  std::string m_name;

  const xAOD::MuonContainer *m_muons = nullptr;
  const xAOD::EventInfo *m_eventInfo = nullptr;
};

TEST_F (ColumnarPhysLiteTest, MuonCalibTool)
{
  asg::AsgToolConfig toolConfig;
  toolConfig.setTypeAndName ("CP::MuonCalibTool/" + makeUniqueName());
  ASSERT_SUCCESS (toolConfig.setProperty ("IsRun3Geo", false));
  ASSERT_SUCCESS (toolConfig.setProperty ("calibMode", 0));
  ASSERT_SUCCESS (toolConfig.setProperty ("ExcludeNSWFromPrecisionLayers", false));
  ASSERT_SUCCESS (toolConfig.setProperty ("skipResolutionCategory", true));
  ToolHandle<CP::MuonCalibTool> myToolHandle;
  std::shared_ptr<void> cleanup;
  ASSERT_SUCCESS (toolConfig.makeTool (myToolHandle, cleanup));

  XAODTestToolCaller callXAOD (*myToolHandle, "AnalysisMuons");

  doCall ({.tool = &*myToolHandle, .name = "MuonCalibTool", .xAODToolCaller = &callXAOD, .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

ATLAS_GOOGLE_TEST_MAIN
