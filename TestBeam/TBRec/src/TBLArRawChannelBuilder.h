/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TBREC_TBLARRAWCHANNELBUILDER_H
#define TBREC_TBLARRAWCHANNELBUILDER_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/IToolSvc.h"
#include "LArRawUtils/LArRawOrdering.h"
#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"
#include "LArCabling/LArOnOffIdMapping.h"

#include "CaloIdentifier/CaloCell_ID.h"
#include "CaloIdentifier/LArEM_ID.h"
#include "CaloIdentifier/LArFCAL_ID.h"
#include "CaloIdentifier/LArHEC_ID.h"

#include "CLHEP/Units/SystemOfUnits.h"

class LArOnlineID;

class TBLArRawChannelBuilder : public AthAlgorithm
{

public:

  TBLArRawChannelBuilder (const std::string& name, ISvcLocator* pSvcLocator);
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode finalize() override;

private:
  //Services & Tools 
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};

  const LArEM_ID*   m_emId{nullptr};
  const LArFCAL_ID* m_fcalId{nullptr};
  const LArHEC_ID*  m_hecId{nullptr};
  const LArOnlineID* m_onlineHelper{nullptr};
  
  LArRawOrdering m_larRawOrdering;
  
  //Algo-properties
  Gaudi::Property<std::string> m_DataLocation{this, "DataLocation", "FREE", "Input LArDigitContainer key"};
  Gaudi::Property<std::string> m_ChannelContainerName{this, "LArRawChannelContainerName", "LArRawChannels", "Output LArRawChannelContainer key"};
  Gaudi::Property<int> m_imaxSamp{this, "maxSamp", 8, "Sample used in FIXED mode"};
  Gaudi::Property<std::string> m_mode{this, "RecoMode", "CUBIC", "Reconstruction mode (MAX, CUBIC, FIXED)"};
  Gaudi::Property<float> m_cubicAdcCut{this, "CubicAdcCut", 50.0, "ADC cut for cubic fit"};
  
  float m_ADCtoMeVFCAL[3]{};
  float m_ADCtoMeVHEC[2]{};
  float m_ADCtoMeVEMECInner[2]{};
  float m_ADCtoMeVEMECOuter[2]{};
  float m_ADCtoMeVEMB[2]{};
  
  unsigned int m_iPedestal{0};                              // jobO ?
};

#endif
