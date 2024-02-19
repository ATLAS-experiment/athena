/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARSC2NTUPLE_H
#define LARSC2NTUPLE_H

#include "LArCalibTools/LArDigits2Ntuple.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "xAODEventInfo/EventInfo.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "TrigT1CaloEvent/TriggerTower.h"
#include "TrigT1CaloCalibToolInterfaces/IL1CaloxAODOfflineTriggerTowerTools.h"

class LArSC2Ntuple : public LArDigits2Ntuple
{
 public:
  LArSC2Ntuple(const std::string & name, ISvcLocator * pSvcLocator);
  virtual ~LArSC2Ntuple() = default;

  // Standard algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute() override;
 protected:

  typedef std::map<HWIdentifier, const LArRawChannel*> rawChanMap_t;
  void fillRODEnergy(HWIdentifier SCId, rawChanMap_t &rawChanMap, 
                     const LArOnOffIdMapping* cabling, const LArOnOffIdMapping* cablingROD); 

 private:

  Gaudi::Property< std::vector<std::string> > m_contKeys{ this, "SCContainerKeys", {},"which containers to dump"};
  Gaudi::Property< bool > m_overwriteEventNumber{this, "OverwriteEventNumber", false, "overwrite the event number from EventInfo ?"};
  Gaudi::Property< unsigned int >  m_Net{this, "Net", 5, "number of energies to store"};
  Gaudi::Property< bool > m_fillRawChan{this, "FillRODEnergy", false, "Trying to fill corresponding cells energies"};
  Gaudi::Property< bool > m_fillTType{this, "FillTriggerType", false, "Trying to fill trigger type word"};
  Gaudi::Property< std::vector<std::string> > m_trigNames{ this, "TrigNames", {"L1_EM3","L1_EM7","L1_EM15"},"which triggers to dump"};
  Gaudi::Property< bool > m_fillCaloTT{this, "FillTriggerTowers", false, "Trying to fill also TriggerTowers from ByteStream"};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeyAdditional{this,"CablingKeyAdditional","LArOnOffIdMap","SG Key of LArOnOffIdMapping object for standard cells"};
  ToolHandle<ICaloSuperCellIDTool>   m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"};
  ToolHandle< Trig::TrigDecisionTool > m_trigDec{this, "TrigDecisionTool", "", "Handle to the TrigDecisionTool"};
  //To get the data-dependency right ... 
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "LArStatusFlag", "EventInfo", "Key for EventInfo object"};
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoDecorKey{this, "EventInfoDecorKey", "EventInfo.larFlags"};

  SG::ReadHandleKey<LArLATOMEHeaderContainer> m_LArLatomeHeaderContainerKey { this, "LArLatomeHeaderKey", "SC_LATOME_HEADER" };
  Gaudi::Property< std::string > m_triggerTowerKey{this, "TriggerTowerKey", "TriggerTowers", "Trigger Tower container"};

  NTuple::Item<short> m_latomeChannel;

  NTuple::Array<float>  m_ROD_energy;
  NTuple::Array<float>  m_ROD_time;
  NTuple::Array<float>  m_ROD_id;

  NTuple::Item<unsigned int> m_TType;


  // From LATOME header
  NTuple::Item<uint16_t> m_bcidLATOMEHEAD;

  NTuple::Item<uint32_t> m_ntNet;

  // DigitContainer
  NTuple::Array<unsigned short> m_bcidVec;
  NTuple::Item<uint32_t> m_latomeSourceId;
  NTuple::Array<short>  m_samples_ADC_BAS;
  NTuple::Array<unsigned short> m_bcidVec_ADC_BAS;

  NTuple::Array<int> m_energyVec_ET;
  NTuple::Array<unsigned short> m_bcidVec_ET;
  NTuple::Array<bool> m_saturVec_ET;

  NTuple::Array<int> m_energyVec_ET_ID;
  NTuple::Array<unsigned short> m_bcidVec_ET_ID;
  NTuple::Array<bool> m_saturVec_ET_ID;

  std::map<std::string,   NTuple::Item<unsigned int> > m_trigNameMap;
  NTuple::Item<uint32_t> m_LArEventBits;
  NTuple::Item<short>    m_LArInError;

  NTuple::Item<uint32_t> m_ntNTT;
  NTuple::Array<int>  m_TTEem;
  NTuple::Array<int>  m_TTEhad;
  NTuple::Array<double>  m_TTeta;
  NTuple::Array<double>  m_TTphi;
};

#endif
