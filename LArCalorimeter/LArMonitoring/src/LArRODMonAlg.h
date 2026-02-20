/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARMONITORING_LArRODMONALG_H
#define LARMONITORING_LArRODMONALG_H

#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArShape.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "CaloConditions/CaloNoise.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArIdentifier/LArOnlineID.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include <atomic>
#include <fstream>


class LArDigit;
class LArFebHeaderContainer;


class LArRODMonAlg: public AthMonitorAlgorithm
{
 public:

  using AthMonitorAlgorithm::AthMonitorAlgorithm;

  /** @brief Default destructor */
  virtual ~LArRODMonAlg();

  virtual StatusCode initialize() override final;

  virtual StatusCode finalize() override final;

  // Called each event
  virtual StatusCode fillHistograms(const EventContext& ctx) const override final;


private:
  const LArOnlineID* m_LArOnlineIDHelper=nullptr;

  enum PARTITION                                              {  EMBC=0,EMBA,  EMECC,  EMECA,  HECC,  HECA,  FCALC,  FCALA,  N_PARTITIONS};
  static constexpr auto m_PARTNAMES=std::to_array<const char*>({"EMBC","EMBA","EMECC","EMECA","HECC","HECA","FCalC","FCalA","UNKNOWN"});
  
  class ERRCOUNTER {
  public:
    ERRCOUNTER() {
      clear();
    };
    void clear();
    std::array<unsigned,3> errors_E,errors_T,errors_Q;
  };
  


  PARTITION getPartition(const HWIdentifier chid) const;
  const char* getPartitionName(const HWIdentifier chid) const;

  struct diff_t {
    float e_on=0;
    float e_off=0;
    float t_on=0;
    float t_off=0;
    float q_on=0;
    float q_off=0;
  };

  diff_t compareChannel(const LArRawChannel& rcDig, 
                        const LArRawChannel& rcBS) const; 


  void detailedOutput(const LArRODMonAlg::diff_t&,
                      const LArDigit& dig, 
                      const EventContext& ctx) const;


  /** @brief Dump a cell's information and calculated energies into a txt file */
  void dumpCellInfo(const HWIdentifier chid,                    // Channel HW ID
		               const int gain,
                   const EventContext& ctx,
                   const diff_t & comp)const;

		   
  

  SG::ReadHandleKey<LArRawChannelContainer> m_channelKey_fromBytestream{this,"LArRawChannelKey_fromBytestream","LArRawChannels","SG key of LArRawChannels produced by teh DSP"};
  SG::ReadHandleKey<LArRawChannelContainer> m_channelKey_fromDigits{this,"LArRawChannelKey_fromDigits","LArRawChannels_FromDigits","SG key of LArRawChannels produced offline"};

  SG::ReadHandleKey<LArDigitContainer> m_digitContainerKey{this,"LArDigitContainerKey","FREE","SG key of LArDigitContainer read from Bytestream"};
  SG::ReadHandleKey<LArFebHeaderContainer> m_headerContainerKey{this,"LArFebHeaderKey","LArFebHeader","SG key of LArFebHeader"};


  SG::ReadCondHandleKey<ILArOFC>         m_keyOFC{this,"KeyOFC","LArOFC","SG key of LArOFC CDO"};
  SG::ReadCondHandleKey<ILArShape>       m_keyShape{this,"KeyShape","LArShape","SG key of LArShape CDO"};
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_keyHVScaleCorr{this,"KeyHVScaleCorr","LArHVScaleCorr","SG key of LArHVScaleCorr CDO"};
  SG::ReadCondHandleKey<ILArPedestal>    m_keyPedestal{this,"LArPedestalKey","LArPedestal","SG key of LArPedestal CDO"};

  SG::ReadCondHandleKey<LArADC2MeV> m_adc2mevKey{this,"LArADC2MeVKey","LArADC2MeV","SG Key of the LArADC2MeV CDO"};
  
  LArBadChannelMask m_bcMask;
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey {this, "BadChanKey", "LArBadChannel", "SG key for LArBadChan object"};
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"}; 


  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this,"CaloNoiseKey","totalNoise","SG Key of CaloNoise data object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping CDO"};

  //To get the data-dependency right ... 
  SG::ReadDecorHandleKey<xAOD::EventInfo> m_eventInfoKey{this, "LArStatusFlag", "EventInfo.larFlags", "Key for EventInfo object"};

  //declaration variables used in joboptions
  Gaudi::Property<std::string> m_MonGroupName  {this, "MonGroup", "RODMon"};
  Gaudi::Property<std::vector<std::string> > m_SubDetNames  {this, "LArRODSubDetNames", {} };
  Gaudi::Property<std::vector<std::string> > m_partitions {this, "LArRODPartitionNames", {} };
  Gaudi::Property<std::vector<int> > m_Nslots {this, "LArRODNslots", {} };

  // Output files names
  Gaudi::Property<std::string>  m_DigitsFileName{this, "DigitsFileName","digits.txt","Digits dump output filename"};
  Gaudi::Property<std::string>  m_EnergyFileName{this,"EnergyFileName","energy.txt"," Energies dump output filename"};
  Gaudi::Property<std::string>  m_AiFileName{this,"AiFileName","Calib_ai.dat","dump output filename"};
  Gaudi::Property<std::string>  m_DumpCellsFileName{this,"DumpCellsFileName","dumpCells.txt","Cells dump output filename"};

  Gaudi::Property<bool> m_doDspTestDump{this, "DoDspTestDump", false, "dsp dump switch"};
  Gaudi::Property<bool> m_doCellsDump{this, "DoCellsDump", false, "cell dump switch"};
  Gaudi::Property<bool> m_doCheckSum{this, "DoCheckSum", true, "checksum test switch"};
  Gaudi::Property<bool> m_doRodStatus{this, "DoRodStatus", true, "ROD status test switch"};
  Gaudi::Property<bool> m_printEnergyErrors{this, "PrintEnergyErrors", true, "energy errors printing"};
  Gaudi::Property<bool> m_removeNoiseBursts{this, "RemoveNoiseBursts", true, "removing events with noise bursts"};


  Gaudi::Property<bool> m_skipKnownProblematicChannels{this, "SkipKnownProblematicChannels", false, "skipping known problems?"};
  Gaudi::Property<bool> m_skipNullPed{this, "SkipNullPed", false, "skipping no pedestal channels ?"};
  Gaudi::Property<bool> m_skipNullQT{this, "SkipNullQT", false, "skipping no quality channe4ls ?"};

  Gaudi::Property<float> m_timeOffset{this, "TimeOffset", 0.};
  Gaudi::Property<short> m_adc_th{this, "ADCthreshold", 50, "Minimal number of ADC amplitude among samples required to compare online/offline"};
  Gaudi::Property<float> m_peakTime_cut{this, "peakTimeCut", 5., "Cut on abs(peak time) to compare online/offline (all quantities)"};

  
  // Expected precision for energy calculation, depending on energy (ranges) :
  // Adding 1 MeV on request of Alexis (truncating difference) (May 2016)
  // Between -213 and 213 MeV (~ 8 GeV)        2**0=1 MeV precision (range 0)
  // Between -216 and 216 MeV (~ 64 GeV)       2**3=8 MeV precision (range 1)
  // Between -219 and 219 MeV (~ 512 GeV)      2**6=64 MeV precision (range 2)
  // Between -222 and 222 MeV (~ 4 TeV)        2**9=512 MeV precision (range 3)

  Gaudi::Property<std::vector<std::pair<int, int>>> m_E_precision{this,"EnergyPrecisionRanges",
                                                                 {{8192,2},{65536,9},{524288,65},{4194304,513},{std::numeric_limits<int>::max(),8193}},
                                                                  "Energy precision ranges vector<pair<upperLimit,ExpectedPrecision>"};
  Gaudi::Property<std::vector<std::pair<int, int>>> m_T_precision{this,"TimePrecisionRanges",
                                                                  {{1000,340},{5000,340},{25000,340},{50000,340},{std::numeric_limits<int>::max(),340}},
                                                                  "Time precision ranges as vector<pair<upperLImit,ExpectedPrecision"};                                                 
  Gaudi::Property<std::vector<std::pair<int, int>>> m_Q_precision{this,"QualityPrecisionRanges",
                                                                  {{std::numeric_limits<int>::max(),3}},
                                                                  "Quality precision ranges as vector<pair<upperLImit,ExpectedPrecision"};       
  const float m_BC=25000; // value of 1 bunch-crossing = 25ns

  /* Histogram grouping (part) */
  std::map<std::string,int> m_histoGroups;

  Gaudi::Property<std::vector<std::string> > m_streams{this, "Streams", {} };

  Gaudi::Property<unsigned> m_max_dump{this, "MaxEvDump", 0, "max number of channels for detailed log-output"};
  mutable std::atomic<unsigned> m_ndump{0};


  //Streams for dump-files. Explicitly disabled in an MT-environment
  mutable std::ofstream m_fai ATLAS_THREAD_SAFE;
  mutable std::ofstream m_fdig ATLAS_THREAD_SAFE;
  mutable std::ofstream m_fen ATLAS_THREAD_SAFE;
  mutable std::ofstream m_fdump ATLAS_THREAD_SAFE;

};


inline 
LArRODMonAlg::PARTITION LArRODMonAlg::getPartition(const HWIdentifier chid) const{
  const int side=m_LArOnlineIDHelper->pos_neg(chid);
  if (m_LArOnlineIDHelper->isEMBchannel(chid)) {
    if (side==0)
      return EMBC;
    else
      return EMBA;
  }
  if (m_LArOnlineIDHelper->isEMECchannel(chid)) {
     if (side==0)
       return EMECC;
     else
       return EMECA;
  }
  if (m_LArOnlineIDHelper->isHECchannel(chid)){
    if (side==0)
      return HECC;
    else
      return HECA;
  }
  if (m_LArOnlineIDHelper->isFCALchannel(chid)) {
    if (side==0)
      return FCALC;
    else
      return FCALA;
  }

  ATH_MSG_FATAL(std::format("Channel {:#x} neither EMB nor EMEC nor HEC nor FCAL???",chid.get_identifier32().get_compact()));
  return N_PARTITIONS;
}

inline 
const char* LArRODMonAlg::getPartitionName(const HWIdentifier chid) const{
  return m_PARTNAMES[getPartition(chid)];
}

#endif
