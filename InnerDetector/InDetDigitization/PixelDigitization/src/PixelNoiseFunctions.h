/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */
#ifndef PixelNoiseFunctions_h
#define PixelNoiseFunctions_h


#include "PixelReadoutGeometry/IPixelReadoutManager.h"
#include <vector>
#include <utility> //for std::pair

class SiChargedDiodeCollection;
class SiTotalCharge;
class PixelModuleData;
class PixelChargeCalibCondData;
class ITkPixSimulationParameters;

namespace CLHEP{
  class HepRandomEngine;
}


namespace PixelDigitization{
  void crossTalk(double crossTalk, SiChargedDiodeCollection& chargedDiodes) ;
  
  void thermalNoise(double thermalNoise, SiChargedDiodeCollection& chargedDiodes,
    CLHEP::HepRandomEngine* rndmEngine);
                    
  void randomNoise(SiChargedDiodeCollection& chargedDiodes, const PixelModuleData *moduleData,
    int nBcid,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout);
    
  void 
  randomNoise(SiChargedDiodeCollection& chargedDiodes, const double totalNoiseOccupancy, 
    const std::vector<float> &noiseShape, float overflowToT,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout);
    
  void 
  randomNoise(SiChargedDiodeCollection& chargedDiodes, const ITkPixSimulationParameters & chipData,
    int nBcid,
    const PixelChargeCalibCondData *chargeCalibData, CLHEP::HepRandomEngine* rndmEngine, 
    InDetDD::IPixelReadoutManager * pixelReadout);
  
  //randomly disables certain elements, using moduleData to get probability
  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes,
    const PixelModuleData *moduleData,
    CLHEP::HepRandomEngine* rndmEngine);
    
  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes,
    const ITkPixSimulationParameters & chipData,
    CLHEP::HepRandomEngine* rndmEngine);
  
  //randomly disables certain elements, probability as a parameter          
  void 
  randomDisable(SiChargedDiodeCollection& chargedDiodes,
    double disableProbability, CLHEP::HepRandomEngine* rndmEngine);
    
  //generate Time-Over-Threshold int values with mean, rms, and valid range
  int 
  generateToT(CLHEP::HepRandomEngine* rndmEngine, double mean, double sd, const std::pair<int, int>& range);
  //
  double 
  getG4Time(const SiTotalCharge& totalCharge);
}//namespace
  
  #endif