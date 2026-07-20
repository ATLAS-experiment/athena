/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
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
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/Definitions/Units.hpp"

#include <variant>

using namespace Acts::UnitLiterals;

namespace ActsTrk {
  /** @brief Tool to extrapolate bound track parameters through the Acts::TrackingGeometry.
   *         Extrapolation can be either done towards a target surface, 
    */
  class IExtrapolationTool : virtual public IAlgTool {
    public:
      DeclareInterfaceID(ActsTrk::IExtrapolationTool, 1, 0);
      /** @brief Abrivation of the recorded material  */
      using RecordedMaterial = Acts::MaterialInteractor::result_type;
      /** @brief Abrivation of the recorded steps and the allocated material. */
      using PropagationOutput = std::pair<std::vector<Acts::detail::Step>, 
                                          RecordedMaterial>;
      /** @brief Abrivation of the recorded surfaces along the propagation */
      using BoundParamVec_t = std::vector<Acts::BoundTrackParameters>;
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
                        double pathLimit = 1._km) const = 0;
      
      /** @brief Configuration struct to steer the propagation with surface record. 
       *         Bound track parameters are created at each intersected surface and
       *         recorded. The configuration expects a propagation target which is 
       *         either a volume or a surface. Further, it can be toggled whether
       *         the crossing at sensitive, material or portal surfaces shall be
       *         recorded */
      struct SurfaceRecordOptions {
          /** @brief Enumeration to define at which stage the propagation shall
           *         be terminated  */
          enum class VolumeAbort: std::uint8_t{
              atEntrance, //Propagation stops at entrance of the target volume
              atExit, // Propagation stops as soon as the mother volume is entered
          };
          /** @brief Record option constructor with a target volume as input
           *  @param targetVol: Pointer to the target volume until which the extrapolator
           *                    shall propagate
           * @param abordCond: Flag togling whether the propagation will be aborted if the
           *                   target volume is exited or if it is entered.*/
          explicit SurfaceRecordOptions(const Acts::TrackingVolume* targetVol,
                                        VolumeAbort abordCond = VolumeAbort::atExit):
                    target{targetVol}, stopVolumeFlag{abordCond} {}
 
          explicit SurfaceRecordOptions(const Acts::Surface* targetSurf,
                                        double extraPath = 0.):
                    target{targetSurf}, extraPathLength{extraPath}{}
          /** @brief Specify the target volume until which the extrapolation
            *        should be persued. */
          std::variant<const Acts::TrackingVolume*,
                       const Acts::Surface*> target{};
          /** @brief Define the extra path length that the propagator may continue
           *         after the target surface has been reached */
          double extraPathLength{0.};
          /** @brief Maximum path limit before aborting the extrapolation */
          double pathLimit{1._km};
          /** @brief Flag toggling how the propagation should end if the
            *        volume is a target */
          VolumeAbort stopVolumeFlag{VolumeAbort::atExit};
          /** @brief Flag to toggle whether track parameters at sensitive
           *         surfaces shall be created if crossed */
          bool recordSensitive{true};
          /** @brief Flag to toggle whether track parameters at material
           *         surfaces shall be created if crossed */
          bool recordMaterial{false};
          /** @brief Flag to toggle whether track parameters at portal
           *          or passive surfaces shall be created if crossed */
          bool recordPassive{false};
      };
      
      /** @brief Propagate the track parameters forward throuht the detector and record the
        *        surface crossings along the trajectory
        * @param ctx: EventContext to access the alignment and the magnetic field
        * @param startParameters: The track parameters from which the propagation shall start
        * @param recordOpts: Record options specifying the target and also the flags at which
        *                    surface types the track parameters shall be recorded */
      virtual Acts::Result<BoundParamVec_t> propagateAndRecord(const EventContext& ctx,
                                                               const Acts::BoundTrackParameters& startParameters,
                                                               const SurfaceRecordOptions& recordOpts) const = 0;

 
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
                                                 double pathLimit = 1._km) const = 0;
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
                                                                  double pathLimit = 1._km) const = 0;
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
                                                                  double pathLimit = 1._km) const = 0;
  };

}

#endif
