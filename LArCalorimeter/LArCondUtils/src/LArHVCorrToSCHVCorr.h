/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCONDUTILS_LARHVTOSCHV_H
#define LARCONDUTILS_LARHVTOSCHV_H 1

// STL includes
#include <string>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "LArRecConditions/LArHVCorr.h"

#include "GaudiKernel/ToolHandle.h"

class LArHVCorrToSCHVCorr
  : public ::AthAlgorithm
{ 
 public: 

  /// Constructor with parameters: 
  LArHVCorrToSCHVCorr( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~LArHVCorrToSCHVCorr()=default; 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute() override {return StatusCode::SUCCESS;};
  virtual StatusCode  stop() override;

 private: 

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"SCCablingKey","LArOnOffIdMapSC","SG Key of SC LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_contKey{this,"ContainerKey","LArHVScaleCorr","SG Key of regular cells HV scale corr object"};
  
  SG::WriteCondHandleKey<LArHVCorr> m_outKey{this,"OutputKey","LARSCHVScaleCorr","SG Key of produced SC HV scale corr object"};

  StringProperty m_folderName{this, "OutputFolder", "/LAR/ElecCalibFlatSC/HVScaleCorr", "Output folder for CondAttrListCollection"};

  StringProperty m_weightsName{this, "PhysicsWeights", "TrigT1CaloCalibUtils/HVcorrPhysicsWeights.txt", "File with layer weights"};

  BooleanProperty m_isHI{this, "IsHeavyIons", false, "are we computing for HI ?"};

  ToolHandle<ICaloSuperCellIDTool> m_scidTool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool"};

  float getWeight(const LArHEC_ID *hecID, const Identifier &id, std::map<int,std::vector<float> > &wmap);
}; 

#endif
