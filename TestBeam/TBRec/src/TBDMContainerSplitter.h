/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBDMCONTAINERSPLITTER_H
#define TBREC_TBDMCONTAINERSPLITTER_H

// 
// class TBDMContainerSplitter 
// Splitting the CalibDMContainer hits to two, leaving all leakage in second
// both are VIEW containers....
//
#include "AthenaBaseComps/AthAlgorithm.h"

class CaloDmDescrManager;

class TBDMContainerSplitter: public AthAlgorithm {
 public:    
  
  TBDMContainerSplitter(const std::string& name, ISvcLocator* pSvcLocator);
  virtual ~TBDMContainerSplitter();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;
  
 private: 
  // Names and pointers
  Gaudi::Property<std::string> m_DMHitsKey{this, "InputDMContainer", "LArCalibrationHitDeadMaterial", "Original DM hits container key"};
  Gaudi::Property<std::string> m_DMHitsCaloKey{this, "OutputDM_Calo", "LArCalibrationHitDeadMaterial_Calo", "DM hits in calo modules container key"};
  Gaudi::Property<std::string> m_DMHitsLeakKey{this, "OutputDM_Leak", "LArCalibrationHitDeadMaterial_Leakage", "DM hits for leakage container key"};

  const CaloDmDescrManager* m_caloDmDescrManager{nullptr};
};

#endif
