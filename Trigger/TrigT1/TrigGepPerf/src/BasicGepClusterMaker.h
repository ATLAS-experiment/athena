/*
    Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGGEPPERF_BASICGEPCLUSTERMAKER_H
#define TRIGGEPPERF_BASICGEPCLUSTERMAKER_H

#include "./IClusterMaker.h"


#include <string>

namespace Gep{
  class BasicGepClusterMaker : virtual public IClusterMaker {

public:

    BasicGepClusterMaker() = default;
    ~BasicGepClusterMaker() = default;

    std::vector<Gep::Cluster>
    makeClusters(const pGepCellMap&) const override;
    
    std::string getName() const override;

private:

    const float m_seed_threshold = 4.0;
    const float m_clustering_threshold = 2.0;
    const int m_max_shells = 9999;

    const std::vector<int> m_disallowed_seed_samplings = {};
    const std::vector<int> m_disallowed_clustering_samplings = {};

    bool isSeedCell (const Gep::GepCaloCell& cell, const std::vector<unsigned int> &seenSeedCells) const;
    bool isInAllowedSampling(int sampling, const std::vector<int>& list_of_samplings) const;
    bool isNewCell(unsigned int id, const std::vector<unsigned int>& seenCells) const;

    std::vector<Gep::GepCaloCell>
    clusterFromCells(const Gep::GepCaloCell& seed, const pGepCellMap&, std::vector<unsigned int> &seenSeedCells) const;
    
    Gep::Cluster getClusterFromListOfCells(const std::vector<Gep::GepCaloCell>& cells) const;
  };
}

#endif
