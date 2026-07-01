/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARBYTESTREAM_LARRAWSCCALIBDATAREADINDINGALG_H
#define LARBYTESTREAM_LARRAWSCCALIBDATAREADINDINGALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "LArByteStream/LArLATOMEDecoder.h"

#include "LArCabling/LArOnOffIdMapping.h"
#include "LArCabling/LArLATOMEMapping.h"
#include "LArRecConditions/LArCalibLineMapping.h"

#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/SystemOfUnits.h"




//Event classes
class LArAccumulatedDigitContainer;
class LArAccumulatedCalibDigitContainer;
class LArLATOMEHeaderContainer;
class LArOnlineID;
class IROBDataProviderSvc;

class LArRawSCCalibDataReadingAlg : public  AthReentrantAlgorithm {
 public:
  LArRawSCCalibDataReadingAlg(const std::string& name, ISvcLocator* pSvcLocator): AthReentrantAlgorithm(name, pSvcLocator) {};

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;

 private:
  // Mapping input
  SG::ReadCondHandleKey<LArLATOMEMapping> m_mapKey {this,"MappingKey","LArLATOMEMap"};
  SG::ReadCondHandleKey<LArCalibLineMapping> m_calibMapKey{this,"CalibCablingKeyLeg","LArCalibLineMap","SG Key of LArCalibLineMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this, "OnOffMapLeg", "LArOnOffIdMap", "SG key for legacy mapping object"};

  //Event output:
  SG::WriteHandleKey<LArAccumulatedDigitContainer> m_accDigitKey{this,"LArSCAccDigitKey",""};
  SG::WriteHandleKey<LArAccumulatedCalibDigitContainer> m_accCalibDigitKey{this,"LArSCAccCalibDigitKey",""};
  SG::WriteHandleKey<LArLATOMEHeaderContainer> m_latomeHeaderCollKey{this, "LArLATOMEHeaderKey", "", "SG key of the LArLATOMEHEaderContainer"};
    
  //Service providing the input data
  ServiceHandle<IROBDataProviderSvc> m_robDataProviderSvc{this,"ROBDataProviderSvc","ROBDataProviderSvc"};
  
  //Other properties:
  BooleanProperty m_failOnCorruption{this,"FailOnCorruption",true,"Return FAILURE if data corruption is found"};

  DoubleProperty m_delayScale{this,"DelayScale",(25./240.)*Gaudi::Units::nanosecond,"One calibration step in time"};


  //Identifier helper
  const LArOnlineID* m_onlineId=nullptr;


  // The LATOME Decoder tool
  ToolHandle<LArLATOMEDecoder> m_latomeDecoder{this, "LATOMEDecoder", "LArByteStream/LATOMEDecoder", "decoder instance"};

  //Switches set in initialize() based of SG keys of output object
  bool m_doAccDigits=false;
  bool m_doAccCalibDigits=false;
  bool m_doLATOMEHeader = true;
 
};

#endif
