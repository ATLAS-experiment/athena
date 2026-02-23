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
#include <ColumnarTestFixtures/ColumnarPhysliteTest.h>
#include <xAODCore/ShallowCopy.h>

#include <ElectronPhotonFourMomentumCorrection/EgammaCalibrationAndSmearingTool.h>
#include <ElectronEfficiencyCorrection/AsgElectronEfficiencyCorrectionTool.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//
// method implementations
//

using columnar::ColumnarPhysLiteTest;
using columnar::TestUtils::IXAODToolCaller;

struct ElectronData
{
  xAOD::ElectronContainer const* electrons = nullptr;
  xAOD::ElectronContainer* electronsCopy = nullptr;
  xAOD::EventInfo const* eventInfo = nullptr;
};

class XAODElectronCalibCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODElectronCalibCaller (const CP::EgammaCalibrationAndSmearingTool& tool, const std::string& name, ElectronData& data)
    : AsgMessaging ("XAODElectronCalibCaller"), m_tool (tool), m_name (name), m_data (data)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    if (!m_data.electrons)
      ANA_CHECK (evtStore.retrieve (m_data.electrons, m_name));
    if (!m_data.eventInfo)
      ANA_CHECK (evtStore.retrieve (m_data.eventInfo, "EventInfo"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    if (!m_data.electronsCopy)
      {
      auto [electronsCopy, electronsAuxCopy] = xAOD::shallowCopyContainer (*m_data.electrons);
      m_data.electronsCopy = electronsCopy;
      ANA_CHECK (evtStore.record (electronsCopy, m_name + postfix));
      ANA_CHECK (evtStore.record (electronsAuxCopy, m_name + postfix + "Aux."));
    }
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_data.electronsCopy, *m_data.eventInfo);
    return StatusCode::SUCCESS;
  }

  virtual void clear () override
  {
    m_data.electrons = nullptr;
    m_data.electronsCopy = nullptr;
    m_data.eventInfo = nullptr;
  }

private:
  const CP::EgammaCalibrationAndSmearingTool& m_tool;
  std::string m_name;

  ElectronData& m_data;
};


class XAODElectronEffCaller final : public IXAODToolCaller, public asg::AsgMessaging
{
public:
  XAODElectronEffCaller (const AsgElectronEfficiencyCorrectionTool& tool, const std::string& name, ElectronData& data)
    : AsgMessaging ("XAODElectronEffCaller"), m_tool (tool), m_name (name), m_data (data)
  {}

  virtual StatusCode retrieve (EventStoreType& evtStore) override
  {
    if (!m_data.electrons)
      ANA_CHECK (evtStore.retrieve (m_data.electrons, m_name));
    if (!m_data.eventInfo)
      ANA_CHECK (evtStore.retrieve (m_data.eventInfo, "EventInfo"));
    return StatusCode::SUCCESS;
  }

  virtual StatusCode copyRecord (EventStoreType& evtStore, const std::string& postfix) override
  {
    if (!m_data.electronsCopy)
    {
      auto [electronsCopy, electronsAuxCopy] = xAOD::shallowCopyContainer (*m_data.electrons);
      m_data.electronsCopy = electronsCopy;
      ANA_CHECK (evtStore.record (electronsCopy, m_name + postfix));
      ANA_CHECK (evtStore.record (electronsAuxCopy, m_name + postfix + "Aux."));
    }
    return StatusCode::SUCCESS;
  }

  virtual StatusCode call () override
  {
    m_tool.callSingleEvent (*m_data.electronsCopy, *m_data.eventInfo);
    return StatusCode::SUCCESS;
  }

  virtual void clear () override
  {
    m_data.electrons = nullptr;
    m_data.electronsCopy = nullptr;
    m_data.eventInfo = nullptr;
  }

private:
  const AsgElectronEfficiencyCorrectionTool& m_tool;
  std::string m_name;
  ElectronData& m_data;
};


TEST_F (ColumnarPhysLiteTest, MultiTool_ElectronCalibAndEfficiency)
{
  // Create and configure the calibration tool
  auto calibTool = std::make_unique<CP::EgammaCalibrationAndSmearingTool> (makeUniqueName());
  ASSERT_SUCCESS (calibTool->setProperty ("ESModel", "es2022_R22_PRE"));
  ASSERT_SUCCESS (calibTool->setProperty ("decorrelationModel", "1NP_v1"));
  ASSERT_SUCCESS (calibTool->setProperty ("useFastSim", 0));
  ASSERT_SUCCESS (calibTool->setProperty ("useMVACalibration", 0));
  ASSERT_SUCCESS (calibTool->setProperty ("useLayerCorrection", 0));
  ASSERT_SUCCESS (calibTool->setProperty ("onlyElectrons", 1));
  ASSERT_SUCCESS (calibTool->initialize ());

  // Create and configure the efficiency tool
  asg::AsgToolConfig effToolConfig;
  effToolConfig.setTypeAndName ("AsgElectronEfficiencyCorrectionTool/" + makeUniqueName());
  ASSERT_SUCCESS (effToolConfig.setProperty ("MapFilePath", "ElectronEfficiencyCorrection/2015_2018/rel21.2/Precision_Summer2020_v1/map4.txt"));
  ASSERT_SUCCESS (effToolConfig.setProperty ("ForceDataType", unsigned (PATCore::ParticleDataType::Full)));
  ASSERT_SUCCESS (effToolConfig.setProperty ("CorrelationModel", "TOTAL"));
  ASSERT_SUCCESS (effToolConfig.setProperty ("RecoKey", "Reconstruction"));
  ToolHandle<AsgElectronEfficiencyCorrectionTool> effToolHandle;
  std::shared_ptr<void> cleanup;
  ASSERT_SUCCESS (effToolConfig.makeTool (effToolHandle, cleanup));

  // Create xAOD callers
  ElectronData electronData;
  XAODElectronCalibCaller calibCaller (*calibTool, "AnalysisElectrons", electronData);
  XAODElectronEffCaller effCaller (*effToolHandle, "AnalysisElectrons", electronData);

  // Run the multi-tool test
  doCallMulti ({
    {.tool = calibTool.get(),
     .name = "EgammaCalibrationAndSmearingTool",
     .xAODToolCaller = &calibCaller,
     .containerRenames = {{"EGamma", "AnalysisElectrons"}}},
    {.tool = &*effToolHandle,
     .name = "AsgElectronEfficiencyCorrectionTool",
     .xAODToolCaller = &effCaller,
     .containerRenames = {{"Electrons", "AnalysisElectrons"}}}
  });
}

ATLAS_GOOGLE_TEST_MAIN
