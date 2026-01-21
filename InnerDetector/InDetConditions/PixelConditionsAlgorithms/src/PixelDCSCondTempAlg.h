/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file PixelConditionsAlgorithms/PixelDCSCondTempAlg.h
 * @author Soshi Tsuno <Soshi.Tsuno@cern.ch>
 * @date November, 2019
 * @brief Created pixel temperature data in PixelDCSTempData.
 */

#ifndef PIXELDCSCONDTEMPALG
#define PIXELDCSCONDTEMPALG

#include "AthenaBaseComps/AthCondAlgorithm.h" //base class

#include "StoreGate/ReadCondHandleKey.h" //templated member
#include "AthenaPoolUtilities/CondAttrListCollection.h" //template argument

#include "StoreGate/WriteCondHandleKey.h"  //templated member
#include "PixelConditionsData/PixelDCSTempData.h" //template argument


class PixelID;

class PixelDCSCondTempAlg : public AthCondAlgorithm {
  public:
    PixelDCSCondTempAlg(const std::string& name, ISvcLocator* pSvcLocator);
    virtual ~PixelDCSCondTempAlg() = default;

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;

  private:
    const PixelID* m_pixelID{nullptr};

    Gaudi::Property<float> m_defaultTemperature
    {this, "DefaultTemperature", -7.0, "Default module temperature when no measured value is available"};
    
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey
    {this, "ReadKey", "/PIXEL/DCS/TEMPERATURE", "Key of input (raw) temperature conditions folder"};

    SG::WriteCondHandleKey<PixelDCSTempData> m_writeKey
    {this, "WriteKey", "PixelDCSTempCondData", "Key of output (derived) temperature conditions folder"};

};

#endif
