/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PIXELCLUSTERDEDXCONDDATA_H
#define PIXELCLUSTERDEDXCONDDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"

/**
 * @file PixelConditionsData/PixeldEdxData.h
 * @author Rebecca Hicks <rhicks@cern.ch>
 * @class PixelClusterdEdxCondData
 * @brief Store pixel cluster dEdx calibration data */

class PixelClusterdEdxCondData {
  public:
    PixelClusterdEdxCondData();
    virtual ~PixelClusterdEdxCondData();
    int getVar() const;
    void setVar(const int value);
 
  private:
    int m_var = -999; //Default for testing
};

CLASS_DEF( PixelClusterdEdxCondData, 1240840447  , 1)
CONDCONT_DEF( PixelClusterdEdxCondData, 1240840448 ); /*These numbers are probably going to break something*/

#endif 
