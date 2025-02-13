//Dear emacs, this is -*-c++-*- 
/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARROD_LARRAWCHANNELBUILDERSCALG_H
#define LARROD_LARRAWCHANNELBUILDERSCALG_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "LArElecCalib/ILArPedestal.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "LArRawConditions/LArDSPThresholdsComplete.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArShape.h" 
#include "LArCabling/LArOnOffIdMapping.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"
#include "LArRecConditions/LArBadChannelCont.h"

#include "CaloDetDescr/CaloDetDescrManager.h"
//Event classes
class LArDigitContainer;
class CaloCellContainer;
class CaloSuperCellDetDescrManager;
class LArOnline_SuperCellID;

class LArRawChannelBuilderSCAlg : public AthReentrantAlgorithm {

 public:
  using AthReentrantAlgorithm::AthReentrantAlgorithm;

  StatusCode initialize() override;
  StatusCode execute(const EventContext& ctx) const override;
  StatusCode finalize() override;


 private:
  //Event input:
  SG::ReadHandleKey<LArDigitContainer> m_digitKey{this, "LArDigitKey","LArDigitSCL2",
      "SG Key of LArDigitContaiiner"};
  //Event output:
  SG::WriteHandleKey<CaloCellContainer> m_cellKey{this,"CaloCellKey","SCellnoBCID",
      "SG key of the output CaloCellContainer"};

  //Conditions input:
  SG::ReadCondHandleKey<ILArPedestal> m_pedestalKey{this,"PedestalKey","LArPedestalSC","SG Key of Pedestal conditions object"};
  SG::ReadCondHandleKey<LArADC2MeV> m_adc2MeVKey{this,"ADC2MeVKey","LArADC2MeVSC","SG Key of ADC2MeV conditions object"};
  SG::ReadCondHandleKey<ILArOFC> m_ofcKey{this,"OFCKey","LArOFCSC","SG Key of OFC conditions object"};
  SG::ReadCondHandleKey<ILArShape> m_shapeKey{this,"ShapeKey","LArShapeSC","SG Key of Shape conditions object"};


  SG::ReadCondHandleKey<LArOnOffIdMapping> m_cablingKey{this,"CablingKey","LArOnOffIdMapSC","SG Key of LArOnOffIdMapping object"};

  /// Bad Channel masking for Super-Cells
  SG::ReadCondHandleKey<LArBadChannelCont> m_bcContKey{this, "LArBadChannelKey", "LArBadChannelSC", "Key of the LArBadChannelCont SC" };
  
  //Other jobOptions:
  Gaudi::Property<float> m_eCutFortQ{this,"ECutFortQ",256.0,"Time and Quality will be computed only for channels with E above this value"};
  //This flag decides if we compute Q and t for cells with negative energy
  Gaudi::Property<bool> m_absECutFortQ{this,"absECut",true,"Cut on fabs(E) for Q and t computation"};
  Gaudi::Property<bool> m_useShapeDer{this,"useShapeDer",true,"Use shape derivative in Q-factor computation"};

  //The following matters only in the MC case, when we have a 32 sample shapes
  Gaudi::Property<int> m_firstSample{this,"firstSample",0,"first of the 32 sampels of the MC shape to be used"};


  //Identifier helper
  const LArOnline_SuperCellID* m_onlineId = nullptr;

  // Super Cell DD manager key
  SG::ReadCondHandleKey<CaloSuperCellDetDescrManager> m_caloSuperCellMgrKey{
    this,"CaloSuperCellDetDescrManager","CaloSuperCellDetDescrManager","SG key of the resulting CaloSuperCellDetDescrManager"};
};



#endif
