// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODTRUTH_VERSIONS_TRUTHVERTEXAUXCONTAINER_V2_H
#define XAODTRUTH_VERSIONS_TRUTHVERTEXAUXCONTAINER_V2_H

// Local include(s).
#include "xAODTruth/TruthParticleContainer.h"

// EDM include(s).
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"

// System include(s).
#include <vector>

namespace xAOD {

/// Auxiliary store for the truth vertices
///
/// @author Andy Buckley <Andy.Buckey@cern.ch>
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class TruthVertexAuxContainer_v2 : public AuxContainerBase {

 public:
  /// Default constructor
  TruthVertexAuxContainer_v2();

 private:
  std::vector<int> status;
  std::vector< int > uid;
  std::vector<std::vector<ElementLink<TruthParticleContainer> > >
      incomingParticleLinks;
  std::vector<std::vector<ElementLink<TruthParticleContainer> > >
      outgoingParticleLinks;
  std::vector<float> x;
  std::vector<float> y;
  std::vector<float> z;
  std::vector<float> t;

};  // class TruthVertexAuxContainer_v2

}  // namespace xAOD

// StoreGate base type registration
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TruthVertexAuxContainer_v2, xAOD::AuxContainerBase);

#endif  // XAODTRUTH_VERSIONS_TRUTHVERTEXAUXCONTAINER_V2_H
