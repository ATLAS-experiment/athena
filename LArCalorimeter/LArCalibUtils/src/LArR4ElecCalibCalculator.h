//Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARR4ELECCALIBCALCULATOR_H
#define LARR4ELECCALIBCALCULATOR_H
#include "AthenaBaseComps/AthAlgorithm.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "LArElecCalib/ILArDAC2uA.h"
#include "LArElecCalib/ILAruA2MeV.h"
#include "LArRawConditions/LArMCSym.h"
#include "CaloDetDescr/CaloDetDescrManager.h"


class LArOnlineID;
class CaloCell_ID;
class LArR4ElecCalibCalculator : public AthAlgorithm {
 
public:
  using AthAlgorithm::AthAlgorithm;

  //standard algorithm methods
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext& ctx) override;
  virtual StatusCode stop() override;


private:
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "OnOffMap", "LArOnOffIdMap", "SG key for mapping object"};
  SG::ReadCondHandleKey<ILAruA2MeV>         m_lAruA2MeVKey{this,"LAruA2MeVKey","LAruA2MeVSym","SG key of uA2MeV object"}; //Always used
  SG::ReadCondHandleKey<ILArDAC2uA>         m_lArDAC2uAKey{this,"LArDAC2uAKey","LArDAC2uASym","SG key of DAC2uA object"}; //Always used
  SG::ReadCondHandleKey<LArMCSym>           m_mcSym{this,"MCSym","LArMCSym","SG Key of LArMCSym object"};
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};  

 
  Gaudi::Property<std::string> m_keyoutput{this,"KeyOutput","LArRamp","SG Key of output object"};

  IntegerProperty m_nADCBits{this,"nADCBits",15,"Assume 15-bit ADC"};

  Gaudi::Property<float>  m_pedestalValue{this,"pedestal",8192,"Pedestal values for the two gains"};
  //FIXME: The pedestal RMS is 4 for the lower gain. But the current LArPedestalMC cond obj doesn't allow gain-dependent values
  Gaudi::Property<float>  m_pedestalRMS{this,"pedestalRMS",17,"Pedestal RMS for the two gains"};
  Gaudi::Property<std::string> m_pedkeyoutput{this,"PedKeyOutput","LArPedestal","SG Key of output object"};

  Gaudi::Property<std::string> m_crestDBStr{this, "CRESTDB","","CREST connection string"};


  const LArOnlineID* m_onlineHelper=nullptr;
  const CaloCell_ID* m_caloCellID=nullptr; 
 
};

#endif
