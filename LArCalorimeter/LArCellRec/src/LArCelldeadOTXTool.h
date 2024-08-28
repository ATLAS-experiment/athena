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

class CaloCellContainer;
class LArOnlineID;
class CaloCell_ID;

class LArCelldeadOTXTool : public extends<AthAlgTool, ICaloCellMakerTool>  {
public:
  using base_class::base_class;
  //LArCelldeadOTXTool (const std::string& type, const std::string& name, 
  //		   const IInterface* parent);

  ~LArCelldeadOTXTool() = default;
  virtual StatusCode initialize() override final;

  //Implements the ICaloCellMaker interface
  virtual StatusCode process(CaloCellContainer* cellCollection, const EventContext& ctx) const override final;

 private: 
  SG::ReadHandleKey<LArRawSCContainer>  m_SCKey{this, "keySC", "SC_ET","Key for SuperCells container"};
  SG::ReadCondHandleKey<LArBadFebCont>     m_MFKey{this, "keyMF", "LArBadFeb", "Key for missing FEBs"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "keyCabling", "LArOnOffIdMap", "Key for the cabling"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this, "keySCCabling", "LArOnOffIdMapSC", "Key for the cabling of the SC"};

  const LArOnlineID* m_onlineID;
  const CaloCell_ID* m_calo_id;
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};

};

#endif     
