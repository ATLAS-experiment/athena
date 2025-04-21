/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Feb. 18, 2024
 * @brief Class for the data preparation pipeline
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
#define EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H

// EFTracking include
#include "EFTrackingFPGAPipeline/IntegrationBase.h"
#include "EFTrackingFPGAUtility/EFTrackingTransient.h"
#include "EFTrackingFPGAUtility/xAODClusterMaker.h"
#include "EFTrackingFPGAUtility/xAODSpacePointMaker.h"
#include "EFTrackingFPGAUtility/PassThroughTool.h"
#include "EFTrackingFPGAUtility/TestVectorTool.h"

// Athena include
#include "StoreGate/ReadHandleKey.h"
#include "xAODInDetMeasurement/PixelClusterContainer.h"
#include "xAODInDetMeasurement/SpacePointContainer.h"
#include "xAODInDetMeasurement/StripClusterContainer.h"

/**
 * @brief This is the class for the data preparation pipeline.
 *
 * The output of this pipeline are the xAOD::StripCluster(Container) and
 * xAOD::PixelCluster(Container). These containers are to be used in other
 * CPU-based algorithms.
 */
class DataPreparationPipeline : public IntegrationBase
{
public:
    using IntegrationBase::IntegrationBase;
    StatusCode initialize() override final;
    StatusCode execute(const EventContext &ctx) const override final;

    StatusCode setupBuffers();
    StatusCode setupKernelArgs();

private:
    // OpenCL kernel objects
    cl::Kernel m_pixelClusteringKernel; //!< Kernel for pixel clustering
    cl::Kernel m_spacepointKernel;      //!< Kernel for spacepoint

    cl::Buffer m_pxlClusteringInBuff;  //!< Buffer for pixel clustering input
    cl::Buffer m_pxlClusteringOutBuff; //!< Buffer for pixel clustering output

    cl::Buffer m_spacepointInBuff;  //!< Buffer for spacepoint input
    cl::Buffer m_spacepointOutBuff; //!< Buffer for spacepoint output

    // Kernel related properties
    Gaudi::Property<std::string> m_xclbin{
        this, "xclbin", "",
        "xclbin path and name"}; //!< Path and name of the xclbin file
    Gaudi::Property<std::string> m_passThroughKernelName{this,
                                                         "PassThroughKernelName", "",
                                                         "Pass through kernel name"}; //!< Pass through kernel name
    Gaudi::Property<std::string> m_pixelClusteringKernelName{this,
                                                             "PixelClusteringKernelName", "",
                                                             "Pixel Clustering kernel name"}; //!< Pixle clustering kernel name
    Gaudi::Property<std::string> m_spacepointKernelName{this,
                                                        "SpacepointKernelName", "",
                                                        "Spacepoint kernel name"}; //!< Spacepoint kernel name

    // Test vector related properties
    Gaudi::Property<std::string> m_pixelClusterTVPath{this, "PixelClusterTV", "",
                                                      "Path to pixel cluster test vector"}; //!< Pixel cluster test vector
    Gaudi::Property<std::string> m_pixelClusterRefTVPath{this, "PixelClusterRefTV", "",
                                                         "Path to pixel cluster reference test vector"}; //!< Pixel cluster reference test vector

    Gaudi::Property<std::string> m_spacepointTVPath{this, "SpacepointTV", "",
                                                    "Path to spacepoint test vector"}; //!< Spacepoint test vector
    Gaudi::Property<std::string> m_spacepointRefTVPath{this, "SpacepointRefTV", "",
                                                       "Path to spacepoint reference test vector"}; //!< Spacepoint reference test vector

    // Flags
    Gaudi::Property<bool> m_usePassThrough{
        this, "RunPassThrough", false,
        "Use the passthrough tool instead of data prep pipeline"};          //!< Use the pass through tool instead of data prep pipeline. It can be either sw or hw, setting in the tool
    Gaudi::Property<bool> m_useTV{this, "UseTV", false, "Use test vector"}; //!< Use test vector

        // Tool handles
    ToolHandle<xAODClusterMaker> m_xAODClusterMaker{
        this, "xAODClusterMaker", "xAODClusterMaker",
        "tool to make cluster"}; //!< Tool handle for xAODClusterMaker

    ToolHandle<xAODSpacePointMaker> m_xAODSpacePointMaker{
        this, "xAODSpacePointMaker", "xAODSpacePointMaker",
        "tool to make space point"}; //!< Tool handle for xAODSpacePointMaker

    ToolHandle<PassThroughTool> m_passThroughTool{
        this, "PassThroughTool", "PassThroughTool",
        "The pass through tool"}; //!< Tool handle for PassThroughTool

    ToolHandle<TestVectorTool> m_testVectorTool{
        this, "TestVectorTool", "TestVectorTool",
        "Tool to prepare test vector"}; //!< Tool handle for TestVectorTool
};

#endif // EFTRACKING_FPGA_INTEGRATION_DATAPREPARATIONPIPELINE_H
