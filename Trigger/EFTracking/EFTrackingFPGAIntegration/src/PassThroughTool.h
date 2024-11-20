/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/PassThroughTool.h
 * @author zhaoyuan.cui@cern.ch
 * @date Oct. 31, 2024
 * @brief Tool for the passing through functionality
 */

#ifndef EFTRACKING_FPGA_INTEGRATION__PASSTHROUGH_TOOL_H
#define EFTRACKING_FPGA_INTEGRATION__PASSTHROUGH_TOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "EFTrackingFPGAIntegration/IEFTrackingFPGAIntegrationTool.h"
#include "EFTrackingDataFormats.h"

#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

class PassThroughTool : public extends<AthAlgTool, IEFTrackingFPGAIntegrationTool>
{
public:
    using extends::extends;

    StatusCode initialize() override;

    /**
     * @brief Call this function at the DataPreparationPipeline to run the pass-through kernels
     * 
     */
    StatusCode runPassThrough(EFTrackingDataFormats::StripClusterAuxInput &scAux,
                              EFTrackingDataFormats::PixelClusterAuxInput &pxAux,
                              EFTrackingDataFormats::Metadata *metadata,
                              const EventContext &ctx) const;

    /**
     * @brief Convert the strip cluster from xAOD container to simple std::vector
     * of EFTrackingDataFormats::StripCluster.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputClusterData(
        const xAOD::StripClusterContainer *sc,
        std::vector<EFTrackingDataFormats::StripCluster> &ef_sc,
        unsigned long N) const;

    /**
     * @brief Convert the pixel cluster from xAOD container to simple std::vector
     * of EFTrackingDataFormats::PixelCluster.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputClusterData(
        const xAOD::PixelClusterContainer *pc,
        std::vector<EFTrackingDataFormats::PixelCluster> &ef_pc,
        unsigned long N) const;

    /**
     * @brief Convert the space point from xAOD container to simple std::vector
     * of EFTrackingDataFormats::SpacePoint.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputSpacePointData(
        const xAOD::SpacePointContainer *sp,
        std::vector<EFTrackingDataFormats::SpacePoint> &ef_sp,
        std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> &sp_meas,
        unsigned long N, bool isStrip) const;

    /**
     * @brief Software version of the pass-through kernel. The purse of this function
     * is to mimic the FPGA output at software level.
     *
     * It takes the EFTrackingDataFormats::StripCluster
     * EFTrackingDataFormats::PixelCluster, and EFTrackingDataFormat::SpacePoints
     * as input arguments and mimic the transfer kernel by giving array output.
     */
    StatusCode passThroughSW(
        const std::vector<EFTrackingDataFormats::StripCluster> &inputSC,
        EFTrackingDataFormats::StripClusterOutput &ef_scOutput,
        // PixelCluster
        const std::vector<EFTrackingDataFormats::PixelCluster> &inputPC,
        EFTrackingDataFormats::PixelClusterOutput &ef_pcOutput,
        // Strip SpacePoint
        const std::vector<EFTrackingDataFormats::SpacePoint> &inputSSP,
        EFTrackingDataFormats::SpacePointOutput &ef_sspOutput,
        // Pixel SpacePoint
        const std::vector<EFTrackingDataFormats::SpacePoint> &inputPSP,
        EFTrackingDataFormats::SpacePointOutput &ef_pspOutput,
        // Metadata
        EFTrackingDataFormats::Metadata *metadata)
        const;

    /**
     * @brief This is a cluster-only version (sw) of the pass-through kernel
     * This is used for cluter level studies
     */
    StatusCode passThroughSW_clusterOnly(
        const std::vector<EFTrackingDataFormats::StripCluster> &inputSC,
        EFTrackingDataFormats::StripClusterOutput &ef_scOutput,
        // PixelCluster
        const std::vector<EFTrackingDataFormats::PixelCluster> &inputPC,
        EFTrackingDataFormats::PixelClusterOutput &ef_pcOutput,
        // Metadata
        EFTrackingDataFormats::Metadata *metadata)
        const;

    /**
     * @brief A getter for the m_runSW property. Determine if the user set the tool to run
     * at software level. If yes, the sw version of the pass-through kernel will be executed.
     * If not, the FPGA (hw) version will be executed
     */
    bool runSW() const { return m_runSW; }

private:
    Gaudi::Property<bool> m_runSW{this, "RunSW", true, "Run software mode"};               //!< Software mode, not running on the FPGA
    Gaudi::Property<bool> m_doSpacepoints{this, "DoSpacepoints", false, "Do spacepoints"}; //!< Temporary flag before spacepoints are ready

    SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
        this, "StripClusterContainerKey", "ITkStripClusters",
        "Key for Strip Cluster Containers"};
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
        this, "PixelClusterContainerKey", "ITkPixelClusters",
        "Key for Pixel Cluster Containers"};
};

#endif // EFTRACKING_FPGA_INTEGRATION__PASSTHROUGH_TOOL_H
