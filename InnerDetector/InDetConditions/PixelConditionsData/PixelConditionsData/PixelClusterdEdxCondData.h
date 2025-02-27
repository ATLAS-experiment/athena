/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef PIXELCLUSTERDEDXCONDDATA_H
#define PIXELCLUSTERDEDXCONDDATA_H

#include "AthenaKernel/CLASS_DEF.h"
#include "AthenaKernel/CondCont.h"
#include <vector>
#include <tuple>

/**
 * @file PixelConditionsData/PixeldEdxData.h
 * @author Rebecca Hicks <rhicks@cern.ch>
 * @class PixelClusterdEdxCondData
 * @brief Store pixel cluster dEdx calibration data */

class PixelClusterdEdxCondData {
  public:
    PixelClusterdEdxCondData();
    virtual ~PixelClusterdEdxCondData();
    long double getVar(const std::tuple<int,int,int>& module_coordinates) const;
    void setVar(const std::vector<std::tuple<std::tuple<int,int,int>,long double>>& value);
 
  private:
    std::vector<std::tuple<std::tuple<int,int,int>,long double>> m_var; //Default for testing
};

CLASS_DEF( PixelClusterdEdxCondData , 112527067 , 1 );
CONDCONT_DEF( PixelClusterdEdxCondData, 267373537);

#endif 
