//Dear emacs, this is -*-c++-*-
/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARROD_LARLATOMEBUILDERALG_H
#define LARROD_LARLATOMEBUILDERALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "LArElecCalib/ILArPedestal.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArRamp.h"
#include "LArElecCalib/ILArDAC2uA.h"
#include "LArElecCalib/ILAruA2MeV.h"
#include "LArElecCalib/ILArHVScaleCorr.h"
#include "LArElecCalib/ILArMphysOverMcal.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArRawEvent/LArRawSCContainer.h"
#include "xAODEventInfo/EventInfo.h"

//Event classes
class LArDigitContainer;
class LArOnlineID_Base;

class LArLATOMEBuilderAlg : public AthReentrantAlgorithm {

 public:
  LArLATOMEBuilderAlg(const std::string& name, ISvcLocator* pSvcLocator);

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;


 private:
  // event info input:
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey{this,"EventInfo","EventInfo","SG Key of EventInfo object"};
  //SC digit input:
  SG::ReadHandleKey<LArDigitContainer> m_digitKey{this, "LArDigitKey","SC", "SG Key of the SC LArDigitContainer"};
  //LArRawSC output:
  SG::WriteHandleKey<LArRawSCContainer> m_larRawSCKey{this,"LArRawSCKey","SC_ET_RECO","SG key of the output LArRawSCContainer"};

  //Conditions input:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};

  SG::ReadCondHandleKey<ILArPedestal> m_keyPedestalSC{this,"LArPedestalKeySC","LArPedestalSC","SG key of LArPedestal conditions object"};
  SG::ReadCondHandleKey<ILArOFC> m_keyOFCSC{this,"LArOFCKeySC","LArOFCSC","SG key of LArOFC conditions object"};
  SG::ReadCondHandleKey<ILArRamp> m_keyRampSC{this,"LArRampKeySC","LArRampSC","SG key of LArRamp conditions object"};
  SG::ReadCondHandleKey<ILArDAC2uA> m_keyDAC2uASC{this,"LArDAC2uAKeySC","LArDAC2uASC","SG key of LArDAC2uA conditions object"};
  SG::ReadCondHandleKey<ILAruA2MeV> m_keyuA2MeVSC{this,"LAruA2MeVKeySC","LAruA2MeVSC","SG key of LAruA2MeV conditions object"};
  SG::ReadCondHandleKey<ILArHVScaleCorr> m_keyHVScaleCorrSC{this,"LArHVScaleCorrKeySC","LArHVScaleCorrSC","SG key of LArHVScaleCorr conditions object"};
  SG::ReadCondHandleKey<ILArMphysOverMcal> m_keyMphysOverMcalSC{this,"LArMphysOverMcalKeySC","LArMphysOverMcalSC","SG key of LArMphysOverMcal conditions object"};


  //The following matters only in the MC case, when we have a 32 sample shapes
  //or we want to test shifted reco
  Gaudi::Property<int> m_startSample{this,"startEnergy",0,"the first energy to compute with respect to the BCID"};
  Gaudi::Property<int> m_nEnergies{this, "nEnergies", 1, "how many energies to compute"};

  Gaudi::Property<bool> m_applyHVCorrection{this, "applyHVCorrection", true, "apply HV correction"};
  Gaudi::Property<bool> m_applyMphysOverMcal{this, "applyMphysOverMcal", true, "apply MphysOverMcal correction"};
  Gaudi::Property<bool> m_useR0{this, "useR0", false, "use R0 from Ramp"};
  Gaudi::Property<bool> m_isADCBas{this, "isADCBas", true, "Digits are ADC BAS"};


  //Identifier helper
  const LArOnlineID_Base* m_onlineId = nullptr;

  static bool floatToInt(float val, int &newval, int hardpoint, int size) ;

};



#endif
