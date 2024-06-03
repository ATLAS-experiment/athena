/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// class header
#include "ProcessSamplingTool.h"

// Tracking
#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/HelperStructs.h"

#include "TruthUtils/MagicNumbers.h"
// CLHEP
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Random/RandFlat.h"

//

#include <iostream>

/*=========================================================================
 *  DESCRIPTION OF FUNCTION:
 *  ==> see headerfile
 *=======================================================================*/
iFatras::ProcessSamplingTool::ProcessSamplingTool( const std::string& t,
                                      const std::string& n,
                                      const IInterface*  p )
  : base_class(t,n,p)
{
}


Trk::PathLimit iFatras::ProcessSamplingTool::sampleProcess(CLHEP::HepRandomEngine *randomEngine, double momentum,double charge,Trk::ParticleHypothesis particle) const
{
  int process=0;
  double x0Max = -1.;

  if ( particle == Trk::electron && charge<0. ) return Trk::PathLimit(x0Max,process);

  if ( particle == Trk::muon || particle == Trk::nonInteractingMuon ) return Trk::PathLimit(x0Max,process);

  double rndx = CLHEP::RandFlat::shoot(randomEngine);

  if ( particle == Trk::photon ) {

    // conversion (14), compton effect (13), photo-effect (12)

    double  p0  =       -7.01612e-03;
    double  p1  =        7.69040e-02;
    double  p2  =       -6.07682e-01;
    // calculate xi
    double xi = p0 + p1*pow(momentum/1000.,p2);
    double attenuation = -7.777e-01*(1.-xi);

    x0Max = log(rndx)/attenuation ;

    // comment additional interactions till properly implemented
    /*
    // first estimate : compton ~ 4% , use attenuation factor as for conversion (till parameterized )
    double rndp = CLHEP::RandFlat::shoot(randomEngine);

    if ( rndp > 0.96 ) { // do Compton scattering
      process = 13;
    } else if (rndp < 5.53/pow(momentum,1.184) ) {
      process = 12;
      attenuation = -0.378;
      x0Max = log(rndx)/attenuation *momentum ;
    } else {
      process = 14;
      //x0Max *= 0.5;
    }
    */

    process = 14;

    return Trk::PathLimit(x0Max,process);
  }

  if ( particle == Trk::electron && charge>0. ) {   // positron

    double mass = Trk::ParticleMasses::mass[Trk::electron];
    double gamma = momentum/mass;

    // annihilation
    // energy dependent factor
    double fpa = log(2*gamma)/gamma;
    // material dependent factor - use Al(Z=13) for the moment
    double fza = 13.;
    double attenuation = -fpa*fza; // adjust

    x0Max = log(rndx)/attenuation ;
    process = 5;

    return Trk::PathLimit(x0Max,process);

  }

  // presumably here we are left with hadrons only
  if (m_hadInt) {

    // sample free path in terms of nuclear interaction length
    double al = 1.;          // scaling here

    /*

      if ( particle == Trk::pion || particle == Trk::kaon || particle == Trk::pi0 || particle == Trk::k0) {
      al *= 1./(1.+ exp(-0.5*(momentum-270.)*(momentum-270.)/60./60.));
      }
      if ( particle == Trk::proton || particle == Trk::neutron ) al *=0.7;
      if ( particle == Trk::pion || particle == Trk::pi0) al *=0.9;
    */

    x0Max = -log(rndx)*al ;

    process = 121;

    //std::cout <<"hadronic path limit:"<<momentum<<","<<al<<","<< x0Max << std::endl;
    return Trk::PathLimit(x0Max,process);
  }


  return Trk::PathLimit(x0Max,process);

}
