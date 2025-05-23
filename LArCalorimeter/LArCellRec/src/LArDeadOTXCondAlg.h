/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCELLREC_LARDEADOTXCONDALG
#define LARCELLREC_LARDEADOTXCONDALG


#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArRecConditions/LArDeadOTXCorrFactors.h"

class CaloCellContainer;
class LArOnlineID;
class CaloCell_ID;

class LArDeadOTXCondAlg : public AthAlgorithm  {
public:
  using AthAlgorithm::AthAlgorithm;


  LArDeadOTXCondAlg() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode execute() override final;
  
 private: 
  SG::ReadCondHandleKey<LArBadFebCont>     m_MFKey{this, "keyMF", "LArBadFeb", "Key for missing FEBs"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_badSCKey{this, "BadSCKey", "LArBadChannelSC", "Key of the LArBadChannelCont SC" };
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "keyCabling", "LArOnOffIdMap", "Key for the cabling"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this, "keySCCabling", "LArOnOffIdMapSC", "Key for the cabling of the SC"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};  

  SG::WriteCondHandleKey<LArDeadOTXCorrFactors> m_outputKey{this,"OutputKey","LArDeadOTXCorrFactors","SG key of output LArDeadOTXCorrFactor object"};

  const LArOnlineID* m_onlineID=nullptr;
  const CaloCell_ID* m_calo_id=nullptr;
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};
  
};
#endif
