///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOCONDPHYSALGS_FCAL_HV_ENERGY_RESCALE_H
#define CALOCONDPHYSALGS_FCAL_HV_ENERGY_RESCALE_H 1
// FrameWork includes
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"  

#include "CxxUtils/checker_macros.h"

class FCAL_HV_Energy_Rescale: public AthAlgorithm
{ 
 public: 
  /// Constructor with parameters: 
  FCAL_HV_Energy_Rescale( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~FCAL_HV_Energy_Rescale(); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext& ctx) override;
  virtual StatusCode  stop ATLAS_NOT_THREAD_SAFE() override;//due to AthenaAttributeList ctor  

private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_scaleCorrKey{this,"LArHVScaleCorr", "LArHVScaleCorrRecomputed", "" };

  Gaudi::Property<std::string> m_folder{this, "Folder", "/LAR/CellCorrOfl/EnergyCorr"};
}; 

#endif //> !CALOCONDPHYSALGS_FCAL_HV_ENERGY_RESCALE_H
