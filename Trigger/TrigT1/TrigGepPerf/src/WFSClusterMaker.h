/*
    Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGL0GEPPERF_WFSCLUSTERMAKER_H
#define TRIGL0GEPPERF_WFSCLUSTERMAKER_H

#include "./IClusterMaker.h"

#include <map>
#include <string>

namespace Gep{
  class WFSClusterMaker : virtual public IClusterMaker {

public:

    WFSClusterMaker() {}
    ~WFSClusterMaker() {}

    std::vector<Gep::Cluster>
    makeClusters(const pGepCellMap&) const override;
    
    std::string getName() const override;

private:

    float m_seed_threshold = 4.0;
    float m_clustering_threshold = 2.0;
    int m_max_shells = 8;
    std::vector<int> m_allowed_seed_samplings = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    std::vector<int> m_allowed_clustering_samplings = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};

    bool isSeedCell (const Gep::GepCaloCell& cell) const;
    bool isInAllowedSampling(int sampling, const std::vector<int>& list_of_samplings) const ;
    bool isNewCell(unsigned int id, const std::vector<unsigned int>& seenCells) const;

    std::vector<Gep::GepCaloCell>
    clusterFromCells(const Gep::GepCaloCell& seed, const pGepCellMap&) const;
    
    Gep::Cluster getClusterFromListOfCells(const std::vector<Gep::GepCaloCell>& cells) const;

    double calculateClusterPhi(double seed_phi, double delta_phi) const;
    void orderClustersInEt(std::vector<Gep::Cluster> &v_clusters) const;
    double getDeltaPhi(double phi, double seed_phi) const;
  };
}

#endif
