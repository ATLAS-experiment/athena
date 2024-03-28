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
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

// Define specializations
namespace ActsTrk {

  // Pixel Clusters
  class PixelClusterDataPreparationAlg
    : public DataPreparationAlg< xAOD::PixelClusterContainer, false > {
  public:
    using DataPreparationAlg<xAOD::PixelClusterContainer, false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::PixelCluster& obj) const override;
  };
  
  class PixelClusterCacheDataPreparationAlg
    : public DataPreparationAlg< xAOD::PixelClusterContainer, true > {
  public:
    using DataPreparationAlg<xAOD::PixelClusterContainer, true>::DataPreparationAlg;
  };
  
  // Strip Clusters
  class StripClusterDataPreparationAlg
    : public DataPreparationAlg< xAOD::StripClusterContainer, false > {
  public:
    using DataPreparationAlg<xAOD::StripClusterContainer, false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::StripCluster& obj) const override;
  };

  class StripClusterCacheDataPreparationAlg
    : public DataPreparationAlg< xAOD::StripClusterContainer, true > {
  public:
    using DataPreparationAlg< xAOD::StripClusterContainer, true >::DataPreparationAlg;
  };
  
  // SpacePoints
  class SpacePointDataPreparationAlg
    : public DataPreparationAlg< xAOD::SpacePointContainer, false> {
  public:
    using  DataPreparationAlg<xAOD::SpacePointContainer, false>::DataPreparationAlg;

  private:
    virtual xAOD::DetectorIDHashType retrieveDetectorIDHash(const xAOD::SpacePoint& obj) const;
  };

} // namespace

#include "CollectionDataPreparationAlg.icc"

#endif
