/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PIXELCLUSTERDEDXCONDALG_H
#define PIXELCLUSTERDEDXCONDALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadCondHandleKey.h"
#include "AthenaPoolUtilities/CondAttrListCollection.h"

#include "StoreGate/WriteCondHandleKey.h"
#include "PixelConditionsData/PixelClusterdEdxCondData.h"
#include "PixelConditionsData/PixelDeadMapCondData.h"

#include "InDetIdentifier/PixelID.h"

#include "Gaudi/Property.h"


class PixelClusterdEdxCondAlg : public AthReentrantAlgorithm {
  public: 
    PixelClusterdEdxCondAlg (const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override final;
    virtual StatusCode execute(const EventContext& ctx) const override final;
    virtual bool isReEntrant() const override final { return false; }

  private:
    const PixelID* m_pixelID{nullptr};

    // Readkey takes precedence over the jsonPath. If readKey not empty json is ignored
    SG::ReadCondHandleKey<CondAttrListCollection> m_readKey
      {this, "ReadKey", "/PIXEL/PixelModuleFeMask", "Input deadmap folder"}; //XXXChange for testing!
  
    SG::WriteCondHandleKey<PixelDeadMapCondData> m_writeKey
      {this, "WriteKey", "PixelDeadMapCondData", "Output deadmap data MY TEST"}; //XXXChange for testing!
  
    Gaudi::Property<std::string> m_JsonLocation{this,"JsonPath","","Path to the JSON file containing list of modules to be masked"};
  
    SG::WriteCondHandleKey<PixelClusterdEdxCondData> m_writeKey2 //Just for testing... trying to create some kind of objec to read in PixelPID
      {this, "WriteKey", "PixelClusterdEdxCondData", "Making a PixelClusterdEdxCondData object for testing --Rebecca"};  
    
};

#endif


