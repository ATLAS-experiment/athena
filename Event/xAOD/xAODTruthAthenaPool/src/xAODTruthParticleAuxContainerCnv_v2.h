// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_V2_H
#define XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_V2_H

// EDM include(s).
#include "xAODTruth/TruthParticleAuxContainer.h"
#include "xAODTruth/versions/TruthParticleAuxContainer_v2.h"

// Gaudi/Athena include(s).
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

/// Converter class used for reading @c xAOD::TruthParticleAuxContainer_v2
///
/// This converter implements the conversion from
/// @c xAOD::TruthParticleAuxContainer_v2
/// to the latest version of the class. Implementing the HepMC2 -> HepMC3
/// schema change.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class xAODTruthParticleAuxContainerCnv_v2
    : public T_AthenaPoolTPCnvBase<xAOD::TruthParticleAuxContainer,
                                   xAOD::TruthParticleAuxContainer_v2> {

 public:
  /// Default constructor
  xAODTruthParticleAuxContainerCnv_v2() = default;

  /// Function converting from the old type to the current one
  virtual void persToTrans(const xAOD::TruthParticleAuxContainer_v2* oldObj,
                           xAOD::TruthParticleAuxContainer* newObj,
                           MsgStream& log) override;
  /// Dummy function inherited from the base class
  virtual void transToPers(const xAOD::TruthParticleAuxContainer*,
                           xAOD::TruthParticleAuxContainer_v2*,
                           MsgStream& log) override;

};  // class xAODTruthParticleAuxContainerCnv_v2

#endif  // XAODTRUTHATHENAPOOL_XAODTRUTHPARTICLEAUXCONTAINERCNV_V2_H
