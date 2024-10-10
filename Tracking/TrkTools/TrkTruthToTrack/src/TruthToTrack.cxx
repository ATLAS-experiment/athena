/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#define TRUTHTOTRACK_IMP
#include "TrkTruthToTrack/TruthToTrack.h"

#include <cmath>
#include <memory>

#include "AtlasHepMC/GenParticle.h"
#include "AtlasHepMC/GenVertex.h"
#include "AtlasHepMC/SimpleVector.h"
#include "TruthUtils/HepMCHelpers.h"

#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthVertex.h"

#include "TrkExInterfaces/IExtrapolator.h"


//================================================================
Trk::TruthToTrack::TruthToTrack(const std::string& type, const std::string& name, const IInterface* parent)
  : ::AthAlgTool(type,name,parent)
  , m_extrapolator("Trk::Extrapolator/AtlasExtrapolator")
{
  declareInterface<ITruthToTrack>(this);
  declareProperty("Extrapolator", m_extrapolator);
}

//================================================================
StatusCode Trk::TruthToTrack::initialize() {
  ATH_CHECK( m_extrapolator.retrieve() );
  return StatusCode::SUCCESS;
}



//================================================================
const Trk::TrackParameters* Trk::TruthToTrack::makeProdVertexParameters(HepMC::ConstGenParticlePtr part) const {
  Trk::TrackParameters *result = nullptr;

  if(part && part->production_vertex()) {
    HepMC::FourVector tv = part->production_vertex()->position();
    Amg::Vector3D hv(tv.x(),tv.y(),tv.z());
    const Amg::Vector3D& globalPos = hv;

    const HepMC::FourVector& fv = part->momentum();
    Amg::Vector3D hv2(fv.px(),fv.py(),fv.pz());
    const Amg::Vector3D& globalMom = hv2;

    const int id = part->pdg_id();
    if (id) {
      const double charge = MC::charge(id);
      Amg::Translation3D tmpTransl(hv);
      Amg::Transform3D tmpTransf = tmpTransl * Amg::RotationMatrix3D::Identity();
      const Trk::PlaneSurface surface(tmpTransf);
      result = new Trk::AtaPlane(globalPos, globalMom, charge, surface);
    }
    else {
      ATH_MSG_WARNING("Could not get particle data for particle ID="<<id);
    }
  }

  return result;
}


//================================================================
const Trk::TrackParameters* Trk::TruthToTrack::makeProdVertexParameters(const xAOD::TruthParticle* part) const {
  Trk::TrackParameters *result = nullptr;

  if(part && part->hasProdVtx()) {
    Amg::Vector3D hv(part->prodVtx()->x(),part->prodVtx()->y(),part->prodVtx()->z());
    const Amg::Vector3D& globalPos = hv;

    Amg::Vector3D hv2(part->p4().Px(),part->p4().Py(),part->p4().Pz());
    const Amg::Vector3D& globalMom = hv2;

    const int id = part->pdg_id();
    if (id) {
      const double charge = MC::charge(id);
      Amg::Translation3D tmpTransl(hv);
      Amg::Transform3D tmpTransf = tmpTransl * Amg::RotationMatrix3D::Identity();
      const Trk::PlaneSurface surface(tmpTransf);
      result = new Trk::AtaPlane(globalPos, globalMom, charge, surface);
    }
    else {
      ATH_MSG_WARNING("Could not get particle data for particle ID="<<id);
    }
  }

  return result;
}



//================================================================
const Trk::TrackParameters* Trk::TruthToTrack::makePerigeeParameters(HepMC::ConstGenParticlePtr part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->production_vertex() && m_extrapolator) {

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(part) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee = m_extrapolator->extrapolateDirectly(
        Gaudi::Hive::currentContext(),
        *productionVertexTrackParams,
        Trk::PerigeeSurface(),
        Trk::anyDirection,
        false,
        Trk::nonInteracting ).release();
    }
  }

  return generatedTrackPerigee;
}



//================================================================
const Trk::TrackParameters* Trk::TruthToTrack::makePerigeeParameters(const xAOD::TruthParticle* part) const {
  const Trk::TrackParameters* generatedTrackPerigee = nullptr;

  if(part && part->hasProdVtx() && m_extrapolator) {

    std::unique_ptr<const Trk::TrackParameters> productionVertexTrackParams( makeProdVertexParameters(part) );
    if(productionVertexTrackParams) {

      // Extrapolate the TrackParameters object to the perigee. Direct extrapolation,
      // no material effects.
      generatedTrackPerigee = m_extrapolator->extrapolateDirectly(
        Gaudi::Hive::currentContext(),
        *productionVertexTrackParams,
        Trk::PerigeeSurface(),
        Trk::anyDirection,
        false,
        Trk::nonInteracting ).release();
    }
  }

  return generatedTrackPerigee;
}
