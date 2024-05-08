/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARSC2NTUPLEEB_H
#define LARSC2NTUPLEEB_H

#include "LArCalibTools/LArCond2NtupleBaseEB.h"
#include "CaloDetDescr/ICaloSuperCellIDTool.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "LArRawEvent/LArLATOMEHeaderContainer.h"
#include "LArRawEvent/LArRawSCContainer.h"



class LArSC2NtupleEB : public LArCond2NtupleBaseEB
{
 public:
  LArSC2NtupleEB(const std::string & name, ISvcLocator * pSvcLocator);

  // Standard algorithm methods
  virtual StatusCode initialize()  override;
  virtual StatusCode execute()  override;

 protected:

  Gaudi::Property< unsigned int >  m_scNet{this, "scNet", 5, "number of energies from raw"};
  Gaudi::Property< unsigned int >  m_recoNet{this, "recoNet", 5, "number of energies from reco"};
  Gaudi::Property< bool > m_fillBCID{this, "FillBCID", false, "if to fill BCID"};

  NTuple::Item<unsigned long long> m_IEvent;
  NTuple::Item<short> m_bcid;

  NTuple::Matrix<float> m_energyVec_ET;
  NTuple::Matrix<float> m_bcidVec_ET;
  NTuple::Matrix<bool> m_saturVec_ET;
  NTuple::Matrix<bool> m_passVec_ET;

  NTuple::Matrix<float> m_energyVec_Reco;
  NTuple::Matrix<float> m_tauVec_Reco;
  NTuple::Matrix<float> m_bcidVec_Reco;
  NTuple::Matrix<bool> m_passVec_Reco;
  NTuple::Matrix<bool> m_saturVec_Reco;

  NTuple::Matrix<float> m_ROD_energy;
  NTuple::Matrix<float> m_ROD_time;
  NTuple::Matrix<float> m_ROD_id;

  SG::ReadHandleKey<LArRawSCContainer> m_sccontKey{this, "EnergyContainerKey", "", "key for LArRawSCContainer from bytestream"};
  SG::ReadHandleKey<LArRawSCContainer> m_reccontKey{this, "RecoContainerKey", "", "key for LArRawSCContainer reconstructed from digits"};
  SG::ReadHandleKey<LArRawChannelContainer> m_rawcontKey{this, "RawChanContainerKey", "", "key for LArRawChannelsContainer"};
   
  SG::ReadHandleKey<xAOD::EventInfo> m_evtInfoKey { this, "EventInfoKey", "EventInfo", "SG for EventInfo Key" };

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeyAdditional{this,"CablingKeyAdditional","LArOnOffIdMap","SG Key of LArOnOffIdMapping object for standard cells"};

  ToolHandle<ICaloSuperCellIDTool>   m_scidtool{this, "CaloSuperCellIDTool", "CaloSuperCellIDTool", "Offline / SuperCell ID mapping tool"}; 

  typedef std::map<HWIdentifier, const LArRawChannel*> rawChanMap_t;
  void fillRODEnergy(HWIdentifier SCId, unsigned cell, rawChanMap_t &rawChanMap,
                     const LArOnOffIdMapping* cabling, const LArOnOffIdMapping* cablingROD);

  typedef std::map<HWIdentifier, const LArRawSC*> rawSCMap_t;
};

#endif
