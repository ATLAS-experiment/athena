/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LArCelldeadOTXAlg_H
#define LARCELLREC_LArCelldeadOTXAlg_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArRecEvent/LArDeadOTXFromSC.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include <mutex>
#include <vector>

class HWIdentifier;
class LArOnlineID;
class LArOnline_SuperCellID;
class CaloCell_ID;

class LArCelldeadOTXAlg : public AthReentrantAlgorithm  {
public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  virtual StatusCode initialize() override final;
  //Implements the ICaloCellMaker interface
  virtual StatusCode execute(const EventContext& ctx) const override final;

 private: 
  SG::ReadHandleKey<LArRawSCContainer>  m_SCKey{this, "keySC", "SC_ET","Key for SuperCells container"};
  SG::WriteHandleKey<LArDeadOTXFromSC>  m_deadOTXFromSCKey{this, "DeadOTXFromSC", "DeadOTXFromSC","DeadOTXFromSC"};
  SG::ReadCondHandleKey<LArBadFebCont>     m_MFKey{this, "keyMF", "LArBadFeb", "Key for missing FEBs"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_badSCKey{this, "BadSCKey", "LArBadChannelSC", "Key of the LArBadChannelCont SC" };
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "keyCabling", "LArOnOffIdMap", "Key for the cabling"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this, "keySCCabling", "LArOnOffIdMapSC", "Key for the cabling of the SC"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};  

  Gaudi::Property<int> m_scCut{this,"SCEneCut",70,"Do not use super-cells with values below this cut"};
  const LArOnlineID* m_onlineID=nullptr;
  const LArOnline_SuperCellID* m_onlineSCID=nullptr;
  const CaloCell_ID* m_calo_id=nullptr;
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};

  void buildMap(const EventContext& ctx, StatusCode& sc) const;
  mutable std::once_flag m_onceFlag ATLAS_THREAD_SAFE;
  
  mutable std::mutex m_mtx;
  mutable std::vector<HWIdentifier> m_febs ATLAS_THREAD_SAFE;
  mutable std::vector<std::vector<float> > m_multipliers ATLAS_THREAD_SAFE;
  mutable std::vector<std::vector<uint32_t> > m_channels ATLAS_THREAD_SAFE;
};

#endif     
