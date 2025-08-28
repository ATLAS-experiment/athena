/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGL0GEPPERF_CLUSTER_H
#define TRIGL0GEPPERF_CLUSTER_H

#include <vector>
#include "TLorentzVector.h"

namespace Gep{

  struct Cluster {
    
    Cluster() {}
    Cluster(const TLorentzVector& tlv):vec{tlv} {}

    bool isEmptyCluster() const {return ncells == 0;}
    float et() const {return vec.Et();}
    
    void erase() {
      ncells = 0;
      time = 0.;
      vec.SetPxPyPzE(0.,0.,0.,0.);
      cell_id = {};
    }

    void setEtEtaPhi(double et, double eta, double phi) {
      vec.SetPtEtaPhiM(et, eta, phi, 0.0);
    }

    
    int ncells {0};
    float time {0};
    TLorentzVector vec;
    std::vector<unsigned int> cell_id;
    
  };
}

#endif //TRIGL0GEPPERF_CUSTOMTOPOCLUSTER_H
