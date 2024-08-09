/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MuonSegmentFitHelperFunctions__H
#define MUONR4__MuonSegmentFitHelperFunctions__H

#include "Acts/Seeding/HoughTransformUtils.hpp"
#include "MuonPatternEvent/MuonHoughDefs.h"

class MsgStream;

namespace MuonR4{
    class CalibratedSpacePoint;

    namespace SegmentFitHelpers{
      /** @brief Calculates the chi2 contribuation to a linear segment line from an uncalibrated measurement.
       *         Currently only Mdt, Tgc & Rpc are supported
       *  @param posInChamber: Position of the chamber crossing expressed at z=0
       *  @param dirInChamber: Segment direction inside the chamber
       *  @param measurement: Mdt measurement
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTerm(const Amg::Vector3D& posInChamber,
                       const Amg::Vector3D& dirInChamber,
                       const MuonR4::HoughHitType& measurement,
                       MsgStream& msg);
      /** @brief Calculates the chi2 contribuation to a linear segment line from an uncalibrated Mdt measurement.
       *  @param posInChamber: Position of the chamber crossing expressed at z=0
       *  @param dirInChamber: Segment direction inside the chamber
       *  @param measurement: Mdt measurement
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTermMdt(const Amg::Vector3D& posInChamber,
                          const Amg::Vector3D& dirInChamber,
                          const MuonR4::HoughHitType& measurement,
                          MsgStream& msg);
      /** @brief Calculates the chi2 contribuation to a linear segment line from an uncalibrated strip measurement.
       *         Currently only Tgc & Rpc are supported
       *  @param posInChamber: Position of the chamber crossing expressed at z=0
       *  @param dirInChamber: Segment direction inside the chamber
       *  @param measurement: Mdt measurement
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTermStrip(const Amg::Vector3D& posInChamber,
                            const Amg::Vector3D& dirInChamber,
                            const MuonR4::HoughHitType& measurement,
                            MsgStream& msg);
      /** @brief Calculates the chi2 contribution from the given measurement. Currently,
        *        MdtDriftCircles, Rpc & Tgc as well as the Beamspot are supported
       *  @param posInChamber: Position of the segment in the chamber frame
       *  @param dirInChamber: Direction of flight of the chamber
       *  @param timeOfArrival: The arrival time of the particle at the measurement's plane. The
       *                         parameter is optional  and only active for Rpcs. If passed, the 
       *                         Rpc timing is considered
       *  @param measurement: Space point to which the chi2 term is calculated
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTerm(const Amg::Vector3D& posInChamber,
                       const Amg::Vector3D& dirInChamber,
                       std::optional<double> timeOfArrival,
                       const CalibratedSpacePoint& measurement,
                       MsgStream& msg);
      /** @brief Calculates the chi2 contribution from a mdt space point to the segment line
       *  @param posInChamber: Position of the segment in the chamber frame
       *  @param dirInChamber: Direction of flight of the chamber
       *  @param mdtSpacePoint: Space point to which the chi2 term is calculated
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTermMdt(const Amg::Vector3D& posInChamber,
                          const Amg::Vector3D& dirInChamber,
                          const CalibratedSpacePoint& mdtSpacePoint,
                          MsgStream& msg);
      /** @brief Calculates the chi2 contribution from a strip measurement to the segment line
       *  @param posInChamber: Position of the segment in the chamber frame
       *  @param dirInChamber: Direction of flight of the chamber
       *  @param timeOfArrival: The arrival time of the particle at the measurement's plane. The
       *                        parameter is optional and if it's given the time information
       *                        enters the chi2.
       *  @param strip: Strip measurement to consider.  
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */      
      double chiSqTermStrip(const Amg::Vector3D& posInChamber,
                            const Amg::Vector3D& dirInChamber,
                            std::optional<double> timeOfArrival,
                            const CalibratedSpacePoint& strip,
                            MsgStream& msg);
      /** @brief Calculates the chi2 contribution from an external beam spot constraint
        *  @param posInChamber: Position of the segment in the chamber frame
        *  @param dirInChamber: Direction of flight of the chamber
        *  @param beamSpotMeas: Strip measurement to consider.  
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      double chiSqTermBeamspot(const Amg::Vector3D& posInChamber,
                               const Amg::Vector3D& dirInChamber,
                               const CalibratedSpacePoint& beamSpotMeas,
                               MsgStream& msg);
    
    }
}

#endif // MUONR4__MuonSegmentFitHelperFunctions__H
