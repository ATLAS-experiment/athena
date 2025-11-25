/*
   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSPACEPOINTCALIBRATOR_ISPACEPOINTCALIBRATOR_H
#define MUONSPACEPOINTCALIBRATOR_ISPACEPOINTCALIBRATOR_H

#include <GaudiKernel/IAlgTool.h>
#include <GaudiKernel/EventContext.h>

#include <GeoPrimitives/GeoPrimitives.h>
///
#include <xAODMuon/MuonSegment.h>
#include <ActsEvent/TrackContainer.h>
#include <Acts/EventData/SourceLink.hpp>

#include <memory>

namespace ActsTrk{
    class GeometryContext;
}
namespace Acts{
    class CalibrationContext;
    class GeometryContext;
}
namespace MuonR4{
    class SpacePoint;
    class CalibratedSpacePoint;
}

namespace MuonR4{
    /** @brief Interface class to refine the space point calibration with an external seed */
    class ISpacePointCalibrator : virtual public IAlgTool {
        public:
            DeclareInterfaceID(ISpacePointCalibrator, 1, 0);
            
            virtual ~ISpacePointCalibrator() = default;

            using CalibSpacePointPtr = std::unique_ptr<CalibratedSpacePoint>;
            using CalibSpacePointVec = std::vector<CalibSpacePointPtr>;
            /** @brief Calibrates a single space point. Mdt drift radii are corrected for time slew, signal
             *         propagation & LorentzAngle effects. The second coordinate of 1D space points is updated according
             *         to the closest approach of the strip to the complemntary coordinate. The space point calibrator
             *         always returns a calibrated object. But the state of the space point is set to FailedCalib in
             *         if the calibration fails. 
             *  @param ctx: EventContext to access conditions data
             *  @param spacePoint: Pointer to the space point to calibrate.
             *  @param seedPosInChamb: Position of the external seed expressed in the sector frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the sector frame
             *  @param timeDelay: Shift in time to be added to the time of flight of a particle going a straight path */
            virtual CalibSpacePointPtr calibrate(const EventContext& ctx,
                                                 const SpacePoint* spacePoint,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const = 0;
            /** @brief Calibrates a single space point. Mdt drift radii are corrected for time slew, signal
             *         propagation & LorentzAngle effects. The second coordinate of 1D space points is updated according
             *         to the closest approach of the strip to the complemntary coordinate. The space point calibrator
             *         always returns a calibrated object. But the state of the space point is set to FailedCalib in
             *         if the calibration fails. 
             *  @param ctx: EventContext to access conditions data
             *  @param spacePoint: Pointer to the space point to calibrate.
             *  @param seedPosInChamb: Position of the external seed expressed in the sector frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the sector frame
             *  @param timeDelay: Shift in time to be added to the time of flight of a particle going a straight path */
            virtual CalibSpacePointPtr calibrate(const EventContext& ctx,
                                                 const CalibratedSpacePoint& spacePoint,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const = 0;
            /** @brief Calibrates a set of space points.
             *  @param ctx: EventContext to access conditions data
             *  @param spacePoints: List of space points that will be calibrated
             *  @param seedPosInChamb: Position of the external seed expressed in the sector frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the sector frame
             *  @param timeDelay: Shift in time to be added to the time of flight of a particle going a straight path */
            virtual CalibSpacePointVec calibrate(const EventContext& ctx,
                                                 const std::vector<const SpacePoint*>& spacePoints,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeDelay) const = 0;
    
            /** @brief Refines the calibration constants of already calibrated space points
             *  @param cctx: Calibration context which is a packed pointer to the current ATLAS EventContext
             *  @param seedPosInChamb: Position of the external seed expressed in the sector frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the sector frame
             *  @param timeDelay: Shift in time to be added to the time of flight of a particle 
             *                    going a straight path 
             *  @param spacePoints: List of already calibrated space points that's eaten by the method */
          
             virtual CalibSpacePointVec calibrate(const Acts::CalibrationContext& cctx,                                            
                                                  const Amg::Vector3D& seedPosInChamb,
                                                  const Amg::Vector3D& seedDirInChamb,
                                                  const double timeDelay,
                                                  const CalibSpacePointVec& spacePoints) const = 0;

            /** @brief Returns the drift velocity for a given drift-circle space point
             *  @param ctx: Calibration context which is a packed pointer to the current ATLAS EventContext
             *  @param spacePoint: Reference to the calibrated space point for which the velocity needs to be calculated. */
            virtual double driftVelocity(const Acts::CalibrationContext& cctx,
                                         const CalibratedSpacePoint& spacePoint) const = 0;
            /** @brief Returns the drift acceleration for a given drift-circle space point
             *  @param ctx: Calibration context which is a packed pointer to the current ATLAS EventContext
             *  @param spacePoint: Reference to the calibrated space point for which the acceleration needs to be calculated. */
            virtual double driftAcceleration(const Acts::CalibrationContext& cctx,
                                             const CalibratedSpacePoint& spacePoint) const = 0;
            /** @brief Function that's hooked to the calibration delegate of the implemented Acts fitters
              *  @param geoctx: The geometry context to fetch the local -> global transformations for the surfaces
              *  @param cctx: Calibration context which is a packed pointer to the current ATLAS EventContext
              *  @param link: Sourcelink to the actual measurement to calibrate
              *  @param state: Proxy to the track-state to which the calibrated constants are written */
            virtual void calibrateSourceLink(const Acts::GeometryContext& geoctx,
                                             const Acts::CalibrationContext& cctx,
                                             const Acts::SourceLink& link,
                                             ActsTrk::MutableTrackStateBackend::TrackStateProxy state) const = 0;
            /** @brief Update the signs of the drift radii for a given straight line track
             *         to fix the left <-> right ambiguity
             *  @param trackPos: Position of the track intercept in the sector frame
             *  @param trackDir: Direction of the track in the sector frame
             *  @param hitsToCalib: List of space points to calibrate */
            virtual void updateSigns(const Amg::Vector3D& trackPos,
                                     const Amg::Vector3D& trackDir,
                                     CalibSpacePointVec& hitsToCalib) const = 0;
            
            /** @brief Stamps the signs of the drift radii w.r.t. the segment line
             *         onto the uncalibrated measurements. The stamped signs are later
             *         picked up by the source link calibration in the context of the 
             *         track fit to stabilize the fit.
             * @param segment: Reference to the reconstructed segment for which the sign
             *                 stamp shall be executed */
            virtual void stampSignsOnMeasurements(const xAOD::MuonSegment& segment) const = 0;
    };

}


#endif
