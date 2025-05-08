/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMONITORING_LARDIGITALTRIGGMON_H
#define LARMONITORING_LARDIGITALTRIGGMON_H

//inheritance:
#include "AthenaMonitoring/AthMonitorAlgorithm.h"

//LAr services:
#include "LArElecCalib/ILArPedestal.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRecConditions/LArBadChannelCont.h"

//STL:
#include <string>

#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#include "GaudiKernel/ToolHandle.h"

//Events infos:
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"

class LArOnline_SuperCellID;
class CaloCell_SuperCell_ID;

class LArDigitalTriggMonAlg: public AthMonitorAlgorithm
{
  
  
public: 
  using AthMonitorAlgorithm::AthMonitorAlgorithm;

  /** @brief Default destructor */
  virtual ~LArDigitalTriggMonAlg();
  
  virtual StatusCode initialize() override;
  
  virtual StatusCode fillHistograms(const EventContext& ctx) const override;
  
private:  

  /** private methods: */
  int whatPartition(HWIdentifier id, int side) const; 
  unsigned getXbinFromSourceID(const unsigned sourceID) const;

  /**declaration variables used in joboptions*/
  Gaudi::Property<std::string>   m_MonGroupName  {this, "LArDigitTriggMonGroupName", "LArDigitTriggMonGroup"};
  Gaudi::Property<unsigned>      m_NLatomeBins{this, "NLatomeBins", 117};
  Gaudi::Property<bool>          m_isADCBaseline{this,"isADCBas",false,"Set true for ADC_BAS (implies dividing ADC-value by 8)"};

  //Added for Stream aware:
  /** Give the name of the streams you want to monitor:*/
  Gaudi::Property<std::vector<std::string> >  m_streams {this, "Streams", {""}};
  
  //Histogram group names
  Gaudi::Property<std::string> m_scMonGroupName {this, "SCMonGroup", "SC"};

  //** Handle to cabling */
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"}; 

  /** Handle to EventData (input) */
  SG::ReadHandleKey<LArDigitContainer> m_digitContainerKey{this,"LArDigitContainerKey","","SG key of LArDigitContainer read from Bytestream"}; //raw ADC 12 bits - ADC axis up to 4096
  SG::ReadHandleKey<LArRawSCContainer> m_rawSCContainerKey{this,"LArRawSCContainerKey","","SG key of LArRawSCContainer read from Bytestream"};
  SG::ReadHandleKey<LArRawSCContainer> m_rawSCEtRecoContainerKey{this,"LArRawSCEtRecoContainerKey","SC_ET_RECO","SG key of LArRawSCContainer read from Bytestream"};
  SG::ReadHandleKey<LArLATOMEHeaderContainer> m_LATOMEHeaderContainerKey{this,"LArLATOMEHeaderContainerKey","SC_LATOME_HEADER","SG key of LArLATOMEHeaderContainer read from Bytestream"};

  /** Handle to bad-channel mask */
  LArBadChannelMask m_bcMask{true}; //isSC=true
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey {this, "BadChanKey", "LArBadChannelSC", "SG key for LArBadChan object"};
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"};

  /** Handle to pedestal */
  SG::ReadCondHandleKey<ILArPedestal>    m_keyPedestalSC{this,"LArPedestalKeySC","LArPedestalSC","SG key of LArPedestal CDO"};

  /** Handle to Super Cell DD Manager */
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this
      ,"CaloSuperCellDetDescrManager"
      ,"CaloSuperCellDetDescrManager"
      ,"SG key of the resulting CaloSuperCellDetDescrManager"};

  SG::ReadDecorHandleKey<xAOD::EventInfo> m_actualMuKey {this, "actualInteractionsPerCrossing",
           "EventInfo.actualInteractionsPerCrossing","Decoration for Actual Number of Interactions Per Crossing"};

  StringArrayProperty m_layerNames{this, "LayerNames", {"EMBPC", "EMBPA", "EMB1C", "EMB1A", "EMB2C", "EMB2A", "EMB3C", "EMB3A", "EMECPC", "EMECPA", "EMEC1C", "EMEC1A", "EMEC2C", "EMEC2A", "EMEC3C", "EMEC3A", "HECC", "HECA", "FCAL1C", "FCAL1A", "FCAL2C", "FCAL2A", "FCAL3C", "FCAL3A", "ALL"},
          "Names of individual layers to monitor"};


  //Enumerate layer-types, ignoring sides. Useful for configuration that is per-definition symmetric 
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
  

 
 const std::map <unsigned, unsigned> m_LatomeDetBinMappingQ{
    {0x48,  1},  //FCALC
    {0x4c,  3},  //EMECC/HEC
    {0x44, 11},  //EMECC
    {0x4a, 27},  //EMB/EMECC
    {0x42, 43},  //EMBC	   
    {0x41, 59},  //EMBA	   
    {0x49, 75},  //EMB/EMECA
    {0x43, 91},  //EMECA
    {0x4b,107},  //EMEC/HECA
    {0x47,115}   //FCALA
  };

  std::map<std::string,int> m_toolmapLayerNames_digi;
  std::map<std::string,int> m_toolmapLayerNames_sc;

   /* Id helpers */
  const LArOnline_SuperCellID*  m_LArOnlineIDHelper=nullptr;
  const CaloCell_SuperCell_ID*  m_SCID_helper=nullptr;


};
#endif
