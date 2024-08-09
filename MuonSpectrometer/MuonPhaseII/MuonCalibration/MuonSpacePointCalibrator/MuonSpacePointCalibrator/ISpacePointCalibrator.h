/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONSPACEPOINTCALIBRATOR_ISPACEPOINTCALIBRATOR_H
#define MUONSPACEPOINTCALIBRATOR_ISPACEPOINTCALIBRATOR_H

#include <GaudiKernel/IAlgTool.h>
#include <GeoPrimitives/GeoPrimitives.h>
#include <memory>

class EventContext;

namespace MuonR4{
    class SpacePoint;
    class CalibratedSpacePoint;
    /** @brief Interface class to refine the space point calibration with an external seed */
    class ISpacePointCalibrator : virtual public IAlgTool{
        public:
            DeclareInterfaceID(ISpacePointCalibrator, 1, 0);
            
            virtual ~ISpacePointCalibrator() = default;

            using CalibSpacePointPtr = std::unique_ptr<CalibratedSpacePoint>;
            using CalibSpacePointVec = std::vector<CalibSpacePointPtr>;
            /** @brief Calibrates a single space point. Mdt drift radii are corrected for time slew, signal
             *         propagation & LorentzAngle effects. The second coordinate of 1D space points is updated according
             *         to the closest approach of the strip to the complemntary coordinate. If calibration fails,
             *         a nullptr is returned.
             *  @param ctx: EventContext to access conditions data
             *  @param spacePoint: Pointer to the space point to calibrate.
             *  @param seedPosInChamb: Position of the external seed expressed in the chamber frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the chamber frame
             *  @param timeOfArrival: Global time of flight to reach the closest point of the space point.
             */
            virtual CalibSpacePointPtr calibrate(const EventContext& ctx,
                                                 const SpacePoint* spacePoint,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeOfArrival) const = 0;
            
            /** @brief Calibrates a set of space points. Failed calibration attempts are removed from the list
             *  @param ctx: EventContext to access conditions data
             *  @param spacePoint: Pointer to the space point to calibrate.
             *  @param seedPosInChamb: Position of the external seed expressed in the chamber frame
             *  @param seedDirInChamb: Direction of the external seed expressed in the chamber frame
             *  @param timeOfArrival: Global time of arrival at the seed position
             */
            virtual CalibSpacePointVec calibrate(const EventContext& ctx,
                                                 const std::vector<const SpacePoint*>& spacePoints,
                                                 const Amg::Vector3D& seedPosInChamb,
                                                 const Amg::Vector3D& seedDirInChamb,
                                                 const double timeOfArrival) const = 0;
    
    
    };

}


#endif