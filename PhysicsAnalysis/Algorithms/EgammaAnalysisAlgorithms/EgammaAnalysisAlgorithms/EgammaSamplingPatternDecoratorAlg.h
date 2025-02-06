/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_SAMPLING_PATTERN_DECORATOR_ALG_H
#define EGAMMA_ANALYSIS_ALGORITHMS__EGAMMA_SAMPLING_PATTERN_DECORATOR_ALG_H

#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/WriteDecorHandleKey.h>
#include <AsgTools/PropertyWrapper.h>
#include <xAODCaloEvent/CaloClusterContainer.h>

namespace CP
{
  /// \brief an algorithm decorating `samplingPattern` on e-gamma clusters
  ///
  /// For historic reason the sampling pattern is stored in the cluster
  /// object instead of as a decoration.  To make this available for
  /// columnar analysis it also needs to exist as a decoration.  For now
  /// that means we have two copies of it, but it was already agreed to
  /// drop it from the cluster object, so this is a temporary situation.

  class EgammaSamplingPatternDecoratorAlg final : public EL::AnaReentrantAlgorithm
  {
    /// \brief the standard constructor
  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize () override;
    virtual StatusCode execute (const EventContext& ctx) const override;


  private:

    /// @brief the e-gamma cluster container
    SG::ReadHandleKey<xAOD::CaloClusterContainer> m_clusterContainer {this, "clusterContainer", "egammaClusters", "the name of the e-gamma cluster container"};

    /// @brief the sampling pattern decoration
    SG::WriteDecorHandleKey<xAOD::CaloClusterContainer> m_samplingPattern {this, "samplingPattern", "egammaClusters.samplingPattern", "the samplingPattern decoration"};
  };
}

#endif
