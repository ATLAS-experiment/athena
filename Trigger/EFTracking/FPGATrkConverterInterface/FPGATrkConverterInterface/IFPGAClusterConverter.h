/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFPGAHITCONVERTER_H
#define IFPGAHITCONVERTER_H

// Include Files
#include <string>
#include "GaudiKernel/IInterface.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "FPGATrackSimObjects/FPGATrackSimHitCollection.h"
#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

namespace InDet {
  class PixelCluster;
  class SCT_Cluster;
}

class IdentifierHash;

class IFPGAClusterConverter : public virtual IAlgTool {

public:

  virtual StatusCode convertHits(const FPGATrackSimHitCollection&,
                                  InDet::PixelClusterCollection &,
                                  InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertHits(const std::vector<const FPGATrackSimHit*>& ,
                                  InDet::PixelClusterCollection &,
                                  InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertHits(const FPGATrackSimHitCollection& hits,
                                  xAOD::PixelClusterContainer& pixelCont,
                                  xAOD::StripClusterContainer& SCTCont) const = 0;

  virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>&,
                                      InDet::PixelClusterCollection &,
                                      InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>& cl,
                                      xAOD::PixelClusterContainer& pixelCont,
                                      xAOD::StripClusterContainer& SCTCont) const = 0;

  virtual StatusCode convertSpacePoints(const std::vector<FPGATrackSimCluster>& fpgaSPs,
                                    xAOD::SpacePointContainer& SPStripCont,
                                    xAOD::SpacePointContainer& SPPixelCont, 
                                    xAOD::StripClusterContainer& stripClusterCont,
                                    xAOD::PixelClusterContainer& pixelClusterCont) const = 0;

  virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::PixelCluster>&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::PixelCluster&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::SCT_Cluster>&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::StripCluster&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::PixelCluster>&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, xAOD::PixelCluster& ) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::SCT_Cluster>&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, xAOD::StripCluster& ) const = 0;
  virtual StatusCode createSP(const FPGATrackSimCluster& cl, xAOD::SpacePoint& sp, xAOD::StripClusterContainer& clustersCont ) const = 0;
  virtual StatusCode createPixelSPs(xAOD::SpacePointContainer& pixelSPs, xAOD::PixelClusterContainer& clustersCont ) const = 0;

  virtual StatusCode getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimCluster& cluster) const = 0;
  virtual StatusCode getStripsInfo(const xAOD::StripCluster& cl, float& halfStripLength, Amg::Vector3D& stripDirection, Amg::Vector3D& stripCenter) const = 0;


};

#endif


