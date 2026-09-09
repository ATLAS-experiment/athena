/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ACTSTRK_DATAPREPARATION_PIXELSPACEPOINTFORMATIONALG_H
#define ACTSTRK_DATAPREPARATION_PIXELSPACEPOINTFORMATIONALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "AthenaMonitoringKernel/GenericMonitoringTool.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "ActsToolInterfaces/IPixelSpacePointFormationTool.h"
#include "ActsEvent/ContextUtility.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <string>

#include "src/Cache.h"

namespace ActsTrk {

    /// @class PixelSpacePointFormationAlg
    /// This version of PixelSpacePointFormationAlg uses xAOD pixel clusters
    /// to find space points in the ITk pixeldetectors.
    /// Pixel space points are obtained directly from the clusters,
    /// with needed evaluation of the space point covariance terms
    /// Space points are then recorded to storegate as ActsTrk::SpacePoint
    /// into an ActsTrk::SpacePointContainer.
    template <bool useCache>
    class PixelSpacePointFormationAlgBase : public AthReentrantAlgorithm {
    public:
        /// @name AthReentrantAlgorithm methods
        //@{
        PixelSpacePointFormationAlgBase(const std::string& name,
				    ISvcLocator* pSvcLocator);
        virtual ~PixelSpacePointFormationAlgBase() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute (const EventContext& ctx) const override;
        virtual StatusCode finalize() override;
      //@}

    private:
        /// @name Disallow constructor without parameters, copy constructor, assignment operator
        //@{
        PixelSpacePointFormationAlgBase() = delete;
        PixelSpacePointFormationAlgBase(const PixelSpacePointFormationAlgBase&) = delete;
        PixelSpacePointFormationAlgBase &operator=(const PixelSpacePointFormationAlgBase&) = delete;
        //@}

        /// @name Input data using SG::ReadHandleKey
        //@{
        SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClusterContainerKey{this, "PixelClusters", "", "name of the input pixel cluster container"};
        //@}

        /// @name Input condition data using SG::ReadCondHandleKey
        //@{
        /// To get detector elements
        SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetectorElements", "ITkPixelDetectorElementCollection", "Key of input SiDetectorElementCollection for Pixel"};
        //@}

        ///@name Output data using SG::WriteHandleKey
        //@{
	      SG::WriteHandleKey<xAOD::SpacePointContainer> m_pixelSpacePointContainerKey {this, "PixelSpacePoints", "", "name of the output pixel space point container"};
        //@}

        /// @name ToolHandle
        //@{
        /// For space point formation
        ToolHandle<ActsTrk::IPixelSpacePointFormationTool> m_spacePointMakerTool{this, "SpacePointFormationTool", "", "Tool dedicated to the creation of pixel space points"};
        /// For monitoring
        ToolHandle<GenericMonitoringTool> m_monTool{this, "MonTool", "", "Monitoring tool"};
        //@}

        /// Only used, and only initialized, if the space point formation tool
        /// reports that it needs a geometry context
        ActsTrk::ContextUtility m_ctxProvider{this};
        bool m_useGeometryContext{false};

        using Cache_IDC = typename Cache::Handles<xAOD::SpacePoint>::IDC;
        using Cache_BackendUpdateHandleKey = typename Cache::Handles<xAOD::SpacePoint>::BackendUpdateHandleKey;
        using Cache_BackendUpdateHandle = typename Cache::Handles<xAOD::SpacePoint>::BackendUpdateHandle;
        using Cache_WriteHandleKey = typename Cache::Handles<xAOD::SpacePoint>::WriteHandleKey;
        using Cache_WriteHandle = typename Cache::Handles<xAOD::SpacePoint>::WriteHandle;

        Cache_WriteHandleKey m_SPCache{this,"SPCache",""};
        Cache_BackendUpdateHandleKey m_SPCacheBackend{this,"SPCacheBackend",""};

    private:
      enum EStat {
	kNClusters,
	kNSpacePoints,
	kNStat
      };
      
      mutable std::array<std::atomic<unsigned int>, kNStat> m_stat ATLAS_THREAD_SAFE {}; 
    };

}

#include "PixelSpacePointFormationAlgBase.icc"

#endif // ACTSTRKSPACEPOINTFORMATION_PIXELSPACEPOINTFORMATIONALG_H
