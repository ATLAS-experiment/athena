/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "TrkTruthToTrack/TruthTrackRecordToTrack.h"

#include <cmath>
#include <memory>

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

#include "TrkExInterfaces/IExtrapolator.h"
#include "TrackRecord/TrackRecord.h"
#include "TruthUtils/MagicNumbers.h"
#include "TruthUtils/HepMCHelpers.h"


//================================================================
Trk::TruthTrackRecordToTrack::TruthTrackRecordToTrack(const std::string& type, const std::string& name,
                                            const IInterface* parent)
  : AthAlgTool(type,name,parent),
    m_extrapolator("Trk::Extrapolator/AtlasExtrapolator")
{
  declareInterface<ITruthToTrack>(this);

  declareProperty("Extrapolator", m_extrapolator);
  declareProperty("TrackRecordKey",m_reccollkey="CosmicPerigee");
}

//================================================================
StatusCode Trk::TruthTrackRecordToTrack::initialize() {

  ATH_CHECK ( m_extrapolator.retrieve() );

  ATH_CHECK( m_reccollkey.initialize() );

  return StatusCode::SUCCESS;
}

//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makeProdVertexParameters(HepMC::ConstGenParticlePtr part) const {

  if (part == nullptr) return nullptr;

  Trk::TrackParameters *result = nullptr;
  Amg::Vector3D prodVertexVector;
  prodVertexVector.setZero();
  Amg::Vector3D globalPos;
  Amg::Vector3D globalMom;
  int id=0;

  SG::ReadHandle<TrackRecordCollection> trackRecordCollection(m_reccollkey);

  if (trackRecordCollection.isValid()) {
    ATH_MSG_ERROR ("Could not get track record!");
    return nullptr;
  }
  ATH_MSG_DEBUG("reading from track record, size=" << trackRecordCollection->size());

  if (trackRecordCollection->empty()) ATH_MSG_WARNING ("action required but TrackRecordCollection size is 0");

  for (const auto & trackRecord : *trackRecordCollection) {

    if ( !HepMC::is_same_particle(trackRecord,part) ) continue;

    id = trackRecord.GetPDGCode();

    CLHEP::Hep3Vector tv = trackRecord.GetPosition();
    prodVertexVector = Amg::Vector3D(tv.x(),tv.y(),tv.z());
    globalPos = prodVertexVector;

    Amg::Vector3D hv2(trackRecord.GetMomentum().x(), trackRecord.GetMomentum().y(),
                      trackRecord.GetMomentum().z());
    globalMom = hv2;

    ATH_MSG_DEBUG("Found particle " << part << ", momentum " << hv2 << " production " << globalPos);


  }   // loop over G4 records

  if (id) {
    const double charge = MC::charge(id);

    Amg::Translation3D prodSurfaceCentre( prodVertexVector.x(),
                                          prodVertexVector.y(),
                                          prodVertexVector.z() );

    Amg::Transform3D tmpTransf =  prodSurfaceCentre *  Amg::RotationMatrix3D::Identity();

    Trk::PlaneSurface planeSurface(tmpTransf, 5., 5. );
    result = new Trk::AtaPlane(globalPos, globalMom, charge, planeSurface);
  } else {
    ATH_MSG_WARNING ("Could not get particle data for particle ID="<<id);
  }
  return result;
}



//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makeProdVertexParameters(const xAOD::TruthParticle* part) const {

  if (part == nullptr) return nullptr;

  Trk::TrackParameters *result{};
  Amg::Vector3D prodVertexVector(0., 0., 0.);
  Amg::Vector3D globalPos(0., 0., 0.);
  Amg::Vector3D globalMom(0., 0., 0.);
  int id=0;
  double charge = 0.0;

  SG::ReadHandle<TrackRecordCollection> trackRecordCollection(m_reccollkey);

  if (trackRecordCollection.isValid()) {
    ATH_MSG_ERROR ("Could not get track record!");
    return nullptr;
  }

  ATH_MSG_DEBUG("reading from track record, size=" << trackRecordCollection->size());

  if (trackRecordCollection->empty()) ATH_MSG_WARNING ("action required but TrackRecordCollection size is 0");

  for (const auto & trackRecord : *trackRecordCollection) {
    if ( HepMC::is_same_particle(trackRecord,part) ) {
      id = trackRecord.GetPDGCode();
      if (!id) continue;
      CLHEP::Hep3Vector  tv = trackRecord.GetPosition();
      prodVertexVector = Amg::Vector3D(tv.x(),tv.y(),tv.z());
      globalPos = prodVertexVector;
      Amg::Vector3D hv2(trackRecord.GetMomentum().x(), trackRecord.GetMomentum().y(), trackRecord.GetMomentum().z());
      globalMom = hv2;
      ATH_MSG_DEBUG("found particle " << part << ", momentum " << hv2 << " production " << globalPos);
    }
  }   // loop over G4 records

  if (id) {
    charge = MC::charge(id);

    Amg::Translation3D prodSurfaceCentre( prodVertexVector.x(),
                                          prodVertexVector.y(),
                                          prodVertexVector.z() );

    Amg::Transform3D tmpTransf =  prodSurfaceCentre *  Amg::RotationMatrix3D::Identity();

    Trk::PlaneSurface planeSurface(tmpTransf, 5., 5. );
    result = new Trk::AtaPlane(globalPos, globalMom, charge, planeSurface);

  } else {
    ATH_MSG_WARNING ("Could not get particle data for particle ID="<<id);
  }
  return result;
}



//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makePerigeeParameters(HepMC::ConstGenParticlePtr part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->production_vertex() && m_extrapolator) {

    MsgStream log(msgSvc(), name());

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(std::move(part)) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee =
        m_extrapolator->extrapolateDirectly(Gaudi::Hive::currentContext(),
                                            *productionVertexTrackParams,
                                            Trk::PerigeeSurface(),
                                            Trk::anyDirection,
                                            false,
                                            Trk::nonInteracting).release();
    }
  }

  return generatedTrackPerigee;
}

//================================================================
const Trk::TrackParameters* Trk::TruthTrackRecordToTrack::makePerigeeParameters(const xAOD::TruthParticle* part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->hasProdVtx() && m_extrapolator) {

    MsgStream log(msgSvc(), name());

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(part) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee =
        m_extrapolator->extrapolateDirectly(Gaudi::Hive::currentContext(),
                                            *productionVertexTrackParams,
                                            Trk::PerigeeSurface(),
                                            Trk::anyDirection,
                                            false,
                                            Trk::nonInteracting).release();
    }
  }

  return generatedTrackPerigee;
}
//================================================================
