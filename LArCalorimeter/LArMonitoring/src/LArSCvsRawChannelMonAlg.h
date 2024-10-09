/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARCELLREC_LARSCVSRAWCHANNELMONALG_H
#define LARCELLREC_LARSCVSRAWCHANNELMONALG_H


#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "CaloEvent/CaloBCIDAverage.h"

class LArOnlineID;
class CaloCell_ID;

class LArSCvsRawChannelMonAlg : public AthMonitorAlgorithm {
public:
  using AthMonitorAlgorithm::AthMonitorAlgorithm;
 
   ~LArSCvsRawChannelMonAlg() = default;
  virtual StatusCode initialize() override final;

  //virtual StatusCode execute(const EventContext& ctx) const override;
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;// {return StatusCode::SUCCESS;}

 private: 
  SG::ReadHandleKey<LArRawSCContainer>  m_SCKey{this, "keySC", "SC_ET_ID","Key for SuperCells container"};
  SG::ReadHandleKey<LArRawChannelContainer>  m_RCKey{this, "keyRC", "LArRawChannels","Key for (regular) LArRawChannel container"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "keyCabling", "LArOnOffIdMap", "Key for the cabling"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingSCKey{this, "keySCCabling", "LArOnOffIdMapSC", "Key for the cabling of the SC"};
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this, "CaloSuperCellDetDescrManager", "CaloSuperCellDetDescrManager", "SG key of the resulting CaloSuperCellDetDescrManager" };
  SG::ReadCondHandleKey<LArBadChannelCont> m_badChanKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_badSCKey{this, "BadSCKey", "LArBadChannelSC", "Key of the LArBadChannelCont SC" };

  SG::ReadHandleKey<CaloBCIDAverage> m_caloBCIDAvg{this,"CaloBCIDAverageKey","CaloBCIDAverage","SG Key of CaloBCIDAverage object (empty: Distable BCID correction)"};

  Gaudi::Property<int> m_scEneCut{this,"SCEnergyCut",70,"Ignore SuperCells below this threshold (in terms of SC output integers"};
  Gaudi::Property<bool> m_warnOffenders{this,"WarnOffenders",false,"Print warning about the worst offenders"};
  Gaudi::Property<std::string> m_MonGroupName  {this, "MonGroupName", "LArSCvsRawMon"};
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"};


  StringArrayProperty m_partitionNames{this, "PartitionNames", {"EMBA","EMBC","EMECA","EMECC","HECA","HECC","FCALA","FCALC"}};
  enum PartitionEnum{EMBA=0,EMBC,EMECA,EMECC,HECA,HECC,FCALA,FCALC,MAXPARTITIONS};

  const LArOnlineID* m_onlineID;
  const CaloCell_ID* m_calo_id;
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};

  LArBadChannelMask m_bcMask;

  int getPartition(const Identifier& scid) const;

};

#endif     
