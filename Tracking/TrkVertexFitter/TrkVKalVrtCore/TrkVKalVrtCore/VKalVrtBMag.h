/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/* General magnetic field in any point access              */
/* If external magnetic field handler is provided as       */
/* either an object inherited from the baseMagFld class    */
/* or a function addrMagHandler - the vkalMagFld class     */
/* will use it, otherwise the vkalMagFld returns           */
/* the constant magnetic field.                            */
/* 							   */
/*  Thread-safe implementation				   */
/*---------------------------------------------------------*/
#ifndef TRKVKALVRTCORE_VKALVRTBMAG_H
#define TRKVKALVRTCORE_VKALVRTBMAG_H

#include "TrkVKalVrtCore/CommonPars.h"
#include <cmath>

namespace Trk {

class VKalVrtControlBase;

typedef void (*addrMagHandler)(double, double, double, double&, double&,
                               double&);

//
//  Base class for concrete magnetic field implementations (e.g. Athena tool) to
//  be called by vkalMagFld
//
class baseMagFld {
 public:
  baseMagFld() = default;
  virtual ~baseMagFld() = default;
  virtual void getMagFld(const double, const double, const double, double&,
                         double&, double&) = 0;
//
//  Function returns effective field for Perigee track parameters.
// Motion equation remains the same in any field.
// Phi and Theta are particle momentum angles at Perigee 
//
  inline double getEffField(double bx, double by, double bz, double phi, double theta){
    return bz-(by*std::sin(phi)+bx*std::cos(phi))/std::tan(theta);
  };

};

//
// Main magnetic field implememtation in VKalVrtCore package.
// Depending on VKalVrtControlBase it either calls external magnetic field
// or uses default fixed magnetic field.
//
class vkalMagFld {
 public:
  static void getMagFld(const double, const double, const double, double&,
                        double&, double&, const VKalVrtControlBase*);
  static double getMagFld(const double xyz[3],
                          const VKalVrtControlBase* FitControl);
  /* Converstion for MeV and mm and Tesla*/
  inline static double getCnvCst() { return vkalMagCnvCst; }
  inline static double getEffField(double bx, double by, double bz, double phi, double theta){
    return bz-(by*std::sin(phi)+bx*std::cos(phi))/std::tan(theta);
  };

};

}  // namespace Trk
#endif
