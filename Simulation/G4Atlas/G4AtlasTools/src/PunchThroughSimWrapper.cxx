/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "PunchThroughSimWrapper.h"

// CLHEP
#include "CLHEP/Random/RandFlat.h"

//Geant4
#include "G4Track.hh"
#include "G4TrackVector.hh"

PunchThroughSimWrapper::PunchThroughSimWrapper(const std::string& type, const std::string& name, const IInterface* parent)
  : base_class(type,name,parent)
{
}

StatusCode PunchThroughSimWrapper::initialize()
{
  return StatusCode::SUCCESS;
}

StatusCode PunchThroughSimWrapper::finalize()
{
  return StatusCode::SUCCESS;
}

void PunchThroughSimWrapper::DoPunchThroughSim(G4ParticleTable &ptable, ATHRNG::RNGWrapper* rngWrapper, const double simE, std::vector<double> simEfrac, const G4FastTrack& fastTrack, G4FastStep& fastStep)
{
  // Get Geant4 primary track
  const G4Track* G4PrimaryTrack = fastTrack.GetPrimaryTrack(); 

  // Get calo-ms variables 
  std::vector<double> caloMSVars = m_PunchThroughG4Tool->getCaloMSVars();

  // Declare array holders
  std::vector<std::map<std::string, double>> secKinematicsMapVect;

  // Create output particle (secondaries) collection
  auto secTrackCont = std::make_unique<G4TrackVector>();

  // Draw flat random number to compare punchthrough probability
  double punchThroughClassifierRand = CLHEP::RandFlat::shoot(*rngWrapper);

  // Calculate probability of punch through using punchThroughClassifier
  double punchThroughProbability = m_PunchThroughG4Classifier->computePunchThroughProbability(fastTrack, simE, simEfrac);

  // Safety condition
  if( punchThroughProbability > punchThroughClassifierRand){
    secKinematicsMapVect = m_PunchThroughG4Tool->computePunchThroughParticles(fastTrack, *rngWrapper, punchThroughProbability, punchThroughClassifierRand);

    // Create secondary tracks
    if(secKinematicsMapVect.size()!=0){
      // Now create all secondary tracks after all consistency checks (conservation of energy etc):
      m_PunchThroughG4Tool->createAllSecondaryTracks(ptable, fastStep, *G4PrimaryTrack, secKinematicsMapVect, *secTrackCont, caloMSVars);
    }
  }
  // done punchthrough procedure
}