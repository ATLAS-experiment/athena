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
 * @brief Store pixel cluster dEdx calibration data 
 **/
class PixeldClusterEdxCondData {
public:
  /*Constructor*/
  PixelClusterdEdxCondData();
  /*Destructor*/
  ~PixelClusterdEdxCondData();

  /*Get Calibration Constants Method*/
  std::vector<float> PixelClusterdEdxCondData::GetConstants();
  /*Set Calibration Constants Method*/
  void PixelClusterdEdxCondData::SetConstants();
private:
/*Conditions data is in the form of a scale factor map*/
/*Map[run,eta,layer,barrel] = sf*/

  std::vector<float> m_Map;

};

CLASS_DEF( PixelClusterdEdxCondData,  114268426 , 1)
CONDCONT_DEF(PixelClusterdEdxCondData, 183220670 , 1) /*These numbers are probably going to break something*/

#endif 
