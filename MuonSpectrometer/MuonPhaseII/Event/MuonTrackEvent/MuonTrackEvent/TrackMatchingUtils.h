/*
   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONCOMBINEDALGSR4_TRACKMATCHINGUTILS_H
#define MUONCOMBINEDALGSR4_TRACKMATCHINGUTILS_H

#include "GeoPrimitives/GeoPrimitives.h"
///
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Geometry/GeometryContext.hpp"
#include "Acts/Utilities/Logger.hpp"
#include "Acts/Definitions/Units.hpp"


namespace MuonCombinedR4 {

    
    /** @brief Calculates the difference parameters between the ID and the MS
     *         If the parameters are expressed on different surfaces and the
     *         MS parameters are close to the boundary, then the MS parameters
     *         are linearliy extrapolated to the ID surface. If extrapolation
     *         fails or the boundary conditions are not fullfilled a std::nullopt
     *         is returned.
     * @param tgContext: The geometry context to propagated the MS track parameters
     *                   to the ID surface if needed
     * @param idParameters: The parameters from the Inner Detector to match
     * @param msParameters: The parameters from the MuonSpectrometer to match
     * @param logger: The logger instance for printing the debug message
     * @param boundTolerance: The maximum distance that the MS parameters may
     *                        be apart from the surface boundary to be extrapolated */
    std::optional<Acts::BoundTrackParameters>
        makeDiffParameters(const Acts::GeometryContext& tgContext,
                           const Acts::BoundTrackParameters& idParameters,
                           const Acts::BoundTrackParameters& msParameters,
                           const Acts::Logger& logger,
                           const double boundTolerance = Acts::UnitConstants::km);
    /** @brief Returns the longitudinal local track parameter which is defined as
      *           - radial parameter in case of discs
      *           - displacement along the cylinder axis for cylinders
      * @param pars: Reference to the parameters of interest
      * @param logger: The logger instance for printing the debug message */
    double longitudinalParam(const Acts::BoundTrackParameters& pars,
                             const Acts::Logger& logger);
    /** @brief Returns the local angular polar angle of the track parameter
      * @param pars: Reference to the parameters of interest
      * @param logger: The logger instance for printing the debug message */
    double localPolarAngle(const Acts::BoundTrackParameters& pars,
                           const Acts::Logger& logger);

}

#endif