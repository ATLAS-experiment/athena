/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/ITkPixChargeCalibAlg.h
 * @author Shaun Roe (based on code by Soshi Tsuno)
 * @date September 2023
 *
 */

#ifndef ITkPixChargeCalibAlg_h
#define ITkPixChargeCalibAlg_h

#include "AthenaBaseComps/AthCondAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"

class PixelID;

class ITkPixChargeCalibAlg : public AthCondAlgorithm {
  public:
    ITkPixChargeCalibAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    const PixelID* m_pixelID{nullptr};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey
    {this, "PixelDetEleCollKey", "ITkPixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};

    SG::WriteCondHandleKey<PixelChargeCalibCondData> m_writeKey
    {this, "WriteKey", "ITkPixelChargeCalibCondData", "Output charge calibration data"};
    
};

#endif
