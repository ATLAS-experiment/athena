/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PIXELCLUSTERDEDXCONDDATA_H
#define PIXELCLUSTERDEDXCONDDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include "Identifier/Identifier.h"
#include "Identifier/IdentifierHash.h"
#include <vector>
#include <tuple>

/**
 * @file PixelConditionsData/PixeldEdxData.h
 * @author Rebecca Hicks <rhicks@cern.ch>
 * @class PixelClusterdEdxCondData
 * @brief Store pixel cluster dEdx equalization scale factors */

class PixelClusterdEdxCondData {
  public:
    PixelClusterdEdxCondData();
    virtual ~PixelClusterdEdxCondData();
    float getScaleFactor(const IdentifierHash& wafer_hashID) const;
    void setScaleFactors(const std::vector<std::tuple<IdentifierHash,float>>& inputScaleFactors);
    void setConfig(const bool& flagValue); 
  private:
    std::vector<std::tuple<IdentifierHash,float>> m_clusterScaleFactors; 
    bool m_configFlag;
};

CLASS_DEF( PixelClusterdEdxCondData , 112527067 , 1 );
CONDCONT_DEF( PixelClusterdEdxCondData, 267373537);

#endif 
