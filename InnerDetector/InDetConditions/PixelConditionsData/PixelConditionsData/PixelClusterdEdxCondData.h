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
    std::string getVar() const;
    void setVar(const std::string& value);
 
  private:
    std::string m_var = "Rebecca was here"; //Default for testing
};

CLASS_DEF( PixelClusterdEdxCondData , 112527067 , 1 );
CONDCONT_DEF( PixelClusterdEdxCondData, 267373537);

#endif 
