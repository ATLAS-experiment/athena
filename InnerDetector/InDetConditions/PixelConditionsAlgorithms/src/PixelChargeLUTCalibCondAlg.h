/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/PixelChargeLUTCalibCondAlg.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date December, 2019
 * @brief Store pixel LUT charge calibration constants in PixelChargeCalibCondData.
 */

#ifndef PIXELCHARGELUTCALIBCONDALG
#define PIXELCHARGELUTCALIBCONDALG

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelModuleData.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"

#include "InDetIdentifier/PixelID.h"
#include "Gaudi/Property.h"

class PixelID;


class PixelChargeLUTCalibCondAlg : public AthReentrantAlgorithm {
  public:
    PixelChargeLUTCalibCondAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual bool isReEntrant() const override final { return false; }
  private:
    const PixelID* m_pixelID{nullptr};
    Gaudi::Property<std::string> m_pixelIDName
    {this, "PixelIDName", "PixelID", "Pixel ID name"};

    SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey
    {this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};

    SG::ReadCondHandleKey<PixelModuleData> m_configKey
    {this, "PixelModuleData", "PixelModuleData", "Pixel module data"};

    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey
    {this, "ReadKey", "/PIXEL/ChargeCalibration", "Input charge calibration folder"};

    SG::WriteCondHandleKey<PixelChargeCalibCondData> m_writeKey
    {this, "WriteKey", "PixelChargeCalibCondData", "Output charge caliblation data"};
    
    Gaudi::Property<bool> m_useLUTRD53 {this, "useLUTRD53",false,"use LUT for RD53 charge calibration"};
    Gaudi::Property<int> m_inputSource {this, "InputSource",0,"Source of data: 0 (parametric formula), 1 (single LUT), 2 (json file)"};
    Gaudi::Property<std::string> m_jsonFileName {this, "DataFile", "ITkPixChargeCalibData.json","Read constants from this file"};
};

#endif
