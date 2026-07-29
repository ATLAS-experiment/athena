/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARDSPTHRESHOLDSFILLINGINLINE_H
#define LARDSPTHRESHOLDSFILLINGINLINE_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "CaloIdentifier/CaloCellGroup.h"
#include "LArRecConditions/LArBadChannelMask.h"
#include "LArRecConditions/LArBadChannelCont.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloDetDescr/CaloDetDescrManager.h"

class LArOnlineID;

class LArDSPThresholdFillInline : public AthReentrantAlgorithm {
 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;
  virtual ~LArDSPThresholdFillInline();
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode stop() override;

 private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager","CaloDetDescrManager","SG Key for CaloDetDescrManager in the Condition Store" };

  const LArOnlineID* m_onlineID = nullptr;

  StringProperty m_nameOfSet { this, "NameOfSet", "" };
  StringProperty m_key       { this, "Key",       "DSPThresholds" };
  StringProperty m_mode      { this, "mode",      "fixed",
                               "Select how to set thresholds. Allowed values are 'fixed','group' and 'noise'" };

  // For mode 'group'
  StringArrayProperty m_cellGroupStr { this, "ThresholdsPerCellGroup", {} };

  // For mode 'fixed'
  FloatProperty m_tqThrsh      { this, "tQThreshold",      250 };
  FloatProperty m_samplesThrsh { this, "samplesThreshold", 1000 };

  // For mode 'Noise'
  FloatProperty m_sigmaNoiseSamples {this, "sigmaNoiseSamples", 0 };
  FloatProperty m_sigmaNoiseQt {this, "sigmaNoiseQt", 0 };
  BooleanProperty m_usePileupNoiseSamples {this, "usePileupNoiseSamples", false };
  BooleanProperty m_usePileupNoiseQt {this, "usePileupNoiseQt", false };

  // For channel masking
  BooleanProperty m_maskBadChannels { this, "MaskBadChannels", false };
  FloatProperty m_maskedtqThrsh { this, "MaskedtQThreshold", static_cast<float>(0x7fffffff) };
  FloatProperty m_maskedsamplesThrsh { this, "MaskedsamplesThreshold", static_cast<float>(0x7fffffff) };

  BooleanProperty m_dump { this, "Dump", false };
  StringProperty m_outFileName { this, "OutFile", "out.txt" };
  BooleanProperty m_fill { this, "Fill", true };

  SG::ReadCondHandleKey<CaloNoise> m_totalNoiseKey
    { this, "TotalNoiseKey", "totalNoise", "SG key for total noise" };
  SG::ReadCondHandleKey<CaloNoise> m_elecNoiseKey
    { this, "ElecNoiseKey", "electronicNoise", "SG key for electronic noise" };

  /** Handle to bad-channel mask */
  LArBadChannelMask m_bcMask;
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey {this, "BadChanKey", "LArBadChannel", "SG key for LArBadChan object"};
  Gaudi::Property<std::vector<std::string> > m_problemsToMask{this,"ProblemsToMask",{}, "Bad-Channel categories to mask"};
  Gaudi::Property<float> m_scaleIW{this,"ScaleIW",0.,"if >0 scale EMEC IW, HEC thresholds with this factor"};
 

  enum mode_t{
    FIXED,GROUP,NOISE
  };

  mode_t m_workmode{FIXED};

  CaloCellGroupList m_thrPerCell;
};

#endif
