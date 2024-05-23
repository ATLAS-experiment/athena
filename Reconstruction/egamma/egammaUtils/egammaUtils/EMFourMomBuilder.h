/*
   Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EGAMMAUTILS_EMFOURMOMBUILDER_H
#define EGAMMAUTILS_EMFOURMOMBUILDER_H

/**
  @class EMFourMomBuilder
  sets the fourmomentum : energy is taken from the cluster and angles either
  from tracking or cluster.
  -
  @author Anastopoulos
  */

#include "egammaUtils/eg_resolution.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

class EMFourMomBuilder final
{
public:
  EMFourMomBuilder();
  void calculate(xAOD::ElectronContainer* electrons) const;
  void calculate(xAOD::PhotonContainer* photons) const;
  void calculate(xAOD::Electron& electron) const;
  void calculate(xAOD::Photon& photon) const;
 private:
  std::unique_ptr<eg_resolution> m_eg_resol;
};

#endif
