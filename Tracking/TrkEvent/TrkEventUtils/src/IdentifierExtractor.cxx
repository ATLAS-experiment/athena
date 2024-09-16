/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// IdentifierExtractor.cxx,         (c) ATLAS Detector software  //
///////////////////////////////////////////////////////////////////

#include "TrkEventUtils/IdentifierExtractor.h"
#include "TrkCompetingRIOsOnTrack/CompetingRIOsOnTrack.h"
#include "TrkMeasurementBase/MeasurementBase.h"

void Trk::IdentifierExtractor::extract(std::vector<Identifier>& ids, const std::vector<const Trk::MeasurementBase*>& measurements)
{
  using namespace std;
  vector<const Trk::MeasurementBase*>::const_iterator it    = measurements.begin();
  vector<const Trk::MeasurementBase*>::const_iterator itEnd = measurements.end();
  ids.clear();
  for (; it!=itEnd ; ++it)
  {
    Identifier id = extract(*it);
    if( id.is_valid() ) { ids.push_back(id);}
  }
}

Identifier Trk::IdentifierExtractor::extract(const Trk::MeasurementBase* m)
{
  Identifier id;
  if (m->type(Trk::MeasurementBaseType::RIO_OnTrack)) {
    const Trk::RIO_OnTrack* rot = static_cast<const Trk::RIO_OnTrack*>(m);
    id = rot->identify();
  } else if (m->type(Trk::MeasurementBaseType::CompetingRIOsOnTrack)) {
    const Trk::CompetingRIOsOnTrack* comprot =
        static_cast<const Trk::CompetingRIOsOnTrack*>(m);
    id = comprot->rioOnTrack(comprot->indexOfMaxAssignProb()).identify();
  } else {
    id.clear();
  }
  return id;
}

