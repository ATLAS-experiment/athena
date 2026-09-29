/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Baptiste Ravina <baptiste.ravina@cern.ch>

#ifndef TRUTH_PARTICLELEVEL_MISSINGET_ALG_H
#define TRUTH_PARTICLELEVEL_MISSINGET_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>

// Framework includes
#include <xAODMissingET/MissingETContainer.h>

namespace CP {
class ParticleLevelMissingETAlg : public EL::AnaReentrantAlgorithm {
 public:
  using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
  virtual StatusCode initialize() final;
  virtual StatusCode execute(const EventContext &ctx) const final;

 private:
  SG::ReadHandleKey<xAOD::MissingETContainer> m_metKey{
      this, "met", "", "the name of the input MET container"};
  SG::WriteDecorHandleKey<xAOD::MissingETContainer> m_decMetKey{
      this, "metDecoration", m_metKey, "met", "the met decoration"};
  SG::WriteDecorHandleKey<xAOD::MissingETContainer> m_decPhiKey{
      this, "phiDecoration", m_metKey, "phi", "the phi decoration"};
};

}  // namespace CP

#endif
