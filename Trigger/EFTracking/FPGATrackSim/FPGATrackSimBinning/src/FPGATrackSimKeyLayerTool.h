// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATrackSimKeyLayerTool_H
#define FPGATrackSimKeyLayerTool_H

/**
 * @file FPGATrackSimKeyLayerTool.h
 * @author Elliot Lipeles
 * @date Sept 10th, 2024
 * @brief Binning Classes for BinTool
 *
 * Declarations in this file (there are a series of small classes):
 *      class FPGATrackSimKeyLayerTool
 *      
 * 
 * Overview of stucture:
 *    -- 
 * 

 * 
 * References:
 *
 */


#include "FPGATrackSimObjects/FPGATrackSimTrackPars.h"
#include "FPGATrackSimBinning/FPGATrackSimBinUtil.h"
#include <cmath>

//-------------------------------------------------------------------------------------------------------
//
// Tool for doing Key Layer Math
//   -- that is converting be the track parameters set by the standard (pT,eta,phi,d0,z0) and 
//      the "KeyLayer" parametes set by the phi and z at to radii (R1,R2) and a deviation from
//      straight line between the two points (xm)
//   -- All the math  for the r-phi plane is based finding the rotated coordinate system where
//      the points (R1,phiR1) and (R2,phiR2) both lie on the x=0 axis. Then the x of a track halfway
//      between the points (called xm) is the sagitta.
// 
//-------------------------------------------------------------------------------------------------------
class FPGATrackSimKeyLayerTool
{
public:
  FPGATrackSimKeyLayerTool(double r1, double r2) : m_R1(r1), m_R2(r2) {}
  FPGATrackSimKeyLayerTool() {} // NOTE r1 and r2 must be set before using class

  struct KeyLyrPars {
    KeyLyrPars() = default;
    KeyLyrPars(const FPGATrackSimBinUtil::ParSet &parset)
        : z1(parset[0]), z2(parset[1]), phi1(parset[2]), phi2(parset[3]), xm(parset[4]) {}
    double z1{};
    double z2{};
    double phi1{};
    double phi2{};
    double xm{};
  };

  // convert (r,phi) to (x,y)
  std::pair<double, double> getXY(double r, double phi) const;

  // Get rotation angles need to rotate the two given points to be only seperated in the y direction (both have same x)
  // results is the sin and cos of rotation
  std::pair<double, double> getRotationAngles(const std::pair<double, double>& xy1, const std::pair<double, double>& xy2) const;

  // Apply a rotation angles (sin and cos) to a point xy
  std::pair<double, double> rotateXY(const std::pair<double, double>& xy, const std::pair<double, double>& ang) const;

  // Simple struct and function to get the rotation angle and full rotated information in one call
  struct rotatedConfig
  {
    std::pair<double, double> xy1p{};   // point 1 after rotation
    std::pair<double, double> xy2p{};   // point 2 after rotation
    std::pair<double, double> rotang{}; // rotation angle
    double y{};                         // y seperation after rotation
  };
  rotatedConfig getRotatedConfig(const KeyLyrPars &keypars) const;

  // get the x,y coordinates of a hit in the rotated coordinate system specified by rotang
  std::pair<double, double> getRotatedHit(const std::pair<double, double>& rotang, double rHit, double phiHit) const;

  // Convert back and forth to standard track parameters
  KeyLyrPars trackParsToKeyPars(const FPGATrackSimTrackPars &pars) const;
  FPGATrackSimTrackPars keyParsToTrackPars(const KeyLyrPars &keypars) const;

  // find expected z hit position given a radius
  double zExpected(const KeyLyrPars& keypars, double r) const;

  // find expected x position of a hit at given a radius in the rotated coordinate system
  double xExpected(const KeyLyrPars& keypars, double rHit, double phiHit) const;

  // takes hit position and calculated what xm would be for track going through that hit
  double xmForHit(const KeyLyrPars& keypars,  double rHit, double phiHit) const;

  // Find shift from nominal track to hit in the "x" direction 
  double deltaX(const KeyLyrPars& keypars,  double rHit, double phiHit) const;

  // accessors
  double R1() const {return m_R1;}
  double R2() const {return m_R2;}
  void setR1(const double r1) {m_R1=r1;}
  void setR2(const double r2) {m_R2=r2;}

  private:
    double m_R1{};
    double m_R2{};
};



#endif // FPGATrackSimKeyLayerTool_H
