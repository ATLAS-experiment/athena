/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ChargeCalibParameters.cxx
 * @author Shaun Roe
 * @date October, 2023
 * @brief stream  of structs for holding charge calibration parameterisation and data
 */
#include "PixelConditionsData/ChargeCalibParameters.h"
#include <cmath>
#include <iostream>

namespace PixelChargeCalib{
  //definition of stream insertion operators for the structs in the header

  std::ostream & operator << (std::ostream & out, const LegacyFitParameters & f){
    out<<"("<<f.A<<", "<<f.E<<", "<<f.C<<", "<<f.maxToT<<")";
    return out;
  }
  std::ostream & operator << (std::ostream & out, const LinearFitParameters & f){
    out<<"("<<f.F<<", "<<f.G<<")";
    return out;
  }
  std::ostream & operator << (std::ostream & out, const Thresholds & t){
    out<<"("<<t.value<<", "<<t.sigma<<", "<<t.noise<<", "<<t.inTimeValue<<")";
     return out;
  }
  std::ostream & operator << (std::ostream & out, const Resolutions & r){
    out<<"("<<r.res1<<", "<<r.res2<<")";
    return out;
  }

}