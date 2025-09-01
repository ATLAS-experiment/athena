/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TrigT1Run3ZDC/TrigT1Run3ZDC.h
 * @author Matthew Hoppesch <mhoppesc@cern.ch>
 * @date May 2024
 * @brief An algorithm to simulate the run3 level 1 ZDC trigger, from run3 data.
 * This triggers simulation uses flash ADC ZDC information to emulate the
 * trigger decisions. It includes functionality for triggers implemented on both
 * the high and low gain flash ADC values i.e. the 2024 ZDC trigger
 * configuration
 */

#ifndef TRIG_T1_RUN_3_ZDC_H
#define TRIG_T1_RUN_3_ZDC_H

#include <string>
#include <vector>

#include "AthContainers/DataVector.h"
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "PathResolver/PathResolver.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

// Input Containers
#include "ZdcByteStream/ZdcLucrodDataContainer.h"
#include "ZdcConditions/ZdcLucrodMapRun3.h"
#include "ZdcIdentifier/ZdcID.h"
#include "xAODForward/ZdcModuleContainer.h"

// Outputs to CTP
#include "TrigT1Interfaces/TrigT1CaloDefs.h"
#include "TrigT1Interfaces/ZdcCTP.h"
#include "ZdcUtils/ZDCTriggerSim.h"
#include "nlohmann/json.hpp"

namespace LVL1 {
/** @brief level 1 ZDC trigger simulation */
class TrigT1Run3ZDC : public AthReentrantAlgorithm {

 public:
  // This is a standard algorithm constructor
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  // These are the functions inherited from Algorithm
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:
  /* Input handles */
  SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleCalibEnergyKey{
      this, "ZdcModuleCalibEnergyKey", "ZdcModules.CalibEnergy",
      "ReadHandleKey for Zdc CalibEnergy AuxData"};

  // Access Bytestream data from Lucrod
  const ZdcID* m_zdcId{};

  SG::ReadHandleKey<ZdcLucrodDataContainer> m_zldContainerName{
      this, "ZdcLucrodDataContainerKey", TrigT1CaloDefs::ZdcLucrodDataContainer,
      "Read Handle key for ZdcLucrodDataContainer"};

  /* Output handles */
  SG::WriteHandleKey<ZdcCTP> m_zdcCTPLocation{this, "ZdcCTPLocation",
                                              TrigT1CaloDefs::ZdcCTPLocation,
                                              "Write handle key for ZdcCTP"};

  /* properties */
  Gaudi::Property<std::string> m_lutFile{
      this, "filepath_LUT", "TrigT1ZDC/zdcRun3T1LUT_v1_30_05_2023.json",
      "path to LUT file"};
  Gaudi::Property<unsigned int> m_minSampleAna{
      this, "MinSampleAna", 7, "First Sample in ZDC LUCROD Analysis window"};
  Gaudi::Property<unsigned int> m_maxSampleAna{
      this, "MaxSampleAna",154, "Last Sample in ZDC LUCROD Analysis window"};
  Gaudi::Property<unsigned int> m_negHG2ndDerivThresh{
      this, "NegHG2ndDerivThresh", 45,
      "Negative 2nd Derivative Threshold for High Gain LUCROD Channel"};
  Gaudi::Property<unsigned int> m_negLG2ndDerivThresh{
      this, "NegLG2ndDerivThresh", 15,
      "Negative 2nd Derivative Threshold for Low Gain LUCROD Channel"};
  Gaudi::Property<unsigned int> m_baselineDelta{
      this, "BaselineDelta", 3,
      "Parameter to take out average offset from raw Flash ADC values"};

  /** Two data member to hold the ZDCTrigger Object
      that computes the LUT logic:
    * shared ptr to ensure cleanup */
  std::shared_ptr<ZDCTriggerSimFADC> m_triggerSimHGPtr;
  std::shared_ptr<ZDCTriggerSimFADC> m_triggerSimLGPtr;

  /** A data member to hold the ZDCTrigger Object that stores
      flash ADC input values:
    * shared ptr to ensure cleanup */
  std::shared_ptr<ZDCTriggerSim::FADCInputs> m_hgFADC_ptr;
  std::shared_ptr<ZDCTriggerSim::FADCInputs> m_lgFADC_ptr;
};
}  // namespace LVL1

#endif
