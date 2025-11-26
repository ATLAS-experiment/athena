/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
// MuonExtrapolationTool.cxx
#include "MuonExtrapolationTool.h"
#include "xAODTruth/TruthParticleContainer.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrkSurfaces/DiscSurface.h"
#include "TrkSurfaces/CylinderSurface.h"
#include "TVector2.h"
#include <memory>

//**********************************************************************

namespace DerivationFramework {

MuonExtrapolationTool::MuonExtrapolationTool(const std::string &t, const std::string& n, const IInterface* p)
  :
    base_class(t, n, p)
{
}

//**********************************************************************


StatusCode MuonExtrapolationTool::initialize()
{
  ATH_CHECK(m_muonContainerName.initialize());
  ATH_CHECK(m_extrapolator.retrieve());
  return StatusCode::SUCCESS;
}


//**********************************************************************

bool MuonExtrapolationTool::extrapolateAndDecorateTrackParticle(const xAOD::TrackParticle* particle, float & eta, float & phi, const EventContext& ctx) const
{

  // decorators used to access or store the information
  static const SG::AuxElement::Decorator< char > Decorated ("DecoratedPivotEtaPhi");
  static const SG::AuxElement::Decorator< float > Eta ("EtaTriggerPivot");
  static const SG::AuxElement::Decorator< float > Phi ("PhiTriggerPivot");

  if (! Decorated.isAvailable(*particle) || !Decorated(*particle)){
    // in the athena release, we can run the extrapolation if needed
    const Trk::TrackParameters* pTag = extrapolateToTriggerPivotPlane(*particle, ctx);
      if(!pTag) {
        Decorated(*particle) = false;
        return false;
      }
      Eta(*particle) = pTag->position().eta();
      Phi(*particle) = pTag->position().phi();
      Decorated(*particle) = true;
      delete pTag;
    }
    // if we get here, the decoration was either already present or just added by us
    // so we can finally read the values
    eta = Eta(*particle);
    phi = Phi(*particle);
    return true;
}

//**********************************************************************

const xAOD::TrackParticle* MuonExtrapolationTool::getPreferredTrackParticle (const xAOD::IParticle* muon) const
{
  if (dynamic_cast<const xAOD::TruthParticle*>(muon)){
    ATH_MSG_WARNING("Pivot plane extrapolation not supported for Truth muons!");
    return 0;
  }
  const xAOD::TrackParticle* muonTrack = dynamic_cast<const xAOD::TrackParticle*>(muon);
  if(!muonTrack && dynamic_cast<const xAOD::Muon*>(muon)) {
    const xAOD::Muon* theMuon = dynamic_cast<const xAOD::Muon*>(muon);
    muonTrack = theMuon->trackParticle( xAOD::Muon::MuonSpectrometerTrackParticle );
    if(!muonTrack) {
      muonTrack = theMuon->primaryTrackParticle();
      if(!muonTrack) {
       muonTrack = theMuon->trackParticle( xAOD::Muon::InnerDetectorTrackParticle );
      }
    }
  }
  if(!muonTrack){
    ATH_MSG_WARNING("no valid track found for extrapolating the muon to the pivot plane!");
  }
  return muonTrack;

}

StatusCode MuonExtrapolationTool::addBranches(const EventContext& ctx) const
{
    SG::ReadHandle<xAOD::MuonContainer> muonContainer{m_muonContainerName, ctx};
    if (!muonContainer.isValid()) {
      ATH_MSG_ERROR("Failed to retrieve MuonContainer with name: " << m_muonContainerName.key());
      return StatusCode::FAILURE;
    }
    for (auto muon : *muonContainer) {
       const xAOD::TrackParticle* track = getPreferredTrackParticle(muon);
       float eta, phi = 0;
       if( !extrapolateAndDecorateTrackParticle( track, eta, phi, ctx )){
           if( muon->pt() > 3500.){
             //only complain if the muon has sufficient pT to actually reach the pivot plane
             //extrapolation will often fail for muons with pT < 3500 MeV
             ATH_MSG_WARNING("Failed to extrapolate+decorate muon with pivot plane coords - Muon params: pt "<<muon->pt()<<", eta "<< muon->eta()<<", phi "<< muon->phi());
           }
        }
    }
    return StatusCode::SUCCESS;
}

  const Trk::TrackParameters* MuonExtrapolationTool::extrapolateToTriggerPivotPlane(const xAOD::TrackParticle& track, const EventContext& ctx) const
{
  // BARREL
  const Trk::Perigee& perigee = track.perigeeParameters();

  // create the barrel as a cylinder surface centered at 0,0,0
  Amg::Vector3D barrelCentre(0., 0., 0.);
  Amg::Transform3D matrix = Amg::Transform3D(Amg::RotationMatrix3D::Identity(), barrelCentre);

  Trk::CylinderSurface* cylinder =
    new Trk::CylinderSurface(matrix,
                             m_barrelPivotPlaneRadius,
                             m_barrelPivotPlaneHalfLength);
  if (!cylinder) {
    ATH_MSG_WARNING("extrapolateToTriggerPivotPlane :: new Trk::CylinderSurface failed.");
    return nullptr;
  }
  // and then attempt to extrapolate our track to this surface, checking for the boundaries of the barrel
  bool boundaryCheck = true;
  const Trk::Surface* surface = cylinder;
  const Trk::TrackParameters* p = m_extrapolator->extrapolate(
    ctx, perigee, *surface, Trk::alongMomentum, boundaryCheck, Trk::muon).release();
  delete cylinder;
  // if the extrapolation worked out (so we are in the barrel) we are done and can return the
  // track parameters at this surface.
  if (p) return p;

  // if we get here, the muon did not cross the barrel surface
  // so we assume it is going into the endcap.
  // ENDCAP

  // After 2 years of using this code, we realised that ATLAS actually has endcaps on both sides ;-)
  // So better make sure we place our endcap at the correct side of the detector!
  // Hopefully no-one will ever read this comment...
  float SignOfEta = track.eta() > 0 ? 1. : -1.;

  Amg::Vector3D endcapCentre(0., 0., m_endcapPivotPlaneZ);
  // much better!
  matrix = Amg::Transform3D(Amg::RotationMatrix3D::Identity(), SignOfEta * endcapCentre);

  std::unique_ptr<Trk::DiscSurface> disc =
    std::make_unique<Trk::DiscSurface>(matrix,
                                       m_endcapPivotPlaneMinimumRadius,
                                       m_endcapPivotPlaneMaximumRadius);
  if (!disc) {
    ATH_MSG_WARNING("extrapolateToTriggerPivotPlane :: new Trk::DiscSurface failed.");
    return nullptr;
  }

  // for the endcap, we turn off the boundary check, extending the EC infinitely to catch stuff heading for the transition region
  boundaryCheck = false;
  surface = disc.get();
  p = m_extrapolator->extrapolate(
    ctx, perigee, *surface, Trk::alongMomentum, boundaryCheck, Trk::muon).release();
  return p;
}
}
