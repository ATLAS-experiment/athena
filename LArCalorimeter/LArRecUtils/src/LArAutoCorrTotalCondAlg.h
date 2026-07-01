/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARAUTOCORRTOTALCONDALG_H
#define LARAUTOCORRTOTALCONDALG_H

#include "AthenaBaseComps/AthCondAlgorithm.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "StoreGate/WriteCondHandleKey.h"

#include "LArRawConditions/LArADC2MeV.h"
#include "LArRawConditions/LArAutoCorrTotal.h"

#include "LArCabling/LArOnOffIdMapping.h"

#include "LArElecCalib/ILArShape.h"
#include "LArElecCalib/ILArAutoCorr.h"
#include "LArElecCalib/ILArNoise.h"
#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArfSampl.h"
#include "LArElecCalib/ILArMinBias.h"

class LArAutoCorrTotal;

class LArAutoCorrTotalCondAlg : public AthCondAlgorithm {
public:

  using AthCondAlgorithm::AthCondAlgorithm;

  virtual ~LArAutoCorrTotalCondAlg() override;

  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) const override;

private:

  SG::ReadCondHandleKey<LArADC2MeV> m_LArADC2MeVObjKey{this,"LArADC2MeVObjKey","LArADC2MeV","SG key of the resulting LArADC2MeV object"};

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_LArOnOffIdMappingObjKey{this,"LArOnOffIdMappingObjKey","LArOnOffIdMap","SG key of LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<ILArShape> m_LArShapeObjKey{this,"LArShapeObjKey", "LArShapeSym","Key to read LArShape object"};
  SG::ReadCondHandleKey<ILArAutoCorr> m_LArAutoCorrObjKey{this,"LArAutoCorrObjKey", "LArAutoCorrSym","Key to read LArAutoCorr object"};
  SG::ReadCondHandleKey<ILArNoise> m_LArNoiseObjKey{this,"LArNoiseObjKey", "LArNoiseSym", "Key to read LArNoise object"};
  SG::ReadCondHandleKey<ILArPedestal> m_LArPedestalObjKey{this,"LArPedestalObjKey", "LArPedestal","Key to read LArPedestal object"};
  SG::ReadCondHandleKey<ILArfSampl> m_LArfSamplObjKey{this,"LArfSamplObjKey", "LArfSamplSym","Key to read LArfSampl object"};
  SG::ReadCondHandleKey<ILArMinBias> m_LArMinBiasObjKey{this,"LArMinBiasObjKey","LArMinBiasSym","Key to read LArMinBias object"};

  SG::WriteCondHandleKey<LArAutoCorrTotal> m_LArAutoCorrTotalObjKey{this,"LArAutoCorrTotalObjKey","LArAutoCorrTotal","Key to write LArAutoCorrTotal object"};

  Gaudi::Property<bool> m_NoPile{this,"NoPileUp",false};
  Gaudi::Property<bool> m_isMC{this,"isMC",true};
  Gaudi::Property<bool> m_isSuperCell{this,"isSuperCell",false};
  Gaudi::Property<int> m_Nsamples{this,"Nsamples",5, "Max number of samples to use"};
  Gaudi::Property<unsigned int> m_firstSample{this,"firstSample",0,"First sample to use for in-time event on the full pulse shape"};
  Gaudi::Property<int> m_deltaBunch{this,"deltaBunch",1,"Delta between filled bunches in 25 ns units"};

  Gaudi::Property<unsigned> m_nGains{this,"NGains",3,"Expected number of gains"};

};

#endif
