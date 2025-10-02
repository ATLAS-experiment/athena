/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_IActsToTrkConverterTool_H
#define ACTSGEOMETRYINTERFACES_IActsToTrkConverterTool_H

// ATHENA
#include <memory>

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/IInterface.h"
#include "StoreGate/WriteHandle.h"
#include "TrkParameters/TrackParameters.h"  //typedef, cannot fwd declare
#include "TrkTrack/TrackCollection.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"  //typedef, cannot fwd declare
#include "xAODTracking/TrackJacobianContainer.h"
#include "xAODTracking/TrackMeasurementContainer.h"
#include "xAODTracking/TrackParametersContainer.h"
#include "xAODTracking/TrackStateContainer.h"
#include "Acts/EventData/TrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsEvent/TrackContainer.h"

namespace Trk {
  class Surface;
  class Track;
  class MeasurementBase;
  class PrepRawData;
}  // namespace Trk

namespace Acts {
class Surface;
class SourceLink;
}

namespace ActsTrk::detail {
    enum class SourceLinkType;
}


namespace ActsTrk {
/** @brief Conversion tool interface to translate surfaces & track parameters between the
  *        Acts & Trk realm */
class IActsToTrkConverterTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(IActsToTrkConverterTool, 1, 0);
  /** @brief Translates the parsed Acts surface into a Trk::Surface via associated detector element.
   *         For the ID measurements a direct link is provided and for the muon measurements the look-up
   *         is performed via the associated Identifier and the detector manager. Exception is thrown
   *         if the mapping fails.
   *  @param actsSurface: Refrence to the acts surface to translate */
  virtual const Trk::Surface& actsSurfaceToTrkSurface(const Acts::Surface& actsSurface) const = 0;
  /** @brief Translate the parsed Trk surface into an Acts surface. The detector element identifier
   *         of the surface needs to be filled into the internal tool's look-up map. Otherwise an exception
   *         is thrown.
   * @param atlasSurface: Refrence to the Trk surface to translate */
  virtual const Acts::Surface& trkSurfaceToActsSurface(const Trk::Surface& atlasSurface) const = 0;

  /** @brief Converts the Trk measurement track states into a vector of Acts::Source links. The 
   *         source links don't take ownership over the measurement states.
   *  @param track: Reference to the track to convert */ 
  virtual std::vector<Acts::SourceLink> trkTrackToSourceLinks(const Trk::Track& track) const = 0;
  /** @brief Converts a vector of Trk measurement states into a vector of Acts source links and appends the
   *         result to the exisiting set of source links
   *  @param measSet: Reference to the measurement set to transform
   *  @param link: Target vector to which the new source links are appended */
  virtual void toSourceLinks(const std::vector<const Trk::MeasurementBase*>& measSet,
                             std::vector<Acts::SourceLink>& links) const = 0;
   /** @brief Converts a vector of Trk::PrepRawData states into a vector of Acts source links and appends the
   *         result to the exisiting set of source links
   *  @param measSet: Reference to the measurement set to transform
   *  @param link: Target vector to which the new source links are appended */
  virtual void toSourceLinks(const std::vector<const Trk::PrepRawData*>& prdSet,
                             std::vector<Acts::SourceLink>& links) const = 0;                           
  /** @brief Translates the Trk track parameters into bound Acts track parameters with a particle hypothesis.
   *  @param atlasParameter: The Trk parameters to translate.
    * @param gcts: Geometry context needed for special treatment of the annulus bounds
    * @param hypothesis: Track hypothesis to use */
  virtual const Acts::BoundTrackParameters trkTrackParametersToActsParameters(
                                            const Trk::TrackParameters& atlasParameter, 
                                            const Acts::GeometryContext& gctx, 
                                            Trk::ParticleHypothesis hypothesis = Trk::pion) const = 0;

  /** @brief Translates the bounded Acts track parameters to Trk parameters. The bound parameter surface
   *         must be translatble by the tool
   *  @param actsParameter: Refrence to the bounded parameters to translate
   *  @param gctx: Geometry context to align the associated surface in global space */
  virtual std::unique_ptr<Trk::TrackParameters> 
    actsTrackParametersToTrkParameters(const Acts::BoundTrackParameters& actsParameter,
                                       const Acts::GeometryContext& gctx) const = 0;
  /** @brief Convert the Acts fit result into a Trk::Track object, if the fit was successful. Otherwise,
   *         a nullptr is returned.
   * @param ctx: EventContext to construct the Geometry & calibration context inside
   * @param tracks: Reference to the Multi trajectory cotnainer
   * @param fitResult: Outcome from the Acts fitter
   * @param fitAuthor: Author flag to be put into the Trk::Track meta data
   * @param slType: Source link type steering how the uncalibrated Acts::SourceLinks are 
   *                turned into Trk::MeasurementBase objects */
  using TrackFitResult_t = Acts::Result<ActsTrk::MutableTrackContainer::TrackProxy, std::error_code>;
  virtual std::unique_ptr<Trk::Track> convertFitResult(const EventContext& ctx,
                                                       ActsTrk::MutableTrackContainer& tracks,
                                                       TrackFitResult_t& fitResult,
                                                       const Trk::TrackInfo::TrackFitter fitAuthor,
                                                       const detail::SourceLinkType slType) const = 0;

  virtual void trkTrackCollectionToActsTrackContainer(
      ActsTrk::MutableTrackContainer &tc,
      const TrackCollection& trackColl,
      const Acts::GeometryContext& gctx) const = 0;

};
}  // namespace ActsTrk

#endif
