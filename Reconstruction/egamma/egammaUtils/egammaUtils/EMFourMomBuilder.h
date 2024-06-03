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

#include "AthContainers/DataVector.h"
#include "egammaUtils/eg_resolution.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/Photon.h"

namespace EMFourMomBuilder
{
  void calculate(xAOD::Electron& electron);
  void calculate(xAOD::Photon& photon);

  template<typename T>
  void calculate(DataVector<T>* egammas)
  {
    if (egammas) {
      for (T* egamma : *egammas) {
        calculate(*egamma);
      }
    }
  }
};

#endif
