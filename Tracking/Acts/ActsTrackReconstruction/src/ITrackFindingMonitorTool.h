#ifndef ACTSTRK_ITRACKFINDINGMONITORTOOL_H
#define ACTSTRK_ITRACKFINDINGMONITORTOOL_H

#include "GaudiKernel/IAlgTool.h"
#include "GaudiKernel/EventContext.h"
#include "ActsEvent/TrackContainer.h"
#include "ActsEvent/SeedContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"
#include "ActsGeometryInterfaces/GeometryContext.h"
#include "src/detail/MeasurementIndex.h"
#include "src/detail/Definitions.h"

namespace ActsTrk {
class ITrackFindingMonitorTool : virtual public IAlgTool
{
  public:
    DeclareInterfaceID(ITrackFindingMonitorTool, 1, 0);

    virtual void newEvent(const EventContext &ctx,
                          const Acts::GeometryContext &tgContext) const = 0;

    virtual void measurements(const EventContext &ctx,
                              const std::vector<const xAOD::UncalibratedMeasurementContainer *> &clusterContainers,
                              const std::vector<size_t> &offsets) const = 0;

    virtual void
    newSeed(const Acts::GeometryContext &tgContext,
            const ActsTrk::Seed &seed,
            const Acts::BoundTrackParameters &initialParameters,
            const detail::MeasurementIndex &measurementIndexer,
            unsigned int iseed,
            bool isKF,
            const char *seedType,
            bool first_seed) const = 0;

    using TrackContainer_t = detail::RecoTrackContainer;
    enum ETrackStatus {kTrackInProgress=0,
                      kTrackIsFinal=1,
                      kTrackFailedPtCut=2,
                      kTrackFailedEtaCut=4,
                      kTrackFailedHoleCut=8,
                      kTrackFailedHitCut=16,
                      kTrackFailedOutlierCut=32,
                      kTrackFailedTrackSelection=64};
    static constexpr unsigned int gDroppedMask = ( kTrackFailedPtCut|kTrackFailedEtaCut|kTrackFailedHoleCut
                                                  |kTrackFailedHitCut|kTrackFailedOutlierCut|kTrackFailedTrackSelection);
    virtual void
    newTrack(const Acts::GeometryContext &tgContext,
             const TrackContainer_t &tracks,
             const typename TrackContainer_t::TrackProxy &track,
             const detail::MeasurementIndex &measurementIndexer,
             bool rejected = false) const = 0;

    using TrackStateProxy_t = detail::RecoTrackContainer::TrackStateProxy;
    virtual bool
    newTrackState(const Acts::GeometryContext &tgContext,
                  const TrackContainer_t &track_container,
                  const typename TrackContainer_t::TrackProxy &track,
                  const TrackStateProxy_t &state,
                  const detail::MeasurementIndex &measurementIndexer,
                  unsigned int status,
                  bool useFiltered = false,
                  bool newLine = true) const = 0;

   virtual void finalizeEvent(const EventContext &ctx) const = 0;
};
}
#endif
