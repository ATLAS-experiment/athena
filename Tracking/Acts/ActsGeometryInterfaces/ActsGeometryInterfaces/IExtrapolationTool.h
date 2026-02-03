/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ActsTrkIExtrapolationTool_H
#define ACTSGEOMETRYINTERFACES_ActsTrkIExtrapolationTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"

#include "ActsGeometryInterfaces/GeometryContext.h"
#include "Acts/MagneticField/MagneticFieldContext.hpp"

#include "Acts/Propagator/MaterialInteractor.hpp"
#include "Acts/Propagator/detail/SteppingLogger.hpp"
#include "Acts/EventData/TrackParameters.hpp"


namespace ActsTrk {
  /** @brief Tool to extrapolate bound track parameters.  */
  class IExtrapolationTool : virtual public IAlgTool {
    public:
      DeclareInterfaceID(ActsTrk::IExtrapolationTool, 1, 0);
      /** @brief Abrivation of the recorded material  */
      using RecordedMaterial = Acts::MaterialInteractor::result_type;
      /** @brief Abrivation of the recorded steps and the allocated material. */
      using PropagationOutput = std::pair<std::vector<Acts::detail::Step>, 
                                          RecordedMaterial>;
      /** @brief Extrapolate the track parameters until the end of the world and 
       *         record the performed steps & the allocated material
       *  @param ctx: EventContext to fetch the alignment & magnetic field
       *              from the conditions store
       *  @param startParameters: Reference to the bound track parameters encoding
       *         the start surface & the associated track parameters on the surface
       *  @param navDir: Run the propagation along (Foward) or opposite (Backward) to
       *         the track parameter's direction
       *  @param pathLimit: Maximum length of the propagated trajectory, if not aborted
       *         by the end of the world condition otherwise. */
      virtual Acts::Result<PropagationOutput> propagationSteps(const EventContext& ctx,
                       const Acts::BoundTrackParameters& startParameters,
                       Acts::Direction navDir = Acts::Direction::Forward(),
                       double pathLimit = std::numeric_limits<double>::max()) const = 0;
      /** @brief Extrapolate the track parameters from a start to a target surface and record
       *         the peformed steps & allocated parameters
       *  @param ctx: EventContext to fetch the alignment & magnetic field
       *              from the conditions store
       *  @param startParameters: Reference to the bound track parameters encoding
       *         the start surface & the associated track parameters on the surface
       *  @param target: Reference to the surface onto which the track shall be extrapolated
       *  @param navDir: Run the propagation along (Foward) or opposite (Backward) to
       *         the track parameter's direction 
       *  @param pathLimit: Maximum length of the propagated trajectory. The extrapolation is
       *         aborted if the limit is exceeded and the surface not yet reached. */
      virtual Acts::Result<PropagationOutput> propagationSteps(const EventContext& ctx,
                                                 const Acts::BoundTrackParameters& startParameters,
                                                 const Acts::Surface& target,
                                                 Acts::Direction navDir = Acts::Direction::Forward(),
                                                 double pathLimit = std::numeric_limits<double>::max()) const = 0;
      /** @brief Extrapolates the track parameters from a start to a target surface and returns the 
       *         extrapolated track parameters on that surface. If the extrapolation fails, a nullopt
       *         is returned 
       *  @param ctx: EventContext to fetch the alignment & magnetic field
       *              from the conditions store
       *  @param startParameters: Reference to the bound track parameters encoding
       *         the start surface & the associated track parameters on the surface
       *  @param target: Reference to the surface onto which the track shall be extrapolated
       *  @param navDir: Run the propagation along (Foward) or opposite (Backward) to
       *         the track parameter's direction 
       *  @param pathLimit: Maximum length of the propagated trajectory. The extrapolation is
       *         aborted if the limit is exceeded and the surface not yet reached. */
      virtual Acts::Result<Acts::BoundTrackParameters> propagate(const EventContext& ctx,
                                                                  const Acts::BoundTrackParameters& startParameters,
                                                                  const Acts::Surface& target,
                                                                  Acts::Direction navDir = Acts::Direction::Forward(),
                                                                  double pathLimit = std::numeric_limits<double>::max()) const = 0;
      /** @brief
       *  @param ctx: EventContext to fetch the alignment & magnetic field
       *              from the conditions store
       *  @param startParameters: Reference to the bound track parameters encoding
       *         the start surface & the associated track parameters on the surface
       *  @param navDir: Run the propagation along (Foward) or opposite (Backward) to
       *         the track parameter's direction 
       *  @param pathLimit: Maximum length of the propagated trajectory. The extrapolation is
       *         aborted if the limit is exceeded and no surface is not yet reached. */
      virtual Acts::Result<Acts::BoundTrackParameters> propagate(const EventContext& ctx,
                                                                  const Acts::BoundTrackParameters& startParameters,
                                                                  Acts::Direction navDir = Acts::Direction::Forward(),
                                                                  double pathLimit = std::numeric_limits<double>::max()) const = 0;
      /** @brief Retrieves the magnetic field conditions from the Conditions store & wraps them into
       *         a Magnetic field context
       *  @param ctx: Event context to fastly access the Conditions store */
      virtual Acts::MagneticFieldContext getMagneticFieldContext(const EventContext& ctx) const = 0;
  };

}

#endif
