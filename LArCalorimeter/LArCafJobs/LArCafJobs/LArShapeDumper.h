/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @class LArShapeDumper
 * @author Nicolas.Berger@cern.ch
 *   */

#ifndef LARCAFJOBS_LARSHAPEDUMPER_H
#define LARCAFJOBS_LARSHAPEDUMPER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/DataHandle.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "LArCafJobs/DataStore.h"
#include "LArRawConditions/LArPhysWaveContainer.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/CaloGain.h"
#include "LArIdentifier/LArOnline_SuperCellID.h"
#include "TrigDecisionTool/TrigDecisionTool.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/SmartDataPtr.h"
#include <fstream>
#include <string>
#include <stdint.h>
#include "TFile.h"
#include "TTree.h"
#include "TRandom.h"
#include "LArCafJobs/ILArShapeDumperTool.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

#include "LArRawConditions/LArADC2MeV.h"
#include "LumiBlockData/BunchCrossingCondData.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "LArElecCalib/ILArAutoCorr.h"
#include "LArElecCalib/ILArPedestal.h"


class MsgStream;
class StoreGateSvc;
class ILArShape;
class HWIdentifier;
class Identifier;
class CaloDetDescrElement;

namespace LArSamples {
  class EventData;
  class RunData;
}

namespace Trig {
  class ChainGroup;
}

class ATLAS_NOT_THREAD_SAFE LArShapeDumper : public AthAlgorithm
{
 public:
  LArShapeDumper(const std::string & name, ISvcLocator * pSvcLocator);

  ~LArShapeDumper();

  //standart algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  virtual StatusCode execute() override;
  virtual StatusCode stop() override;
  virtual StatusCode finalize() override;

  int makeEvent(LArSamples::EventData*& eventData, int run, int event, int lumiBlock, int bunchXing) const;
  
 private:
   
  std::string m_fileName;

  int m_prescale;  
  unsigned m_count;  
  unsigned m_maxChannels;
  unsigned m_nWrongBunchGroup;
  unsigned m_nPrescaledAway;
  unsigned m_nLArError;
  unsigned m_nNoDigits;
  unsigned m_nNoDigitsSC;


  std::string m_caloType; // EM, HEC, FCAL, SC
  double m_energyCut, m_noiseSignifCut;
  double m_energyCutSC;
  int m_minADCMax,m_minADCMaxSC ;
  std::string m_gainSpec;
  bool m_dumpDisc;
  std::vector<std::string> m_triggerNames;

  ToolHandle<ILArShapeDumperTool> m_dumperTool{this,"LArShapeDumperTool","LArShapeDumperTool"};
  ToolHandle<ILArShapeDumperTool> m_dumperToolSC{this,"LArShapeDumperToolSC","LArShapeDumperToolSC"};

  LArBadChannelMask m_bcMask;
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to patch"};
  LArBadChannelMask m_bcMaskSC;
  Gaudi::Property<std::vector<std::string> > m_problemsToMaskSC{this,"ProblemsToMaskSC",{}, "Bad-Channel categories to mask"};
  SG::ReadHandleKey<LArDigitContainer> m_digitsKey{this, "DigitsKey", "FREE", "key for LArDigitContainer"};
  SG::ReadHandleKey<LArRawChannelContainer> m_channelsKey{this, "ChannelsKey", "LArRawChannels", "key for LArRawChannels"};

  SG::ReadHandleKey<LArDigitContainer> m_digitsKeySC{this, "DigitsKeySC", "SC_ADC_BAS", "key for LArDigitContainer for SC"};
  SG::ReadHandleKey<LArRawSCContainer> m_rawscKey{this, "RawSCKey", "SC_ET_ID", "key for LArRawSCContainer"};
  SG::ReadHandleKey<LArRawSCContainer> m_rawRecomputedscKey{this, "RecomputedSCKey", "SC_ET_RECO", "key for recomputed LArRawSCContainer"};


  SG::ReadCondHandleKey<LArADC2MeV>   m_adc2mevKey{this,"ADC2MeVKey","LArADC2MeV","SG Key of ADC2MeV conditions object"};
 
  PublicToolHandle< Trig::TrigDecisionTool > m_trigDec{this, "TrigDecisionTool", "", "Handle to the TrigDecisionTool"};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"CablingKeySC","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object for SC"};

  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKey{this, "BadChanKey", "LArBadChannel", "SG bad channels key"};
  SG::ReadCondHandleKey<LArBadChannelCont> m_BCKeySC{this, "BadChanKeySC", "LArBadChannelSC", "SG bad channels key"};

  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};

  SG::ReadCondHandleKey<BunchCrossingCondData> m_bcDataKey {this, "BunchCrossingCondDataKey", "BunchCrossingData" ,"SG Key of BunchCrossing CDO"};


  SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{this,"PedestalKey","LArPedestal","SG Key of LArPedestal object"};
  SG::ReadCondHandleKey<ILArPedestal> m_pedestalKeySC{this,"PedestalKeySC","LArPedestalSC","SG Key of LArPedestal object"};

  SG::ReadCondHandleKey<ILArAutoCorr> m_acorrKey{this,"AutocorrKey","LArAutoCorr","SG Key of LArAutoCorr object"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey { this
      , "CaloDetDescrManager"
      , "CaloDetDescrManager"
      , "SG Key for CaloDetDescrManager in the Condition Store" };
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{this, 
     "CaloSuperCellDetDescrManager", "CaloSuperCellDetDescrManager", 
     "SG key of the resulting CaloSuperCellDetDescrManager" };
  const LArOnlineID* m_onlineHelper;
  const	LArOnline_SuperCellID* m_onlineHelperSC;
  //const DataHandle<ILArAutoCorr> m_autoCorr;
  //const DataHandle<LArPhysWaveContainer> m_physWave;

  bool m_doStream, m_doTrigger, m_doOFCIter, 
	m_doAllEvents, m_doRoIs, m_doAllLvl1, m_dumpChannelInfos;
  bool m_doEM, m_doHEC, m_doFCAL, m_doSC;
  bool m_gains[CaloGain::LARNGAIN]{};

  bool m_onlyEmptyBC;

  LArSamples::DataStore* m_samples;
  std::unique_ptr<LArSamples::RunData> m_runData;
  std::vector<const Trig::ChainGroup*> m_triggerGroups;
  TRandom m_random;
};

#endif
