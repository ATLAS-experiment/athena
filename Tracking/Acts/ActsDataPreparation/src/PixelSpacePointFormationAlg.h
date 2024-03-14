/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
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

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/SpacePointAuxContainer.h"

#include <string>

namespace ActsTrk {

    /// @class PixelSpacePointFormationAlg
    /// This version of PixelSpacePointFormationAlg uses xAOD pixel clusters
    /// to find space points in the ITk pixeldetectors.
    /// Pixel space points are obtained directly from the clusters,
    /// with needed evaluation of the space point covariance terms
    /// Space points are then recorded to storegate as ActsTrk::SpacePoint
    /// into an ActsTrk::SpacePointContainer.

    class PixelSpacePointFormationAlg : public AthReentrantAlgorithm {
    public:
        /// @name AthReentrantAlgorithm methods
        //@{
        PixelSpacePointFormationAlg(const std::string& name,
				    ISvcLocator* pSvcLocator);
        virtual ~PixelSpacePointFormationAlg() = default;
        virtual StatusCode initialize() override;
        virtual StatusCode execute (const EventContext& ctx) const override;
        //@}

    private:
        /// @name Disallow constructor without parameters, copy constructor, assignment operator
        //@{
        PixelSpacePointFormationAlg() = delete;
        PixelSpacePointFormationAlg(const PixelSpacePointFormationAlg&) = delete;
        PixelSpacePointFormationAlg &operator=(const PixelSpacePointFormationAlg&) = delete;
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

  };

}

#endif // ACTSTRKSPACEPOINTFORMATION_PIXELSPACEPOINTFORMATIONALG_H
