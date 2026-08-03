/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/* Athena includes */
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ThreadLocalContext.h"

#include "CxxUtils/inline_hints.h"
/* Header include */
#include "FastCaloSimCaloTransportation.h"

/* ISF includes */
#include "ISF_FastCaloSimEvent/FastCaloSim_CaloCell_ID.h"
#include "ISF_FastCaloSimEvent/TFCSTruthState.h"
#include "MCTruth/AtlasG4EventUserInfo.h"

/* Tracking includes */
#include "TrkGeometry/TrackingGeometry.h"

/* Geometry primitives */
#include "GeoPrimitives/GeoPrimitivesHelpers.h"

/* Transport steps will be return as G4FieldTracks*/
#include "G4FieldTrack.hh"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"

#include "TruthUtils/HepMCHelpers.h"

FastCaloSimCaloTransportation::FastCaloSimCaloTransportation(const std::string& t, const std::string& n, const IInterface* p)
  : base_class(t,n,p)
{

}

StatusCode FastCaloSimCaloTransportation::initialize()
{
  ATH_MSG_INFO( "Initializing FastCaloSimCaloTransportation" );

  // Get TimedExtrapolator
  if(!m_extrapolator.empty()){
      ATH_CHECK(m_extrapolator.retrieve());
      ATH_MSG_INFO("Extrapolator retrieved "<< m_extrapolator);
  }

  return StatusCode::SUCCESS;

}


StatusCode FastCaloSimCaloTransportation::finalize(){
  ATH_MSG_INFO( "Finalizing FastCaloSimCaloTransportation" );
  return StatusCode::SUCCESS;
}


std::vector<G4FieldTrack> FastCaloSimCaloTransportation::transport(const TFCSTruthState* truth, bool forceNeutral) const{
  if (auto* eventInfo = AtlasG4EventUserInfo::GetEventUserInfo()) {
    Gaudi::Hive::setCurrentContext(eventInfo->GetEventContext());
  }

  // Start calo extrapolation
  ATH_MSG_DEBUG ("[ fastCaloSim transport ] processing particle "<<truth->pdgid() );

  auto hitVector = std::make_unique<std::vector<Trk::HitInfo>>();

  int     pdgId    = truth->pdgid();
  double  charge   = MC::charge(pdgId);
  if (forceNeutral) charge   = 0.;

  // particle Hypothesis for the extrapolation
  Trk::ParticleHypothesis pHypothesis = m_pdgToParticleHypothesis.convert(pdgId, charge);

  ATH_MSG_DEBUG ("particle hypothesis "<< pHypothesis);

  // geantinos not handled by PdgToParticleHypothesis - fix there
  if (pdgId == 999) pHypothesis = Trk::geantino;

  Amg::Vector3D pos = Amg::Vector3D(truth->vertex().X(), truth->vertex().Y(), truth->vertex().Z());

  Amg::Vector3D mom(truth->X(), truth->Y(), truth->Z());

  ATH_MSG_DEBUG( "[ fastCaloSim transport ] x from position eta="<<pos.eta()<<" phi="<<pos.phi()<<" d="<<pos.mag()<<" pT="<<mom.perp() );

  // input parameters : curvilinear parameters
  Trk::CurvilinearParameters inputPar(pos,mom,charge);

  double freepath = -1.;
  double tDec = freepath > 0. ? freepath : -1.;
  int decayProc = 0;
  
  Trk::TimeLimit timeLim(tDec, 0., decayProc);       
  Trk::PathLimit pathLim(-1., 0);
  Trk::GeometrySignature nextGeoID=Trk::Calo;

  // first extrapolation to reach the ID boundary
  ATH_MSG_DEBUG( "[ fastCaloSim transport ] before calo entrance ");

  // get CaloEntrance if not done already
  if(!m_caloEntrance.get()){
    m_caloEntrance.set(m_extrapolator->trackingGeometry()->trackingVolume(m_caloEntranceName));
    if(!m_caloEntrance.get())
      ATH_MSG_WARNING("CaloEntrance not found");
    else
      ATH_MSG_DEBUG("CaloEntrance found");
  }

  ATH_MSG_DEBUG( "[ fastCaloSim transport ] after calo entrance ");

  std::unique_ptr<const Trk::TrackParameters> caloEntry = nullptr;

  if(m_caloEntrance.get() && m_caloEntrance.get()->inside(pos, 0.001) && !m_extrapolator->trackingGeometry()->atVolumeBoundary(pos,m_caloEntrance.get(), 0.001)){
      std::vector<Trk::HitInfo>* dummyHitVector = nullptr;
      if (charge == 0){
        caloEntry =
          m_extrapolator->transportNeutralsWithPathLimit(inputPar,
                                                         pathLim,
                                                         timeLim,
                                                         Trk::alongMomentum,
                                                         pHypothesis,
                                                         dummyHitVector,
                                                         nextGeoID,
                                                         m_caloEntrance.get());
      }else{
        caloEntry = m_extrapolator->extrapolateWithPathLimit(inputPar,
                                                             pathLim,
                                                             timeLim,
                                                             Trk::alongMomentum,
                                                             pHypothesis,
                                                             dummyHitVector,
                                                             nextGeoID,
                                                             m_caloEntrance.get());
      }
  } else
    caloEntry = inputPar.uniqueClone();

  ATH_MSG_DEBUG( "[ fastCaloSim transport ] after calo caloEntry ");

  if(caloEntry){
    std::unique_ptr<const Trk::TrackParameters> eParameters = nullptr;

      // save Calo entry hit (fallback info)
      hitVector->push_back(Trk::HitInfo(caloEntry->uniqueClone(), timeLim.time, nextGeoID, 0.));

      ATH_MSG_DEBUG(
        "[ fastCaloSim transport ] starting Calo transport from position eta="
        << caloEntry->position().eta() << " phi=" << caloEntry->position().phi()
        << " d=" << caloEntry->position().mag());

      std::vector<Trk::HitInfo>* rawHitVector = hitVector.get();
      if (charge == 0){
        eParameters =
          m_extrapolator->transportNeutralsWithPathLimit(*caloEntry,
                                                         pathLim,
                                                         timeLim,
                                                         Trk::alongMomentum,
                                                         pHypothesis,
                                                         rawHitVector,
                                                         nextGeoID);
      }else{
        eParameters =
          m_extrapolator->extrapolateWithPathLimit(*caloEntry,
                                                   pathLim,
                                                   timeLim,
                                                   Trk::alongMomentum,
                                                   pHypothesis,
                                                   rawHitVector,
                                                   nextGeoID);
      }
      // save Calo exit hit (fallback info)
      if(eParameters) hitVector->push_back(Trk::HitInfo(std::move(eParameters), timeLim.time, nextGeoID, 0.));
    } //if caloEntry

  //used to identify ID-Calo boundary in tracking tools
  int IDCaloBoundary = 3000;

  if(msgLvl(MSG::DEBUG)){
    std::vector<Trk::HitInfo>::iterator it = hitVector->begin();
    while (it < hitVector->end()){
        int sample=(*it).detID;
        Amg::Vector3D hitPos = (*it).trackParms->position();
        ATH_MSG_DEBUG(" HIT: layer="<<sample<<" sample="<<sample-IDCaloBoundary<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" d="<<hitPos.mag());
        ++it;
    }
  }

  std::vector<Trk::HitInfo>::iterator it2 = hitVector->begin();
  while(it2 < hitVector->end()){
    int sample=(*it2).detID;
    Amg::Vector3D hitPos = (*it2).trackParms->position();
    ATH_MSG_DEBUG(" HIT: layer="<<sample<<" sample="<<sample-IDCaloBoundary<<" eta="<<hitPos.eta()<<" phi="<<hitPos.phi()<<" r="<<hitPos.perp()<<" z="<<hitPos[Amg::z]);
    ++it2;
  }

  // Extrapolation may fail for very low pT charged particles. Enforce charge 0 to prevent this 
  if (!forceNeutral && hitVector->empty()){
      ATH_MSG_DEBUG("forcing neutral charge in FastCaloSimCaloTransportation::caloHits");
      transport(truth, true);
  }
  // Don't expect this ever to happen. Nevertheless, error handling should be improved. 
  // This may require changes in periphery (adjustments after setting function type to StatusCode)
  else if(hitVector->empty()) ATH_MSG_ERROR("Empty hitVector even after forcing neutral charge. This may cause a segfault soon.");


  std::vector<G4FieldTrack> caloSteps = convertToFieldTrack(*hitVector);
  
  return caloSteps;

}


std::vector<G4FieldTrack> FastCaloSimCaloTransportation::convertToFieldTrack(const std::vector<Trk::HitInfo>& vec) const{

  std::vector<G4FieldTrack> caloSteps;
  for (auto& step : vec){
    G4FieldTrack track = G4FieldTrack(' ');
    track.SetPosition(Amg::EigenToHep3Vector(step.trackParms->position()));
    track.SetMomentum(Amg::EigenToHep3Vector(step.trackParms->momentum()));
    track.SetChargeAndMoments(step.trackParms->charge());
    caloSteps.push_back(track);
  }

  return caloSteps;

}
