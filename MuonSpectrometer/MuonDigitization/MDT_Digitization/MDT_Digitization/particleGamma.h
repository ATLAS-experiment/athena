/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDT_DIGITIZATION_PARTICLEGAMMA_H
#define MDT_DIGITIZATION_PARTICLEGAMMA_H
/*-----------------------------------------------

  Created 07-10-2011 by Oleg.Bulekov@cern.ch
  Function particleGamma returns the value of gamma factor for Qball particle.
  -----------------------------------------------*/

#include <iostream>
#include <sstream>
#include <vector>

#include "CLHEP/Units/PhysicalConstants.h"
#include "GeneratorObjects/HepMcParticleLink.h"
#include "MdtCalibData/MdtFullCalibData.h"
#include "MdtCalibData/MdtTubeCalibContainer.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonSimData/MuonSimData.h"
#include "MuonSimData/MuonSimDataCollection.h"
#include "MuonSimEvent/MDTSimHitCollection.h"
#include "PathResolver/PathResolver.h"
#include "PileUpTools/PileUpMergeSvc.h"
#include "StoreGate/StoreGateSvc.h"
#include "TrkDetDescrUtils/GeometryStatics.h"

// SB
#include "AtlasHepMC/GenParticle.h"
//

double particleGamma(const EventContext& ctx, const MDTSimHit& hit, unsigned short eventId = 0) {
  double QGamma = -9999.;
  const HepMcParticleLink trkParticle = HepMcParticleLink::getRedirectedLink(hit.particleLink(), eventId, ctx); // This link should now correctly resolve to the TruthEvent McEventCollection in the main StoreGateSvc.
  HepMC::ConstGenParticlePtr genParticle = trkParticle.cptr();
  if (genParticle) {
    const int particleEncoding = genParticle->pdg_id();
    if ((((int)(std::abs(particleEncoding) / 10000000) == 1) && ((int)(std::abs(particleEncoding) / 100000) == 100)) ||
        (((int)(std::abs(particleEncoding) / 10000000) == 2) && ((int)(std::abs(particleEncoding) / 100000) == 200))) { // TODO Use functions from TruthUtils/AtlasPID.h instead?
      const double QPx = genParticle->momentum().px();
      const double QPy = genParticle->momentum().py();
      const double QPz = genParticle->momentum().pz();
      const double QE = genParticle->momentum().e();
      const double QM2 = pow(QE, 2) - pow(QPx, 2) - pow(QPy, 2) - pow(QPz, 2);
      if (QM2 >= 0.) {
        const double QM = std::sqrt(QM2);
        if (QM > 0.) {
          QGamma = QE / QM;
        }
      }
    }
  }
  return QGamma;
}

#endif
