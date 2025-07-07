/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__MuonSegmentFitHelperFunctions__H
#define MUONR4__MuonSegmentFitHelperFunctions__H

#include "MuonPatternEvent/MuonHoughDefs.h"
#include "Acts/Seeding/HoughTransformUtils.hpp"


class MsgStream;

namespace MuonR4{
    class CalibratedSpacePoint;
    class SpacePoint;

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
                       const SpacePoint& measurement,
                       MsgStream& msg);
      /** @brief Calculates the chi2 contribuation to a linear segment line from an uncalibrated Mdt measurement.
       *  @param posInChamber: Position of the chamber crossing expressed at z=0
       *  @param dirInChamber: Segment direction inside the chamber
       *  @param measurement: Mdt measurement
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTermMdt(const Amg::Vector3D& posInChamber,
                          const Amg::Vector3D& dirInChamber,
                          const SpacePoint& measurement,
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
                            const SpacePoint& measurement,
                            MsgStream& msg);


      /** @brief Calculates the chi2 contribution from the given measurement. Currently,
        *        MdtDriftCircles, Rpc & Tgc as well as the Beamspot are supported
       *  @param posInChamber: Position of the segment in the sector frame
       *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
       *  @param timeShift:    Shift from the nominal arrival time, that's calculated as R / c. 
       *  @param arrivalTime:  Arrival time of the particle at the segment's refernce plane. 
       *                       If the arrival time is passed, the timing information of the measurement is
       *                       explicitly taken into account.
       *  @param measurement: Space point to which the chi2 term is calculated
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTerm(const Amg::Vector3D& posInChamber,
                       const Amg::Vector3D& dirInChamber,
                       const double timeShift,
                       std::optional<double> arrivalTime,
                       const CalibratedSpacePoint& measurement,
                       MsgStream& msg);
      /** @brief Calculates the chi2 contribution from a mdt space point to the segment line
       *  @param posInChamber: Position of the segment in the sector frame
       *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
       *  @param mdtSpacePoint: Space point to which the chi2 term is calculated
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */
      double chiSqTermMdt(const Amg::Vector3D& posInChamber,
                          const Amg::Vector3D& dirInChamber,
                          const CalibratedSpacePoint& mdtSpacePoint,
                          MsgStream& msg);
      /** @brief Calculates the chi2 contribution from a strip measurement to the segment line
       *  @param posInChamber: Position of the segment in the sector frame
       *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
       *  @param timeShift:    Shift from the nominal arrival time, that's calculated as R / c.
       *  @param arrivalTime:  Arrival time of the particle at the segment's refernce plane. 
       *                       If the arrival time is passed, the timing information of the measurement is
       *                       explicitly taken into account.
       *  @param strip: Strip measurement to consider.  
       *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
       *              then all relevant parameters are printed */      
      double chiSqTermStrip(const Amg::Vector3D& posInChamber,
                            const Amg::Vector3D& dirInChamber,
                            const double timeShift,
                            std::optional<double> arrivalTime,
                            const CalibratedSpacePoint& strip,
                            MsgStream& msg);
      /** @brief Calculates the chi2 contribution from an external beam spot constraint
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param beamSpotMeas: Strip measurement to consider.  
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      double chiSqTermBeamspot(const Amg::Vector3D& posInChamber,
                               const Amg::Vector3D& dirInChamber,
                               const CalibratedSpacePoint& beamSpotMeas,
                               MsgStream& msg);

      /** @brief Calculates whether a segment line travereses the tube measurements on the left (-1) or 
       *         right (1) side of the tube wire. Strip measurements & nullptrs are assigned with 0
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param uncalibHits: List of uncalibrated measurements
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      std::vector<int> driftSigns(const Amg::Vector3D& posInChamber,
                                  const Amg::Vector3D& dirInChamber,
                                  const std::vector<const SpacePoint*>& uncalibHits,
                                  MsgStream& msg);
      /** @brief Calculates whether a segment line travereses the tube measurements on the left (-1) or 
       *         right (1) side of the tube wire. Strip measurements & nullptrs are assigned with 0
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param calibHits: List of calibrated measurements
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      std::vector<int> driftSigns(const Amg::Vector3D& posInChamber,
                                  const Amg::Vector3D& dirInChamber,
                                  const std::vector<std::unique_ptr<CalibratedSpacePoint>>& calibHits,
                                  MsgStream& msg);
       /** @brief Calculates whether a segement line travereses the tube measurement on the left (-1) or 
        *         right (1) side of the tube wire. Strip measurements & nullptrs are assigned with 0
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param uncalibHit:  Uncalibrated hit to consider
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      int driftSign (const Amg::Vector3D& posInChamber,
                     const Amg::Vector3D& dirInChamber,
                     const SpacePoint& uncalibHit,
                     MsgStream& msg);

       /** @brief Calculates whether a segement line travereses the tube measurement on the left (-1) or 
        *         right (1) side of the tube wire. Strip measurements & nullptrs are assigned with 0
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param calibHit: Calibrated hit to consider
        *  @param msg: Reference to the callers msgStream. If the level is VERBOSE,
        *              then all relevant parameters are printed */
      int driftSign(const Amg::Vector3D& posInChamber,
                    const Amg::Vector3D& dirInChamber,
                    const CalibratedSpacePoint& calibHit,
                    MsgStream& msg);
      /** @brief Extrapolates the segment line onto the plane defined by the by the space-point's 
        *        position & plane normal vector
        *  @param posInChamber: Position of the segment in the sector frame
        *  @param dirInChamber: Direction of flight of the  segment expressed in the sector frame
        *  @param sp: Reference to the space point defining the plane */
      Amg::Vector3D extrapolateToPlane(const Amg::Vector3D& pos,
                                       const Amg::Vector3D& dir,
                                       const SpacePoint& hit);
    }
}

#endif // MUONR4__MuonSegmentFitHelperFunctions__H
