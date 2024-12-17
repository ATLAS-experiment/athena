/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH__PARTICLELEVEL_MISSINGET__ALG_H
#define TRUTH__PARTICLELEVEL_MISSINGET__ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgDataHandles/ReadHandle.h>
#include <AsgDataHandles/ReadHandleKey.h>

// Framework includes
#include <xAODMissingET/MissingETContainer.h>

namespace CP {
class ParticleLevelMissingETAlg : public EL::AnaAlgorithm {
 public:
  using EL::AnaAlgorithm::AnaAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute() final;

 private:
  SG::ReadHandleKey<xAOD::MissingETContainer> m_metKey{
      this, "met", "", "the name of the input MET container"};
};

}  // namespace CP

#endif
