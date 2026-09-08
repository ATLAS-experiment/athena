/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSGEOMETRYINTERFACES_ITrackConverterTool_H
#define ACTSGEOMETRYINTERFACES_ITrackConverterTool_H

// ATHENA
#include <memory>
#include <variant>

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
#include "Acts/EventData/BoundTrackParameters.hpp"
#include "Acts/EventData/VectorTrackContainer.hpp"
#include "ActsEvent/TrackContainer.h"

namespace Trk {
  class Track;
  class MeasurementBase;
  class PrepRawData;
}  // namespace Trk

namespace Acts {
class Surface;
class SourceLink;
}


namespace ActsTrk {
/** @brief ConversionTool between the Acts Track EDM and the Trk::Track EDM. The tool converts entire TrackCollections 
  *        (vector of Trk::Tracks) into an Acts::TrackContainers and vice versa. The conversion of single Acts::Tracks
  *        into a Trk::Track object is also possible. To prepare the refits from Trk::Tracks the tool also converts
  *        all meaurement states from a Trk::Track into Acts::SourceLinks. */
class ITrackConverterTool : virtual public IAlgTool {
 public:
  DeclareInterfaceID(ITrackConverterTool, 1, 0);
  /** @brief Abrivate the Track Proxy */
  using ConstTrack_t = TrackContainer::ConstTrackProxy;
  using Track_t = MutableTrackContainer::TrackProxy;
  using TrackFitResult_t = Acts::Result<Track_t, std::error_code>;
  /** @brief Converts the Trk measurement track states into a vector of Acts::Source links. The 
   *         source links don't take ownership over the measurement states.
   *  @param track: Reference to the track to convert */ 
  virtual std::vector<Acts::SourceLink> trkTrackToSourceLinks(const Trk::Track& track) const = 0;
  /** @brief Convert the Acts fit result into a Trk::Track object, if the fit was successful. Otherwise,
   *         a nullptr is returned.
   * @param ctx: EventContext to construct the Geometry & calibration context inside
   * @param fitResult: Outcome from the Acts fitter
   * @param fitAuthor: Author flag to be put into the Trk::Track meta data */
  using ActsTrack_t = ActsTrk::MutableTrackContainer::TrackProxy;
  virtual std::unique_ptr<Trk::Track> convertActsToTrk(const EventContext& ctx,
                                                       const ActsTrack_t& actsTrack,
                                                       const Trk::TrackInfo::TrackFitter fitAuthor) const = 0;
  /** @brief Converts a const Acts::Track into
    * @param ctx: EventContext to access the current conditions (alignment, calibrations, etc.)
    * @param trackProxy: The acts track for conversion */
  virtual std::unique_ptr<Trk::Track> convertTrack(const EventContext& ctx, 
                                                   const ConstTrack_t& trackProxy) const = 0;
  /** @brief Convert the `Trk::Track` in the passed TrackCollection into Acts tracks and appends the
   *         result to the passed mutable track container
   *  @param ctx: EventContext to access the current conditions (alignment, calibrations, etc.)
   *  @param trackColl: The Trk::Track container to be converted
   *  @param outTrackColl: Reference to the mutable track container to which the converted
   *                       tracks are appended. */
  virtual void convertTrkToActsContainer(const EventContext& ctx,
                                         const TrackCollection& trackColl,
                                         ActsTrk::MutableTrackContainer& outTrackColl) const = 0;

  /** @brief Converts the Acts track container to a Trk::Track collection
   *  @param ctx: EventContext to access the current conditions (alignment, calibration, etc.)
   *  @param trackCont: Reference to the track container for legacy converstion */
  virtual std::unique_ptr<TrackCollection> 
      convertActsToTrkContainer(const EventContext& ctx,
                                const ActsTrk::TrackContainer& trackCont) const = 0;

};
}  // namespace ActsTrk

#endif
