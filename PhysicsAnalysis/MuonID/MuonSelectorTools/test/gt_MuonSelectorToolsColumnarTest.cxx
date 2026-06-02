/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// Columnar-framework equivalent of ut_MuonSelectorToolsTester_{data,MC}.sh.
///
/// Uses ColumnarPhysLiteTest to exercise callSingleEvent on a PHYSLITE file
/// (ASG_TEST_FILE_LITE_MC) for all seven standard working points.  The xAOD
/// caller runs in mode 0 (default); the columnar column test runs in mode 2.

#include <AsgMessaging/AsgMessaging.h>
#include <AsgTesting/UnitTest.h>
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>
#include <ColumnarTestFixtures/IXAODToolCaller.h>
#include <MuonSelectorTools/MuonSelectionTool.h>
#include <ColumnarEventInfo/EventInfoDef.h>
#include <xAODEventInfo/EventInfo.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODCore/ShallowCopy.h>

using columnar::ColumnarPhysLiteTest;
using columnar::TestUtils::IXAODToolCaller;

/// xAOD caller wrapper for one MuonSelectionTool instance.
///
/// retrieve() reads the original (const) container from the event store;
/// copyRecord() makes a shallow copy that the tool can decorate;
/// call() runs callSingleEvent on the non-const shallow-copy container.
class XAODMuonSelectionToolCaller final
  : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODMuonSelectionToolCaller (const CP::MuonSelectionTool& tool,
                                const std::string& containerName)
    : AsgMessaging ("XAODMuonSelectionToolCaller"),
      m_tool (tool), m_containerName (containerName)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    ANA_CHECK (evtStore.retrieve (m_retrieved,  m_containerName));
    ANA_CHECK (evtStore.retrieve (m_eventInfo, "EventInfo"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore,
                                 const std::string& postfix) override
  {
    auto [copy, auxCopy] = xAOD::shallowCopy (*m_retrieved);
    m_muons = copy.get();

    ANA_CHECK (evtStore.record (std::move (copy),    m_containerName + postfix));
    ANA_CHECK (evtStore.record (std::move (auxCopy), m_containerName + postfix + "Aux."));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_muons, columnar::EventInfoId (*m_eventInfo));
    return StatusCode::SUCCESS;
  }

  virtual void clear () override
  {
    m_retrieved = nullptr;
    m_muons     = nullptr;
    m_eventInfo = nullptr;
  }

private:
  const CP::MuonSelectionTool&    m_tool;
  std::string                     m_containerName;
  const xAOD::MuonContainer*      m_retrieved = nullptr;
  xAOD::MuonContainer*            m_muons     = nullptr;
  const xAOD::EventInfo*          m_eventInfo = nullptr;
};


/// Helper: build and initialize a MuonSelectionTool for the given quality WP.
///
/// IsRun3Geo defaults to false (Run-2); update if running against a Run-3
/// PHYSLITE file (run number >= 330 000 for MC / >= 400 000 for data).
static std::unique_ptr<CP::MuonSelectionTool>
makeTool (const std::string& name, int quality, bool useMVA = false)
{
  auto tool = std::make_unique<CP::MuonSelectionTool> (name);
  EXPECT_SUCCESS (tool->setProperty ("MuQuality",     quality));
  EXPECT_SUCCESS (tool->setProperty ("MaxEta",         2.7));
  EXPECT_SUCCESS (tool->setProperty ("IsRun3Geo",      false));
  EXPECT_SUCCESS (tool->setProperty ("TurnOffMomCorr", true));
  if (useMVA)
    EXPECT_SUCCESS (tool->setProperty ("UseMVALowPt", true));
  EXPECT_SUCCESS (tool->initialize ());
  return tool;
}


// ---------------------------------------------------------------------------
// One TEST_F per working point, mirroring the seven WPs in
// ut_MuonSelectorToolsTester_{data,MC}.sh
// ---------------------------------------------------------------------------

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_Tight)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 0);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_Tight",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_Medium)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 1);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_Medium",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_Loose)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 2);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_Loose",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_VeryLoose)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 3);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_VeryLoose",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_HighPt)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 4);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_HighPt",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_LowPt)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 5);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_LowPt",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

TEST_F (ColumnarPhysLiteTest, MuonSelectionTool_LowPtMVA)
{
  if (!checkMode ()) return;
  auto tool = makeTool (makeUniqueName (), 5, /*useMVA=*/true);
  XAODMuonSelectionToolCaller caller (*tool, "AnalysisMuons");
  doCall ({.tool            = tool.get (),
           .name            = "MuonSelectionTool_LowPtMVA",
           .xAODToolCaller  = &caller,
           .containerRenames = {{"Muons", "AnalysisMuons"}}});
}

ATLAS_GOOGLE_TEST_MAIN
