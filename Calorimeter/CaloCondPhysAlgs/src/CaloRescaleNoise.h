/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// CaloRescaleNoise.h
//

#ifndef CALOCONDPHYSALGS_CALORESCALENOISE_H
#define CALOCONDPHYSALGS_CALORESCALENOISE_H

// Gaudi includes

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloIdentifier/CaloIdManager.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "StoreGate/ReadCondHandleKey.h"  

#include "LArCabling/LArOnOffIdMapping.h"
#include "GaudiKernel/ITHistSvc.h"

#include <string>

class CaloNoise;
class TTree;
class CaloCell_Base_ID;

class CaloRescaleNoise : public AthAlgorithm {

  public:
    //Gaudi style constructor and execution methods
    /** Standard Athena-Algorithm Constructor */
    CaloRescaleNoise(const std::string& name, ISvcLocator* pSvcLocator);
    /** Default Destructor */
    virtual ~CaloRescaleNoise();
    
    /** standard Athena-Algorithm method */
    virtual StatusCode          initialize() override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          execute(const EventContext& ctx) override;
    /** standard Athena-Algorithm method */
    virtual StatusCode          stop() override;
    
  private:

  //---------------------------------------------------
  // Member variables
  //---------------------------------------------------
  ServiceHandle<ITHistSvc> m_thistSvc{this,"THistSvc","THistSvc"};

  const CaloCell_Base_ID*       m_calo_id{};

  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };
  SG::ReadCondHandleKey<CaloNoise> m_pileupNoiseKey
    { this, "PileupNoiseKey", "pileupNoise", "SG key for pileup noise" };

  SG::ReadCondHandleKey<ILArHVScaleCorr> m_scaleCorrKey
    { this, "LArHVScaleCorr", "LArHVScaleCorrRecomputed", "" };
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey
    { this, "CablingKey", "LArOnOffIdMap", "SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_onlineScaleCorrKey
    { this, "OnlineLArHVScaleCorr", "LArHVScaleCorr", "" };

  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey
    { this, "CaloDetDescrManager", "CaloDetDescrManager", "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSCMgrKey
    {this,"CaloSuperCellDetDescrManager", "CaloSuperCellDetDescrManager", "SG Key for CaloSuperCellDetDescrManager in the Condition Store" };

  BooleanProperty m_isSC{this, "SuperCell", false};
  BooleanProperty m_absScaling{this, "absScaling", false};
  
  int m_iCool{0};
  int m_SubHash{0};
  int m_Hash{0};
  int m_OffId{0};
  float m_eta{0.};
  float m_phi{0.};
  int m_layer{0};
  int m_Gain{0};
  float m_elecNoise{0.};
  float m_pileupNoise{0.}; 
  float m_elecNoiseRescaled{0.};
  TTree* m_tree{};
};
#endif
