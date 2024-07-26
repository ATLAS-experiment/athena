/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// @author Tomas Dado

#ifndef JET_RECLUSTERING_ALG_H
#define JET_RECLUSTERING_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <SystematicsHandles/SysReadHandle.h>
#include <SystematicsHandles/SysWriteHandle.h>
#include <SelectionHelpers/SysReadSelectionHandle.h>
#include <SystematicsHandles/SysWriteDecorHandle.h>

#include <xAODJet/JetAuxContainer.h>
#include <xAODJet/JetContainer.h>

#include "fastjet/ClusterSequence.hh"

#include <memory>
#include <vector>

namespace CP {

  class JetReclusteringAlg final : public EL::AnaAlgorithm {

  public:
    using EL::AnaAlgorithm::AnaAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute() override;

  private:
    CP::SysListHandle m_systematicsList {this};

    CP::SysReadHandle<xAOD::JetContainer> m_jetsHandle {
      this, "jets", "", "the jet container to use"
    };
    CP::SysReadSelectionHandle m_jetSelection {
      this, "jetSelection", "", "the selection on the input jets"
    };

    CP::SysWriteDecorHandle<std::vector<int>> m_smallRjetIndicesDecor {
      this, "smallRjetIndices", "smallRjetIndices_%SYS%", "indices of the small R jets used to build a given RC jet"
    };

    CP::SysWriteDecorHandle<float> m_rcEnergyDecor {
      this, "rcJetEnergy", "e_%SYS%", "energy of the RC jet"
    };

    // output container
    CP::SysWriteHandle<xAOD::JetContainer, xAOD::JetAuxContainer> m_outHandle {
      this, "reclusteredJets", "reclusteredJets_%SYS%", "reclustered jets collection"
    };

    // reclustering properties
    Gaudi::Property<std::string> m_clusteringAlgorithm {
      this, "clusteringAlgorithm", "AntiKt", "algorithm to use to recluster the jets"
    };
    Gaudi::Property<float> m_reclusteredJetsRadius {
      this, "reclusteredJetsRadius", 1.0, "radius parameter for the reclustered jets"
    };
    std::unique_ptr<fastjet::JetDefinition> m_fastjetClustering;

  private:
    std::vector<int> matchRCjets(const std::vector<const xAOD::Jet*>& smallJets, const std::vector<fastjet::PseudoJet>& constituents) const;
  };

} // namespace

#endif
