/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FPGATrackSimCLUSTERINGTOOL_H
#define FPGATrackSimCLUSTERINGTOOL_H

/*
 * httClustering
 * ---------------
 *
 * Routines to perform clustering in the pixels, based on FPGATrackSim
 *
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "FPGATrackSimMaps/FPGATrackSimClusteringToolI.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include <vector>
#include <memory>

namespace FPGATrackSimCLUSTERING {
  void attachTruth(std::vector<FPGATrackSimHit> &);
  bool updatePixelCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster);
  bool updateStripCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster);
  void updateClusterContents(FPGATrackSimCluster &currentCluster, int &clusterRow, int &clusterRowWidth, int &clusterCol, int &clusterColWidth, FPGATrackSimHit &incomingHit);
  bool sortITkInputEta(const FPGATrackSimHit& hitA, const FPGATrackSimHit& hitB);
  bool sortITkInputPhi(const FPGATrackSimHit& hitA, const FPGATrackSimHit& HitB);
}

class FPGATrackSimClusteringTool : public extends <AthAlgTool,FPGATrackSimClusteringToolI> {
public:

  FPGATrackSimClusteringTool(const std::string&, const std::string&, const IInterface*);

  virtual ~FPGATrackSimClusteringTool() = default;

  virtual StatusCode DoClustering(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) const override;

 private:

  Gaudi::Property<bool> m_reduceCoordPrecision {this, "ReduceCoordPrecision", false, "flag to enable reducing the precision of global coordinates" };
  Gaudi::Property<float> m_coordRPrecision {this, "CoordRPrecision", 1./64., "fixed point precision of r coordinate" };
  Gaudi::Property<float> m_coordPhiPrecision {this, "CoordPhiPrecision", 1./8192., "fixed point precision of phi coordinate" };
  Gaudi::Property<float> m_coordZPrecision {this, "CoordZPrecision", 1./32., "fixed point precision of z coordinate" };

  //FPGATrackSim pixel clustering using the FPGATrackSim objects
  void SortedClustering(const std::vector<std::vector<FPGATrackSimHit> >& sorted_hits, std::vector<FPGATrackSimCluster> &) const;
  void Clustering(std::vector<FPGATrackSimHit>, std::vector<FPGATrackSimCluster> &) const ;

  // Other helper functions
  void reduceGlobalCoordPrecision(FPGATrackSimCluster &cluster) const;
  void reduceGlobalCoordPrecision(FPGATrackSimHit &hit) const;
  void splitAndSortHits(std::vector<FPGATrackSimHit>& hits, std::vector<std::vector<FPGATrackSimHit> >& hitsPerModule, int& eta_phi) const;
  void splitAndSortHits(std::vector<FPGATrackSimHit>& hits, std::vector<std::vector<FPGATrackSimHit> >& hitsPerModule) const;
  void splitHitsToModules(std::vector<FPGATrackSimHit>& hits, std::vector<std::vector<FPGATrackSimHit> >& hitsPerModule) const;
  void normaliseClusters(std::vector<FPGATrackSimCluster> &clusters) const;
  void sortHitsOnModules(std::vector<std::vector<FPGATrackSimHit> >& hitsPerModule, int& eta_phi) const;
  void sortHitsOnModules(std::vector<std::vector<FPGATrackSimHit> >& hitsPerModule) const;
  bool etaOrPhi(const FPGATrackSimHit& hit) const;
  bool sortIBLInput(const std::unique_ptr<FPGATrackSimHit>& i, const std::unique_ptr<FPGATrackSimHit>& j) const;
  bool sortPixelInput(const std::unique_ptr<FPGATrackSimHit>& i, const  std::unique_ptr<FPGATrackSimHit>& j) const;



};

#endif // FPGATrackSimCLUSTERINGTOOL_H
