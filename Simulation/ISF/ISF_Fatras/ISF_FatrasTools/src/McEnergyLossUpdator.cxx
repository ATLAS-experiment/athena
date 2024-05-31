/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "McEnergyLossUpdator.h"

// Trk
#include "TrkGeometry/MaterialProperties.h"
#include "TrkMaterialOnTrack/EnergyLoss.h"

#include "CLHEP/Random/RandLandau.h"
#include "CLHEP/Random/RandGaussZiggurat.h"
#include "TrkEventPrimitives/ParticleHypothesis.h"



iFatras::McEnergyLossUpdator::McEnergyLossUpdator( const std::string& type, const std::string& name, const IInterface* parent )
  :
  base_class( type, name, parent )
{
}


StatusCode iFatras::McEnergyLossUpdator::initialize()
{
  ATH_MSG_VERBOSE( "initialize()" );

  // Retrieve the energy loss updator tool
  ATH_CHECK ( m_energyLossUpdator.retrieve() );

  // get the random generator serice
  ATH_CHECK ( m_rndGenSvc.retrieve() );

  //Get own engine with own seeds:
  m_randomEngine = m_rndGenSvc->GetEngine(m_randomEngineName);
  if (!m_randomEngine) {
      ATH_MSG_FATAL( "Could not get random engine '" << m_randomEngineName << "'" );
      return StatusCode::FAILURE;
  }

  return StatusCode::SUCCESS;
}


double iFatras::McEnergyLossUpdator::dEdX( const Trk::MaterialProperties& materialProperties,
                                          double momentum,
                                          Trk::ParticleHypothesis particleHypothesis ) const
{
  // simply forward the dEdX from the standard energy loss updator
  return m_energyLossUpdator->dEdX( materialProperties, momentum, particleHypothesis );
}

Trk::EnergyLoss
iFatras::McEnergyLossUpdator::energyLoss(
  const Trk::MaterialProperties& materialProperties,
  double momentum,
  double pathCorrection,
  Trk::PropDirection direction,
  Trk::ParticleHypothesis particleHypothesis,
  bool) const
{


  // get the number of the material effects distribution
  Trk::EnergyLoss sampledEloss =
    m_energyLossUpdator->energyLoss(materialProperties,
                                    momentum,
                                    pathCorrection,
                                    direction,
                                    particleHypothesis,
                                    m_usePDGformula);


  // smear according to preselected distribution
  switch (m_energyLossDistribution) {
    // no straggling
  case 0 : { } break;
    // gaussian smearing
  case 1 : {
    float deIoni = sampledEloss.sigmaIoni() * CLHEP::RandGaussZiggurat::shoot(m_randomEngine);
    float deRad  = sampledEloss.sigmaRad() * CLHEP::RandGaussZiggurat::shoot(m_randomEngine);
    sampledEloss.update(deIoni,0.,deRad,0.,false);
  } break;
  case 2 : {
    float deIoni = -sampledEloss.sigmaIoni() * CLHEP::RandLandau::shoot(m_randomEngine);  // TODO :check sign
    sampledEloss.update(deIoni,0.,0.,0.,false);
  } break;
    // landau smearing
    default : {
      float deIoni = -sampledEloss.sigmaIoni() * CLHEP::RandLandau::shoot(m_randomEngine);  // TODO :check sign
      float deRad  = -sampledEloss.sigmaRad() * CLHEP::RandLandau::shoot(m_randomEngine);   // TODO :check sign
      sampledEloss.update(deIoni,0.,deRad,0.,false);
    } break;
  }

  // protection due to straggling - maximum energy loss is E-m
  double m     = Trk::ParticleMasses::mass[particleHypothesis];
  double E     = sqrt(momentum*momentum+m*m);

  if (sampledEloss.deltaE()+E<m ) {    // particle stopping - rest energy
      float dRad_rest = m-E-sampledEloss.deltaE();
      sampledEloss.update(0.,0.,dRad_rest,0.,false);
  }

  return sampledEloss;
}
