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
#include "EFTrackingTransient.h"

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
    StatusCode runPassThrough(
        EFTrackingTransient::StripClusterAuxInput &scAux,
        EFTrackingTransient::PixelClusterAuxInput &pxAux,
        EFTrackingTransient::SpacePointAuxInput &stripSpAux,
        EFTrackingTransient::SpacePointAuxInput &pixelSpAux,
        EFTrackingTransient::Metadata *metadata,
        const EventContext &ctx) const;

    /**
     * @brief Convert the strip cluster from xAOD container to simple std::vector
     * of EFTrackingTransient::StripCluster.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputClusterData(
        const xAOD::StripClusterContainer *sc,
        std::vector<EFTrackingTransient::StripCluster> &ef_sc,
        unsigned long N) const;

    /**
     * @brief Convert the pixel cluster from xAOD container to simple std::vector
     * of EFTrackingTransient::PixelCluster.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputClusterData(
        const xAOD::PixelClusterContainer *pc,
        std::vector<EFTrackingTransient::PixelCluster> &ef_pc,
        unsigned long N) const;

    /**
     * @brief Convert the space point from xAOD container to simple std::vector
     * of EFTrackingTransient::SpacePoint.
     *
     * This is needed for the kernel input.
     */
    StatusCode getInputSpacePointData(
        const xAOD::SpacePointContainer *sp,
        std::vector<EFTrackingTransient::SpacePoint> &ef_sp,
        std::vector<std::vector<const xAOD::UncalibratedMeasurement *>> &sp_meas,
        unsigned long N, bool isStrip) const;

    /**
     * @brief Software version of the pass-through kernel. The purse of this function
     * is to mimic the FPGA output at software level.
     *
     * It takes the EFTrackingTransient::StripCluster
     * EFTrackingTransient::PixelCluster, and EFTrackingDataFormat::SpacePoints
     * as input arguments and mimic the transfer kernel by giving array output.
     */
    StatusCode passThroughSW(
        const std::vector<EFTrackingTransient::StripCluster> &inputSC,
        EFTrackingTransient::StripClusterOutput &ef_scOutput,
        // PixelCluster
        const std::vector<EFTrackingTransient::PixelCluster> &inputPC,
        EFTrackingTransient::PixelClusterOutput &ef_pcOutput,
        // Strip SpacePoint
        const std::vector<EFTrackingTransient::SpacePoint> &inputSSP,
        EFTrackingTransient::SpacePointOutput &ef_sspOutput,
        // Pixel SpacePoint
        const std::vector<EFTrackingTransient::SpacePoint> &inputPSP,
        EFTrackingTransient::SpacePointOutput &ef_pspOutput,
        // Metadata
        EFTrackingTransient::Metadata *metadata)
        const;

    /**
     * @brief This is a cluster-only version (sw) of the pass-through kernel
     * This is used for cluter level studies
     */
    StatusCode passThroughSW_clusterOnly(
        const std::vector<EFTrackingTransient::StripCluster> &inputSC,
        EFTrackingTransient::StripClusterOutput &ef_scOutput,
        // PixelCluster
        const std::vector<EFTrackingTransient::PixelCluster> &inputPC,
        EFTrackingTransient::PixelClusterOutput &ef_pcOutput,
        // Metadata
        EFTrackingTransient::Metadata *metadata)
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
    Gaudi::Property<bool> m_clusterOnlyPassThrough{this, "ClusterOnlyPassThrough", false,
                                                   "Use the cluster-only pass-through kernel"}; //!< Use the cluster-only pass through tool       
    Gaudi::Property<unsigned int> m_maxClusterNum{this, "MaxClusterNum", 500000,
                                                   "Maximum number of clusters that can be processed"};
    Gaudi::Property<unsigned int> m_maxSpacePointNum{this, "MaxSpacePointNum", 500000,
                                                   "Maximum number of space points that can be processed"};
                                                   
    
    SG::ReadHandleKey<xAOD::StripClusterContainer> m_stripClustersKey{
        this, "StripClusterContainerKey", "ITkStripClusters",
        "Key for Strip Cluster Containers"};
    SG::ReadHandleKey<xAOD::PixelClusterContainer> m_pixelClustersKey{
        this, "PixelClusterContainerKey", "ITkPixelClusters",
        "Key for Pixel Cluster Containers"};
    SG::ReadHandleKey<xAOD::SpacePointContainer> m_spacePointsKey{
        this, "SpacePointContainerKey", "ITkPixelSpacePoints",
        "Key for Space Point Containers"};
    
};

#endif // EFTRACKING_FPGA_INTEGRATION__PASSTHROUGH_TOOL_H
