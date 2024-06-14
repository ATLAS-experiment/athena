/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IFPGAHITCONVERTER_H
#define IFPGAHITCONVERTER__H

// Include Files
#include <string>
#include "GaudiKernel/IInterface.h"
#include "AthenaKernel/IOVSvcDefs.h"
#include "FPGATrackSimObjects/FPGATrackSimHit.h"
#include "FPGATrackSimObjects/FPGATrackSimCluster.h"
#include "FPGATrackSimObjects/FPGATrackSimClusterCollection.h"
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/SCT_ClusterCollection.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"

namespace InDet {
  class PixelCluster;
  class SCT_Cluster;
}

class IdentifierHash;

class IFPGAClusterConverter : public virtual IAlgTool {

public:

  virtual StatusCode convertHits(const std::vector<FPGATrackSimHit>& ,
                                  InDet::PixelClusterCollection &,
                                  InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertHits(const std::vector<const FPGATrackSimHit*>& ,
                                  InDet::PixelClusterCollection &,
                                  InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertHits(const std::vector<FPGATrackSimHit>& hits,
                                  xAOD::PixelClusterContainer& pixelCont,
                                  xAOD::StripClusterContainer& SCTCont) const = 0;

  virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>&,
                                      InDet::PixelClusterCollection &,
                                      InDet::SCT_ClusterCollection &) const = 0;
  virtual StatusCode convertClusters(const std::vector<FPGATrackSimCluster>& cl,
                                      xAOD::PixelClusterContainer& pixelCont,
                                      xAOD::StripClusterContainer& SCTCont) const = 0;

  virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::PixelCluster>&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::PixelCluster&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, std::unique_ptr<InDet::SCT_Cluster>&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimHit& h, const std::vector<Identifier>& rdoList, xAOD::StripCluster&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::PixelCluster>&) const = 0;
  virtual StatusCode createPixelCluster(const FPGATrackSimCluster&, xAOD::PixelCluster& ) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, std::unique_ptr<InDet::SCT_Cluster>&) const = 0;
  virtual StatusCode createSCTCluster(const FPGATrackSimCluster&, xAOD::StripCluster& ) const = 0;

  virtual StatusCode getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimCluster& cluster) const = 0;
  virtual StatusCode getRdoList(std::vector<Identifier> &rdoList, const FPGATrackSimHit& hit) const = 0;

};

#endif


