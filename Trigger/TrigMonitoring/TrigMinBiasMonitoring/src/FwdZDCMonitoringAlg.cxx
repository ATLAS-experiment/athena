/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FwdZDCMonitoringAlg.h"
#include "ZdcUtils/ZdcEventInfo.h"


FwdZDCMonitoringAlg::FwdZDCMonitoringAlg(const std::string &name, ISvcLocator *pSvcLocator)
    : AthMonitorAlgorithm(name, pSvcLocator) {}

FwdZDCMonitoringAlg::~FwdZDCMonitoringAlg() {}

StatusCode FwdZDCMonitoringAlg::initialize()
{
  ATH_CHECK(m_zdcModuleContainerKey.initialize());
  ATH_CHECK(m_zdcModuleCalibEnergyKey.initialize());
  ATH_CHECK( m_EventInfoKey.initialize() );
  ATH_CHECK( m_eventInfoDecorKey.initialize() );
  return AthMonitorAlgorithm::initialize();
}

StatusCode FwdZDCMonitoringAlg::fillHistograms(const EventContext &context) const
{
  const auto &trigDecTool = getTrigDecisionTool();
  
  SG::ReadHandle<xAOD::EventInfo> eventInfo (m_EventInfoKey, context);
  // access ZDC modules
  SG::ReadHandle<xAOD::ZdcModuleContainer> zdcModules(m_zdcModuleContainerKey, context);
  // access ZDC aux data 
  SG::ReadDecorHandle<xAOD::ZdcModuleContainer, float> zdcModuleCalibEnergyHandle( m_zdcModuleCalibEnergyKey, context);
  
  if (!zdcModules.isValid())
  {
    ATH_MSG_DEBUG("evtStore() does not contain Zdc Collection with name " << m_zdcModuleContainerKey << ", is the Zdc present in this sample?");
    return StatusCode::SUCCESS;
  }
  if (eventInfo->isEventFlagBitSet(xAOD::EventInfo::ForwardDet, ZdcEventInfo::DECODINGERROR )){
    ATH_MSG_WARNING("Error in LUCROD decoding, cannot write monitoring histograms.  Skipping this event");
    return StatusCode::SUCCESS;
  }
  const std::string allStr{"ZDCall"};
  for (const auto &trig : m_triggerList)
  {
     if (!trigDecTool->isPassed(trig, TrigDefs::Physics))
    {
      continue;
    }

    ATH_MSG_DEBUG("Chain " << trig << " is passed: YES");

    auto TrigCounts = Monitored::Scalar<std::string>("TrigCounts", trig);
    fill(allStr, TrigCounts);

    // declare the quantities which should be monitored
    auto e_A = Monitored::Scalar<float>("e_A", 0.0);     // ZDC ADC on Side A
    auto e_C = Monitored::Scalar<float>("e_C", 0.0);     // ZDC ADC on Side C
    auto moduleEnergy = Monitored::Scalar<float>("moduleEnergy", 0.0);
    auto moduleNum = Monitored::Scalar<float>("moduleNum", 0.0);
    const std::string trigExpertStr = trig + "_expert";
    // read single modules
    for (const auto zdcModule : *zdcModules)
    {
      if (zdcModule->zdcType() == 0)
      { // type = 0 are big modules, type = 1 the pixels
        // Side A
        if (zdcModule->zdcSide() > 0)
        {
          moduleEnergy = zdcModuleCalibEnergyHandle(*zdcModule);
          moduleNum = zdcModule->zdcModule();
          e_A += zdcModuleCalibEnergyHandle(*zdcModule);
          fill(trigExpertStr, moduleEnergy, moduleNum);
          fill(allStr, moduleEnergy, moduleNum);
        }
        // Side C
        if (zdcModule->zdcSide() < 0)
        {
          moduleEnergy = zdcModuleCalibEnergyHandle(*zdcModule);
          moduleNum = zdcModule->zdcModule() + 4.;
          e_C += zdcModuleCalibEnergyHandle(*zdcModule);
          fill(trigExpertStr, moduleEnergy, moduleNum);
          fill(allStr, moduleEnergy, moduleNum);
        }
      }
    }
    fill(trigExpertStr, e_A, e_C);
    fill(allStr, e_A, e_C);
    }
  return StatusCode::SUCCESS;
}
