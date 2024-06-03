/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/PhysicalConstants.h"
#include "GeneratorObjects/McEventCollection.h"

#include "ForwardTransportSvc.h"
#include "ForwardTransportSvc/IForwardTransportSvc.h"
#include "ForwardTracker/TwissFile.h"

#include <fstream>
#include "TTree.h"

ForwardTransportSvc::ForwardTransportSvc(const std::string& name, ISvcLocator* svc) :
  base_class(name,svc)
{
  declareProperty("TwissFile1",    m_cData.twissFile1);
  declareProperty("TwissFile2",    m_cData.twissFile2);
  declareProperty("PositionC1",    m_cData.positionC1);
  declareProperty("PositionC2",    m_cData.positionC2);
  declareProperty("ApertureC1",    m_cData.apertureC1);
  declareProperty("ApertureC2",    m_cData.apertureC2);
  declareProperty("EndMarker",     m_cData.endMarker);
}

StatusCode ForwardTransportSvc::initialize() {

  ATH_MSG_INFO("ForwardTransportSvc::initialize");
  return StatusCode::SUCCESS;
}

bool ForwardTransportSvc::selectedParticle(G4ThreeVector momentum, int pid) {

  std::ifstream pfile(m_cData.twissFile1.c_str());
  double beamEnergy = ForwardTracker::GetBeamEnergy(pfile)*Gaudi::Units::GeV;

  double eta = fabs(momentum.pseudoRapidity());
  double xi  = momentum.mag()/beamEnergy;

  ATH_MSG_INFO(" pseudoRapidity: " << std::setprecision(9) << eta);
  ATH_MSG_INFO(" p/beamEnergy:   " << std::setprecision(9) << xi);

  if (eta < m_etaCut) return false;
  if (xi  < m_xiCut)  return false;

  if (m_transportFlag) {

    if (pid == 2212) return true; // proton
  }
  else {

    if (pid == 2112) return true; // neutron
    if (pid == 22)   return true; // gamma
    if (pid == 3122) return true; // lambda
  }

  return false;
}
