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
#include "FPGATrackSimLorentzAngle/FPGATrackSimLorentzAngleTool.h"
#include <vector>
#include <memory>

namespace FPGATrackSimCLUSTERING {
  void attachTruth(std::vector<FPGATrackSimHit> &);
  bool updatePixelCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster, bool digitalClustering);
  bool updateStripCluster(FPGATrackSimCluster &currentCluster, FPGATrackSimHit &incomingHit, bool newCluster, bool digitalClustering);
  bool updateClusterContents(FPGATrackSimCluster &currentCluster, int &clusterRow, int &clusterRowWidth, int &clusterCol, int &clusterColWidth, FPGATrackSimHit &incomingHit, bool digitalClustering);
  bool sortITkInputEta(const std::unique_ptr<FPGATrackSimHit>& hitA, const std::unique_ptr<FPGATrackSimHit>& hitB);
  bool sortITkInputPhi(const std::unique_ptr<FPGATrackSimHit>& hitA, const std::unique_ptr<FPGATrackSimHit>& HitB);
}

class FPGATrackSimClusteringTool : public extends <AthAlgTool,FPGATrackSimClusteringToolI> {
public:

  FPGATrackSimClusteringTool(const std::string&, const std::string&, const IInterface*);

  virtual ~FPGATrackSimClusteringTool() = default;
  virtual StatusCode initialize() override;

  virtual StatusCode DoClustering(FPGATrackSimLogicalEventInputHeader &, std::vector<FPGATrackSimCluster> &) const override;

 private:

  Gaudi::Property<bool> m_digitalClustering {this, "DigitalClustering", true, "flag to enable digital clustering instead of ToT weighted position calculation" };
  Gaudi::Property<bool> m_reduceCoordPrecision {this, "ReduceCoordPrecision", false, "flag to enable reducing the precision of global coordinates" };
  Gaudi::Property<float> m_coordRPrecision {this, "CoordRPrecision", 1./64., "fixed point precision of r coordinate" };
  Gaudi::Property<float> m_coordPhiPrecision {this, "CoordPhiPrecision", 1./8192., "fixed point precision of phi coordinate" };
  Gaudi::Property<float> m_coordZPrecision {this, "CoordZPrecision", 1./32., "fixed point precision of z coordinate" };
  Gaudi::Property<int>  m_LorentzAngleShift { this, "LorentzAngleShift", -1, "flag for Lorentz angle shift. -1 means off, 0 means full corrections from first version, 1 means smaller LUT, 2 means very small LUT" };

  ToolHandle<FPGATrackSim::LorentzAngleTool> m_lorentzAngleTool {this, "LorentzAngleTool", "", "FPGATrackSim tool to retrieve Lorentz angle"};

  using HitPtrCollection = std::vector<std::unique_ptr<FPGATrackSimHit>>;
  using HitPtrContainer = std::vector<HitPtrCollection>;

  //FPGATrackSim pixel clustering using the FPGATrackSim objects
  void SortedClustering(HitPtrContainer&& sorted_hits, std::vector<FPGATrackSimCluster> &) const;
  void Clustering(HitPtrCollection&&, std::vector<FPGATrackSimCluster> &) const;

  // Other helper functions
  void reduceGlobalCoordPrecision(FPGATrackSimCluster &cluster) const;
  void reduceGlobalCoordPrecision(FPGATrackSimHit &hit) const;
  void splitAndSortHits(HitPtrCollection&& hits, HitPtrContainer& hitsPerModule, int& eta_phi) const;
  void splitAndSortHits(HitPtrCollection&& hits, HitPtrContainer& hitsPerModule) const;
  void splitHitsToModules(HitPtrCollection&& hits, HitPtrContainer& hitsPerModule) const;
  void normaliseClusters(std::vector<FPGATrackSimCluster> &clusters) const;
  void sortHitsOnModules(HitPtrContainer& hitsPerModule, int& eta_phi) const;
  void sortHitsOnModules(HitPtrContainer& hitsPerModule) const;
  bool etaOrPhi(const FPGATrackSimHit& hit) const;
  bool sortIBLInput(const std::unique_ptr<FPGATrackSimHit>& i, const std::unique_ptr<FPGATrackSimHit>& j) const;
  bool sortPixelInput(const std::unique_ptr<FPGATrackSimHit>& i, const  std::unique_ptr<FPGATrackSimHit>& j) const;
  void SetMinMaxIndicies(FPGATrackSimCluster &cluster) const;

};

#endif // FPGATrackSimCLUSTERINGTOOL_H
