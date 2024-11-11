/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_OBJ_DATA_PREPARATION_ALG_H
#define ACTSTRK_OBJ_DATA_PREPARATION_ALG_H

#include "details/DataPreparationAlg.h"

// EDM
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/PixelClusterAuxContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"
#include "xAODInDetMeasurement/StripClusterAuxContainer.h"
#include "xAODInDetMeasurement/HGTDClusterContainer.h"
#include "xAODInDetMeasurement/HGTDClusterAuxContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

// Det Ele Collections
#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "HGTD_ReadoutGeometry/HGTD_DetectorElementCollection.h"

// Define specializations
namespace ActsTrk {

  // Pixel Clusters
  class PixelClusterDataPreparationAlg
    : public DataPreparationAlg< xAOD::PixelClusterContainer,
				 InDetDD::SiDetectorElementCollection,
				 false > {
  public:
    using DataPreparationAlg<xAOD::PixelClusterContainer,
			     InDetDD::SiDetectorElementCollection,
			     false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::PixelCluster& obj) const override;
  };
  
  class PixelClusterCacheDataPreparationAlg
    : public DataPreparationAlg< xAOD::PixelClusterContainer,
				 InDetDD::SiDetectorElementCollection,
				 true > {
  public:
    using DataPreparationAlg<xAOD::PixelClusterContainer,
			     InDetDD::SiDetectorElementCollection,
			     true>::DataPreparationAlg;
  };
  
  // Strip Clusters
  class StripClusterDataPreparationAlg
    : public DataPreparationAlg< xAOD::StripClusterContainer,
				 InDetDD::SiDetectorElementCollection,
				 false > {
  public:
    using DataPreparationAlg<xAOD::StripClusterContainer,
			     InDetDD::SiDetectorElementCollection,
			     false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::StripCluster& obj) const override;
  };

  class StripClusterCacheDataPreparationAlg
    : public DataPreparationAlg< xAOD::StripClusterContainer,
				 InDetDD::SiDetectorElementCollection,
				 true > {
  public:
    using DataPreparationAlg< xAOD::StripClusterContainer,
			      InDetDD::SiDetectorElementCollection,
			      true >::DataPreparationAlg;
  };

  // HGTD Clusters
  class HgtdClusterDataPreparationAlg
    : public DataPreparationAlg< xAOD::HGTDClusterContainer,
				 InDetDD::HGTD_DetectorElementCollection,
				 false > {
  public:
    using DataPreparationAlg<xAOD::HGTDClusterContainer,
			     InDetDD::HGTD_DetectorElementCollection,
			     false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::HGTDCluster& obj) const override;

    // HGTD does not support regional selection and has no regional selection tool
    // Se we need to override this function
    virtual StatusCode fetchIdHashes(const EventContext& ctx,
                                     std::set<IdentifierHash>& hashes) const override;
  };
  
  // SpacePoints
  class SpacePointDataPreparationAlg
    : public DataPreparationAlg< xAOD::SpacePointContainer,
				 InDetDD::SiDetectorElementCollection,
				 false> {
  public:
    using  DataPreparationAlg<xAOD::SpacePointContainer,
			      InDetDD::SiDetectorElementCollection,
			      false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::SpacePoint& obj) const;
  };

  class SpacePointCacheDataPreparationAlg
    : public DataPreparationAlg< xAOD::SpacePointContainer,
				 InDetDD::SiDetectorElementCollection,
				 true > {
  public:
    using DataPreparationAlg<xAOD::SpacePointContainer,
			     InDetDD::SiDetectorElementCollection,
			     true>::DataPreparationAlg;
  };

} // namespace

#include "CollectionDataPreparationAlg.icc"

#endif
