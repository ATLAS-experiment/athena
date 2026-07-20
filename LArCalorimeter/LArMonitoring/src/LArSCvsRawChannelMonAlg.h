/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
  //StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPC", "EMBPA", "EMB1C", "EMB1A", "EMB2C", "EMB2A", "EMB3C", "EMB3A", "EMECPC", "EMECPA", "EMEC1C", "EMEC1A", "EMEC2C", "EMEC2A", "EMEC3C", "EMEC3A", "HECC", "HECA", "FCAL1C", "FCAL1A", "FCAL2C", "FCAL2A", "FCAL3C", "FCAL3A", "ALL"},
  StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPC", "EMBPA", "EMB1C", "EMB1A", "EMB2C", "EMB2A", "EMB3C", "EMB3A", "EMECPC", "EMECPA", "EMEC1C", "EMEC1A", "EMEC2C", "EMEC2A", "EMEC3C", "EMEC3A", "HECC", "HECA", "FCAL1C", "FCAL1A", "FCAL2C", "FCAL2A", "FCAL3C", "FCAL3A"},
          "Names of individual layers to monitor"};


  enum LayerEnumNoSides{EMBPNS=0,EMB1NS,EMB2NS,EMB3NS,EMECPNS,EMEC1NS,EMEC2NS,EMEC3NS,HECNS,FCAL1NS,FCAL2NS,FCAL3NS,MAXLYRNS};

  // Mapping of CaloCell nomencature to CaloCellMonitoring nomencature
  // From Calorimeter/CaloGeoHelpers/CaloGeoHelpers/CaloSampling.def
  const std::array<unsigned,CaloSampling::Unknown> m_caloSamplingToLyrNS{ 
    EMBPNS,   //CALOSAMPLING(PreSamplerB, 1, 0) //  0
    EMB1NS,   //CALOSAMPLING(EMB1,        1, 0) //  1
    EMB2NS,   //CALOSAMPLING(EMB2,        1, 0) //  2
    EMB3NS,   //CALOSAMPLING(EMB3,        1, 0) //  3
    EMECPNS, //CALOSAMPLING(PreSamplerE, 0, 1) //  4
    EMEC1NS,  //CALOSAMPLING(EME1,        0, 1) //  5
    EMEC2NS,  //CALOSAMPLING(EME2,        0, 1) //  6
    EMEC3NS,  //CALOSAMPLING(EME3,        0, 1) //  7
    HECNS,  //CALOSAMPLING(HEC0,        0, 1) //  8
    HECNS,  //CALOSAMPLING(HEC0,        0, 1) //  9
    HECNS,  //CALOSAMPLING(HEC0,        0, 1) //  19
    HECNS,  //CALOSAMPLING(HEC0,        0, 1) //  11
    MAXLYRNS, //CALOSAMPLING(TileBar0,    1, 0) // 12
    MAXLYRNS, //CALOSAMPLING(TileBar1,    1, 0) // 13
    MAXLYRNS, //CALOSAMPLING(TileBar2,    1, 0) // 14
    MAXLYRNS, //CALOSAMPLING(TileGap1,    1, 0) // 15
    MAXLYRNS, //CALOSAMPLING(TileGap2,    1, 0) // 16
    MAXLYRNS, // CALOSAMPLING(TileGap3,    1, 0) // 17
    MAXLYRNS, //CALOSAMPLING(TileExt0,    1, 0) // 18
    MAXLYRNS, //CALOSAMPLING(TileExt1,    1, 0) // 19
    MAXLYRNS, //CALOSAMPLING(TileExt2,    1, 0) // 20
    FCAL1NS,  //CALOSAMPLING(FCAL0,       0, 1) // 21
    FCAL2NS,  //CALOSAMPLING(FCAL1,       0, 1) // 22
    FCAL3NS,  //ALOSAMPLING(FCAL2,       0, 1) // 23
    MAXLYRNS, //CALOSAMPLING(MINIFCAL0,   0, 1) // 24
    MAXLYRNS, //CALOSAMPLING(MINIFCAL1,   0, 1) // 25
    MAXLYRNS, //CALOSAMPLING(MINIFCAL2,   0, 1) // 26
    MAXLYRNS, //CALOSAMPLING(MINIFCAL3,   0, 1) // 27
  };
  
  std::map<std::string,int> m_toolmapPerLayer;
  
  const LArOnlineID* m_onlineID{};
  const CaloCell_ID* m_calo_id{};
  ToolHandle<ICaloSuperCellIDTool>  m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};

  LArBadChannelMask m_bcMask;

  int getPartition(const Identifier& scid) const;

};

#endif     
