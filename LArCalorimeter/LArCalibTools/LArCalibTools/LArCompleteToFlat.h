//-*- C++ -*-

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#ifndef LARCALIBTOOLS_LARCOMPLETETOFLAT_H
#define LARCALIBTOOLS_LARCOMPLETETOFLAT_H 1

#include <string>
#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "LArRawConditions/LArConditionsContainer.h"
#include "LArRawConditions/LArSingleFloatP.h"
#include "GaudiKernel/ToolHandle.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "LArCOOLConditions/LArfSamplSC.h"



class LArOnlineID_Base; 
class CondAttrListCollection;
class AthenaAttributeList;
class ILArPedestal;
class ILArOFC;
class ILArRamp;
class LArShapeComplete;
class ILArDAC2uA;
class ILAruA2MeV;
class LArDSPThresholdsComplete;

class LArCompleteToFlat: public AthReentrantAlgorithm
{ 

  /////////////////////////////////////////////////////////////////// 
  // Public methods: 
  /////////////////////////////////////////////////////////////////// 
 public: 
  /// Constructor with parameters: 
  LArCompleteToFlat( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor: 
  virtual ~LArCompleteToFlat(); 

  // Athena algorithm's Hooks
  virtual StatusCode  initialize() override;
  virtual StatusCode  execute(const EventContext&) const override {return StatusCode::SUCCESS;}
  virtual StatusCode  stop() override;

 private: 
  /// Default constructor: 
  LArCompleteToFlat() = delete;


  CondAttrListCollection* singleFloatFlat(const char* blobName, const LArConditionsContainer<LArSingleFloatP>* input, 
					  const std::string& outputName, const unsigned nGain, const bool withFCAL=true);
  CondAttrListCollection* DAC2uAFlat(const ILArDAC2uA* input, const std::string& outputName);
  CondAttrListCollection* uA2MeVFlat(const ILAruA2MeV* input, const std::string& outputName);
  CondAttrListCollection* pedestalFlat(const ILArPedestal* input, const std::string& outputName);
  CondAttrListCollection* rampFlat(const ILArRamp* input, const std::string& outputName);
  CondAttrListCollection* ofcFlat(const ILArOFC* input, const std::string& outputName, const LArfSamplSC* weights=nullptr);
  CondAttrListCollection* shapeFlat(const LArShapeComplete* input, const std::string& outputName);
  AthenaAttributeList* DSPThresholdsFlat(const LArDSPThresholdsComplete* input, const std::string& outputName);


  void errIfConnected(const HWIdentifier chid, const int gain, const char* objName, const char* message=0) const;

  unsigned m_hashMax = 0;
  const LArOnlineID_Base*  m_onlineID = nullptr;

  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMap","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKeySC{this,"CablingSCKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};
  SG::ReadCondHandleKey<LArfSamplSC> m_weightsKeySC{this,"WeightsSCKey","","SG Key of weights object"};

  ///InputSGKeys
  StringProperty m_uA2MeVInput { this, "uA2MeVInput",  "" }; // LAruA2MeV
  StringProperty m_DAC2uAInput { this, "DAC2uAVInput", "" }; // LArDAC2uA
  StringProperty m_HVScaleCorrInput { this, "HVScaleCorrInput", "" }; // LArHVScaleCorr
  StringProperty m_PedestalInput { this, "PedestalInput", "" }; // Pedestal
  StringProperty m_RampInput { this, "RampInput", "" }; // LArRamp
  StringProperty m_MphysOverMcalInput { this, "MphysOverMcalInput", "" }; // LArMphysOverMcal
  std::string m_OFCInput;
  std::string m_OFCCaliInput;
  std::string m_ShapeInput;
  std::string m_DSPThresholdsInput;

  // DSPThreshold set name
  std::string m_nameOfSet;

  BooleanProperty m_isSC = { this, "isSC", false };
  bool m_forceStop;
  bool m_fakeEMBPSLowGain;
}; 

#endif //> !LARCALIBTOOLS_LARCOMPLETETOFLAT_H
