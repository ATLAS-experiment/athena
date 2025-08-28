// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
//
#ifndef XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_V1_H
#define XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_V1_H

// EDM include(s).
#include "xAODTruth/TruthVertexAuxContainer.h"
#include "xAODTruth/versions/TruthVertexAuxContainer_v1.h"

// Gaudi/Athena include(s).
#include "AthenaPoolCnvSvc/T_AthenaPoolTPConverter.h"

/// Converter class used for reading @c xAOD::TruthVertexAuxContainer_v1
///
/// This converter implements the conversion from
/// @c xAOD::TruthVertexAuxContainer_v1
/// to the latest version of the class. Implementing the HepMC2 -> HepMC3
/// schema change.
///
/// @author Attila Krasznahorkay <Attila.Krasznahorkay@cern.ch>
///
class xAODTruthVertexAuxContainerCnv_v1
    : public T_AthenaPoolTPCnvConstBase<xAOD::TruthVertexAuxContainer,
                                        xAOD::TruthVertexAuxContainer_v1> {

 public:
  /// Default constructor
  xAODTruthVertexAuxContainerCnv_v1() = default;

  // Use the base class's converter functions.
  using base_class::persToTrans;
  using base_class::transToPers;

  /// Function converting from the old type to the current one
  virtual void persToTrans(const xAOD::TruthVertexAuxContainer_v1* oldObj,
                           xAOD::TruthVertexAuxContainer* newObj,
                           MsgStream& log) const override;
  /// Dummy function inherited from the base class
  virtual void transToPers(const xAOD::TruthVertexAuxContainer*,
                           xAOD::TruthVertexAuxContainer_v1*,
                           MsgStream& log) const override;

};  // class xAODTruthVertexAuxContainerCnv_v1

#endif  // XAODTRUTHATHENAPOOL_XAODTRUTHVERTEXAUXCONTAINERCNV_V1_H
