/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCLUSTERDEDXCONDALG_H
#define PIXELCLUSTERDEDXCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "Gaudi/Property.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelClusterdEdxCondData.h"


class PixelClusterdEdxCondAlg : public AthReentrantAlgorithm {
  public: 
    PixelClusterdEdxCondAlg (const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual bool isReEntrant() const override final { return false; }

  private:
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey
      {this, "ReadKey", "/PIXEL/test", "Rebecca Test read key"}; //XXXChange for testing!  /PIXEL/test
  
    SG::WriteCondHandleKey<PixelClusterdEdxCondData> m_writeKey //Just for testing... trying to create some kind of objec to read in PixelPID
      {this, "WriteKey", "PixelClusterdEdxCondData", "Making a PixelClusterdEdxCondData object for testing --Rebecca"};  
   
    Gaudi::Property<bool> m_configStatus{this, "ConfigStatus",true,"Switching Pixel dEdx Calib on and off --Rebecca" };
 
};  
#endif


