/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGFPGATrackSimOBJECTS_FPGATrackSimCLUSTER_H
#define TRIGFPGATrackSimOBJECTS_FPGATrackSimCLUSTER_H

/*
 * FPGATrackSimCluster.h: This file declares the class used to represent clusters.
 * Declarations in this file:
 *      class FPGATrackSimCluster
 * Author: Alex Martyniuk
 * Email: martyniu@cern.ch
 */

#include "FPGATrackSimObjects/FPGATrackSimHit.h"

 /*
  * Clusters resulting from FPGATrackSim clustering algorithms as stored in m_clusterEquiv as a FPGATrackSimHit
  * The raw hits used to form this cluster are stored in the m_hitlist, the first entry is the seed that initiated this cluster.
  */
typedef std::vector<FPGATrackSimHit> hitVector;
class FPGATrackSimCluster
{
public:
  FPGATrackSimCluster() = default;
  virtual ~FPGATrackSimCluster() = default;

  // get private members
  hitVector const& getHitList() const { return m_hitlist; }
  FPGATrackSimHit const& getClusterEquiv() const { return m_clusterEquiv; }

  // set private members
  void setHitList(const hitVector& input) { m_hitlist = input; }
  void setClusterEquiv(const FPGATrackSimHit& input) { m_clusterEquiv = input; }

  // filling functions
  void push_backHitList(const FPGATrackSimHit& input) {
    m_hitlist.push_back(input);
    m_clusterEquiv.addIDHashToVec(input.getIdentifierHash());
    m_clusterEquiv.addPhiIndexToVec(input.getPhiIndex());
    m_clusterEquiv.addEtaIndexToVec(input.getEtaIndex());
  }

private:
  hitVector m_hitlist; // list of hits that make the cluster, the seed of the cluster will be the first entry in this list.
  FPGATrackSimHit m_clusterEquiv; // This is the cluster

  ClassDefNV(FPGATrackSimCluster, 4);
};

std::ostream& operator<<(std::ostream& o, const FPGATrackSimCluster& cluster);


#endif // FPGATrackSimCLUSTER_H
