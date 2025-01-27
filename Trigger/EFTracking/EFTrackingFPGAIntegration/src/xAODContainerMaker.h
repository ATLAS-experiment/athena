/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/xAODContainerMaker.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Apr. 15, 2024
 * @brief Tool for making xAOD container
 */

#ifndef EFTRACKING_FPGA_INTEGRATION__XAOD_CONTAINER_MAKER_H
#define EFTRACKING_FPGA_INTEGRATION__XAOD_CONTAINER_MAKER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EFTrackingDataFormats.h"
#include "EFTrackingFPGAIntegration/IEFTrackingFPGAIntegrationTool.h"
#include "StoreGate/WriteHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

class xAODContainerMaker
    : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool>
{
public:
    using extends::extends;

    StatusCode initialize() override;

    /**
     * @brief Create xAOD::StripClusterContainer by creating xAOD::StripCluster
     * objects one by one
     */
    StatusCode makeStripClusterContainer(
        const EFTrackingDataFormats::StripClusterAuxInput &scAux,
        const EFTrackingDataFormats::Metadata *metadata,
        const EventContext &ctx) const;

    /**
     * @brief Create xAOD::PixelClusterContainer by creating xAOD::PixelCluster
     * objects one by one
     */
    StatusCode makePixelClusterContainer(
        const EFTrackingDataFormats::PixelClusterAuxInput &pxAux,
        const EFTrackingDataFormats::Metadata *metadata,
        const EventContext &ctx) const;

    /**
     * @brief Create xAOD::SpacePointContainer by creating xAOD::SpacePoint for
     * Pixel SpacePoint objects one by one
     */
    StatusCode makePixelSpacePointContainer(
        const EFTrackingDataFormats::SpacePointAuxInput &psAux,
        const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>&
            pixelsp_meas,
        const EFTrackingDataFormats::Metadata *metadata,
        const EventContext &ctx) const;

    /**
     * @brief Create xAOD::SpacePointContainer by creating xAOD::SpacePoint for
     * Strip SpacePoint objects one by one
     */
    StatusCode makeStripSpacePointContainer(
        const EFTrackingDataFormats::SpacePointAuxInput &sspAux,
        const std::vector<std::vector<const xAOD::UncalibratedMeasurement *>>&
            stripsp_meas,
        const EFTrackingDataFormats::Metadata *metadata,
        const EventContext &ctx) const;

private:
    SG::WriteHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
        this, "OutputPixelName", "fpgaPixelClusters",
        "Output container name"}; //!< Key for the pixel cluster container
    SG::WriteHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
        this, "OutputStripName", "fpgaStripClusters",
        "Output container name"}; //!< Key for the strip cluster container
    SG::WriteHandleKey<xAOD::SpacePointContainer> m_pixelSpacePointsKey{
        this, "OutputPixelSpacePointName", "fpgaPixelSpacePoints",
        "Output container name"}; //!< Key for the Pixel SpacePoint container
    SG::WriteHandleKey<xAOD::SpacePointContainer> m_stripSpacePointsKey{
        this, "OutputStripSpacePointName", "fpgaStripSpacePoints",
        "Output container name"}; //!< Key for the Strip SpacePoint container
};

#endif // FTRACKING_FPGA_INTEGRATION__XAOD_CONTAINER_MAKER_H
