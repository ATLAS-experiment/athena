/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// TrackFitInputPreparator, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#include "TrkFitterUtils/TrackFitInputPreparator.h"
#include "TrkEventUtils/PrepRawDataComparisonFunction.h"
#include "TrkMeasurementBase/MeasurementBase.h"
#include "TrkRIO_OnTrack/RIO_OnTrack.h"
#include "TrkTrack/Track.h"


// give back the Measurements stripped of a track+measurement input combination.
//
Trk::MeasurementSet
Trk::TrackFitInputPreparator::stripMeasurements(
  const Trk::Track& inputTrk,
  const Trk::MeasurementSet& inputMbs)
{
  MeasurementSet newMbSet;
  // collect MBs from Track (speed: assume use for extending track at end)
  DataVector<const MeasurementBase>::const_iterator it =
    inputTrk.measurementsOnTrack()->begin();
  for (; it != inputTrk.measurementsOnTrack()->end(); ++it)
    if ((*it))
      newMbSet.push_back(*it);
  // add MBs from input list
  MeasurementSet::const_iterator itSet = inputMbs.begin();
  for (; itSet != inputMbs.end(); ++itSet)
    if ((*itSet))
      newMbSet.push_back(*itSet);
  return newMbSet;
}

// Create a vector of PrepRawData* from the mixed input of track and single
// PRDs.
//
Trk::PrepRawDataSet
Trk::TrackFitInputPreparator::stripPrepRawData(
  const Trk::Track& inputTrk,
  const Trk::PrepRawDataSet& inputPrds,
  const SortInputFlag doSorting,
  const bool reintegrateOutliers)
{
  // apped PRDs to end of track. For pre-pend make a parameter
  PrepRawDataSet newPrdSet;

  // collect PrepRawData pointers from input track ROTs
  TS_iterator it = inputTrk.trackStateOnSurfaces()->begin();
  for (; it != inputTrk.trackStateOnSurfaces()->end(); ++it) {
    if ((*it)->measurementOnTrack() &&
        (!((*it)->type(TrackStateOnSurface::Outlier)) || reintegrateOutliers)) {

      const Trk::RIO_OnTrack* rot = nullptr;
      if ((*it)->measurementOnTrack()->type(
              Trk::MeasurementBaseType::RIO_OnTrack)) {
        rot = static_cast<const Trk::RIO_OnTrack*>((*it)->measurementOnTrack());
      }
      if (rot) {
        const PrepRawData* prepRD = rot->prepRawData();
        if (prepRD) {
          newPrdSet.push_back(prepRD);
        }
      }
    }
  }
  Trk::PrepRawDataSet::const_iterator itSet = inputPrds.begin();
  for (; itSet != inputPrds.end(); ++itSet)
    if ((*itSet))
      newPrdSet.push_back(*itSet);

  if (doSorting) {
    const Trk::PrepRawDataComparisonFunction PRD_CompFunc(
      (*inputTrk.trackParameters()->begin())->position(),
      (*inputTrk.trackParameters()->begin())->momentum());

    if (!std::is_sorted(newPrdSet.begin(), newPrdSet.end(), PRD_CompFunc))
      std::sort(newPrdSet.begin(), newPrdSet.end(), PRD_CompFunc);
  }
  return newPrdSet;
}

