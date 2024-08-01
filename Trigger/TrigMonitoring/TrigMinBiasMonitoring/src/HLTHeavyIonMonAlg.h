/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGMINBIASMONITORING_HLTHeavyIonMonAlg_H
#define TRIGMINBIASMONITORING_HLTHeavyIonMonAlg_H

// Framework includes
#include <AthenaMonitoring/AthMonitorAlgorithm.h>
#include <xAODHIEvent/HIEventShapeContainer.h>
#include <xAODTrigger/EnergySumRoI.h>
#include <xAODTrigger/jFexSumETRoIContainer.h>

// STL includes
#include <string>

/**
 * @class HLTHeavyIonMonAlg
 * @brief Monitoring algorithms for HLT_mb_*_hi_* chains
 **/
class HLTHeavyIonMonAlg : public AthMonitorAlgorithm {
public:
  HLTHeavyIonMonAlg(const std::string &name, ISvcLocator *pSvcLocator);

  virtual StatusCode initialize() override;
  virtual StatusCode fillHistograms(const EventContext &context) const override;

private:
  SG::ReadHandleKey<xAOD::EnergySumRoI> m_lvl1EnergySumROIKey {
      this, "lvl1EnergySumROIKey", "LVL1EnergySumRoI", "Name of Sum of Energy info object produced by the Legacy L1 system."};

  SG::ReadHandleKey<xAOD::jFexSumETRoIContainer> m_jFexSumETRoIKey {
      this, "jFexSumETRoIKey", "L1_jFexSumETRoI", "Name of Sum of Energy info object produced by the jFex system."};

  SG::ReadHandleKey<xAOD::HIEventShapeContainer> m_HIEventShapeKey {
      this, "HIEventShapeKey", "HLT_HIEventShapeEG", "Name of HI event shape info object produced by the HLT system."};

  Gaudi::Property<std::vector<std::string>> m_triggerListMon {this, "triggerListMon", {}, "List of triggers to be monitored"};

  StatusCode monitorSumEt(const EventContext& context) const;
};

#endif // TRIGMINBIASMONITORING_HLTHeavyIonMonAlg_H
