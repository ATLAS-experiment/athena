/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file TrigT1Run2ZDC/TrigT1Run2ZDC.h
 * @author Matthew Hoppesch <mhoppesc@cern.ch>
 * @date May 2023
 * @brief An algorithm to simulate the run3 level 1 ZDC trigger.  Currently, runs on run2 data due to missing ZDC Monte This algorithm records a data object of ZdcCTP type into StoreGate.  The object contains the input bits for the CTP simulation.
 */

#ifndef TRIG_T1_RUN_2_ZDC_H
#define TRIG_T1_RUN_2_ZDC_H

#include <string>
#include <vector>

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthContainers/DataVector.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadDecorHandle.h"
#include "StoreGate/ReadDecorHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "PathResolver/PathResolver.h"

// Input Containers
#include "xAODForward/ZdcModuleContainer.h"

// Outputs to CTP
#include "TrigT1Interfaces/ZdcCTP.h"
#include "TrigT1Interfaces/TrigT1CaloDefs.h"

#include "TrigT1Run3ZDC.h"
#include "nlohmann/json.hpp"

namespace LVL1 {
  /** @brief level 1 ZDC trigger simulation */
  class TrigT1Run2ZDC : public AthReentrantAlgorithm {

  public:

    // This is a standard algorithm constructor
    TrigT1Run2ZDC (const std::string& name, ISvcLocator* pSvcLocator);

  // These are the functions inherited from Algorithm
   virtual StatusCode initialize() override;
   virtual StatusCode execute(const EventContext& ctx) const override;

  private :
   /* Input handles */
   SG::ReadHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleKey{
       this, "ZdcModuleLocation", TrigT1CaloDefs::xAODZdcModuleLocation,
       "Read handle key for ZdcModuleContainer"};
       
   SG::ReadDecorHandleKey<xAOD::ZdcModuleContainer> m_zdcModuleCalibEnergyKey 
   {this, "ZdcModuleCalibEnergyKey", "ZdcModules.CalibEnergy", "ReadHandleKey for Zdc CalibEnergy AuxData"};

   /* Output handles */
   SG::WriteHandleKey<ZdcCTP> m_zdcCTPLocation{
       this, "ZdcCTPLocation", TrigT1CaloDefs::ZdcCTPLocation,
       "Write handle key for ZdcCTP"};

   /* properties */
   Gaudi::Property<std::string> m_lutFile{this, "filepath_LUT", "", "path to LUT file"};
   Gaudi::Property<float> m_energyToADCScaleFactor{this, "EnergyADCScale", 0.4, "Energy [GeV] / ADC conversion factor"};

   /** A data member to hold the ZDCTrigger Object that stores input floats: shared ptr to ensure cleanup */
   std::shared_ptr<ZDCTriggerSim::ModuleAmplInputsFloat> m_modInputs_p;

   /** A data member to hold the ZDCTrigger Object that computes the LUT logic: shared ptr to ensure cleanup */
   std::shared_ptr<ZDCTriggerSimModuleAmpls> m_simTrig;
  };
}

#endif
