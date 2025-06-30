// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODTRUTH_VERSIONS_TRUTHPARTICLEAUXCONTAINER_V2_H
#define XAODTRUTH_VERSIONS_TRUTHPARTICLEAUXCONTAINER_V2_H

// Local include(s).
#include "xAODTruth/TruthVertexContainer.h"

// Athena include(s).
#include "AthLinks/ElementLink.h"
#include "xAODCore/AuxContainerBase.h"

// System include(s).
#include <vector>

namespace xAOD {

/// Auxiliary store for the truth particles
///
/// @author Andy Buckley <Andy.Buckey@cern.ch>
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
/// @author Jovan Mitrevski <Jovan.Mitrevski@cern.ch>
///
class TruthParticleAuxContainer_v2 : public AuxContainerBase {

 public:
  /// Default constructor
  TruthParticleAuxContainer_v2();

 private:
  std::vector<int> pdgId;
  std::vector< int > uid;
  std::vector<int> status;
  std::vector<ElementLink<TruthVertexContainer> > prodVtxLink;
  std::vector<ElementLink<TruthVertexContainer> > decayVtxLink;
  std::vector<float> px;
  std::vector<float> py;
  std::vector<float> pz;
  std::vector<float> e;
  std::vector<float> m;  // needed since not necessarily on shell

};  // class TruthParticleAuxContainer_v2

}  // namespace xAOD

// StoreGate base type registration
#include "xAODCore/BaseInfo.h"
SG_BASE(xAOD::TruthParticleAuxContainer_v2, xAOD::AuxContainerBase);

#endif  // XAODTRUTH_VERSIONS_TRUTHPARTICLEAUXCONTAINER_V2_H
