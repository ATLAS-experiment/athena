/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LArCelldeadOTXTool_H
#define LARCELLREC_LArCelldeadOTXTool_H


#include "CaloInterface/ICaloCellMakerTool.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class CaloCellContainer;
class LArOnlineID;
class CaloCell_ID;

class LArCelldeadOTXTool : public extends<AthAlgTool, ICaloCellMakerTool>  {
public:
  using base_class::base_class;

  ~LArCelldeadOTXTool() = default;
  virtual StatusCode initialize() override final;
  virtual StatusCode finalize() override final;
  //Implements the ICaloCellMaker interface
  virtual StatusCode process(CaloCellContainer* cellCollection, const EventContext& ctx) const override final;

 private: 
  SG::ReadHandleKey<LArRawSCContainer>  m_SCKey{this, "keySC", "SC_ET","Key for SuperCells container"};
  SG::ReadCondHandleKey<LArBadFebCont>     m_MFKey{this, "keyMF", "LArBadFeb", "Key for missing FEBs"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_badSCKey{this, "BadSCKey", "LArBadChannelSC", "Key of the LArBadChannelCont SC" };
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "keyCabling", "LArOnOffIdMap", "Key for the cabling"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this, "keySCCabling", "LArOnOffIdMapSC", "Key for the cabling of the SC"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};  

  Gaudi::Property<int> m_scCut{this,"SCEneCut",70,"Do not use super-cells with values below this cut"};
  Gaudi::Property<bool> m_testMode{this,"TestMode",false};
  const LArOnlineID* m_onlineID=nullptr;
  const CaloCell_ID* m_calo_id=nullptr;
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};

  typedef std::map<HWIdentifier,std::vector<std::pair<IdentifierHash,float> > > scToDeadCellMap_t; 
  //map key: SuperCell HWIdentifer, value.first: dead feb-cell hash, value.second: conversion factor from SC-ET integer to cell E in MeV

  void buildMap(const EventContext& ctx, scToDeadCellMap_t& map, StatusCode& sc) const;
  mutable std::once_flag m_onceFlag ATLAS_THREAD_SAFE;
  mutable scToDeadCellMap_t m_scToDead ATLAS_THREAD_SAFE; //protected by the once_flag above
  
  mutable std::unordered_map<int,std::pair<float,int> > m_testMap ATLAS_THREAD_SAFE; //Only used in testMode + mtx-protected
  mutable std::mutex m_mtx;

  mutable std::atomic<int> m_nWarnings{0};

};

#endif     
