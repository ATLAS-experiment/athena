/*
  Copyright (C) 2002-2018 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASSOCIATIONUTILS_OVERLAPREMOVALDEFS_H
#define ASSOCIATIONUTILS_OVERLAPREMOVALDEFS_H

// Infrastructure
#include "AthContainers/AuxElement.h"
#include "AthLinks/ElementLink.h"

// EDM
#include "xAODBase/IParticleContainer.h"

namespace ort
{

  /// Input decoration type for overlap removal
  typedef char inputFlag_t;

  /// Output decoration type for overlap removal
  typedef char outputFlag_t;

  /// Input object accessor
  typedef SG::ConstAccessor<inputFlag_t> inputAccessor_t;

  /// Input object decorator (for convenience if users want it)
  typedef SG::Decorator<inputFlag_t> inputDecorator_t;

  /// Output object accessor (for convenience)
  typedef SG::ConstAccessor<outputFlag_t> outputAccessor_t;

  /// Output object decorator
  typedef SG::Decorator<outputFlag_t> outputDecorator_t;

  /// Overlap object link
  typedef ElementLink<xAOD::IParticleContainer> objLink_t;

  /// Overlap object link accessor (for convenience)
  typedef SG::ConstAccessor<objLink_t> objLinkAccessor_t;

  /// Overlap object link decorator
  typedef SG::Decorator<objLink_t> objLinkDecorator_t;

  /// Output / input decoration type for flag-running mode of ele-muon overlap removal
  typedef char sharedTrackFlag_t;

  /// Output object decorator
  typedef SG::Decorator<sharedTrackFlag_t> sharedTrackDecorator_t;

  /// Input object accessor
  typedef SG::ConstAccessor<sharedTrackFlag_t> sharedTrackAccessor_t;

} // namespace ort

#endif
